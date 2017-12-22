#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "header.h"
#include "symbolTable.h"

#define GENCODE(fmt, ...) { fprintf(output, "\t" fmt "\n", ##__VA_ARGS__); }
#define GENLABEL(name, ...) { fprintf(output, name ":\n" , ##__VA_ARGS__); }
#define ALIGN(size) { fprintf(output, "\t.align %d\n", size); }

#define CONST_TYPE(node) (node)->semantic_value.const1->const_type
#define _CONST_OF_int(node) (node)->semantic_value.const1->const_u.intval
#define _CONST_OF_float(node) (node)->semantic_value.const1->const_u.fval
#define _CONST_OF_str(node) (node)->semantic_value.const1->const_u.sc
#define CONST_VAL(node, type) _CONST_OF_##type(node)

#define IS_LOCAL(node) (ENTRY(node)->nestingLevel > 0)
#define NAME(node)  (node)->semantic_value.identifierSemanticValue.identifierName
#define STMT(node)  (node)->semantic_value.stmtSemanticValue
#define ID(node)    (node)->semantic_value.identifierSemanticValue
#define ENTRY(node) (node)->semantic_value.identifierSemanticValue.symbolTableEntry

#define FATAL(x) { \
    fprintf(stderr, "Fatal : " x "\nAborted.\n"); \
    exit(1); \
}

#define FOR_SIBLINGS(name, node) \
    for(AST_NODE* (name) = (node) \
        ; (name) \
        ; (name) = (name)->rightSibling)

static enum {NoneType, TextType, DataType} mode = NoneType;

static FILE *output;
static int _AR_offset;
static int _local_var_offset;
static int _const; // constant for constants and statments
static int regs[32];
static int FPregs[32];

int __allocReg_int(){
    // candidates: x9-x15, x19-x29
    for(int i = 9; i <= 29; i++) {
        if(i == 16) i = 19;
        if(!regs[i]) {
            regs[i] = 1;
            return i;
        }
    }
    return -1;
}

int __allocReg_float(){
    for(int i = 16; i <= 31; i++)
        if(!FPregs[i]) {
            FPregs[i] = 1;
            return i;
        }
    return -1;
}

#define allocReg(type) __allocReg_##type()

static __inline__ void __freeReg_int(int reg) { regs[reg] = 0 ; }
static __inline__ void __freeReg_float(int reg) {FPregs[reg] = 0; }
#define freeReg(reg, type) __freeReg_##type(reg)

static __inline__ void switchToText() {
    if(mode != TextType) fprintf(output, ".text\n");
}

static __inline__ void switchToData() {
    if(mode != DataType) fprintf(output, ".data\n");
}

static __inline__ void genRead() {
    GENCODE("bl _read_int");
}

static __inline__ void genFread() {
    GENCODE("bl _read_float");
}

static __inline__ void genWriteInt(int reg) {
    GENCODE("mov w0, w%d", reg);
    GENCODE("bl _write_int");
}

static __inline__ void genWriteFloat(int reg) {
    GENCODE("fmov s0, s%d", reg);
    GENCODE("bl _write_float");
}

static __inline__ void genWriteStr(int reg) {
    GENCODE("mov x0, x%d", reg);
    GENCODE("bl _write_str");
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
    switch(node->nodeType){
        case VARIABLE_DECL_LIST_NODE:
            genLocalDeclarations(node);
            break;
        case STMT_LIST_NODE:
            genStatementList(node);
            break;
        case NONEMPTY_ASSIGN_EXPR_LIST_NODE:
            FOR_SIBLINGS(child, node->child)
                genAssignOrExpr(child);
            break;
        case NONEMPTY_RELOP_EXPR_LIST_NODE:
            FOR_SIBLINGS(child, node->child)
                genExprRelatedNode(child);
            break;
        case NUL_NODE:
        default:
            break;
    }
}

