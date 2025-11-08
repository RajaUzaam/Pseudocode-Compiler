#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef enum Token {
    IDENTIFIER,
    ASSIGNMENT,
    NUMBER,
    OPER,
    OUT,
    IDENTIFIER_NUM,
    _NULL,
    END
} Token;

typedef enum Oper {
    ADD,
    SUB,
    MUL,
    DIV,
    NONE
} Oper;

typedef struct Statement {
    Token tokens[100];
    char str_tokens[10][100];
    Oper op;
    int token_num;
} Statement;

char* slice(const char* s, int left, int right) {
    int len = right - left + 1;
    char *r = malloc(len + 1);
    memcpy(r, s + left, len);
    r[len] = '\0';
    return r;
}

bool check(char* str, const char* arr[], int size) {
    for (int i = 0; i < size; i++) {
        if (strcmp(str, arr[i]) == 0) {
            return true;
        }
    }
    return false;
}

bool checkGrammer(Token* grammer_tokens, Token* check_tokens, int grammer_len) {
    for (int i = 0; i < grammer_len; i++) {
        if (grammer_tokens[i] != check_tokens[i]) {
            if ((grammer_tokens[i] == IDENTIFIER_NUM) && (check_tokens[i] == IDENTIFIER || check_tokens[i] == NUMBER)){
                continue;
            }
            return false;
        }
    }
    return true;
}



bool isIdentifier(char* literal) {
    int len = strlen(literal);
    if (literal[0] >= '0' && literal[0] <= '9') {
        return false;
    }
    for (int i = 0; i < len; i++) {
        if (!((literal[i] >= 'a' && literal[i] <= 'z') 
        || (literal[i] >= 'A' && literal[i] <= 'Z') 
        || (literal[i] >= '0' && literal[i] <= '9') 
        || (literal[i] == '_'))) {
            return false;
        }
    }
    return true;
}

bool isOper(char literal) {
    const char opers[] = {'+', '-', '*', '/'};
    for (int i = 0; i < 4; i++) {
        if (opers[i] == literal) {
            return true;
        }
    }
    return false;
}
Oper operType(char literal) {
    if (literal == '+') {return ADD;}
    else if (literal == '-') {return SUB;}
    else if (literal == '/') {return DIV;}
    else if (literal == '*') {return MUL;}
    return NONE;
}

bool isNumber(char* literal) {
    int len = strlen(literal);
    for (int i = 0; i < len; i++) {
        if (!(literal[i] >= '0' && literal[i] <= '9')) {
            return false;
        }
    }
    return true;
}

char **symbols = NULL;
int symbols_len = 0;

int checkSymbol(char* literal) {
    for (int i = 0; i < symbols_len; i++) {
        if (!(strcmp(literal, symbols[i]))) {
            return i;
        }
    }
    return -1;
}

bool addSymbol(char* literal) {
    char **tmp = realloc(symbols, sizeof(char*) * (symbols_len + 1));
    if (!tmp) return false;
    symbols = tmp;

    symbols[symbols_len] = malloc(strlen(literal) + 1);
    if (!symbols[symbols_len]) return false;

    strcpy(symbols[symbols_len], literal);
    symbols_len++;
    return true;
}

Statement TokenizeStatement(char* statement) {
    int statement_len = strlen(statement);
    Statement tokenized_statement;
    int right = 0, left = 0, i = 0, len = strlen(statement);
    while (left <= len && right <= len) {
        if (isOper(statement[right])) {
            tokenized_statement.tokens[i] = OPER;
            char oper_str[2]; oper_str[1] = '\0';
            oper_str[0] = statement[right];
            strcpy(tokenized_statement.str_tokens[i], oper_str);
            tokenized_statement.op = operType(statement[right]);
            i++;
            left = ++right;
        }
        else if (statement[right] == '='){
            tokenized_statement.tokens[i] = ASSIGNMENT;
            strcpy(tokenized_statement.str_tokens[i], "=");
            i++;
            left = ++right;
        }
        else if (statement[right] == ' ' || statement[right] == '\n') {
            --right;
            if (right < left) {
                right++;right++;
                left++;
                continue;
            }
            char* substr = slice(statement, left, right);
            if (isNumber(substr)) {
                tokenized_statement.tokens[i] = NUMBER;
                char num_str[256]; num_str[0] = '#'; num_str[1] = '\0';
                strcpy(tokenized_statement.str_tokens[i], strcat(num_str, substr));
            }
            else if (!(strcmp("output", substr))) {
                tokenized_statement.tokens[i] = OUT,
                strcpy(tokenized_statement.str_tokens[i], "output");
            }
            else if (!(strcmp("null", substr))) {
                tokenized_statement.tokens[i] = _NULL,
                strcpy(tokenized_statement.str_tokens[i], "null");
            }
            else if (isIdentifier(substr)) {
                if (checkSymbol(substr) == -1) {
                    addSymbol(substr);
                }
                char str[256];
                sprintf(str, "[%d]", checkSymbol(substr));
                tokenized_statement.tokens[i] = IDENTIFIER;
                strcpy(tokenized_statement.str_tokens[i], str);
            }
            else {}
            free(substr);
            right++;
            left = ++right;
            i++;
        }
        else {
            right++;
        }
    }
    tokenized_statement.tokens[i] = END;
    tokenized_statement.token_num = ++i;
    return tokenized_statement;
}

Token gs1[] = {IDENTIFIER, ASSIGNMENT, IDENTIFIER_NUM, END};
Token gs2[] = {IDENTIFIER, ASSIGNMENT, IDENTIFIER_NUM, OPER, IDENTIFIER_NUM, END};
Token gs3[] = {OUT, IDENTIFIER_NUM, END};

char* mapOperBC(Oper oper) {
    switch (oper) {
        case ADD: return "ADD";
        case SUB: return "SUB";
        case DIV: return "DIV";
        case MUL: return "MUL";
        default:  return "NONE";
    }
}

void EchoBC(FILE* bc_file, char* statement) {
    Statement tokenized_statement = TokenizeStatement(statement);
    if (checkGrammer(gs1, tokenized_statement.tokens, 4)) {
        fprintf(bc_file, "PUSH %s\nSTORE %s\n", tokenized_statement.str_tokens[2], tokenized_statement.str_tokens[0]);
    }
    else if (checkGrammer(gs2, tokenized_statement.tokens, 6)) {
        fprintf(bc_file, "PUSH %s\nPUSH %s\n%s\nSTORE %s\n", tokenized_statement.str_tokens[2], tokenized_statement.str_tokens[4], mapOperBC(tokenized_statement.op), tokenized_statement.str_tokens[0]);
    }
    else if (checkGrammer(gs3, tokenized_statement.tokens, 3)) {
        fprintf(bc_file, "PUSH %s\nOUT\n", tokenized_statement.str_tokens[1]);
    }
}

void FinalizeBC(FILE* bc_file, FILE* out_file) {

}

int main(int argc, char* argv[]) {
    FILE* bc_file = fopen(argv[2], "w");
    FILE* ir_file = fopen(argv[1], "r");
    char str[256];
    while (fgets(str, 256, ir_file)) {
        if (strlen(str) > 1) {
            EchoBC(bc_file, str);
        }
    }
    fprintf(bc_file, "END");
    fclose(ir_file);
    fclose(bc_file);
    return 0;
}