#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>

#define OPSIZE 32

// Lexer
bool check(char* str, const char* arr[], int size) {
    for (int i = 0; i < size; i++) {
        if (strcmp(str, arr[i]) == 0) {
            return true;
        }
    }
    return false;
}

char* slice(char* str, int left, int right) {
    right++;
    char* substr = malloc(100);
    str += left;
    for (int i = 0; i < (right-left); i++) {
        substr[i] = *(str + i);
        substr[i+1] = '\0';
    }
    return substr;
}

bool isDelimiter(char c) {
    const char* delimeters[] = {" ", "\n"};
    char t[] = {c, '\0'};
    return check(t, delimeters, 2);
}

bool isIdentifier(char* str) {
    int len = strlen(str);
    if (str[0] >= '0' && str[0] <= '9') {
        return false;
    }
    for (int i = 0; i < len; i++) {
        if (!((str[i] >= 'a' && str[i] <= 'z') 
        || (str[i] >= 'A' && str[i] <= 'Z') 
        || (str[i] >= '0' && str[i] <= '9') 
        || (str[i] == '_'))) {
            return false;
        }
    }
    return true;
}

bool isInt(char* str) {
    int len = strlen(str);
    for (int i = 0; i < len; i++) {
        if (!(str[i] >= '0' && str[i] <= '9')) {
            return false;
        }
    }
    return true;
}

bool isReal(char* str) {
    int len = strlen(str);
    bool hasDecimal = false;
    for (int i = 0; i < len; i++) {
        if (hasDecimal && str[i] == '.') {
            return false;
        }
        else if (str[i] == '.') {
            hasDecimal = true;
        }
        else if (!(str[i] >= '0' && str[i] <= '9')) {
            return false;
        }
    }
    if (hasDecimal) {return true;}
    else {return false;}
}

bool isSpecialSym(char* str) {
    const char* specialSyms[] = {":", ";", "[", "]", "{", "}", "(", ")"};
    return check(str, specialSyms, 8);
}

bool isKeyword(char* str) {
    const char* keywords[] = {"DECLARE", "INTEGER", "REAL", "STRING", "OUTPUT"};
    return check(str, keywords, 5);
}

bool isOper(char* str) {
    const char* opers[] = {"+", "-", "*", "/", "<-"};
    return check(str, opers, 5);
}

void parse(char* str) {
    int left = 0, right = 0, len = strlen(str);
    while (right <= len && left <= len) {
        if (!isDelimiter(str[right])) {
            right++;
        }
        else if (isDelimiter(str[right]) && left != right) {
            char* substr = slice(str, left, right-1);
            if (strcmp(substr, "//") == 0) {
                return;
            }
            else if (isOper(substr)) {
                printf("[%s] is an Operator\n", substr);
            } else if (isKeyword(substr)) {
                printf("[%s] is a Keyword\n", substr);
            } else if (isSpecialSym(substr)) {
                printf("[%s] is a Special Symbol\n", substr);
            } else if (isIdentifier(substr)) {
                printf("[%s] is an Identifier\n", substr);
            } else if (isInt(substr)) {
                printf("[%s] is an Integer\n", substr);
            } else if (isReal(substr)) {
                printf("[%s] is an Real\n", substr);
            } else {
                printf("[%s] Not Valid\n", substr);
            }
            free(substr);
            left = ++right;
        }
    }
}

// AST
//=======================
// Types
typedef enum {
    NODE_PROGRAM,
    NODE_VAR_DECL,
    NODE_ASSIGN,
    NODE_BINARY_OP,
    NODE_IDENTIFIER,
    NODE_LITERAL
} NodeType;

typedef enum {
    INT,
    FLOAT,
    CHAR
} VarType;

typedef enum {
    ADD,
    MUL,
    SUB,
    DIV
} OpType;

// AST Node
typedef struct ASTNode{
    NodeType type;

    union {
        char *name;
        int value;
        VarType var_type;
        OpType op;
    } data;

    struct ASTNode **children;
    int child_count;

} ASTNode;

// AST Helper Functions
ASTNode *new_node(NodeType type) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = type;
    node->children = NULL;
    node->child_count = 0;
    return node;
}

ASTNode *create_var_decl(VarType vtype, char *name) {
    ASTNode *node = new_node(NODE_VAR_DECL);
    node->data.var_type = vtype;

    // variable name as a child IDENTIFIER node
    node->children = malloc(sizeof(ASTNode*));
    node->children[0] = create_identifier(name);
    node->child_count = 1;

    return node;
}

ASTNode *create_number(int value) {
    ASTNode *node = new_node(NODE_VAR_DECL);
    node->data.value = value;
    return node;
}

ASTNode *create_identifier(char *name) {
    ASTNode *node = new_node(NODE_IDENTIFIER);
    node->data.name = strdup(name);
    return node;
}

ASTNode *create_bin_op(OpType op, ASTNode *left, ASTNode *right) {
    ASTNode *node = new_node(NODE_BINARY_OP);
    node->data.op = op;

    node->children = malloc(sizeof(ASTNode*) * 2);
    node->children[0] = left;
    node->children[1] = right;
    node->child_count = 2;

    return node;
}

ASTNode *create_assignment(ASTNode *id, ASTNode *expr) {
    ASTNode *node = new_node(NODE_ASSIGN);

    node->children = malloc(sizeof(ASTNode*) * 2);
    node->children[0] = id;
    node->children[1] = expr;
    node->child_count = 2;

    return node;
}

int main(void) {
    char str[100];
    FILE* file;
    file = fopen("./main.pseu", "r");
    while (fgets(str, 100, file)) {
        parse(str);
    }
    fclose(file);
    return 0;
}