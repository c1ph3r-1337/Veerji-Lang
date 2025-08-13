#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int tokenize(const char* line, Token* tokens) {
    int count = 0;

    if (strncmp(line, "ਲਿਖੋ", strlen("ਲਿਖੋ")) == 0) {
        tokens[count++] = (Token){ TOKEN_PRINT, "ਲਿਖੋ" };

        const char* sep = strstr(line, "☬");
        if (sep != NULL) {
            tokens[count++] = (Token){ TOKEN_SEPARATOR, "☬" };

            const char* str = sep + strlen("☬");
            while (*str == ' ') str++; // skip spaces

            Token stringToken = { TOKEN_STRING, "" };
            strncpy(stringToken.value, str, sizeof(stringToken.value) - 1);
            stringToken.value[strcspn(stringToken.value, "\n")] = 0; // remove newline

            tokens[count++] = stringToken;
        }
    } else {
        tokens[count++] = (Token){ TOKEN_UNKNOWN, "???" };
    }

    return count;
}
