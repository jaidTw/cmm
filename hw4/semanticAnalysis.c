#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "header.h"
#include "symbolTable.h"
// This file is for reference only, you are not required to follow the implementation. //
// You only need to check for errors stated in the hw4 assignment document. //

#define FATAL(x) { \
    fprintf(stderr, "Fatal : " x "\nAborted.\n"); \
    exit(1); \
}

int g_anyErrorOccur = 0;

DATA_TYPE getBiggerType(DATA_TYPE dataType1, DATA_TYPE dataType2);
void processProgramNode(AST_NODE *programNode);
void processDeclarationNode(AST_NODE* declarationNode);
void declareIdList(AST_NODE* typeNode, SymbolAttributeKind isVariableOrTypeAttribute, int ignoreArrayFirstDimSize);
void declareFunction(AST_NODE* returnTypeNode);
int processDeclDimList(AST_NODE* variableDeclDimList, TypeDescriptor* desc, int ignoreFirstDimSize);
DATA_TYPE processTypeNode(AST_NODE* typeNode);
void processBlockNode(AST_NODE* blockNode);
void processStmtNode(AST_NODE* stmtNode);
void processGeneralNode(AST_NODE *node);
void checkAssignOrExpr(AST_NODE* assignOrExprRelatedNode);
void checkWhileStmt(AST_NODE* whileNode);
void checkForStmt(AST_NODE* forNode);
void checkAssignmentStmt(AST_NODE* assignmentNode);
void checkIfStmt(AST_NODE* ifNode);
void checkWriteFunction(AST_NODE* functionCallNode);
DATA_TYPE checkFunctionCall(AST_NODE* functionCallNode);
DATA_TYPE processExprRelatedNode(AST_NODE* exprRelatedNode);
void checkParameterPassing(Parameter* formalParameter, AST_NODE* actualParameter);
void checkReturnStmt(AST_NODE* returnNode);
DATA_TYPE processExprNode(AST_NODE* exprNode);
void processVariableLValue(AST_NODE* idNode);
void processVariableRValue(AST_NODE* idNode);
void processConstValueNode(AST_NODE* constValueNode);
DATA_TYPE getExprOrConstValue(AST_NODE* exprOrConstNode, int* iValue, float* fValue);
void evaluateExprValue(AST_NODE* exprNode, DATA_TYPE datatype);

void processVariableDeclListNode(AST_NODE* decl_list);

void dumpNode(AST_NODE *node);
void dumpExprNode(AST_NODE *node);
void dumpConstNode(AST_NODE *node);

static __inline__ char* getIdByNode(AST_NODE *node) {
    // just return the variable name
    return node->semantic_value.identifierSemanticValue.identifierName;
}

typedef enum ErrorMsgKind
{
    SYMBOL_IS_NOT_TYPE,
    SYMBOL_REDECLARE,
    SYMBOL_UNDECLARED,
    NOT_FUNCTION_NAME,
    TRY_TO_INIT_ARRAY,
    EXCESSIVE_ARRAY_DIM_DECLARATION,
    RETURN_ARRAY,
    VOID_VARIABLE,
    TYPEDEF_VOID_ARRAY,
    PARAMETER_TYPE_UNMATCH,
    TOO_FEW_ARGUMENTS,
    TOO_MANY_ARGUMENTS,
    RETURN_TYPE_UNMATCH,
    INCOMPATIBLE_ARRAY_DIMENSION,
    NOT_ASSIGNABLE,
    NOT_ARRAY,
    IS_TYPE_NOT_VARIABLE,
    IS_FUNCTION_NOT_VARIABLE,
    STRING_OPERATION,
    ARRAY_SIZE_NOT_INT,
    ARRAY_SIZE_NEGATIVE,
    ARRAY_SUBSCRIPT_NOT_INT,
    PASS_ARRAY_TO_SCALAR,
    PASS_SCALAR_TO_ARRAY
} ErrorMsgKind;

/*
void printErrorMsgSpecial(AST_NODE* node1, char* name2, ErrorMsgKind errorMsgKind)
{
    g_anyErrorOccur = 1;
    printf("Error found in line %d\n", node1->linenumber);
    
    switch(errorMsgKind)
    {
        printf("Unhandled case in void printErrorMsg(AST_NODE* node, ERROR_MSG_KIND* errorMsgKind)\n");
        break;
    }
}
*/


