// codegen.c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef enum {
    STMT_PRINT
} StatementType;

typedef struct {
    StatementType type;
    char* value;
    
} Statement;

typedef struct {
    Statement* statements;
    int count;
} Program;

// === Codegen logic ===

void generate_code(Program program, const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        perror("Cannot open output file");
        exit(1);
    }

    fprintf(f, "section .data\n");

    for (int i = 0; i < program.count; i++) {
        if (program.statements[i].type == STMT_PRINT) {
            fprintf(f, "msg%d db \"%s\", 0xA\n", i, program.statements[i].value);
            fprintf(f, "len%d equ $ - msg%d\n", i, i);
        }
    }

    fprintf(f, "\nsection .text\n");
    fprintf(f, "global _start\n");
    fprintf(f, "_start:\n");

    for (int i = 0; i < program.count; i++) {
        if (program.statements[i].type == STMT_PRINT) {
            fprintf(f, "    mov rax, 1\n");
            fprintf(f, "    mov rdi, 1\n");
            fprintf(f, "    mov rsi, msg%d\n", i);
            fprintf(f, "    mov rdx, len%d\n", i);
            fprintf(f, "    syscall\n\n");
        }
    }

    // Exit syscall
    fprintf(f, "    mov rax, 60\n");
    fprintf(f, "    xor rdi, rdi\n");
    fprintf(f, "    syscall\n");

    fclose(f);
}
