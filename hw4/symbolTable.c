#include "symbolTable.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
// This file is for reference only, you are not required to follow the implementation. //

#define ERROR(x) { fprintf(stderr, x); exit(-1);}

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
    int i = 0; 
    SymbolTableEntry *entry = NULL;
    for(i = symbolTable.currentLevel; i >= 0; i--) {
        SymbolTable *table = symbolTable.stack[i];
        SymbolTableEntry *e = table->hashTable[hashIndex];
        /* follow the chain */
        /* if(found) {
         *     entry = e
         * }
         */
    }
    if(!entry) {
        /* Error message : undeclared symbol */
        exit(-1);
    }
    return entry;
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

void openScope()
{
    symbolTable.currentLevel += 1;
    symbolTable.top += 1;
}

void closeScope()
{
    symbolTable.currentLevel -= 1;
    symbolTable.top -= 1;
}

void initializeNameSpace() {
    nameSpace.size = 0;
    newSegment();
}

void newSegment() {
    if(nameSpace.size >= NAMESPACE_SIZE)
        ERROR("Maximum namespace size reached.");

    nameSpace.size += 1;
    nameSpace.segments[nameSpace.size - 1] = (char *) calloc(NAMESPACE_SEGMENT_SIZE + 1, 1);
    nameSpace.currentOffset = 0;
}

int enterSymbolNS(char *symbolName) {
    int len = strlen(symbolName);
    if(len > NAMESPACE_SEGMENT_SIZE)
        ERROR("Symbol name exceeds segment size of namespace");

    int i;
    for(i = 0; i < nameSpace.size; ++i) {
        char* p = strstr(nameSpace.segments[i], symbolName);
        if(p)
            return makeIndex(p - nameSpace.segments[i], i);
    }

    if(nameSpace.currentOffset + len > NAMESPACE_SEGMENT_SIZE) {
        newSegment();
    }
    strncpy(currentEmpty(), symbolName, len);
    nameSpace.currentOffset += len;
    return makeIndex(nameSpace.currentOffset, nameSpace.size - 1);
}

__inline__ char *currentEmpty() {
    return nameSpace.segments[nameSpace.size - 1] + nameSpace.currentOffset;
}

__inline__ int makeIndex(int offset, int segment) {
    return offset + NAMESPACE_SEGMENT_SIZE * segment;
}
