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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_OPERATOR,
    TOKEN_KEYWORD,
    TOKEN_SEMICOLON,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_COMMA,
    TOKEN_EOF,
    TOKEN_INVALID
} TokenType;

typedef struct {
    TokenType type;
    char value[256];
} Token;

Token* lexer(const char* input) {
    Token* tokens = malloc(sizeof(Token) * 1024); // Simple token array
    int token_count = 0;
    
    for (int i = 0; input[i] != '\0';) {
        if (isspace(input[i])) {
            i++;
            continue;
        }
        
        if (isalpha(input[i])) { // Identifier or Keyword
            int j = 0;
            while (isalnum(input[i]) || input[i] == '_') {
                tokens[token_count].value[j++] = input[i++];
            }
            tokens[token_count].value[j] = '\0';
            tokens[token_count].type = TOKEN_IDENTIFIER; // For simplicity, treat all as identifiers
            token_count++;
            continue;
        }

        if (isdigit(input[i])) { // Number
            int j = 0;
            while (isdigit(input[i])) {
                tokens[token_count].value[j++] = input[i++];
            }
            tokens[token_count].value[j] = '\0';
            tokens[token_count].type = TOKEN_NUMBER;
            token_count++;
            continue;
        }

        if (input[i] == '"') { // String
            i++;
            int j = 0;
            while (input[i] != '"' && input[i] != '\0') {
                tokens[token_count].value[j++] = input[i++];
            }
            tokens[token_count].value[j] = '\0';
            tokens[token_count].type = TOKEN_STRING;
            token_count++;
            if (input[i] == '"') i++;
            continue;
        }

        if (strchr("+-*/();{}.,", input[i])) { // Operators and punctuation
            tokens[token_count].value[0] = input[i];
            tokens[token_count].value[1] = '\0';
            tokens[token_count].type = (input[i] == ';') ? TOKEN_SEMICOLON :
                                       (input[i] == '{') ? TOKEN_LBRACE :
                                       (input[i] == '}') ? TOKEN_RBRACE :
                                       (input[i] == '(') ? TOKEN_LPAREN :
                                       (input[i] == ')') ? TOKEN_RPAREN :
                                       TOKEN_OPERATOR;
            token_count++;
            i++;
            continue;
        }

        // Handle invalid tokens
        tokens[token_count].type = TOKEN_INVALID;
        tokens[token_count].value[0] = input[i];
        tokens[token_count].value[1] = '\0';
        token_count++;
        i++;
    }
    
    tokens[token_count].type = TOKEN_EOF; // End of file token
    return tokens;
}
