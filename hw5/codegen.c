#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "header.h"
#include "symbolTable.h"

enum MODE_TYPE {TextType, DataType};

FILE *output;
int _offset; // count AR size
int _const = 0; // constant for constants and statments

int regs[32] = {0};

int getReg(){
    // candidates: x9-x15, x19-x29
    int i;
    for(i = 9; i <= 15; i++){
        if(!regs[i]){
            regs[i] = 1;
            return i;
        }
    }
    for(i = 19; i <= 29; i++){
        if(!regs[i]){
            regs[i] = 1;
            return i;
        }
    }
    printf("Out of registers\n");
    exit(1);
}

void freeReg(int reg){
    regs[reg] = 0;
}

void genPrologue(char *name);
void genEpilogue(char *name);

void genAssignOrExpr(AST_NODE *node);
void genExprRelatedNode(AST_NODE *node);
void genExprNode(AST_NODE *expr_node);
void genFunctionCall(AST_NODE *function_node);
void genAssignmentStmt(AST_NODE *assign_node);
void genBlockNode(AST_NODE *block_node);
void genForStmt(AST_NODE *for_node);
void genWhileStmt(AST_NODE *while_node);
void genReturnStmt(AST_NODE *return_node);
void genIfStmt(AST_NODE *if_node);
void genStmtNode(AST_NODE *stmt_node);
void genStatementList(AST_NODE* stmt_list_node);
void genLocalDeclarations(AST_NODE* decl_list_node);
void genGlobalDeclration(AST_NODE *decl);
void genGlobalDeclrations(AST_NODE *decl_list_node);
void genProgramNode(AST_NODE *program_node);
void genFunctionDeclration(AST_NODE *func_decl);
void genVariableRvalue(AST_NODE *id_node);
void genConstValueNode(AST_NODE *const_node);
void genGeneralNode(AST_NODE *node);

static __inline__ char* getIdByNode(AST_NODE *node) {
    return node->semantic_value.identifierSemanticValue.identifierName;
}

void genGeneralNode(AST_NODE *node)
{
    AST_NODE *traverseChild = node->child;
    switch(node->nodeType){
        case VARIABLE_DECL_LIST_NODE:
            //genLocalDeclarations(node);
            break;
        case STMT_LIST_NODE:
            genStatementList(node);
            break;
        case NONEMPTY_ASSIGN_EXPR_LIST_NODE:
            while(traverseChild){
                genAssignOrExpr(traverseChild);
                traverseChild = traverseChild->rightSibling;
            }
            break;
        case NONEMPTY_RELOP_EXPR_LIST_NODE:
            while(traverseChild){
                genExprRelatedNode(traverseChild);
                traverseChild = traverseChild->rightSibling;
            }
            break;
        case NUL_NODE:
            break;
    }
}

void genExprRelatedNode(AST_NODE *node){
    switch(node->nodeType){
        case EXPR_NODE:
            //genExprNode(node);
            break;
        case STMT_NODE:
            // function call
            //genFunctionCall(node);
            break;
        case IDENTIFIER_NODE:
            //genVariableRvalue(node);
            break;
        case CONST_VALUE_NODE:
            //genConstValueNode(node);
            break;
    }
}

void genAssignOrExpr(AST_NODE *node){
    if(node->nodeType == STMT_NODE){
        if(node->semantic_value.stmtSemanticValue.kind == ASSIGN_STMT){
            //genAssignmentStmt(node);
        }
        else if(node->semantic_value.stmtSemanticValue.kind == FUNCTION_CALL_STMT){
            //genFunctionCall(node);
        }
    }
    else{
        genExprRelatedNode(node);
    }
}


void genBlockNode(AST_NODE *block_node){
    openScope();

    AST_NODE *traverseListNode = block_node->child;
    while(traverseListNode)
    {
        genGeneralNode(traverseListNode);
        traverseListNode = traverseListNode->rightSibling;
    }
    closeScope();
}

void genStmtNode(AST_NODE *stmt_node){
    if(stmt_node->nodeType == NUL_NODE){
        return;
    }
    else if(stmt_node->nodeType == BLOCK_NODE){
        genBlockNode(stmt_node);
    }
    else{
        switch(stmt_node->semantic_value.stmtSemanticValue.kind){
            case WHILE_STMT:
                //genWhileStmt(stmt_node);
                break;
            case FOR_STMT:
                //genForStmt(stmt_node);
                break;
            case ASSIGN_STMT:
                //genAssignmentStmt(stmt_node);
                break;
            case IF_STMT:
                //genIfStmt(stmt_node);
                break;
            case FUNCTION_CALL_STMT:
                //genFunctionCall(stmt_node);
                break;
            case RETURN_STMT:
                //genReturnStmt(stmt_node);
                break;
        }
    }
}

