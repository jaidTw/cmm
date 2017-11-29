#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"
#include "symbolTable.h"
// This file is for reference only, you are not required to follow the implementation. //
// You only need to check for errors stated in the hw4 assignment document. //
int g_anyErrorOccur = 0;

DATA_TYPE getBiggerType(DATA_TYPE dataType1, DATA_TYPE dataType2);
void processProgramNode(AST_NODE *programNode);
void processDeclarationNode(AST_NODE* declarationNode);
void declareIdList(AST_NODE* typeNode, SymbolAttributeKind isVariableOrTypeAttribute, int ignoreArrayFirstDimSize);
void declareFunction(AST_NODE* returnTypeNode);
void processDeclDimList(AST_NODE* variableDeclDimList, TypeDescriptor* desc, int ignoreFirstDimSize);
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
void checkFunctionCall(AST_NODE* functionCallNode);
DATA_TYPE processExprRelatedNode(AST_NODE* exprRelatedNode);
void checkParameterPassing(Parameter* formalParameter, AST_NODE* actualParameter);
void checkReturnStmt(AST_NODE* returnNode);
void processExprNode(AST_NODE* exprNode);
void processVariableLValue(AST_NODE* idNode);
void processVariableRValue(AST_NODE* idNode);
void processConstValueNode(AST_NODE* constValueNode);
void getExprOrConstValue(AST_NODE* exprOrConstNode, int* iValue, float* fValue);
void evaluateExprValue(AST_NODE* exprNode);

void processVariableDeclListNode(AST_NODE* decl_list);

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

void printErrorMsgSpecial(AST_NODE* node1, char* name2, ErrorMsgKind errorMsgKind)
{
    g_anyErrorOccur = 1;
    printf("Error found in line %d\n", node1->linenumber);
    /*
    switch(errorMsgKind)
    {
    default:
        printf("Unhandled case in void printErrorMsg(AST_NODE* node, ERROR_MSG_KIND* errorMsgKind)\n");
        break;
    }
    */
}


void printErrorMsg(AST_NODE* node, ErrorMsgKind errorMsgKind)
{
    g_anyErrorOccur = 1;
    printf("Error found in line %d\n", node->linenumber);
    /*
    switch(errorMsgKind)
    {
        printf("Unhandled case in void printErrorMsg(AST_NODE* node, ERROR_MSG_KIND* errorMsgKind)\n");
        break;
    }
    */
}

const char* idName(AST_NODE *node){
    // just return the variable name
    return node->semantic_value.identifierSemanticValue.identifierName;
}

void semanticAnalysis(AST_NODE *root)
{
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
            declareFunction(declarationNode)
        case FUNCTION_PARAMETER_DECL:
        default:
            printf("Invalid declaration type\n");
            exit(1)
    }
}


DATA_TYPE processTypeNode(AST_NODE* idNodeAsType)
{
    char *type = idName(idNodeAsType);
    SymbolTableEntry* entry = retrieveSymbol(type);
    DATA_TYPE type = ERROR_TYPE;
    if(entry){
        if(entry->attribute->attributeKind != TYPE_ATTRIBUTE){
            printErrorMsg(idNodeAsType, SYMBOL_IS_NOT_TYPE);
        }
        else{
            // this is not necessary?
            idNodeAsType->semantic_value.identifierSemanticValue.symbolTableEntry = entry;
            type = entry->attribute->attr.typeDescriptor->properties.dataType;
        }
    }
    else{
        printErrorMsg(idNodeAsType, SYMBOL_IS_NOT_TYPE);
    }
    return type;
}

void declareIdList(AST_NODE* declarationNode, SymbolAttributeKind isVariableOrTypeAttribute, int ignoreArrayFirstDimSize)
{
    switch(isVariableOrTypeAttribute){
        case VARIABLE_ATTRIBUTE:
            // process var decl
            AST_NODE* idNode = declarationNode->child; // the first child is type
            DATA_TYPE datatype = processTypeNode(idNode);

            AST_NODE* id_list = idNode->rightSibling; 
            while(id_list){
                SymbolAttribute *symbolattr = newAttribute(VARIABLE_ATTRIBUTE);
                switch(id_list->semantic_value.identifierSemanticValue.kind){
                    case NORMAL_ID:
                        TypeDescriptor* typedesc = newTypeDesc(SCALAR_TYPE_DESCRIPTOR);
                        typedesc->properties.dataType = datatype;
                        symbolattr->attr.typeDescriptor = typedesc;
                        if(enterSymbol(idName(id_list), symbolattr) == NULL){
                            printErrorMsg(id_list, SYMBOL_REDECLARE);
                        }
                        break;
                    case ARRAY_ID:
                        // child: dim_decl
                        TypeDescriptor* typedesc = newTypeDesc(ARRAY_TYPE_DESCRIPTOR);
                        typedesc->properties.arrayProperties.elementType = datatype;
                        processDeclDimList(id_list->child, typedesc, 0);
                        int i = 0;
                        AST_NODE *dim_decl = id_list->child;
                        while(dim_decl){
                            // process cexpr i.e const expression
                            process
                            typedesc->properties.arrayProperties.;
                            dim_decl = dim_decl->rightSibling;
                        }
                        break;
                    case WITH_INIT_ID:
                }
            }
        case TYPE_ATTRIBUTE:
    }
}