void printErrorMsg(AST_NODE* node, ErrorMsgKind errorMsgKind)
{
    g_anyErrorOccur = 1;
    printf("Error found in line %d\n", node->linenumber);
    switch(errorMsgKind)
    {
    case SYMBOL_IS_NOT_TYPE:
        printf("<%s> doesn't named a type.\n", getIdByNode(node));
        break;
    case SYMBOL_REDECLARE:
        printf("ID <%s> redeclared.\n", getIdByNode(node));
        break;
    case SYMBOL_UNDECLARED:
        printf("ID <%s> undeclared.\n", getIdByNode(node));
        break;
    case NOT_FUNCTION_NAME:
        printf("ID <%s> is not a function.\n", getIdByNode(node));
        break;
    case TOO_FEW_ARGUMENTS:
        printf("too few arguments to function <%s>.\n", getIdByNode(node));
        break;
    case TOO_MANY_ARGUMENTS:
        printf("too many arguments to function <%s>.\n", getIdByNode(node));
        break;
    case RETURN_TYPE_UNMATCH:
        printf("Incompatible return type.\n");
        break;
    case INCOMPATIBLE_ARRAY_DIMENSION:
        printf("Incompatible array dimensions.\n");
        break;
    case ARRAY_SIZE_NOT_INT:
        printf("Array size is not an integer.\n");
        break;
    case ARRAY_SIZE_NEGATIVE:
        printf("Array size is negative.\n");
        break;
    case ARRAY_SUBSCRIPT_NOT_INT:
        printf("Array subscript is not an integer.\n");
        break;
    /* */
    case VOID_VARIABLE:
        printf("ID <%s> is of type void.\n", getIdByNode(node));
        break;
    case NOT_ARRAY:
        printf("ID <%s> is not an array.\n", getIdByNode(node));
        break;
    case TRY_TO_INIT_ARRAY:
    case EXCESSIVE_ARRAY_DIM_DECLARATION:
    case RETURN_ARRAY:
    case TYPEDEF_VOID_ARRAY:
    case PARAMETER_TYPE_UNMATCH:
    case NOT_ASSIGNABLE:
    case IS_TYPE_NOT_VARIABLE:
    case IS_FUNCTION_NOT_VARIABLE:
    case STRING_OPERATION:
    case PASS_ARRAY_TO_SCALAR:
    case PASS_SCALAR_TO_ARRAY:
    default:
        FATAL("Invalid ErrorMsgKind!");
        break;
    }
}


SymbolAttribute* newAttribute(SymbolAttributeKind attributeKind)
{
    SymbolAttribute *temp = (SymbolAttribute*)malloc(sizeof(struct SymbolAttribute));
    temp->attributeKind = attributeKind;
    // need to specify attr later
    return temp;
}

TypeDescriptor* newTypeDesc(TypeDescriptorKind kind)
{
    TypeDescriptor *temp = (TypeDescriptor*)malloc(sizeof(struct TypeDescriptor));
    temp->kind = kind;
    // need to sepcify properties later
    return temp;
}

SymbolTableEntry* insertType(char *name, DATA_TYPE type){
    SymbolAttribute *attr = newAttribute(TYPE_ATTRIBUTE);
    TypeDescriptor *desc = newTypeDesc(SCALAR_TYPE_DESCRIPTOR);
    desc->properties.dataType = type;
    attr->attr.typeDescriptor = desc;
    return enterSymbol(name, attr);
}

void semanticAnalysis(AST_NODE *root)
{
    // Preinsert types
    insertType(SYMBOL_TABLE_INT_NAME, INT_TYPE);
    insertType(SYMBOL_TABLE_FLOAT_NAME, FLOAT_TYPE);
    insertType(SYMBOL_TABLE_VOID_NAME, VOID_TYPE);
    // preinsert read, fread?

    processProgramNode(root);
}


DATA_TYPE getBiggerType(DATA_TYPE dataType1, DATA_TYPE dataType2)
{
    if(dataType1 == FLOAT_TYPE || dataType2 == FLOAT_TYPE) {
        return FLOAT_TYPE;
    } else {
        return INT_TYPE;
    }
}

void processProgramNode(AST_NODE *programNode)
{
    AST_NODE *global_decl_list = programNode->child;
    while(global_decl_list){
        // process global_decl
        switch(global_decl_list->nodeType){
            // two case: decl_list, function_decl
            case VARIABLE_DECL_LIST_NODE:
                processVariableDeclListNode(global_decl_list);
                break;
            case DECLARATION_NODE:
                processDeclarationNode(global_decl_list);
                break;
            default:
                printf("Invalid program\n");
                exit(1);
        }
        global_decl_list = global_decl_list->rightSibling;
    }
}

void processVariableDeclListNode(AST_NODE *decl_list_node){
    AST_NODE *decl_list = decl_list_node->child;
    while(decl_list){
        // process decl
        processDeclarationNode(decl_list);
        decl_list = decl_list->rightSibling;
    }
}

void processDeclarationNode(AST_NODE* declarationNode)
{
    assert(declarationNode->nodeType == DECLARATION_NODE);
    switch(declarationNode->semantic_value.declSemanticValue.kind){
        case VARIABLE_DECL:
            // var_decl
            declareIdList(declarationNode, VARIABLE_ATTRIBUTE, 0);
            break;
        case TYPE_DECL:
            // type_decl
            declareIdList(declarationNode, TYPE_ATTRIBUTE, 0);
            break;
        case FUNCTION_DECL:
            declareFunction(declarationNode);
            break;
        case FUNCTION_PARAMETER_DECL:
            // impossible, we can not see function parameter decl before function decl.
            printf("Crazy AST\n");
            exit(1);
        default:
            printf("Invalid declaration type\n");
    }
}


