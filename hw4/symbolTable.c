#include "symbolTable.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
// This file is for reference only, you are not required to follow the implementation. //

#define ERROR(x) { fprintf(stderr, (x)); exit(-1);}
#define MIN(x, y) ((x) < (y) ? (x) : (y))

int HASH(char * str) {
    int idx=0;
    while (*str){
		    idx = idx << 1;
        idx+=*str;
        str++;
    }
    return (idx & (HASH_TABLE_SIZE-1));
}

TableStack symbolTable;
NameSpace nameSpace;

SymbolTableEntry* newSymbolTableEntry(int nestingLevel)
{
    SymbolTableEntry* symbolTableEntry = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));
    if(!symbolTableEntry) {
        ERROR("Failed to allocate symbol table entry");
    }
    symbolTableEntry->nextInHashChain = NULL;
    symbolTableEntry->prevInHashChain = NULL;
/*
    symbolTableEntry->nextInSameLevel = NULL;
    symbolTableEntry->sameNameInOuterLevel = NULL;
*/
    symbolTableEntry->attribute = NULL;
    symbolTableEntry->nameIndex = 0;
    symbolTableEntry->nameLength = 0;
    symbolTableEntry->nestingLevel = nestingLevel;
    return symbolTableEntry;
}

void removeFromHashChain(int hashIndex, SymbolTableEntry* entry)
{
}

void enterIntoHashChain(int hashIndex, SymbolTableEntry* entry)
{
    SymbolTableEntry **head = &(currentScope()->hashTable[hashIndex]);
    if(!*head) {
        *head = entry;
        return;
    }

    while((*head)->nextInHashChain != NULL)
        head = &((*head)->nextInHashChain);

    (*head)->nextInHashChain = entry;
    entry->prevInHashChain = *head;
}

SymbolAttribute* newAttribute(SymbolAttributeKind attributeKind)
{
    SymbolAttribute *temp = (SymbolAttribute*)malloc(sizeof(struct SymbolAttribute));
    temp->attributeKind = attributeKind;
    return temp;
}

TypeDescriptor* newTypeDesc(TypeDescriptorKind kind)
{
    TypeDescriptor *temp = (TypeDescriptor*)malloc(sizeof(struct TypeDescriptor));
    temp->kind = kind;
    return temp;
}

FunctionSignature* newFuncSign(int n_params, Parameter* parameterList, DATA_TYPE returnType){
    FunctionSignature *temp = (FunctionSignature*)malloc(sizeof(struct FunctionSignature));
    temp->parametersCount = n_params;
    temp->parameterList = parameterList;
    temp->returnType = returnType;
    return temp
}

void initializeSymbolTable()
{
    initializeNameSpace();
    symbolTable.currentLevel = 0;
    symbolTable.top = 0;
    memset(currentScope()->hashTable, 0, HASH_TABLE_SIZE * sizeof(void *));

    // pre-insert default data types
    enterSymbol(SYMBOL_TABLE_INT_NAME, newAttribute(TYPE_ATTRIBUTE));
    enterSymbol(SYMBOL_TABLE_FLOAT_NAME, newAttribute(TYPE_ATTRIBUTE));
    enterSymbol(SYMBOL_TABLE_VOID_NAME, newAttribute(TYPE_ATTRIBUTE));
    // pre-insert default function name
    SymbolAttribute* read_attr = newAttribute(FUNCTION_SIGNATURE);
    
    enterSymbol(SYMBOL_TABLE_SYS_LIB_READ,);
    enterSymbol(SYMBOL_TABLE_SYS_LIB_FREAD,) ;

}

void symbolTableEnd()
{
}

SymbolTableEntry* retrieveSymbol(char* symbolName)
{
    int hashIndex = HASH(symbolName);
    int len = strlen(symbolName);
    int i = 0;
    for(i = symbolTable.currentLevel; i >= 0; i--)
    {
        SymbolTable *table = &symbolTable.stack[i];
        SymbolTableEntry **head = &(table->hashTable[hashIndex]);

        while(*head != NULL)
        {
            SymbolTableEntry *e = *head;
            if(len == e->nameLength && !strncmp(symbolName, getName(e->nameIndex), e->nameLength))
                return e;

            head = &((*head)->nextInHashChain);
        }
    }
    /* TODO : Error message : undeclared symbol */
    return NULL
}

SymbolTableEntry* enterSymbol(char* symbolName, SymbolAttribute* attribute)
{
    SymbolTableEntry* entry = newSymbolTableEntry(symbolTable.currentLevel);
    entry->nameLength = strlen(symbolName);
    entry->nameIndex = enterSymbolNS(symbolName);
    entry->attribute = attribute;
    enterIntoHashChain(HASH(symbolName), entry);
    return entry;
}

//remove the symbol from the current scope
void removeSymbol(char* symbolName)
{

}

int declaredLocally(char* symbolName)
{
    int hashIndex = HASH(symbolName);
    int len = strlen(symbolName);
    SymbolTableEntry **head = &(currentScope()->hashTable[hashIndex]);

    while(*head != NULL)
    {
        SymbolTableEntry *e = *head;
        if(len == e->nameLength && !strncmp(symbolName, getName(e->nameIndex), e->nameLength))
            return 1;

        head = &((*head)->nextInHashChain);
    }
    return 0;
}

SymbolTable* currentScope()
{
    return &symbolTable.stack[symbolTable.currentLevel];
}

void openScope()
{
    symbolTable.currentLevel += 1;
    symbolTable.top += 1;
    memset(currentScope()->hashTable, 0, HASH_TABLE_SIZE * sizeof(void *));
}

void closeScope()
{
    symbolTable.currentLevel -= 1;
    symbolTable.top -= 1;
}

void initializeNameSpace()
{
    nameSpace.size = 0;
    newSegment();
}

void newSegment()
{
    if(nameSpace.size >= NAMESPACE_SIZE)
        ERROR("Maximum namespace size reached.");

    nameSpace.size += 1;
    nameSpace.segments[nameSpace.size - 1] = (char *) calloc(NAMESPACE_SEGMENT_SIZE + 1, 1);
    nameSpace.currentOffset = 0;
}

int enterSymbolNS(char *symbolName)
{
    int len = strlen(symbolName);
    if(len > NAMESPACE_SEGMENT_SIZE)
        ERROR("Symbol name exceeds segment size of namespace");

    int i;
    for(i = 0; i < nameSpace.size; ++i)
    {
        char* p = strstr(nameSpace.segments[i], symbolName);
        if(p)
            return makeIndex(p - nameSpace.segments[i], i);
    }

    if(nameSpace.currentOffset + len > NAMESPACE_SEGMENT_SIZE)
        newSegment();

    strncpy(currentEmpty(), symbolName, len);
    nameSpace.currentOffset += len;
    return makeIndex(nameSpace.currentOffset - len, nameSpace.size - 1);
}

__inline__ char *currentEmpty()
{
    return nameSpace.segments[nameSpace.size - 1] + nameSpace.currentOffset;
}

__inline__ int makeIndex(int offset, int segment)
{
    return offset + NAMESPACE_SEGMENT_SIZE * segment;
}

__inline__ char *getName(int index)
{
    return nameSpace.segments[index / NAMESPACE_SEGMENT_SIZE] + index % NAMESPACE_SEGMENT_SIZE;
}
