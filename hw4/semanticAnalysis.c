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
void processDeclDimList(AST_NODE* variableDeclDimList, TypeDescriptor* typeDescriptor, int ignoreFirstDimSize);
void processTypeNode(AST_NODE* typeNode);
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
void processExprRelatedNode(AST_NODE* exprRelatedNode);
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
    if(programNode->child){
        switch(programNode->child.nodeType){
            case VARIABLE_DECL_LIST_NODE:
                processVariableDeclListNode(programNode->child);
            case DECLARATION_NODE:
                processDeclarationNode(programNode->child);
                break;
            default:
                printf("Invalid program\n");
                exit(1);
        }
    }
}

void processVariableDeclListNode(AST_NODE *decl_list){
    AST_NODE *decl = decl_list->child;
    while(decl){
        processDeclarationNode(decl);
        decl = decl->rightSibling;
    }
}

void processDeclarationNode(AST_NODE* declarationNode)
{
    switch(declarationNode->semantic_value.declSemanticValue.kind){
        case VARIABLE_DECL:
            declareIdList(declarationNode, VARIABLE_ATTRIBUTE, 0);
            break;
        case TYPE_DECL:
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


void processTypeNode(AST_NODE* idNodeAsType)
{
}

SymbolAttribute* newAttribute(SymbolAttributeKind attributeKind)
{
    SymbolAttribute *temp = (SymbolAttribute*)malloc(sizeof(struct SymbolAttribute));
    temp->attributeKind = attributeKind;
    return temp;
}

TypeDescriptor* newTypeDesc()

void declareIdList(AST_NODE* declarationNode, SymbolAttributeKind isVariableOrTypeAttribute, int ignoreArrayFirstDimSize)
{
    switch(isVariableOrTypeAttribute){
        case VARIABLE_ATTRIBUTE:
            AST_NODE* idNode = declarationNode->child;
            AST_NODE* id_list = idNode->rightSibling;
            while(id_list){
                char *symbolName = id_list->semantic_value.identifierSemanticValue.identifierName;
                SymbolAttribute *symbolattr = newAttribute(VARIABLE_ATTRIBUTE);
                TypeDescriptor *typedescriptor = 
                switch(id_list->semantic_value.identifierSemanticValue.kind){
                    case NORMAL_ID:
                        enterSymbol(symbolName)
                    case ARRAY_ID:
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


void processExprRelatedNode(AST_NODE* exprRelatedNode)
{
}

void getExprOrConstValue(AST_NODE* exprOrConstNode, int* iValue, float* fValue)
{
}

void evaluateExprValue(AST_NODE* exprNode)
{
}


void processExprNode(AST_NODE* exprNode)
{
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

void processDeclDimList(AST_NODE* idNode, TypeDescriptor* typeDescriptor, int ignoreFirstDimSize)
{
}


void declareFunction(AST_NODE* declarationNode)
{
}