DATA_TYPE processTypeNode(AST_NODE* idNodeAsType)
{
    char *type = getIdByNode(idNodeAsType);
    SymbolTableEntry* entry = retrieveSymbol(type);
    if(entry){
        if(entry->attribute->attributeKind != TYPE_ATTRIBUTE){
            return ERROR_TYPE;
        }
        else{
            // this is not necessary?
            idNodeAsType->semantic_value.identifierSemanticValue.symbolTableEntry = entry;
            return entry->attribute->attr.typeDescriptor->properties.dataType;
        }
    }
    else{
        return ERROR_TYPE;
    }
}

void declareIdList(AST_NODE* declarationNode, SymbolAttributeKind isVariableOrTypeAttribute, int ignoreArrayFirstDimSize)
{
    if(isVariableOrTypeAttribute == VARIABLE_ATTRIBUTE){
        AST_NODE* type_node = declarationNode->child;
        DATA_TYPE datatype = processTypeNode(type_node);
        if(datatype == ERROR_TYPE){
            printErrorMsg(type_node, SYMBOL_IS_NOT_TYPE);
            return;
        } else if(datatype == VOID_TYPE) {
            printErrorMsg(type_node->rightSibling, VOID_VARIABLE);
        }

        AST_NODE* id_list = type_node->rightSibling; 
        while(id_list){
            AST_NODE *expr_node;
            DATA_TYPE init_type;
            TypeDescriptor *typedesc;
            SymbolAttribute *symbolattr = newAttribute(VARIABLE_ATTRIBUTE);
            switch(id_list->semantic_value.identifierSemanticValue.kind){
                case WITH_INIT_ID:
                    expr_node = id_list->child;
                    init_type = processExprRelatedNode(expr_node);
                    if(init_type == ERROR_TYPE){
                        // error: invalid init value
                        break;
                    }
                    if(datatype == INT_TYPE && init_type == FLOAT_TYPE){
                        // error: can not assign float to int
                        break;
                    }
                    typedesc = newTypeDesc(SCALAR_TYPE_DESCRIPTOR);
                    typedesc->properties.dataType = datatype;
                    symbolattr->attr.typeDescriptor = typedesc;
                    if(enterSymbol(getIdByNode(id_list), symbolattr) == NULL){
                        printErrorMsg(id_list, SYMBOL_REDECLARE);
                        break;
                    }
                    break;
                case NORMAL_ID:
                    typedesc = newTypeDesc(SCALAR_TYPE_DESCRIPTOR);
                    typedesc->properties.dataType = datatype;
                    symbolattr->attr.typeDescriptor = typedesc;
                    if(enterSymbol(getIdByNode(id_list), symbolattr) == NULL){
                        printErrorMsg(id_list, SYMBOL_REDECLARE);
                    }
                    break;
                case ARRAY_ID:
                    // child: dim_decl
                    typedesc = newTypeDesc(ARRAY_TYPE_DESCRIPTOR);
                    typedesc->properties.arrayProperties.elementType = datatype;
                    processDeclDimList(id_list->child, typedesc, 0);

                    symbolattr->attr.typeDescriptor = typedesc;
                    if(enterSymbol(getIdByNode(id_list), symbolattr) == NULL){
                        printErrorMsg(id_list, SYMBOL_REDECLARE);
                    }
                    break;
                default:
                    printf("unexpected kind of variable\n");
                    break;
            }
            id_list = id_list->rightSibling;
        }
    }
    else if(isVariableOrTypeAttribute == TYPE_ATTRIBUTE){
        AST_NODE* type_node = declarationNode->child;
        DATA_TYPE datatype = processTypeNode(type_node);
        if(datatype == ERROR_TYPE){
            printErrorMsg(type_node, SYMBOL_IS_NOT_TYPE);
            return;
        }
        AST_NODE* id_list = type_node->rightSibling;
        while(id_list){
            if(insertType(getIdByNode(id_list), datatype) == NULL){
                printErrorMsg(id_list, SYMBOL_REDECLARE);
            }
            id_list = id_list->rightSibling;
        }
        return;
    }
}

void checkAssignOrExpr(AST_NODE* assignOrExprRelatedNode)
{
    switch(assignOrExprRelatedNode->nodeType) {
        /* only identifier */
        case IDENTIFIER_NODE:
            processVariableLValue(assignOrExprRelatedNode);
            break;
        /* Assignment */
        case STMT_NODE:
            processStmtNode(assignOrExprRelatedNode);
            break;
        /* Relop Expr */
        case EXPR_NODE:
            processExprRelatedNode(assignOrExprRelatedNode);
            break;
    }
}

