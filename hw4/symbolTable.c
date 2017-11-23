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

void initializeSymbolTable()
{
    symbolTable.currentLevel = 0;
    symbolTable.top = 0;
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
        SymbolTable *table = symbolTable.stack[i];
        SymbolTableEntry **head = &(table->hashTable[hashIndex]);

        while(head != NULL)
        {
            SymbolTableEntry *e = *head;
            if(!strncmp(symbolName, getName(e->nameIndex), MIN(len, e->nameLength)))
                return e;

            head = &((*head)->nextInHashChain);
        }
    }
    /* TODO : Error message : undeclared symbol */
    exit(-1);
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
}

SymbolTable* currentScope()
{
    return symbolTable.stack[symbolTable.currentLevel];
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
    {
        newSegment();
    }
    strncpy(currentEmpty(), symbolName, len);
    nameSpace.currentOffset += len;
    return makeIndex(nameSpace.currentOffset, nameSpace.size - 1);
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
