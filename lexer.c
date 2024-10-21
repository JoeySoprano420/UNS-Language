#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    TOKEN_NUMBER,
    TOKEN_IDENTIFIER,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    char* value;
} Token;

Token* create_token(TokenType type, const char* value) {
    Token* token = (Token*)malloc(sizeof(Token));
    token->type = type;
    token->value = strdup(value);
    return token;
}

void free_token(Token* token) {
    free(token->value);
    free(token);
}

typedef struct {
    const char* source;
    size_t index;
} Lexer;

Lexer* create_lexer(const char* source) {
    Lexer* lexer = (Lexer*)malloc(sizeof(Lexer));
    lexer->source = source;
    lexer->index = 0;
    return lexer;
}

Token* lexer_next_token(Lexer* lexer) {
    while (lexer->index < strlen(lexer->source)) {
        char current = lexer->source[lexer->index];

        if (isspace(current)) {
            lexer->index++;
            continue;
        }

        if (isdigit(current)) {
            size_t start = lexer->index;
            while (isdigit(lexer->source[lexer->index])) {
                lexer->index++;
            }
            char* number = strndup(lexer->source + start, lexer->index - start);
            return create_token(TOKEN_NUMBER, number);
        }

        if (isalpha(current)) {
            size_t start = lexer->index;
            while (isalnum(lexer->source[lexer->index])) {
                lexer->index++;
            }
            char* identifier = strndup(lexer->source + start, lexer->index - start);
            return create_token(TOKEN_IDENTIFIER, identifier);
        }

        switch (current) {
            case '+':
                lexer->index++;
                return create_token(TOKEN_PLUS, "+");
            case '-':
                lexer->index++;
                return create_token(TOKEN_MINUS, "-");
            case '*':
                lexer->index++;
                return create_token(TOKEN_MULTIPLY, "*");
            case '/':
                lexer->index++;
                return create_token(TOKEN_DIVIDE, "/");
        }

        lexer->index++;
    }

    return create_token(TOKEN_EOF, "");
}