void checkWhileStmt(AST_NODE* whileNode)
{
    processExprRelatedNode(whileNode->child);
    processStmtNode(whileNode->child->rightSibling);
}


void checkForStmt(AST_NODE* forNode)
{
    AST_NODE *cur = forNode->child;
    AST_NODE *child;

    if(cur->nodeType == NONEMPTY_ASSIGN_EXPR_LIST_NODE) {
        child = cur->child;
        while(child){
            checkAssignOrExpr(child);
            child = child->rightSibling;
        }
    }
    cur = cur->rightSibling;
    if(cur->nodeType == NONEMPTY_RELOP_EXPR_LIST_NODE) {
        child = cur->child;
        while(child){
            checkAssignOrExpr(child);
            child = child->rightSibling;
        }
    }
    cur = cur->rightSibling;
    if(cur->nodeType == NONEMPTY_ASSIGN_EXPR_LIST_NODE) {
        child = cur->child;
        while(child){
            checkAssignOrExpr(child);
            child = child->rightSibling;
        }
    }
    cur = cur->rightSibling;
    processStmtNode(cur);
}


void checkAssignmentStmt(AST_NODE* assignmentNode)
{
}


void checkIfStmt(AST_NODE* ifNode)
{
    AST_NODE *cur;
    cur = ifNode->child;
    processExprRelatedNode(cur);
    cur = cur->rightSibling;
    processStmtNode(cur);
    cur = cur->rightSibling;
    if(cur->nodeType != NUL_NODE){
        processStmtNode(cur);
    }
}

void checkWriteFunction(AST_NODE* functionCallNode)
{
}

DATA_TYPE checkFunctionCall(AST_NODE* functionCallNode)
{
}

void checkParameterPassing(Parameter* formalParameter, AST_NODE* actualParameter)
{
}

DATA_TYPE processExprRelatedNode(AST_NODE* exprRelatedNode)
{
    // exprnode, constnode, stmtnode(function call), idnode(variable reference), nullnode
    // Now I process assign_stmt together because it is more straightforward.
    if(exprRelatedNode->nodeType == EXPR_NODE){
        return processExprNode(exprRelatedNode);
    }
    else if(exprRelatedNode->nodeType == CONST_VALUE_NODE){
        CON_Type *const1 = exprRelatedNode->semantic_value.const1;
        switch(const1->const_type){
            case INTEGERC:
                return INT_TYPE;
            case FLOATC:
                return FLOAT_TYPE;
            case STRINGC:
                return CONST_STRING_TYPE;
        }
    }
    else if(exprRelatedNode->nodeType == STMT_NODE){
        AST_NODE *lvalue_node;
        DATA_TYPE ldatatype, rdatatype;
        switch(exprRelatedNode->semantic_value.stmtSemanticValue.kind){ 
            case FUNCTION_CALL_STMT:
                return checkFunctionCall(exprRelatedNode);
            case ASSIGN_STMT:
                lvalue_node = exprRelatedNode->child;
                ldatatype = processExprRelatedNode(lvalue_node);
                rdatatype = processExprRelatedNode(lvalue_node->rightSibling);
                // what if rdatatype == ERROR_TYPE?
                // any error message?
                return ldatatype;
            default:
                /* TODO */
                return INT_TYPE;
        }
    }
    else if(exprRelatedNode->nodeType == IDENTIFIER_NODE){
        SymbolTableEntry *id_entry = retrieveSymbol(getIdByNode(exprRelatedNode));
        if(id_entry == NULL){
            printErrorMsg(exprRelatedNode, SYMBOL_UNDECLARED);
            return ERROR_TYPE;
        }

        SymbolAttribute *attr = id_entry->attribute;
        if(attr->attributeKind != VARIABLE_ATTRIBUTE){
            // error: symbol is not variable
            return ERROR_TYPE;
        }

        TypeDescriptor *desc = attr->attr.typeDescriptor;
        switch(desc->kind){
            case SCALAR_TYPE_DESCRIPTOR:
                if(exprRelatedNode->semantic_value.identifierSemanticValue.kind == ARRAY_ID) {
                    printErrorMsg(exprRelatedNode, NOT_ARRAY);
                    return ERROR_TYPE;
                }
                return desc->properties.dataType;
            case ARRAY_TYPE_DESCRIPTOR:
                if(exprRelatedNode->semantic_value.identifierSemanticValue.kind != ARRAY_ID) {
                    // error: not array
                    printErrorMsg(exprRelatedNode, INCOMPATIBLE_ARRAY_DIMENSION);
                    return ERROR_TYPE;
                }
                if(processDeclDimList(exprRelatedNode->child, NULL, 0) != desc->properties.arrayProperties.dimension) {
                    printErrorMsg(exprRelatedNode, INCOMPATIBLE_ARRAY_DIMENSION);
                    return ERROR_TYPE;
                }
                return desc->properties.arrayProperties.elementType;
        }
    }
    else if(exprRelatedNode->nodeType == NUL_NODE){
        // Maybe we don't need this?
        return VOID_TYPE;
    }
    else{
        printf("Unexpected ExprRelated Node\n");
        return ERROR_TYPE;
    }
    return ERROR_TYPE;
}

