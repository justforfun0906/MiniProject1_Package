#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codeGen.h"
int stack_top = 0;
int memory_queue_back = 252;
int check_hasID(BTNode *root){
    if(root->data==ID)return 1;
    if(root->left!=NULL){
        if(check_hasID(root->left))return 1;
    }
    if(root->right!=NULL){
        if(check_hasID(root->right))return 1;
    }
    return 0;
}
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
                    if(rv==0&&check_hasID(root->right)==0){
                        error(DIVZERO);
                    }else if(rv==0){
                        retval = 1;
                    }else{
                        retval = lv / rv;
                    }
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
int get_depth(BTNode *root){
    if(root==NULL)return 0;
    int l = get_depth(root->left);
    int r = get_depth(root->right);
    return (l>r?l:r)+1;
}
//FIXME: error occur input = ++b
void print_allocate(BTNode *root){
    if(root->data==ID){
        printf("MOV r%d, [%d]\n", stack_top, getpos(root->lexeme));
    }else if(root->data==INT){
        printf("MOV r%d, %s\n", stack_top, root->lexeme);
    }
    else{
        error(SYNTAXERR);
    }
    root->reg = stack_top++;
}
void print_assign(BTNode *root){
    if(root->left->data!=ID){
        error(SYNTAXERR);
    }
    printAssemble(root->right);
    printf("MOV [%d], r%d\n", getpos(root->left->lexeme), root->right->reg);
    root->reg = root->right->reg;
}
void print_Arith(BTNode* root){
    //TODO: think about the situation that some of the reg are put in the memory
    //in such case, which reg should be set as the former one?
    //also, how should we ensure the set is still correct?
    if(get_depth(root->left)>=get_depth(root->right)){
        printAssemble(root->left);
        printAssemble(root->right);
    }else{
        printAssemble(root->right);
        printAssemble(root->left);
    }
    int former_reg, latter_reg;
    if(root->left->reg > root->right->reg){//right reg is smaller
        former_reg = root->right->reg;
        latter_reg = root->left->reg;
    }else{//left reg is smaller
        former_reg = root->left->reg;
        latter_reg = root->right->reg;
    }
    switch(root->lexeme[0]){
        case '+':
            printf("ADD r%d r%d\n", former_reg, latter_reg);
            break;
        case '-':
            printf("SUB r%d r%d\n", former_reg, latter_reg);
            break;
        case '*':
            printf("MUL r%d r%d\n", former_reg, latter_reg);
            break;
        case '/':
            printf("DIV r%d r%d\n", former_reg, latter_reg);
            break;
        case '|':
            printf("OR r%d r%d\n", former_reg, latter_reg);
            break;
        case '^':
            printf("XOR r%d r%d\n", former_reg, latter_reg);
            break;
        case '&':
            printf("AND r%d r%d\n", former_reg, latter_reg);
            break;
        default:
            error(SYNTAXERR);
    }
    root->reg = former_reg;
    stack_top--;
}
//adapt to new framework
void printAssemble(BTNode *root) {
    if(root==NULL)return;
    if(root->data == ID||root->data == INT){
        print_allocate(root);
    }else if(root->data == ASSIGN){
        print_assign(root);
    }else{
        print_Arith(root);
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
