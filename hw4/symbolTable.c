#include "symbolTable.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
// This file is for reference only, you are not required to follow the implementation. //

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
    symbolTableEntry->nextInHashChain = NULL;
    symbolTableEntry->prevInHashChain = NULL;
    symbolTableEntry->nextInSameLevel = NULL;
    symbolTableEntry->sameNameInOuterLevel = NULL;
    symbolTableEntry->attribute = NULL;
    symbolTableEntry->name_index = 0;
    symbolTableEntry->name_length = 0;
    symbolTableEntry->nestingLevel = nestingLevel;
    return symbolTableEntry;
}

void removeFromHashTrain(int hashIndex, SymbolTableEntry* entry)
{
}

void enterIntoHashTrain(int hashIndex, SymbolTableEntry* entry)
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
}

SymbolTableEntry* enterSymbol(char* symbolName, SymbolAttribute* attribute)
{
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
    nameSpace.size += 1;
    nameSpace.segments[nameSpace.size - 1] = calloc(NAMESPACE_SEGMENT_SIZE + 1, 1);
    nameSpace.currentOffset = 0;
}

int enterSymbolNS(char *symbolName) {
    int len = strlen(symbolName);

    for(int i = 0; i < nameSpace.size; ++i) {
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