DATA_TYPE getExprOrConstValue(AST_NODE* exprOrConstNode, int* iValue, float* fValue)
{
    if(exprOrConstNode->nodeType == CONST_VALUE_NODE){
        switch(exprOrConstNode->semantic_value.const1->const_type){
            case INTEGERC:
                *iValue = exprOrConstNode->semantic_value.const1->const_u.intval;
                return INT_TYPE;
            case FLOATC:
                *fValue = exprOrConstNode->semantic_value.const1->const_u.fval;
                return FLOAT_TYPE;
            default:
                return ERROR_TYPE;
        }
    }
    else if(exprOrConstNode->nodeType == EXPR_NODE &&
            exprOrConstNode->semantic_value.exprSemanticValue.isConstEval == 1){
        switch(exprOrConstNode->dataType){
            case INT_TYPE:
                *iValue = exprOrConstNode->semantic_value.exprSemanticValue.constEvalValue.iValue;
                return INT_TYPE;
            case FLOAT_TYPE:
                *fValue = exprOrConstNode->semantic_value.exprSemanticValue.constEvalValue.fValue;
                return FLOAT_TYPE;
            default:
                return ERROR_TYPE;
        }
    }
    else{
        return ERROR_TYPE;
    }
}

void evaluateExprValue(AST_NODE* exprNode, DATA_TYPE datatype)
{
    if(exprNode->semantic_value.exprSemanticValue.kind == BINARY_OPERATION){
        AST_NODE* lexpr = exprNode->child;
        AST_NODE* rexpr = lexpr->rightSibling;
        
        int l_int, r_int;
        float l_float, r_float;

        DATA_TYPE l_datatype = getExprOrConstValue(lexpr, &l_int, &l_float);
        DATA_TYPE r_datatype = getExprOrConstValue(rexpr, &r_int, &r_float);
        if(l_datatype == ERROR_TYPE || r_datatype == ERROR_TYPE){
            // error message?
            return;
        }
        if(datatype == INT_TYPE){
            assert(l_datatype == INT_TYPE);
            assert(r_datatype == INT_TYPE);
            int val;
            switch(exprNode->semantic_value.exprSemanticValue.op.binaryOp){
                case BINARY_OP_ADD:
                    val = l_int + r_int;
                    break;
                case BINARY_OP_SUB:
                    val = l_int - r_int;
                    break;
                case BINARY_OP_MUL:
                    val = l_int * r_int;
                    break;
                case BINARY_OP_DIV:
                    val = l_int / r_int;
                    break;
                case BINARY_OP_EQ:
                    val = (l_int == r_int);
                    break;
                case BINARY_OP_GE:
                    val = (l_int >= r_int);
                    break;
                case BINARY_OP_LE:
                    val = (l_int <= r_int);
                    break;
                case BINARY_OP_NE:
                    val = (l_int != r_int);
                    break;
                case BINARY_OP_GT:
                    val = (l_int > r_int);
                    break;
                case BINARY_OP_LT:
                    val = (l_int < r_int);
                    break;
                case BINARY_OP_AND:
                    val = (l_int && r_int);
                    break;
                case BINARY_OP_OR:
                    val = (l_int || r_int);
                    break;
            }
            exprNode->semantic_value.exprSemanticValue.isConstEval = 1;
            exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = val;
        }
        else if(datatype == FLOAT_TYPE){
            if(l_datatype == INT_TYPE){
                l_float = (float)l_int;
            }
            if(r_datatype == INT_TYPE){
                r_float = (float)r_int;
            }
            float val;
            switch(exprNode->semantic_value.exprSemanticValue.op.binaryOp) {
                case BINARY_OP_ADD:
                    val = l_float + r_float;
                    break;
                case BINARY_OP_SUB:
                    val = l_float - r_float;
                    break;
                case BINARY_OP_MUL:
                    val = l_float * r_float;
                    break;
                case BINARY_OP_DIV:
                    val = l_float / r_float;
                    break;
                case BINARY_OP_EQ:
                    val = (l_float == r_float);
                    break;
                case BINARY_OP_GE:
                    val = (l_float >= r_float);
                    break;
                case BINARY_OP_LE:
                    val = (l_float <= r_float);
                    break;
                case BINARY_OP_NE:
                    val = (l_float != r_float);
                    break;
                case BINARY_OP_GT:
                    val = (l_float > r_float);
                    break;
                case BINARY_OP_LT:
                    val = (l_float < r_float);
                    break;
                case BINARY_OP_AND:
                    val = (l_float && r_float);
                    break;
                case BINARY_OP_OR:
                    val = (l_float || r_float);
                    break;
            }
            exprNode->semantic_value.exprSemanticValue.isConstEval = 1;
            exprNode->semantic_value.exprSemanticValue.constEvalValue.fValue = val;
        }
    }
    else if(exprNode->semantic_value.exprSemanticValue.kind == UNARY_OPERATION){
        AST_NODE *lexpr = exprNode->child;
        int l_int;
        float l_float;
        DATA_TYPE l_datatype = getExprOrConstValue(lexpr, &l_int, &l_float);
        if(l_datatype == ERROR_TYPE){
            // error message?
            return;
        }
        if(datatype == INT_TYPE){
            assert(l_datatype == INT_TYPE);
            int val;
            switch(exprNode->semantic_value.exprSemanticValue.op.unaryOp){
                case UNARY_OP_POSITIVE:
                    val = l_int;
                    break;
                case UNARY_OP_NEGATIVE:
                    val = -l_int;
                    break;
                case UNARY_OP_LOGICAL_NEGATION:
                    val = (~l_int);
                    break;
            }
            exprNode->semantic_value.exprSemanticValue.isConstEval = 1;
            exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = val;
        }
        else if(datatype == FLOAT_TYPE){
            if(l_datatype == INT_TYPE){
                l_float = (float)l_int;
            }
            float val;
            switch(exprNode->semantic_value.exprSemanticValue.op.unaryOp){
                case UNARY_OP_POSITIVE:
                    val = l_float;
                    break;
                case UNARY_OP_NEGATIVE:
                    val = -l_float;
                    break;
                case UNARY_OP_LOGICAL_NEGATION:
                    // Error: bit complement on float,
                    // Maybe we need to exit immediately?
                    break;
            }
            exprNode->semantic_value.exprSemanticValue.isConstEval = 1;
            exprNode->semantic_value.exprSemanticValue.constEvalValue.fValue = val;
        }
    }
}


