#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "codeGen.h"

Symbol table[TBLSIZE];
int sbcount = 0;
void initTable(void) {
    strcpy(table[0].name, "x");
    table[0].val = 0;
    strcpy(table[1].name, "y");
    table[1].val = 0;
    strcpy(table[2].name, "z");
    table[2].val = 0;
    sbcount = 3;
}
int getpos(char *str){
    int ans = -1;
    for(int i=0;i<sbcount;i++){
        if(strcmp(table[i].name, str)==0){
            ans = i*4;
            break;
        }
    }
    return ans;
}

int getval(char *str) {
    int i = 0;

    for (i = 0; i < sbcount; i++)
        if (strcmp(str, table[i].name) == 0)
            return table[i].val;

    if (sbcount >= TBLSIZE)
        error(RUNOUT);
    
    strcpy(table[sbcount].name, str);
    table[sbcount].val = 0;
    sbcount++;
    return 0;
}

int setval(char *str, int val) {
    int i = 0;

    for (i = 0; i < sbcount; i++) {
        if (strcmp(str, table[i].name) == 0) {
            table[i].val = val;
            return val;
        }
    }

    if (sbcount >= TBLSIZE)
        error(RUNOUT);
    
    strcpy(table[sbcount].name, str);
    table[sbcount].val = val;
    sbcount++;
    return val;
}

BTNode *makeNode(TokenSet tok, const char *lexe) {
    BTNode *node = (BTNode*)malloc(sizeof(BTNode));
    strcpy(node->lexeme, lexe);
    node->data = tok;
    node->val = 0;
    node->left = NULL;
    node->right = NULL;
    return node;
}

void freeTree(BTNode *root) {
    if (root != NULL) {
        freeTree(root->left);
        freeTree(root->right);
        free(root);
    }
}

// statement := ENDFILE | END | expr END
void statement(void) {
    BTNode *retp = NULL;
    if (match(ENDFILE)) {
        printf("MOV r0 [0]\nMOV r1 [4]\nMOV r2 [8]\nEXIT 0\n");
        exit(0);
    } else if (match(END)) {
        //printf(">> ");
        advance();
    } else {
        retp = assign_expr();
        if (match(END)) {
            /*printf("%d\n", evaluateTree(retp));
            printf("Prefix traversal: ");
            printPrefix(retp);
            printf("\n");
            printf("Postfix traversal: ");
            printPostfix(retp);
            printf("\n");
            printf("Assembly code: \n");
            */
            printAssemble(retp);
            freeTree(retp);
            //printf(">> ");
            stack_top --;
            advance();
        } else {
            error(SYNTAXERR);
        }
    }
}

// assign_expr := ID ASSIGN assign_expr | ID ADDSUB_ASSIGN assign_expr | or_expr
// possible error(handled): left side of assignment must be a variable
BTNode *assign_expr(void){
    BTNode *retp = NULL, *left = NULL;
    left = or_expr();
    // don't need to call advance() here because or_expr() will call advance() in the end
    // Since we have a practice that for every "makeNode" function, 
    // we call advance() after it,
    // to make sure the currToken is new and ready for the next "makeNode".
    if(match(ASSIGN)){
        if(left->data != ID){
            error(NOTLVAL);
        }else{
            retp = makeNode(ASSIGN, getLexeme());
            advance();
            retp->left = left;
            retp->right = assign_expr();
        }
    }else if(match(ADDSUB_ASSIGN)){
        if(left->data != ID){
            error(NOTLVAL);
        }else{
            char* str = getLexeme();
            retp = makeNode(ASSIGN, "=");
            retp->left = left;
            if(str[0]=='+')retp->right = makeNode(ADDSUB, "+");
            else retp->right = makeNode(ADDSUB, "-");
            BTNode* temp_left = makeNode(ID, left->lexeme);
            retp->right->left = temp_left;
            advance();
            retp->right->right = assign_expr();
        }
    }else{
        retp = left;
    }
    return retp;
}

// or_expr := xor_expr or_expr_tail
BTNode *or_expr(void){
    BTNode *node = xor_expr();
    return or_expr_tail(node);
}

