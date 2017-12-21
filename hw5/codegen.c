#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "header.h"
#include "symbolTable.h"

#define gencode(...) { fprintf(output, __VA_ARGS__); }
#define genlabel(...) { fprintf(output, __VA_ARGS__); }

#define ID_NAME(node) (node)->semantic_value.identifierSemanticValue.identifierName
#define ID_KIND(node) (node)->semantic_value.identifierSemanticValue.kind
#define CONST_TYPE(node) (node)->semantic_value.const1->const_type;
#define _CONST_OF_int(node) (node)->semantic_value.const1->const_u.intval
#define _CONST_OF_float(node) (node)->semantic_value.const1->const_u.fval
#define CONST_VAL_OF(node, type) _CONST_OF_##type(node)
#define STMT_KIND(node) (node)->semantic_value.stmtSemanticValue.kind

static enum {NoneType, TextType, DataType} mode = NoneType;

static FILE *output;
static int _AR_offset; // count AR size
static int _offset;
static int _const = 0; // constant for constants and statments

static int regs[32] = {0};

int allocReg(){
    // candidates: x9-x15, x19-x29
    int i;
    for(i = 9; i <= 29; i++){
        if(i == 16) i = 19;
        if(!regs[i]) return (regs[i] = 1);
    }
    printf("Out of registers\n");
    exit(1);
}

static __inline__ void freeReg(int reg){
    regs[reg] = 0;
}

static __inline__ void switchToText() {
    if(mode != TextType)
        genlabel(".text\n");
}
static __inline__ void switchToData() {
    if(mode != DataType)
        genlabel(".data\n");
}

static __inline__ void genRead(int offset) {
    gencode("\tbl _read_int\n"
            "\tmov w9, w0\n" /* maybe we should allocReg instead of fixed w9 */
            "\tstr w9, [x29, #-%d]\n", offset);
}

static __inline__ void genFread(int offset) {
    gencode("\tbl _read_float\n"
            "\tfmov s16, s0\n"
            "\tstr w9, [x29, #-%d]\n", offset);
}

static __inline__ void genWriteInt(int offset) {
    gencode("\tldr w9, [x29, #-%d]\n"
            "\tmov w0, w9\n"
            "\tbl _write_int\n", offset);
}

static __inline__ void genWriteFloat(int offset) {
    gencode("ldr s16, [x29, #-%d]"
            "mov s0, s16"
            "bl _write_float", offset);
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

void genGeneralNode(AST_NODE *node)
{
    AST_NODE *child= node->child;
    switch(node->nodeType){
        case VARIABLE_DECL_LIST_NODE:
            genLocalDeclarations(node);
            break;
        case STMT_LIST_NODE:
            genStatementList(node);
            break;
        case NONEMPTY_ASSIGN_EXPR_LIST_NODE:
            while(child){
                genAssignOrExpr(child);
                child = child->rightSibling;
            }
            break;
        case NONEMPTY_RELOP_EXPR_LIST_NODE:
            while(child){
                genExprRelatedNode(child);
                child= child->rightSibling;
            }
            break;
        case NUL_NODE:
        default:
            break;
    }
}

void genLocalDeclarations(AST_NODE* node) {
    for(AST_NODE *decl_node = node->child
        ; decl_node
        ; decl_node = decl_node->rightSibling) {
        AST_NODE *type_node = decl_node->child;
        /* TODO: type specific initialization */
        if(type_node->dataType == INT_TYPE) {
            _offset += 4;
            /* insert into symbol table ? */
        } else if(type_node->dataType == FLOAT_TYPE) {
        }
        if(ID_KIND(type_node->rightSibling) == WITH_INIT_ID) {
            /* initialize */
            AST_NODE *val_node = type_node->rightSibling->child;
            if(type_node->dataType == INT_TYPE) {
                int reg = allocReg();
                node->place = reg;
                gencode("\tmov x%d, #%d\n", reg, CONST_VAL_OF(val_node, int));
                gencode("\tstr x%d, [x29, #-%d]\n", reg, _offset);
            } else if(type_node->dataType == FLOAT_TYPE) {
                /* TODO generate constant data and label*/
            }
        }
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
        default:
            break;
    }
}

void genAssignOrExpr(AST_NODE *node){
    if(node->nodeType == STMT_NODE){
        if(STMT_KIND(node) == ASSIGN_STMT){
            //genAssignmentStmt(node);
        }
        else if(STMT_KIND(node) == FUNCTION_CALL_STMT){
            //genFunctionCall(node);
        }
    }
    else{
        genExprRelatedNode(node);
    }
}


void genBlockNode(AST_NODE *block_node){
    for(AST_NODE *child = block_node->child
        ; child
        ; child = child->rightSibling)
        genGeneralNode(child);
}

void genStmtNode(AST_NODE *stmt_node){
    if(stmt_node->nodeType == NUL_NODE){
        return;
    }
    else if(stmt_node->nodeType == BLOCK_NODE){
        genBlockNode(stmt_node);
    }
    else{
        switch(STMT_KIND(stmt_node)){
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
    for(AST_NODE *stmt_list = stmt_list_node->child
        ; stmt_list
        ; stmt_list = stmt_list->rightSibling)
        genStmtNode(stmt_list);
}

void genGlobalDeclration(AST_NODE *decl){
    /* copy-past from semanticAnalysis.c */
    AST_NODE *type_node = decl->child;
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
                genlabel("_%s: .word 0\n", ID_NAME(id_list));
            }
            else{
                int array_size = 4;
                int array_dim = attribute->attr.typeDescriptor->properties.arrayProperties.dimension;
                int * array_dims = attribute->attr.typeDescriptor->properties.arrayProperties.sizeInEachDimension;
                for(int i = 0; i < array_dim; ++i){
                   array_size *= array_dims[i];
                }
                genlabel("_%s: .skip %d\n", ID_NAME(id_list), array_size);
            }
        }
        attribute->global = 1;
        id_list = id_list->rightSibling;
    }
}