DATA_TYPE processExprNode(AST_NODE* exprNode)
{
    assert(exprNode->nodeType == EXPR_NODE);
    DATA_TYPE datatype, ldatatype, rdatatype;
    switch(exprNode->semantic_value.exprSemanticValue.kind){
        case BINARY_OPERATION:
            ldatatype = processExprRelatedNode(exprNode->child);
            rdatatype = processExprRelatedNode(exprNode->child->rightSibling);
            if(ldatatype != INT_TYPE && ldatatype != FLOAT_TYPE){
                // error: type error
                return ERROR_TYPE;
            }
            if(rdatatype != INT_TYPE && rdatatype != FLOAT_TYPE){
                // error: type error
                return ERROR_TYPE;
            }

            switch(exprNode->semantic_value.exprSemanticValue.op.binaryOp){
                /* ----- expr ----- */
                case BINARY_OP_ADD:
                case BINARY_OP_SUB:
                case BINARY_OP_MUL:
                case BINARY_OP_DIV:
                    datatype = getBiggerType(ldatatype, rdatatype);
                    evaluateExprValue(exprNode, datatype);
                    break;

                /* ----- relop_expr ----- */
                case BINARY_OP_OR:
                case BINARY_OP_AND:
                case BINARY_OP_EQ:
                case BINARY_OP_GE:
                case BINARY_OP_LE:
                case BINARY_OP_NE:
                case BINARY_OP_GT:
                case BINARY_OP_LT:
                    datatype = INT_TYPE;
                    evaluateExprValue(exprNode, datatype);
                    break;
            }
            break;
        case UNARY_OPERATION:
            // negation or logical negation
            datatype = processExprRelatedNode(exprNode->child);
            if(datatype != INT_TYPE && datatype != FLOAT_TYPE){
                // error: invalid type
                datatype = ERROR_TYPE;
                break;
            }
            if(exprNode->semantic_value.exprSemanticValue.op.unaryOp == UNARY_OP_LOGICAL_NEGATION){
                datatype = INT_TYPE;
            }
            evaluateExprValue(exprNode, datatype);
            break;
        default:
            printf("Invalid operation\n");
            exit(1);
    }
    exprNode->dataType = datatype;
    return datatype;
}


void processVariableLValue(AST_NODE* idNode)
{
    SymbolTableEntry *entry = retrieveSymbol(getIdByNode(idNode));
    if(entry == NULL) {
        printErrorMsg(idNode, SYMBOL_UNDECLARED);
    }
}

void processVariableRValue(AST_NODE* idNode)
{
}


void processConstValueNode(AST_NODE* constValueNode)
{
}


void checkReturnStmt(AST_NODE* returnNode)
{
}


void processBlockNode(AST_NODE* blockNode)
{
    assert(blockNode->nodeType == BLOCK_NODE);
    AST_NODE *block = blockNode->child;
    AST_NODE *child;
    while(block){
        switch(block->nodeType){
            case VARIABLE_DECL_LIST_NODE:
                processVariableDeclListNode(block);
                break;
            case STMT_LIST_NODE:
                child = block->child;
                while(child){
                    processStmtNode(child);
                    child = child->rightSibling;
                }
                break;
            default:
                printf("unexpected block type\n");
                break;
        }
        block = block->rightSibling;
    }
}


