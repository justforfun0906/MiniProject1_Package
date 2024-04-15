#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codeGen.h"
int stack_top = 0;
int evaluateTree(BTNode *root) {
    int retval = 0, lv = 0, rv = 0;

    if (root != NULL) {
        switch (root->data) {
            case ID:
                retval = getval(root->lexeme);
                break;
            case INT:
                retval = atoi(root->lexeme);
                break;
            case ASSIGN:
                rv = evaluateTree(root->right);
                retval = setval(root->left->lexeme, rv);
                break;
            case ADDSUB:
                if (root->left != NULL) {
                    lv = evaluateTree(root->left);
                }
                if (root->right != NULL) {
                    rv = evaluateTree(root->right);
                }
                if (strcmp(root->lexeme, "+") == 0) {
                    retval = lv + rv;
                } else if (strcmp(root->lexeme, "-") == 0) {
                    retval = lv - rv;
                }
                break;
            case MULDIV:
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                if (strcmp(root->lexeme, "+") == 0) {
                    retval = lv + rv;
                } else if (strcmp(root->lexeme, "-") == 0) {
                    retval = lv - rv;
                } else if (strcmp(root->lexeme, "*") == 0) {
                    retval = lv * rv;
                } else if (strcmp(root->lexeme, "/") == 0) {
                    if (rv == 0)
                        error(DIVZERO);
                    retval = lv / rv;
                }
                break;
            case INCDEC:
                if (root->left != NULL) {
                    lv = evaluateTree(root->left);
                }
                if (strcmp(root->lexeme, "++") == 0) {
                    retval = lv + 1;
                } else if (strcmp(root->lexeme, "--") == 0) {
                    retval = lv - 1;
                }
                break;
            case ADDSUB_ASSIGN:
                rv = evaluateTree(root->right);
                lv = getval(root->left->lexeme);
                if (strcmp(root->lexeme, "+=") == 0) {
                    retval = lv + rv;
                } else if (strcmp(root->lexeme, "-=") == 0) {
                    retval = lv - rv;
                }
                setval(root->left->lexeme, retval);
                break;
            case AND:
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                retval = lv & rv;
                break;
            case OR:
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                retval = lv | rv;
                break;
            case XOR:
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                retval = lv ^ rv;
                break;
            default:
                retval = 0;
        }
    }
    return retval;
}
void printCode(BTNode *root){
    if(root->data==INT){
        printf("MOV r%d %d\n", stack_top, atoi(root->lexeme));
        //fprintf(stdout, "MOV r%d %d\n", stack_top, atoi(root->lexeme));
    }
    if(root->data==ID){
        int pos = 0;
        getval(root->lexeme);
        pos = getpos(root->lexeme);
        printf("MOV r%d [%d]\n", stack_top, pos);
        //fprintf(stdout, "MOV r%d [%d]\n", stack_top, pos);
    }
    if(root->data==INCDEC){
        int pos = 0;
        pos = getpos(root->left->lexeme);
        printf("MOV r%d [%d]\n", stack_top++, pos);
        if(strcmp(root->lexeme, "++")==0){
            printf("MOV r%d 1\n", stack_top);
            printf("ADD r%d r%d\n", stack_top-1, stack_top);
            //fprintf(stdout, "ADD r%d 1\n", stack_top);
        }else if(strcmp(root->lexeme, "--")==0){
            printf("MOV r%d 1\n", stack_top);
            printf("SUB r%d r%d\n", stack_top-1, stack_top);
            //fprintf(stdout, "SUB r%d 1\n", stack_top);
        }
        printf("MOV [%d] r%d\n", pos, stack_top-1);
        stack_top--;
    }
    if(root->data==ADDSUB){//TODO: think about how to implement this
        if(strcmp(root->lexeme, "+")==0){
            printf("ADD r%d r%d\n", stack_top-2, stack_top-1);
            //fprintf(stdout, "ADD r%d r%d\n", stack_top-2, stack_top-1);
        }else if(strcmp(root->lexeme, "-")==0){
            printf("SUB r%d r%d\n", stack_top-2, stack_top-1);
            //fprintf(stdout, "SUB r%d r%d\n", stack_top-2, stack_top-1);
        }
        stack_top--;
    }if(root->data==MULDIV){
        if(strcmp(root->lexeme, "*")==0){
            printf("MUL r%d r%d\n", stack_top-2, stack_top-1);
            //fprintf(stdout, "MUL r%d r%d\n", stack_top-2, stack_top-1);
        }else if(strcmp(root->lexeme, "/")==0){
            printf("DIV r%d r%d\n", stack_top-2, stack_top-1);
            //fprintf(stdout, "DIV r%d r%d\n", stack_top-2, stack_top-1);
        }
        stack_top--;
    }if(root->data==OR){
        printf("OR r%d r%d\n", stack_top-2, stack_top-1);
        //fprintf(stdout, "OR r%d r%d\n", stack_top-2, stack_top-1);
        stack_top--;
    }if(root->data==AND){
        printf("AND r%d r%d\n", stack_top-2, stack_top-1);
        //fprintf(stdout, "AND r%d r%d\n", stack_top-2, stack_top-1);
        stack_top--;
    }if(root->data==XOR){
        printf("XOR r%d r%d\n", stack_top-2, stack_top-1);
        //fprintf(stdout, "XOR r%d r%d\n", stack_top-2, stack_top-1);
        stack_top--;
    }
}
//TODO: check where the stack_top is wrong(overly decreased)
//TODO: check the ADDSUB_ASSIGN and INCDEC (no more ADDSUB_ASSIGN)
//FIXME: INCDEC doesn't make the value stored in the memory
void printAssemble(BTNode *root) {
    if (root->left != NULL && root->right!=NULL) {
        if(root->data==ASSIGN){
            printAssemble(root->right);
            getval(root->left->lexeme);
            int pos = 0;
            pos = getpos(root->left->lexeme);
            /*for(int i=0;i<sbcount;i++){
                if(strcmp(table[i].name, root->left->lexeme)==0){
                    pos = i*4;
                    break;
                }
            }*/
            printf("MOV [%d] r%d\n", pos, stack_top-1);
            //fprintf(stdout, "MOV r%d [%d]\n", pos, stack_top-1);
            //stack_top--;
        }
        else{
            printAssemble(root->left);
            printAssemble(root->right);
            printCode(root);
        }
    }else{//leaf node
        printCode(root);
        stack_top++;
    }
    return;
}

void printPrefix(BTNode *root) {
    if (root != NULL) {
        printf("%s ", root->lexeme);
        printPrefix(root->left);
        printPrefix(root->right);
    }
}
void printPostfix(BTNode *root) {
    if (root != NULL) {
        printPostfix(root->left);
        printPostfix(root->right);
        printf("%s ", root->lexeme);
    }
}