void genGlobalDeclrations(AST_NODE *decl_list_node){
    for(AST_NODE *decl = decl_list_node->child
        ; decl
        ; decl = decl->rightSibling)
        genGlobalDeclration(decl);
}

void genProgramNode(AST_NODE *program_node){
    for(AST_NODE *global_decl_list = program_node->child
        ; global_decl_list
        ; global_decl_list = global_decl_list->rightSibling) {
        if(global_decl_list->nodeType == VARIABLE_DECL_LIST_NODE) 
            genGlobalDeclrations(global_decl_list);
        else if(global_decl_list->nodeType == DECLARATION_NODE)
            genFunctionDeclration(global_decl_list);
    }
}

void genFunctionDeclration(AST_NODE *func_decl){
    char *name = ID_NAME(func_decl->child->rightSibling);
    AST_NODE *paramListNode = func_decl->child->rightSibling->rightSibling;
    genPrologue(name);

    _offset = 0;
    for(AST_NODE *list_node = paramListNode->rightSibling->child
        ; list_node
        ; list_node = list_node->rightSibling)
        genGeneralNode(list_node);

    genEpilogue(name);
}

void codeGeneration(AST_NODE *root) {
    output = fopen("output.s", "w");
    genProgramNode(root);
    fclose(output);
}

void genEpilogue(char *name){
    genlabel("_end_%s:\n", name);

    int i, offset = 8;
    for(i = 9; i <= 15; i++, offset += 8)
        gencode("\tldr x%d, [sp, #%d]\n", i, offset);
    for(i = 19; i <= 29; i++, offset += 8)
        gencode("\tldr x%d, [sp, #%d]\n", i, offset);

    gencode("\tldr x30, [x29, #8]\n"
            "\tadd sp, sp, #8\n"
            "\tldr x29, [x29, #0]\n"
            "\tret x30\n");
    switchToData();
    gencode("\t_frameSize_%s: .word %d\n", name, _AR_offset);
    switchToText();
}

void genPrologue(char *name){
    switchToText();
    genlabel("_start_%s:\n", name);
    gencode("\tstr x30, [sp, #0]\n"
            "\tstr x29, [sp, #-8]\n"
            "\tadd x29, sp, #-8\n"
            "\tadd sp, sp, #-16\n"
            "\tldr x30, =_frameSize_%s\n"
            "\tldr w30, [x30, #0]\n"
            "\tsub sp, sp, w30\n", name);
    
    int offset = 8;
    int i;
    // caller saved registers
    for(i = 9; i <= 15; i++, offset += 8)
        gencode("\tstr x%d, [sp, #%d]\n", i, offset);
    // callee saved registers
    for(i = 19; i <= 29; i++, offset += 8)
        gencode("\tstr x%d, [sp, #%d]\n", i, offset);

    _AR_offset = offset - 4;
    genlabel("_begin_%s:\n", name);
}