void genLocalDeclarations(AST_NODE* node) {
    FOR_SIBLINGS(decl_node, node->child) {
        AST_NODE *type_node = decl_node->child;
        /* TODO: type specific initialization */
        SymbolTableEntry *entry = ENTRY(type_node->rightSibling);
        _local_var_offset += 4;
        entry->offset = _local_var_offset;
        if(ID(type_node->rightSibling).kind == WITH_INIT_ID) {
            /* initialize */
            AST_NODE *relop = type_node->rightSibling->child;
            genExprRelatedNode(relop);
            GENCODE("str %c%d, [x29, #-%d]",
                type_node->dataType == INT_TYPE ? 'w' : 's',
                relop->place, entry->offset);
            type_node->dataType == INT_TYPE ?
                freeReg(relop->place, int) : freeReg(relop->place, float);
        }
    }
}

void genExprRelatedNode(AST_NODE *node) {
    switch(node->nodeType){
        case EXPR_NODE:
            //genExprNode(node);
            break;
        case STMT_NODE:
            genFunctionCall(node);
            break;
        case IDENTIFIER_NODE:
            genVariableRvalue(node);
            break;
        case CONST_VALUE_NODE:
            genConstValueNode(node);
            break;
        default:
            break;
    }
}

void genAssignOrExpr(AST_NODE *node) {
    if(node->nodeType == STMT_NODE){
        if(STMT(node).kind == ASSIGN_STMT){
            //genAssignmentStmt(node);
        }
        else if(STMT(node).kind == FUNCTION_CALL_STMT){
            genFunctionCall(node);
        }
    }
    else{
        genExprRelatedNode(node);
    }
}

void genFunctionCall(AST_NODE *node) {
    char *func_name = NAME(node->child);
    AST_NODE *relop_expr = node->child->rightSibling;
    genExprRelatedNode(relop_expr->child);
    relop_expr->place = relop_expr->child->place;
    if(!strcmp(func_name, "write")) {
        if(relop_expr->child->dataType == INT_TYPE)
            genWriteInt(relop_expr->place);
        else if(relop_expr->child->dataType == FLOAT_TYPE)
            genWriteFloat(relop_expr->place);
        else if(relop_expr->child->dataType == CONST_STRING_TYPE)
            genWriteStr(relop_expr->place);
    }
    else if(!strcmp(func_name, "read"))
        ;
    else if(!strcmp(func_name, "fread"))
        ;
}


void genBlockNode(AST_NODE *block_node){
    FOR_SIBLINGS(child, block_node->child)
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
        switch(STMT(stmt_node).kind){
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
                genFunctionCall(stmt_node);
                break;
            case RETURN_STMT:
                //genReturnStmt(stmt_node);
                break;
        }
    }
}

void genStatementList(AST_NODE* node){
    FOR_SIBLINGS(stmt, node->child)
        genStmtNode(stmt);
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
                fprintf(output, "_%s: .word 0\n", NAME(id_list));
            }
            else{
                int array_size = 4;
                int array_dim = attribute->attr.typeDescriptor->properties.arrayProperties.dimension;
                int * array_dims = attribute->attr.typeDescriptor->properties.arrayProperties.sizeInEachDimension;
                for(int i = 0; i < array_dim; ++i){
                   array_size *= array_dims[i];
                }
               fprintf(output, "_%s: .skip %d\n", NAME(id_list), array_size);
            }
        }
        attribute->global = 1;
        id_list = id_list->rightSibling;
    }
}

void genVariableRvalue(AST_NODE *id_node) {
    if(id_node->dataType == INT_TYPE)
        id_node->place = allocReg(int);
    else if(id_node->dataType == FLOAT_TYPE)
        id_node->place = allocReg(float);
    else
        FATAL("invalid type");

    /* localor global */
    if(IS_LOCAL(id_node)) {
        /* TODO: array */
        GENCODE("ldr %c%d, [x29, #-%d]",
            id_node->dataType == INT_TYPE ? 'w' : 's',
            id_node->place, ENTRY(id_node)->offset);
    } else {
        /* TODO: array */
        GENCODE("ldr %c%d, =_%s",
            id_node->dataType == INT_TYPE ? 'w' : 's',
            id_node->place, NAME(id_node));
    }
}

