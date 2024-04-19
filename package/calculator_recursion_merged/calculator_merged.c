#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


// for lex
#define MAXLEN 256

// Token types
typedef enum {
    UNKNOWN, END, ENDFILE, 
    INT, ID,
    ADDSUB, MULDIV,
    ASSIGN, 
    LPAREN, RPAREN,
    INCDEC, ADDSUB_ASSIGN,
    AND, OR, XOR
} TokenSet;

// Test if a token matches the current token 
extern int match(TokenSet token);

// Get the next token
extern void advance(void);

// Get the lexeme of the current token
extern char *getLexeme(void);

// for parser
#define TBLSIZE 64

// Set PRINTERR to 1 to print error message while calling error()
// Make sure you set PRINTERR to 0 before you submit your code
#define PRINTERR 0

// Call this macro to print error message and exit the program
// This will also print where you called it in your program
#define error(errorNum) { \
    if (PRINTERR) \
        fprintf(stderr, "error() called at %s:%d: ", __FILE__, __LINE__); \
    err(errorNum); \
}

// Error types
typedef enum {
    UNDEFINED, MISPAREN, NOTNUMID, NOTFOUND, RUNOUT, NOTLVAL, DIVZERO, SYNTAXERR
} ErrorType;

// Structure of the symbol table
typedef struct {
    int val;
    char name[MAXLEN];
} Symbol;

// Structure of a tree node
typedef struct _Node {
    TokenSet data;
    int val;
    int reg;
    char lexeme[MAXLEN];
    struct _Node *left; 
    struct _Node *right;
} BTNode;

// The symbol table
extern Symbol table[TBLSIZE];

// Initialize the symbol table with builtin variables
extern void initTable(void);

// Get the value of a variable
extern int getval(char *str);
extern int getpos(char *str);

// Set the value of a variable
extern int setval(char *str, int val);

// Make a new node according to token type and lexeme
extern BTNode *makeNode(TokenSet tok, const char *lexe);

// Free the syntax tree
extern void freeTree(BTNode *root);
extern void statement(void);
extern BTNode *assign_expr(void);
extern BTNode *or_expr(void);
extern BTNode *or_expr_tail(BTNode *left);
extern BTNode *and_expr(void);
extern BTNode *and_expr_tail(BTNode *left);
extern BTNode *xor_expr(void);
extern BTNode *xor_expr_tail(BTNode *left);
extern BTNode *addsub_expr(void);
extern BTNode *addsub_expr_tail(BTNode *left);
extern BTNode *muldiv_expr(void);
extern BTNode *muldiv_expr_tail(BTNode *left);
extern BTNode *unary_expr(void);
extern BTNode *factor(void);

// Print error message and exit the program
extern void err(ErrorType errorNum);
extern int sbcount;

// for codeGen
// Evaluate the syntax tree
extern int evaluateTree(BTNode *root);

// Print the syntax tree in prefix
extern void printPrefix(BTNode *root);
extern void printPostfix(BTNode *root);
extern void printAssemble(BTNode *root);
extern int stack_top;
extern int memory_queue_back;

/*============================================================================================
lex implementation
============================================================================================*/

static TokenSet getToken(void);
static TokenSet curToken = UNKNOWN;
static char lexeme[MAXLEN];
static int isVariableName(char c){
    return isdigit(c) || c=='_'||isalpha(c);
}
TokenSet getToken(void)
{
    int i = 0;
    char c = '\0';

    while ((c = fgetc(stdin)) == ' ' || c == '\t');

    if (isdigit(c)) {
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while (isdigit(c) && i < MAXLEN) {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);
        if(i>=MAXLEN){//TODO: check is there an error type is for this case
            error(SYNTAXERR);
            exit(0);
        }
        lexeme[i] = '\0';
        return INT;
    } else if (c == '+' || c == '-') {
        char next_c = fgetc(stdin);
        lexeme[0] = c;
        if(next_c==c){
            lexeme[1] = next_c;
            lexeme[2] = '\0';
            return INCDEC;
        }else if(next_c=='='){
            lexeme[1] = next_c;
            lexeme[2] = '\0';
            return ADDSUB_ASSIGN;
        }else{
            lexeme[1] = '\0';
            ungetc(next_c, stdin);//push back next_c
            return ADDSUB;
        }
        lexeme[0] = c;
        lexeme[1] = '\0';
        return ADDSUB;
    } else if (c == '*' || c == '/') {
        lexeme[0] = c;
        lexeme[1] = '\0';
        return MULDIV;
    } else if (c == '\n') {
        lexeme[0] = '\0';
        return END;
    } else if (c == '=') {
        strcpy(lexeme, "=");
        return ASSIGN;
    } else if (c == '(') {
        strcpy(lexeme, "(");
        return LPAREN;
    } else if (c == ')') {
        strcpy(lexeme, ")");
        return RPAREN;
    } else if (c == EOF) {
        return ENDFILE;
    } else if(c=='&'){
        strcpy(lexeme, "&");
        return AND;
    } else if(c=='|'){
        strcpy(lexeme, "|");
        return OR;    
    } else if(c=='^'){
        strcpy(lexeme, "^");
        return XOR;
    } else if(isVariableName(c)){//since if it is a number it is already checked
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while (isVariableName(c) && i < MAXLEN) {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);
        lexeme[i] = '\0';
        if(i>=MAXLEN){
            error(SYNTAXERR);
            exit(0);
        }
        return ID; 
    }else {
        return UNKNOWN;
    }
}

