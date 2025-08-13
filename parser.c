#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Match lexer token type (same as in main.c)
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

// === Parser structs ===

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

// === Main Parser Logic ===

Program parse_tokens(Token* tokens, int token_count) {
    Program prog;
    prog.statements = malloc(sizeof(Statement) * token_count);
    prog.count = 0;

    for (int i = 0; i < token_count;) {
        if (i + 2 < token_count &&
            tokens[i].type == TOKEN_PRINT &&
            tokens[i+1].type == TOKEN_SEPARATOR &&
            tokens[i+2].type == TOKEN_STRING) {

            Statement stmt;
            stmt.type = STMT_PRINT;
            stmt.value = strdup(tokens[i+2].value);

            prog.statements[prog.count++] = stmt;
            i += 3;
        } else {
            fprintf(stderr, "Syntax error at token %d: '%s'\n", i, tokens[i].value);
            exit(1);
        }
    }

    return prog;
}