void genStatementList(AST_NODE* stmt_list_node){
    AST_NODE *stmt_list = stmt_list_node->child;
    while(stmt_list){
        genStmtNode(stmt_list);
        stmt_list = stmt_list->rightSibling;
    }
}

void genGlobalDeclration(AST_NODE *decl){
    /* copy-past from semanticAnalysis.c */
    AST_NODE *type_node = decl->child;
    processTypeNode(type_node);
    AST_NODE *id_list = type_node->rightSibling;
    while(id_list){
        SymbolAttribute * attribute = malloc(sizeof(SymbolAttribute));
        if(decl->semantic_value.declSemanticValue.kind == VARIABLE_DECL){
            attribute->attributeKind = VARIABLE_ATTRIBUTE;
        }
        else{
            attribute->attributeKind = TYPE_ATTRIBUTE;
        }
        switch(id_list->semantic_value.identifierSemanticValue.kind){
            case NORMAL_ID:
                attribute->attr.typeDescriptor = type_node->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor;
                break;
            case WITH_INIT_ID:
                attribute->attr.typeDescriptor = type_node->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor;
                break;
            case ARRAY_ID:
                attribute->attr.typeDescriptor = (TypeDescriptor*)malloc(sizeof(TypeDescriptor));
                processDeclDimList(id_list, attribute->attr.typeDescriptor, 0);
                if(type_node->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor->kind == SCALAR_TYPE_DESCRIPTOR){
                    attribute->attr.typeDescriptor->properties.arrayProperties.elementType =
                        type_node->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor->properties.dataType;
                }else if(type_node->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor->kind == ARRAY_TYPE_DESCRIPTOR){
                    int typeArrayDimension = type_node->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor->properties.arrayProperties.dimension;
                    int idArrayDimension = attribute->attr.typeDescriptor->properties.arrayProperties.dimension;
                    attribute->attr.typeDescriptor->properties.arrayProperties.elementType = 
                        type_node->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor->properties.arrayProperties.elementType;
                    attribute->attr.typeDescriptor->properties.arrayProperties.dimension = 
                        typeArrayDimension + idArrayDimension;
                    int indexType = 0;
                    int indexId = 0;
                    for(indexType = 0, indexId = idArrayDimension; indexId < idArrayDimension + typeArrayDimension; ++indexType, ++indexId)
                    {
                        attribute->attr.typeDescriptor->properties.arrayProperties.sizeInEachDimension[indexId] = 
                            type_node->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor->properties.arrayProperties.sizeInEachDimension[indexType];
                    }
                 }
            break;
        }
        /* copy-past end */
        if(decl->semantic_value.declSemanticValue.kind == VARIABLE_DECL){
            if(attribute->attr.typeDescriptor->kind == SCALAR_TYPE_DESCRIPTOR){
                fprintf(output, "_%s: .word 0\n", getIdByNode(id_list));
            }
            else{
                int array_size = 4;
                int array_dim = attribute->attr.typeDescriptor->properties.arrayProperties.dimension;
                int * array_dims = attribute->attr.typeDescriptor->properties.arrayProperties.sizeInEachDimension;
                for(int i = 0; i < array_dim; ++i){
                   array_size *= array_dims[i];
                }
                fprintf(output, "_%s: .skip %d\n", getIdByNode(id_list), array_size);
            }
        }
        attribute->global = 1;
        id_list->semantic_value.identifierSemanticValue.symbolTableEntry =
          enterSymbol(id_list->semantic_value.identifierSemanticValue.identifierName, attribute);
        id_list = id_list->rightSibling;
    }
}

void genGlobalDeclrations(AST_NODE *decl_list_node){
    AST_NODE *decls = decl_list_node->child;
    fprintf(output, ".data\n");
    while(decls){
        genGlobalDeclration(decls);
        decls = decls->rightSibling;
    }
}


void genProgramNode(AST_NODE *program_node){
    AST_NODE *global_decl_list = program_node->child;
    while(global_decl_list){
        if(global_decl_list->nodeType == VARIABLE_DECL_LIST_NODE){
            genGlobalDeclrations(global_decl_list);
        }
        else if(global_decl_list->nodeType == DECLARATION_NODE){
            genFunctionDeclration(global_decl_list);
        }
        global_decl_list = global_decl_list->rightSibling;
    }
}