void checkAssignOrExpr(AST_NODE* assignOrExprRelatedNode)
{
}

void checkWhileStmt(AST_NODE* whileNode)
{
}


void checkForStmt(AST_NODE* forNode)
{
}


void checkAssignmentStmt(AST_NODE* assignmentNode)
{
}


void checkIfStmt(AST_NODE* ifNode)
{
}

void checkWriteFunction(AST_NODE* functionCallNode)
{
}

void checkFunctionCall(AST_NODE* functionCallNode)
{
}

void checkParameterPassing(Parameter* formalParameter, AST_NODE* actualParameter)
{
}

DATA_TYPE processExprRelatedNode(AST_NODE* exprRelatedNode)
{
    // exprnode, constnode, stmtnode(function call), idnode(variable reference)
    switch(exprRelatedNode->nodeType){
        case EXPR_NODE:
            processExprNode(exprRelatedNode);
            return ex;
        case CONST_VALUE_NODE:
            CON_Type *const1 = exprRelatedNode->semantic_value.const1;
            switch(const1->const_type){
                case INTEGERC:
                    return INT_TYPE;
                case FLOATC:
                    return FLOAT_TYPE;
                case STRINGC:
                    return CONST_STRING_TYPE;
            }
        case STMT_NODE:
        case IDENTIFIER_NODE:
            SymbolTableEntry *id_entry = retrieveSymbol(idName(exprRelatedNode));
            if(id_entry == NULL){
                printErrorMsg(exprRelatedNode, SYMBOL_UNDECLARED);
                return ERROR_TYPE;
            }

            SymbolAttribute *attr = id_entry->attribute;
            if(attr->attributeKind != VARIABLE_ATTRIBUTE){
                // print error message but I can't find 'symbol is not variable'
                return ERROR_TYPE;
            }

            TypeDescriptor *desc = attr->attr.typeDescriptor;
            switch(desc->kind){
                case SCALAR_TYPE_DESCRIPTOR:
                    if(exprRelatedNode->semantic_value.identifierSemanticValue.kind != NORMAL_ID) {
                       // print some error message
                       return ERROR_TYPE;
                    }
                    return desc->properties.dataType;
                case ARRAY_TYPE_DESCRIPTOR:
                    if(exprRelatedNode->semantic_value.identifierSemanticValue.kind != ARRAY_ID) {
                        // print some error message
                        return ERROR_TYPE;
                    }
                    // TODO: check array dimension, may be something like
                    if(processDeclDimList(assignOrExprRelatedNode, NULL, 0) != desc->arrayProperties.dimension) {
                    }
                    return desc->properties.arrayProperties.elementType;
            }


    }
    return exprRelatedNode->dataType;
}

void getExprOrConstValue(AST_NODE* exprOrConstNode, int* iValue, float* fValue)
{
}

void evaluateExprValue(AST_NODE* exprNode)
{
}


void processExprNode(AST_NODE* exprNode)
{
    assert(exprNode->nodeType == EXPR_NODE);
    exprNode->semantic_value.exprSemanticValue.isConstEval
    switch(exprNode->semantic_value.exprSemanticValue.kind){
        case BINARY_OPERATION:
            l_datatype = processExprRelatedNode(exprNode->child);
            r_datatype = processExprRelatedNode(exprNode->child->rightSibling);
            switch(exprNode->semantic_value.exprSemanticValue.op.binaryOp){
                /* ----- expr ----- */
                case BINARY_OP_ADD:
                // child: expr, term
                case BINARY_OP_SUB:
                // child: expr, term
                case BINARY_OP_MUL:
                // child: term, factor
                case BINARY_OP_DIV:
                // child: term, factor

                /* ----- relop_expr ----- */
                case BINARY_OP_OR:
                // child: relop_expr, relop_term
                case BINARY_OP_AND:
                // child: relop_term, relop_factor
                
                // child: expr, expr
                case BINARY_OP_EQ:
                case BINARY_OP_GE:
                case BINARY_OP_LE:
                case BINARY_OP_NE:
                case BINARY_OP_GT:
                case BINARY_OP_LT:


            }
        case UNARY_OPERATION:
            switch(exprNode->semantic_value.exprSemanticValue.op.unaryOp){
                case UNARY_OP_NEGATIVE:
                // child: relop_expr | CONST | FUNCTION_CALL | var_ref
                case UNARY_OP_LOGICAL_NEGATION:
                // child: relop_expr | CONST | FUNCTION_CALL | var_ref
            }
    }
}


void processVariableLValue(AST_NODE* idNode)
{
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
}


void processStmtNode(AST_NODE* stmtNode)
{
}


void processGeneralNode(AST_NODE *node)
{
}

void processDeclDimList(AST_NODE* idNode, TypeDescriptor* desc, int ignoreFirstDimSize)
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
                    printErrorMsg(idNode, ARRAY_SUBSCRIPT_NOT_INT);
                    size = -2;
                }
                else{
                    size = idNode->semantic_value.const1->const_u.intval;
                }
            }
            else if(idNode->nodeType == EXPR_NODE){
                if(processExprRelatedNode(idNode) != INT_TYPE){
                    printErrorMsg(idNode, ARRAY_SUBSCRIPT_NOT_INT);
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

}


void declareFunction(AST_NODE* declarationNode)
{
}