void processStmtNode(AST_NODE* stmtNode)
{
    switch(stmtNode->nodeType){
        case BLOCK_NODE:
            processBlockNode(stmtNode);
            return;
        case NUL_NODE:
            return;
        case STMT_NODE:
            switch(stmtNode->semantic_value.stmtSemanticValue.kind){
                case WHILE_STMT:
                    checkWhileStmt(stmtNode);
                    break;
                case FOR_STMT:
                    checkForStmt(stmtNode);
                    break;
                case ASSIGN_STMT:
                    processExprRelatedNode(stmtNode);
                    break;
                case IF_STMT:
                    checkIfStmt(stmtNode);
                    break;
                case FUNCTION_CALL_STMT:
                    checkFunctionCall(stmtNode);
                    break;
                case RETURN_STMT:
                    checkReturnStmt(stmtNode);
                    break;
                default:
                    printf("unexpected stmt type\n");
                    break;
            }
            break;
        default:
            printf("unexpected node type\n");
            break;
    }
}


void processGeneralNode(AST_NODE *node)
{
}

int processDeclDimList(AST_NODE* idNode, TypeDescriptor* desc, int ignoreFirstDimSize)
{
    int dim = 0;
    while(idNode){
        if(idNode->nodeType == NUL_NODE){
            if(ignoreFirstDimSize){
                if(desc != NULL){
                    desc->properties.arrayProperties.sizeInEachDimension[dim] = -1;
                }
            }
            else{
                // print error message
            }
        }
        else{
            int size;
            if(idNode->nodeType == CONST_VALUE_NODE){
                if(idNode->semantic_value.const1->const_type != INTEGERC){
                    printErrorMsg(idNode, ARRAY_SIZE_NOT_INT);
                    size = -2;
                }
                else{
                    size = idNode->semantic_value.const1->const_u.intval;
                }
            }
            else if(idNode->nodeType == EXPR_NODE){
                if(processExprRelatedNode(idNode) != INT_TYPE){
                    printErrorMsg(idNode, ARRAY_SIZE_NOT_INT);
                    size = -2;
                }
                else{
                    size = idNode->semantic_value.exprSemanticValue.constEvalValue.iValue;
                }
            }
            if(desc != NULL){
                desc->properties.arrayProperties.sizeInEachDimension[dim] = size;
            }
        }
        dim += 1;
        ignoreFirstDimSize = 0;
        idNode = idNode->rightSibling;
    }
    if(desc != NULL) {
        desc->properties.arrayProperties.dimension = dim;
    }
    return dim;
}


void declareFunction(AST_NODE* declarationNode)
{
    assert(declarationNode->nodeType == DECLARATION_NODE);
    assert(declarationNode->semantic_value.declSemanticValue.kind == FUNCTION_DECL);
    
    AST_NODE *ret_node, *name_node, *param_node, *block_node;
    ret_node = declarationNode->child;
    name_node = ret_node->rightSibling;
    param_node = name_node->rightSibling;
    block_node = param_node->rightSibling;

    FunctionSignature *func = (FunctionSignature*)malloc(sizeof(struct FunctionSignature));
    func->returnType = processTypeNode(ret_node);
    if(func->returnType == ERROR_TYPE){
        printErrorMsg(ret_node, SYMBOL_IS_NOT_TYPE);
        return;
    }

    SymbolAttribute* attr = newAttribute(FUNCTION_SIGNATURE);
    attr->attr.functionSignature = func;
    if(enterSymbol(getIdByNode(name_node), attr) == NULL){
        printErrorMsg(name_node, SYMBOL_REDECLARE);
        return;
    }

    openScope();
    
    func->parametersCount = 0;
    AST_NODE *param_list = param_node->child;
    Parameter *new_param = NULL, *last_param = NULL;
    while(param_list){
        func->parametersCount += 1;
        new_param = (Parameter*)malloc(sizeof(struct Parameter));

        AST_NODE *param_type_node = param_list->child;
        AST_NODE *param_id_node = param_type_node->rightSibling;
        DATA_TYPE param_type = processTypeNode(param_type_node);
        if(param_type == ERROR_TYPE){
            printErrorMsg(param_type_node, SYMBOL_IS_NOT_TYPE);
            // break? continue?
        }
        TypeDescriptor *desc;
        switch(param_id_node->semantic_value.identifierSemanticValue.kind){
            case NORMAL_ID:
                desc = newTypeDesc(SCALAR_TYPE_DESCRIPTOR);
                desc->properties.dataType = param_type;
                attr = newAttribute(VARIABLE_ATTRIBUTE);
                attr->attr.typeDescriptor = desc;
                
                if(enterSymbol(getIdByNode(param_id_node), attr) == NULL){
                  printErrorMsg(param_id_node, SYMBOL_REDECLARE);
                  break ;
                }
                new_param->type = desc;
                new_param->parameterName = getIdByNode(param_id_node);
                break;
            
            case ARRAY_ID:
                desc = newTypeDesc(ARRAY_TYPE_DESCRIPTOR);
                desc->properties.arrayProperties.elementType = param_type;
                processDeclDimList(param_id_node, desc, 1);
                attr = newAttribute(VARIABLE_ATTRIBUTE);
                attr->attr.typeDescriptor = desc;
                
                if(enterSymbol(getIdByNode(param_id_node), attr) == NULL){
                  printErrorMsg(param_id_node, SYMBOL_REDECLARE);
                  break;
                }
                new_param->type = desc;
                new_param->parameterName = getIdByNode(param_id_node);
                break;
            default:
                printf("Unexpected kind of variable\n");
                break;
        }
        new_param->next = NULL;
        if(last_param == NULL){
            func->parameterList = new_param;
        }
        else{
            last_param->next = new_param;
        }
        last_param = new_param;
        
        param_list = param_list->rightSibling;
    }
    processBlockNode(block_node);
    closeScope();
    return;
}