// or_expr_tail := OR xor_expr or_expr_tail | NiL
BTNode *or_expr_tail(BTNode *left){
    BTNode *node = NULL;
    if(match(OR)){
        node = makeNode(OR, getLexeme());
        advance();
        node -> left = left;
        node -> right = xor_expr();
        return or_expr_tail(node);
    }else{
        return left;
    }
}
// xor_expr := and_expr xor_expr_tail
BTNode *xor_expr(void){
    BTNode *node = and_expr();
    return xor_expr_tail(node);
}
// xor_expr_tail := XOR and_expr xor_expr_tail | NiL
BTNode *xor_expr_tail(BTNode *left){
    BTNode *node = NULL;
    if(match(XOR)){
        node = makeNode(XOR, getLexeme());
        advance();
        node -> left = left;
        node -> right = and_expr();
        return xor_expr_tail(node);
    }else{
        return left;
    }
}
// and_expr() = addsub_expr and_expr_tail
BTNode *and_expr(void){
    BTNode *node = addsub_expr();
    return and_expr_tail(node);
}
// and_expr_tail() = AND addsub_expr and_expr_tail | NiL
BTNode *and_expr_tail(BTNode* left){
    BTNode *node = NULL;
    if(match(AND)){
        node = makeNode(AND, getLexeme());
        advance();
        node -> left = left;
        node -> right = addsub_expr();
        return and_expr_tail(node);
    }else{
        return left;
    }
}
// addsub_expr := muldiv_expr addsub_expr_tail
BTNode *addsub_expr(void){
    BTNode *node = muldiv_expr();
    return addsub_expr_tail(node);
}
// addsub_expr_tail := ADDSUB muldiv_expr addsub_expr_tail | NiL
BTNode *addsub_expr_tail(BTNode *left){
    BTNode *node = NULL;
    if(match(ADDSUB)){
        node = makeNode(ADDSUB, getLexeme());
        advance();
        node -> left = left;
        node -> right = muldiv_expr();
        return addsub_expr_tail(node);
    }else{
        return left;
    }
}
// muldiv_expr := unary_expr muldiv_expr_tail
BTNode *muldiv_expr(void){
    BTNode *node = unary_expr();
    return muldiv_expr_tail(node);
}
// muldiv_expr_tail := MULDIV unary_expr muldiv_expr_tail | NiL
BTNode *muldiv_expr_tail(BTNode *left){
    BTNode *node = NULL;
    if(match(MULDIV)){
        node = makeNode(MULDIV, getLexeme());
        advance();
        node -> left = left;
        node -> right = unary_expr();
        return muldiv_expr_tail(node);
    }else{
        return left;
    }
}
// unary_expr := ADDSUB factor | factor
// 
BTNode *unary_expr(void){
    int is_neg = 0;
    while(match(ADDSUB)){
        is_neg ^= getLexeme()[0]=='-';
        advance();
    }
    BTNode *retp = factor();
    if(is_neg){//making the node = 0 - factor value
        BTNode *node = makeNode(ADDSUB, "-");
        node -> left = makeNode(INT, "0");
        node -> right = retp;
        retp = node;
    }
    return retp;
}
// factor := INT |
//		   	 ID  |
//           INCDEC ID|
//		   	 LPAREN assign_expr RPAREN |
BTNode *factor(void) {
    BTNode *retp = NULL, *left = NULL;

    if (match(INT)) {
        retp = makeNode(INT, getLexeme());
        advance();
    } else if (match(ID)) {
        retp = makeNode(ID, getLexeme());
        advance();
    } else if (match(INCDEC)) {//INCDEC must be followed by ID
        retp = makeNode(INCDEC, getLexeme());
        advance();
        if (match(ID)) {
            retp->left = makeNode(ID, getLexeme());
            advance();
        } else {
            error(NOTNUMID);
        }
    } else if (match(LPAREN)) {
        advance();
        retp = assign_expr();
        if (match(RPAREN))
            advance();
        else
            error(MISPAREN);
    } else {
        error(NOTNUMID);
    }
    return retp;
}
void err(ErrorType errorNum) {
    printf("EXIT 1\n")
    if (PRINTERR) {
        fprintf(stderr, "error: ");
        switch (errorNum) {
            case MISPAREN:
                fprintf(stderr, "mismatched parenthesis\n");
                break;
            case NOTNUMID:
                fprintf(stderr, "number or identifier expected\n");
                break;
            case NOTFOUND:
                fprintf(stderr, "variable not defined\n");
                break;
            case RUNOUT:
                fprintf(stderr, "out of memory\n");
                break;
            case NOTLVAL:
                fprintf(stderr, "lvalue required as an operand\n");
                break;
            case DIVZERO:
                fprintf(stderr, "divide by constant zero\n");
                break;
            case SYNTAXERR:
                fprintf(stderr, "syntax error\n");
                break;
            default:
                fprintf(stderr, "undefined error\n");
                break;
        }
    }
    exit(0);
}
