#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "header.h"
#include "symbolTable.h"

enum MODE_TYPE {TextType, DataType};

FILE *output;

void genPrologue();
void genEpilogue();

static __inline__ char* getIdByNode(AST_NODE *node) {
    return node->semantic_value.identifierSemanticValue.identifierName;
}

void genGlobalDeclration(AST_NODE *decl){
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

void genFunctionDeclration(AST_NODE *func_decl){
    
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

void codeGeneration(AST_NODE *root) {
    output = fopen("output.s", "w");
    genProgramNode(root);
    fclose(output);
}