void genConstValueNode(AST_NODE *node) {
    if(CONST_TYPE(node) == INTEGERC) {
        node->place = allocReg(int);
        GENCODE("mov w%d, #%d", node->place, CONST_VAL(node, int));
    } else if(CONST_TYPE(node) == FLOATC) {
        node->place = allocReg(float);
        switchToData();
        GENCODE("_CONST_%d: .float %f", ++_const, CONST_VAL(node, float));
        switchToText();
        GENCODE("ldr s%d, _CONST_%d", node->place, _const);
    } else {
        node->place = allocReg(int);
        switchToData();
        GENCODE("_CONST_%d: .string %s", ++_const, CONST_VAL(node, str));
        ALIGN(4);
        switchToText();
        GENCODE("ldr x%d, =_CONST_%d", node->place, _const);
    }
}

void genGlobalDeclrations(AST_NODE *node){
    FOR_SIBLINGS(decl, node->child)
        genGlobalDeclration(decl);
}

void genProgramNode(AST_NODE *program_node){
    FOR_SIBLINGS(global_decl_list, program_node->child) {
        if(global_decl_list->nodeType == VARIABLE_DECL_LIST_NODE) 
            genGlobalDeclrations(global_decl_list);
        else if(global_decl_list->nodeType == DECLARATION_NODE)
            genFunctionDeclration(global_decl_list);
    }
}

void genFunctionDeclration(AST_NODE *func_decl){
    _AR_offset = 0;
    char *name = NAME(func_decl->child->rightSibling);
    AST_NODE *paramListNode = func_decl->child->rightSibling->rightSibling;
    genPrologue(name);

    _local_var_offset = 0;
    FOR_SIBLINGS(list_node, paramListNode->rightSibling->child)
        genGeneralNode(list_node);

    genEpilogue(name);
}

void codeGeneration(AST_NODE *root) {
    output = fopen("output.s", "w");
    genProgramNode(root);
    fclose(output);
}

void genPrologue(char *name){
    switchToText();
    GENLABEL("_start_%s", name);
    GENCODE("str x30, [sp, #0]");
    GENCODE("str x29, [sp, #-8]");
    GENCODE("add x29, sp, #-8");
    GENCODE("add sp, sp, #-16");
    GENCODE("ldr x30, =_frameSize_%s", name);
    GENCODE("ldr w30, [x30, #0]");
    GENCODE("sub sp, sp, w30");

    int offset = 8;
    // push callee saved registers
    for(int i = 19; i <= 29; i++, offset += 8)
        GENCODE("str x%d, [sp, #%d]", i, offset);
    // VFP regs
    for(int i = 16; i <= 23; i++, offset += 8)
        GENCODE("ldr s%d, [sp, #%d]", i, offset);

    _AR_offset = offset;
    GENLABEL("_begin_%s", name);
}

void genEpilogue(char *name){
    GENLABEL("_end_%s", name);

    int offset = 8;
    for(int i = 19; i <= 29; i++, offset += 8)
        GENCODE("ldr x%d, [sp, #%d]", i, offset);
    for(int i = 16; i <= 23; i++, offset += 8)
        GENCODE("ldr s%d, [sp, #%d]", i, offset);

    GENCODE("ldr x30, [x29, #8]")
    GENCODE("add sp, x29, #8")
    GENCODE("ldr x29, [x29, #0]")
    GENCODE("ret x30");
    switchToData();
    fprintf(output, "_frameSize_%s: .word %d\n", name, _AR_offset);
    switchToText();
}
