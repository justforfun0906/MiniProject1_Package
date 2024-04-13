#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codeGen.h"

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

void printPrefix(BTNode *root) {
    if (root != NULL) {
        printf("%s ", root->lexeme);
        printPrefix(root->left);
        printPrefix(root->right);
    }
}