void genFunctionDeclration(AST_NODE *func_decl){
    /* copy-past from semanticAnalysis.c */
    AST_NODE* returnTypeNode = func_decl->child;
    AST_NODE* functionNameID = returnTypeNode->rightSibling;
    SymbolAttribute * attribute = NULL;
    attribute = (SymbolAttribute*)malloc(sizeof(SymbolAttribute));
    attribute->attributeKind = FUNCTION_SIGNATURE;
    attribute->attr.functionSignature = (FunctionSignature*)malloc(sizeof(FunctionSignature));
    attribute->attr.functionSignature->returnType = returnTypeNode->dataType;
    attribute->attr.functionSignature->parameterList = NULL;

    enterSymbol(getIdByNode(functionNameID), attribute);

    openScope();

    AST_NODE * parameterListNode = functionNameID->rightSibling;
    AST_NODE * traverseParameter = parameterListNode->child;
    int parametersCount = 0;
    if(traverseParameter)
    {
        ++parametersCount;
        processDeclarationNode(traverseParameter);
        AST_NODE *parameterID = traverseParameter->child->rightSibling;
        Parameter *parameter = (Parameter*)malloc(sizeof(Parameter));
        parameter->next = NULL;
        parameter->parameterName = parameterID->semantic_value.identifierSemanticValue.identifierName;
        parameter->type = parameterID->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor;
        attribute->attr.functionSignature->parameterList = parameter;
        traverseParameter = traverseParameter->rightSibling;
    }

    Parameter *parameterListTail = attribute->attr.functionSignature->parameterList;

    while(traverseParameter)
    {
        ++parametersCount;
        processDeclarationNode(traverseParameter);
        AST_NODE *parameterID = traverseParameter->child->rightSibling;
        Parameter *parameter = (Parameter*)malloc(sizeof(Parameter));
        parameter->next = NULL;
        parameter->parameterName = parameterID->semantic_value.identifierSemanticValue.identifierName;
        parameter->type = parameterID->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor;
        parameterListTail->next = parameter;
        parameterListTail = parameter;
        traverseParameter = traverseParameter->rightSibling;
    }
    attribute->attr.functionSignature->parametersCount = parametersCount;
    /* copy-past end*/

    /* Prologue start */
    fprintf(output, ".text\n");
    fprintf(output, "_start_%s:\n", getIdByNode(functionNameID));
    genPrologue(getIdByNode(functionNameID));
    
    AST_NODE *block_node = parameterListNode->rightSibling;
    AST_NODE *block_list_node = block_node->child;
    int offset;
    while(block_list_node){
        genGeneralNode(block_list_node);
        block_list_node = block_list_node->rightSibling;
    }
    fprintf(output, "_end_%s:\n", getIdByNode(functionNameID));
    genEpilogue(getIdByNode(functionNameID));
    closeScope();
}

void genEpilogue(char *name){
    int offset = 8;
    int i;
    for(i = 9; i <= 15; i++){
        fprintf(output, "ldr x%d, [sp, #%d]\n", i, offset);
        offset += 8;
    }
    for(i = 19; i <= 29; i++){
        fprintf(output, "ldr x%d, [sp, #%d]\n", i, offset);
        offset += 8;
    }
    for(i = 16; i <= 23; i++){
        fprintf(output, "ldr s%d, [sp, #%d]\n", i, offset);
        offset += 4;
    }

    fprintf(output, "ldr x30, [x29, #8]\n");
    fprintf(output, "add sp, sp, #8\n");
    fprintf(output, "ldr x29, [x29, #0]\n");
    fprintf(output, "RET x30\n");

    fprintf(output, ".data\n");
    fprintf(output, "_frameSize_%s: .word %d\n", name, _offset);
    fprintf(output, ".text\n");
}

void genPrologue(char *name){
    fprintf(output, "str x30, [sp, #0]\n");
    fprintf(output, "str x29, [sp, #-8]\n");
    fprintf(output, "add x29, sp, #-8\n");
    fprintf(output, "add sp, sp, #-16\n");
    fprintf(output, "ldr x30, _frameSize_%s\n", name);
    fprintf(output, "ldr w30, [x30, #0]\n");
    fprintf(output, "sub sp, sp, w30\n");
    
    int offset = 8;
    int i;
    // caller saved registers
    for(i = 9; i <= 15; i++){
        fprintf(output, "str x%d, [sp, #%d]\n", i, offset);
        offset += 8;
    }
    // callee saved registers
    for(i = 19; i <= 29; i++){
        fprintf(output, "str x%d, [sp, #%d]\n", i, offset);
        offset += 8;
    }
    // TA's template
    for(i = 16; i <= 23; i++){
        fprintf(output, "str s%d, [sp, #%d]\n", i, offset);
        offset += 4;
    }
    
    _offset = offset - 4;
}

void codeGeneration(AST_NODE *root) {
    output = fopen("output.s", "w");
    genProgramNode(root);
    fclose(output);
}
