#ifndef __CODEGEN__
#define __CODEGEN__

#include "parser.h"

// Evaluate the syntax tree
extern int evaluateTree(BTNode *root);

// Print the syntax tree in prefix
extern void printPrefix(BTNode *root);
extern void printPostfix(BTNode *root);
extern void printAssemble(BTNode *root);
extern int stack_top;
extern int memory_queue_back;
#endif // __CODEGEN__
