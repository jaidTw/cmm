#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "header.h"
#include "symbolTable.h"

enum MODE_TYPE {TextType, DataType};

void genPrologue();
void genEpilogue();

void codeGeneration(AST_NODE *root) {
    
}
