#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>

#define OPSIZE 32

// Lexer

typedef enum {
    TOK_DECLARE,
    TOK_IDENTIFIER,
    TOK_COLON,
    TOK_TYPE_INT,
    TOK_INT,
    TOK_ASSIGN,
    TOK_TYPE_REAL,
    TOK_REAL,
    TOK_PLUS,
    TOK_MINUS,
    TOK_STAR,
    TOK_SLASH,
    TOK_END,
    TOK_EOF,
    TOK_PAREN
} TokenType;

typedef struct {
    TokenType type;
    char *lexeme;
    int value;
} Token;

bool check(char* str, const char* arr[], int size) {
    for (int i = 0; i < size; i++) {
        if (strcmp(str, arr[i]) == 0) {
            return true;
        }
    }
    return false;
}

char* slice(const char* s, int left, int right) {
    int len = right - left + 1;
    char *r = malloc(len + 1);
    memcpy(r, s + left, len);
    r[len] = '\0';
    return r;
}

bool isDelimiter(char c) {
    const char* delimeters[] = {" ", "\n", "\0"};
    char t[] = {c, '\0'};
    return check(t, delimeters, 3);
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

bool isSpecialSym(char* str, TokenType* type) {
    const char* specialSyms[] = {":", "<-", "(", ")"};
    if (!check(str, specialSyms, 4)) {return false;}
    if (!strcmp(str, ":")) {*type = TOK_COLON;}
    else if (!strcmp(str, "<-")) {*type = TOK_ASSIGN;}
    else if (!strcmp(str, "(") || !strcmp(str, ")")) {*type = TOK_PAREN;}
    return true;
}

bool isKeyword(char* str, TokenType* type) {
    const char* keywords[] = {"DECLARE", "INTEGER", "REAL", "STRING", "OUTPUT"};
    if (!check(str, keywords, 5)) {return false;}
    if (!strcmp(str, "DECLARE")) {*type = TOK_DECLARE;}
    if (!strcmp(str, "INTEGER")) {*type = TOK_TYPE_INT;}
    if (!strcmp(str, "REAL")) {*type = TOK_REAL;}
    return true;
}

bool isOper(char* str, TokenType* type) {
    const char* opers[] = {"+", "-", "*", "/"};
    if (!check(str, opers, 4)) {return false;}
    if (!strcmp(str, "+")) {*type = TOK_PLUS;}
    else if (!strcmp(str, "-")) {*type = TOK_MINUS;}
    else if (!strcmp(str, "*")) {*type = TOK_STAR;}
    else if (!strcmp(str, "/")) {*type = TOK_SLASH;}
    return true;
}

// AST
//=======================
// Types
typedef enum {
    NODE_NUMBER,
    NODE_PROGRAM,
    NODE_VAR_DECL,
    NODE_ASSIGN,
    NODE_BINARY_OP,
    NODE_IDENTIFIER,
    NODE_LITERAL
} NodeType;

typedef enum {
    INT,
    REAL,
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

void add_child(ASTNode* parent, ASTNode* child) {
    parent->children = realloc(parent->children,
                               sizeof(ASTNode*) * (parent->child_count + 1));
    parent->children[parent->child_count++] = child;
}

ASTNode *create_identifier(char *name) {
    ASTNode *node = new_node(NODE_IDENTIFIER);
    node->data.name = _strdup(name);
    return node;
}

ASTNode *create_number(int value) {
    ASTNode *node = new_node(NODE_NUMBER);
    node->data.value = value;
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

// Tokenizer
int tokenize(char* str, Token* tokens, int max_tokens) {
    TokenType type;
    int left = 0, right = 0, i = 0, len = strlen(str);
    while (right <= len && left <= len) {
        if (!isDelimiter(str[right])) { right++; }
        else if (isDelimiter(str[right]) && left == right) { left = ++right; }
        else if (isDelimiter(str[right]) && left != right) {
            if (i >= max_tokens-1) {
                printf("OverflowError!\n");
                exit(1);
            }
            char* substr = slice(str, left, right-1);
            if (strcmp(substr, "//") == 0) {
                tokens[i].type = TOK_END;
                tokens[i].lexeme = _strdup("END");
                free(substr);
                return i+1;
            }
            else if (isOper(substr, &type)) {
                tokens[i].type = type;
                tokens[i].lexeme = _strdup(substr);
            } else if (isKeyword(substr, &type)) {
                tokens[i].type = type;
                tokens[i].lexeme = _strdup(substr);
            } else if (isSpecialSym(substr, &type)) {
                tokens[i].type = type;
                tokens[i].lexeme = _strdup(substr);
            } else if (isIdentifier(substr)) {
                tokens[i].type = TOK_IDENTIFIER;
                tokens[i].lexeme = _strdup(substr);
            } else if (isReal(substr)) {
                tokens[i].type = TOK_REAL;
                tokens[i].value = atof(substr);
                tokens[i].lexeme = _strdup(substr);
            } else if (isInt(substr)) {
                tokens[i].type = TOK_INT;
                tokens[i].value = atoi(substr);
                tokens[i].lexeme = _strdup(substr);
            } else {
                printf("[%s] Not Valid\n", substr);
                free(substr);
                exit(1);
            }
            free(substr);
            left = ++right;
            i++;
        }
    }
    tokens[i].type = TOK_END;
    tokens[i].lexeme = _strdup("END");
    return i+1;
}

// AST Parser
Token tokens[100];
int current_token = 0;
int token_count = 0;

Token* peekToken(int offset) { 
    if (current_token + offset >= token_count) { return &tokens[token_count - 1]; }
    return &tokens[current_token + offset];
}
Token* nextToken(void) { 
    if (current_token < token_count - 1) { current_token++; }
    return &tokens[current_token];
}
bool matchTokens(TokenType type) {
    if (peekToken(0)->type == type) { return true; }
    return false;
}
void checkToken(TokenType type) {
    if (matchTokens(type)) { nextToken(); }
    else {
        printf("Expected %d, got %d", type, peekToken(0)->type);
        exit(1);
    }
}
VarType mapType(TokenType tok) {
    switch (tok) {
        case TOK_TYPE_INT: return INT;
        case TOK_TYPE_REAL:    return REAL;
        default:
            printf("Unknown type token: %d\n", tok);
            exit(1);
    }
}

ASTNode* parse_decl(void) {
    if (!matchTokens(TOK_IDENTIFIER)) {
        printf("Expected Identifier after declaration!\n");
        exit(1);
    }
    char* name = _strdup(peekToken(0)->lexeme);
    nextToken();
    if (!matchTokens(TOK_COLON)) {
        printf("Expected Colon after Identifier!\n");
        exit(1);
    }
    nextToken();
    if (!matchTokens(TOK_TYPE_INT) && !matchTokens(TOK_TYPE_REAL)) {
        printf("Expected Valid Type after Identifier!\n");
        exit(1);
    }
    VarType vtype = mapType(peekToken(0)->type);
    nextToken();
    checkToken(TOK_END);
    return create_var_decl(vtype, name);
}

ASTNode* parse_exp(void) {
    ASTNode* stack[100];
    int top = -1;

    while (!matchTokens(TOK_END)) {
        if (matchTokens(TOK_INT) || matchTokens(TOK_REAL)) {
            stack[++top] = create_number(peekToken(0)->value);
        } 
        else if (matchTokens(TOK_IDENTIFIER)) {
            stack[++top] = create_identifier(peekToken(0)->lexeme);
        } 
        else if (matchTokens(TOK_PLUS) || matchTokens(TOK_MINUS) ||
                 matchTokens(TOK_STAR) || matchTokens(TOK_SLASH)) {

                if (top < 1) {
                    printf("Invalid RPN expression!\n");
                    exit(1);
                }
                ASTNode* right = stack[top--];
                ASTNode* left = stack[top--];

                OpType op;
                switch (peekToken(0)->type) {
                    case TOK_PLUS:  op = ADD; break;
                    case TOK_MINUS: op = SUB; break;
                    case TOK_STAR:  op = MUL; break;
                    case TOK_SLASH: op = DIV; break;
                    default: printf("Unknown operator!\n"); exit(1);
                }
                stack[++top] = create_bin_op(op, left, right);
        } 
        else {
            printf("Unexpected token in expression!\n");
            exit(1);
        }
        nextToken();
    }

    if (top != 0) {
        printf("Expression stack not reduced to single node!\n");
        exit(1);
    }
    return stack[0];
}

ASTNode* parse_assign(void) {
    char* name = _strdup(peekToken(0)->lexeme);
    ASTNode* id = create_identifier(name);
    free(name);
    nextToken();
    if (!matchTokens(TOK_ASSIGN)) {
        printf("Expected \"<-\" after Identifier!\n");
        exit(1);
    }
    nextToken();
    ASTNode* expr = parse_exp();
    checkToken(TOK_END);
    return create_assignment(id, expr);
}

void free_tokens(Token* tokens, int count) {
    for (int i = 0; i < count; i++) {
        if (tokens[i].lexeme != NULL) {
            free(tokens[i].lexeme);
            tokens[i].lexeme = NULL;
        }
    }
}

ASTNode* parse_statement(char* statement) {
    ASTNode* result = NULL;
    token_count = tokenize(statement, tokens, 100);
    current_token = 0;
    if (token_count == 0) { return NULL; }
    if (matchTokens(TOK_DECLARE)) {
        nextToken();
        result = parse_decl();
    } else if (matchTokens(TOK_IDENTIFIER)) {
        result = parse_assign();
    } else if (matchTokens(TOK_END)) {
        return NULL;
    } else {
        printf("Invalid Statement!\n");
        exit(1);
    }
    free_tokens(tokens, token_count);
    return result;
}

// FOR PRINTING PURPOSES ==============================================================
void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  "); // 2 spaces
    }
}

const char* opname(OpType op) {
    switch (op) {
        case ADD: return "+";
        case SUB: return "-";
        case MUL: return "*";
        case DIV: return "/";
        default:  return "?";
    }
}

const char* vartype(VarType t) {
    switch (t) {
        case INT: return "int";
        case REAL: return "real";
        case CHAR: return "char";
        default:   return "?";
    }
}

void print_ast(ASTNode* node, int indent) {
    if (!node) return;

    print_indent(indent);

    switch (node->type) {
        case NODE_PROGRAM:
            printf("Program\n");
            for (int i = 0; i < node->child_count; i++) {
                print_ast(node->children[i], indent + 1);
            }
            break;

        case NODE_VAR_DECL:
            printf("VarDecl(type=%s)\n", vartype(node->data.var_type));
            for (int i = 0; i < node->child_count; i++) {
                print_ast(node->children[i], indent + 1);
            }
            break;

        case NODE_ASSIGN:
            printf("Assign\n");
            for (int i = 0; i < node->child_count; i++) {
                print_ast(node->children[i], indent + 1);
            }
            break;

        case NODE_BINARY_OP:
            printf("BinaryOp(%s)\n", opname(node->data.op));
            for (int i = 0; i < node->child_count; i++) {
                print_ast(node->children[i], indent + 1);
            }
            break;

        case NODE_IDENTIFIER:
            printf("Identifier(%s)\n", node->data.name);
            break;

        case NODE_NUMBER:
            printf("Number(%d)\n", node->data.value);
            break;

        case NODE_LITERAL:
            printf("Literal(%s)\n", node->data.name);
            break;

        default:
            printf("UnknownNode\n");
    }
}
//===============================================================================

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <source.pseu>\n", argv[0]);
        return 1;
    }

    char str[256];
    FILE* file = fopen(argv[1], "r");
    if (!file) {
        perror("fopen");
        return 1;
    }

    ASTNode* program = new_node(NODE_PROGRAM);

    while (fgets(str, sizeof(str), file)) {
        ASTNode* stmt = parse_statement(str);
        if (stmt) {
            add_child(program, stmt);
        }
    }

    fclose(file);
    print_ast(program, 0);
    return 0;
}