void advance(void) {
    curToken = getToken();
}

int match(TokenSet token) {
    if (curToken == UNKNOWN)
        advance();
    return token == curToken;
}

char *getLexeme(void) {
    return lexeme;
}


/*============================================================================================
parser implementation
============================================================================================*/

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
            return i*4;
        }
    }
    error(NOTFOUND);
}

int getval(char *str) {
    int i = 0;

    for (i = 0; i < sbcount; i++)
        if (strcmp(str, table[i].name) == 0)
            return table[i].val;

    if (sbcount >= TBLSIZE)
        error(RUNOUT);
    
    error(NOTFOUND);
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
    node->reg = 0;
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
            int temp = evaluateTree(retp);
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
        char* str=malloc(sizeof(char)*MAXLEN);
        strcpy(str, getLexeme());
        retp = makeNode(ASSIGN, "=");
        advance();
        if (match(ID)) {
            if(str[0]=='+')retp->right = makeNode(ADDSUB, "+");
            else retp->right = makeNode(ADDSUB, "-");
            BTNode* temp_left = makeNode(ID, getLexeme());
            retp->right->left = temp_left;
            retp->right->right = makeNode(INT, "1");
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
    printf("EXIT 1\n");
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


/*============================================================================================
codeGen implementation
============================================================================================*/
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
                    if (rv == 0){
                        if(check_hasID(root->right)==0){
                            error(DIVZERO);
                        }
                        else retval = 1;//if it is a constant, then return 0 (since it is a constant zero
                    }
                    else retval = lv / rv;
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
int check_addNewVar(char* str){
    int found = 0;
    for(int i = 0;i<sbcount;i++){
        if(strcmp(table[i].name, str)==0){
            found = 1;
            break;
        }
    }
    if(!found){
        strcpy(table[sbcount].name, str);
        table[sbcount].val = 0;
        sbcount++;
    }
}
void print_allocate(BTNode *root){
    if(root->data==ID){
        check_addNewVar(root->lexeme);
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
    check_addNewVar(root->left->lexeme);
    printAssemble(root->right);
    printf("MOV [%d], r%d\n", getpos(root->left->lexeme), root->right->reg);
    root->reg = root->right->reg;
}
static int mem = 63;
void print_Arith(BTNode* root){
    //TODO: think about the situation that some of the reg are put in the memory
    //in such case, which reg should be set as the former one?
    //also, how should we ensure the set is still correct?
    int need_release = stack_top == 7;
    if(need_release){
        printf("MOV [%d] r6\n", mem*4);
        stack_top--;
        mem--;
    }
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
    if(need_release){
    //swap the reg
    //if realease is needed
    //to make sure the result is stored at r7
    //making the r6 available for the memory
        int temp = former_reg;
        former_reg = latter_reg;
        latter_reg = temp;
    }
    switch(root->lexeme[0]){
        case '+':
            printf("ADD r%d r%d\n", former_reg, latter_reg);
            break;
        case '-':
            printf("SUB r%d r%d\n", root->left->reg, root->right->reg);
            if(root->left->reg!=former_reg){
                printf("MOV r%d r%d\n", former_reg, root->left->reg);
            }
            break;
        case '*':
            printf("MUL r%d r%d\n", former_reg, latter_reg);
            break;
        case '/':
            printf("DIV r%d r%d\n", root->left->reg, root->right->reg);
            if(root->left->reg!=former_reg){
                printf("MOV r%d r%d\n", former_reg, root->left->reg);
            }
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
    if(need_release)
        printf("MOV r6 [%d]\n", (++mem)*4);
    else
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


/*============================================================================================
main
============================================================================================*/

// This package is a calculator
// It works like a Python interpretor
// Example:
// >> y = 2
// >> z = 2
// >> x = 3 * y + 4 / (2 * z)
// It will print the answer of every line
// You should turn it into an expression compiler
// And print the assembly code according to the input

// This is the grammar used in this package
// You can modify it according to the spec and the slide
// statement  :=  ENDFILE | END | expr END
// expr    	  :=  term expr_tail
// expr_tail  :=  ADDSUB term expr_tail | NiL
// term 	  :=  factor term_tail
// term_tail  :=  MULDIV factor term_tail| NiL
// factor	  :=  INT | ADDSUB INT |
//		   	      ID  | ADDSUB ID  |
//		   	      ID ASSIGN expr |
//		   	      LPAREN expr RPAREN |
//		   	      ADDSUB LPAREN expr RPAREN

int main() {
    freopen("input.txt", "w", stdout);
    initTable();
    while (1) {
        statement();
    }
    return 0;
}
