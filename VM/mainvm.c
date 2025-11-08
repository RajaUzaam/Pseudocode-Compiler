#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define STACK_SIZE 1024
#define MEM_SIZE 512
#define LINE_SIZE 128

// Stack
int stack[STACK_SIZE];
int sp = -1;

// Memory
int mem[MEM_SIZE];

// Push value onto stack
void push(int val) {
    if (sp >= STACK_SIZE - 1) {
        printf("Stack overflow!\n");
        exit(1);
    }
    stack[++sp] = val;
}

// Pop value from stack
int pop() {
    if (sp < 0) {
        printf("Stack underflow!\n");
        exit(1);
    }
    return stack[sp--];
}
char* trim(char* str) {
    while(*str == ' ' || *str == '\t') str++;
    char* end = str + strlen(str) - 1;
    while(end > str && (*end==' ' || *end=='\t' || *end=='\n')) *end-- = '\0';
    return str;
}

int main(int argc, char* argv[]) {
    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        printf("Error: Cannot open output.pseubc\n");
        return 1;
    }

    char line[LINE_SIZE];


    while (fgets(line, sizeof(line), fp)) {
        char lineCopy[LINE_SIZE];
        strcpy(lineCopy, line);
        char* instr = trim(strtok(lineCopy, " "));

        if (instr[0] == '\0' || instr[0] == ';') continue; // skip empty lines/comments

        if (strcmp(instr, "PUSH") == 0) {
            char* arg = trim(strtok(NULL, " "));
            if (arg[0] == '#') { // literal
                push(atoi(arg + 1));
            } else if (arg[0] == '[') { // memory reference
                int idx = atoi(arg + 1);
                push(mem[idx]);
            }
        } else if (strcmp(instr, "STORE") == 0) {
            char* arg = trim(strtok(NULL, " "));
            int idx = atoi(arg + 1);
            mem[idx] = pop();
        } else if (strcmp(instr, "LOAD") == 0) {
            char* arg = trim(strtok(NULL, " "));
            int idx = atoi(arg + 1);
            push(mem[idx]);
        } else if (strcmp(instr, "ADD") == 0) {
            int b = pop();
            int a = pop();
            push(a + b);
        } else if (strcmp(instr, "DIV") == 0) {
            int b = pop();
            int a = pop();
            push(a / b);
        } else if (strcmp(instr, "OUT") == 0) {
            int val = pop();
            printf("%d\n", val);
        } else if (strcmp(instr, "END") == 0) {
            break;
        } else {
            printf("Unknown instruction: %s\n", instr);
            return 1;
        }
    }

    fclose(fp);
    return 0;
}
