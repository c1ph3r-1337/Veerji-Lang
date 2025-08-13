#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ==== Lexer declarations ====

typedef enum {
    TOKEN_PRINT,
    TOKEN_SEPARATOR,
    TOKEN_STRING,
    TOKEN_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    char value[512];
} Token;

int tokenize(const char* line, Token* tokens);  // from lexer.c

// ==== Parser declarations ====

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

Program parse_tokens(Token* tokens, int token_count);  // from parser.c

// ==== Codegen declaration ====

void generate_code(Program program, const char* filename);  // from codegen.c

// ==== Main ====

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <file.veerji>\n", argv[0]);
        return 1;
    }

    FILE* f = fopen(argv[1], "r");
    if (!f) {
        perror("File open failed");
        return 1;
    }

    char line[1024];
    fgets(line, sizeof(line), f);
    fclose(f);

    Token tokens[10];
    int count = tokenize(line, tokens);

    printf("=== TOKENS ===\n");
    for (int i = 0; i < count; i++) {
        printf("Token %d: Type=%d, Value=%s\n", i, tokens[i].type, tokens[i].value);
    }

    Program program = parse_tokens(tokens, count);

    printf("=== PARSED ===\n");
    for (int i = 0; i < program.count; i++) {
        printf("Statement %d: PRINT \"%s\"\n", i, program.statements[i].value);
    }

    // Generate NASM Assembly
    generate_code(program, "out.s");
    printf("Assembly written to out.s\n");

    return 0;
}