void dumpNode(AST_NODE *node) {
  printf("====START====\n");
  switch(node->nodeType) {
  case EXPR_NODE:
      dumpExprNode(node);
      break;
  case CONST_VALUE_NODE:
      dumpConstNode(node);
      break;
  case STMT_NODE:
      dumpStmtNode(node);
      break;
  case IDENTIFIER_NODE:
      printf("Id Node\n");
      break;
  case NUL_NODE:
      printf("Nul Node\n");
      break;
  case PROGRAM_NODE:
      printf("Program Node\n");
      break;
  case DECLARATION_NODE:
      printf("Declaration Node\n");
      break;
  case BLOCK_NODE:
      printf("Block Node\n");
      break;
  case NONEMPTY_ASSIGN_EXPR_LIST_NODE:
      printf("Nonempty Assign Expr List Node\n");
      break;
  case NONEMPTY_RELOP_EXPR_LIST_NODE:
      printf("Nonempty Relop Expr List Node\n");
      break;
  default:
      break;
  }
  printf("=====END=====\n");
}
void dumpExprNode(AST_NODE *node) {
    printf("Expr Node\n");

    EXPRSemanticValue expr = node->semantic_value.exprSemanticValue;
    printf("Operation Kind : %s\n", expr.kind == BINARY_OPERATION ? "Binary" : "Unary");
    switch(expr.op.binaryOp) {
        case BINARY_OP_ADD:
            printf("Operator : +\n");
            break;
        case BINARY_OP_MUL:
            printf("Operator : *\n");
            break;
        case BINARY_OP_SUB:
            printf("Operator : -\n");
            break;
        case BINARY_OP_DIV:
            printf("Operator : /\n");
            break;
        default:
            break;
    }
    switch(node->dataType) {
        case INT_TYPE:
            printf("Type : INT_TYPE\n");
            if(expr.isConstEval) {
                printf("Value : %d\n",expr.constEvalValue.iValue);
            }
            break;
        case FLOAT_TYPE:
            printf("Type : FLOAT_TYPE\n");
            if(expr.isConstEval) {
                printf("Value : %lf\n",expr.constEvalValue.fValue);
            }
            break;
        case VOID_TYPE:
            printf("Type : VOID_TYPE\n");
            break;
        case INT_PTR_TYPE:
            printf("Type : INT_PTR_TYPE\n");
            break;
        case FLOAT_PTR_TYPE:
            printf("Type : FLOAT_PTR_TYPE\n");
            break;
        case CONST_STRING_TYPE:
            printf("Type : CONST_STRING_TYPE\n");
            break;
        case NONE_TYPE:
            printf("Type : NONE_TYPE\n");
            break;
        case ERROR_TYPE:
            printf("Type : ERROR_TYPE\n");
            break;
    }
}


void dumpConstNode(AST_NODE *node) {
    printf("Const Node\n");
    CON_Type *con = node->semantic_value.const1;
    switch(con->const_type) {
        case INTEGERC:
            printf("Type : INTEGERC\n");
            printf("Value : %d\n", con->const_u.intval);
            break;
        case FLOATC:
            printf("Type : FLOATC\n");
            printf("Value : %lf\n", con->const_u.fval);
            break;
        case STRINGC:
            printf("Type : STRINGC\n");
            printf("Value : %s\n", con->const_u.sc);
            break;
        default:
            break;
    }
}

void dumpStmtNode(AST_NODE *node) {
    printf("Stmt Node\n");
    STMTSemanticValue stmt = node->semantic_value.stmtSemanticValue;
    switch(stmt.kind) {
        case ASSIGN_STMT:
            printf("Kind : ASSIGN_STMT\n");
        default: break;
    }
}
