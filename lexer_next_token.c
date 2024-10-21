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

        if (current == '"') { // Handle string literals
            lexer->index++;
            size_t start = lexer->index;
            while (lexer->source[lexer->index] != '"' && lexer->source[lexer->index] != '\0') {
                lexer->index++;
            }
            if (lexer->source[lexer->index] == '"') {
                char* string = strndup(lexer->source + start, lexer->index - start);
                lexer->index++; // Skip closing quote
                return create_token(TOKEN_STRING, string);
            }
        }

        if (isalpha(current) || current == '_') {
            size_t start = lexer->index;
            while (isalnum(lexer->source[lexer->index]) || lexer->source[lexer->index] == '_') {
                lexer->index++;
            }
            char* identifier = strndup(lexer->source + start, lexer->index - start);
            if (strcmp(identifier, "if") == 0) {
                return create_token(TOKEN_IF, identifier);
            }
            if (strcmp(identifier, "else") == 0) {
                return create_token(TOKEN_ELSE, identifier);
            }
            if (strcmp(identifier, "while") == 0) {
                return create_token(TOKEN_WHILE, identifier);
            }
            if (strcmp(identifier, "return") == 0) {
                return create_token(TOKEN_RETURN, identifier);
            }
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
            case '=':
                lexer->index++;
                return create_token(TOKEN_ASSIGN, "=");
            case ';':
                lexer->index++;
                return create_token(TOKEN_SEMICOLON, ";");
            case '(':
                lexer->index++;
                return create_token(TOKEN_LPAREN, "(");
            case ')':
                lexer->index++;
                return create_token(TOKEN_RPAREN, ")");
            case '{':
                lexer->index++;
                return create_token(TOKEN_LBRACE, "{");
            case '}':
                lexer->index++;
                return create_token(TOKEN_RBRACE, "}");
            case ',':
                lexer->index++;
                return create_token(TOKEN_COMMA, ",");
        }

        lexer->index++;
    }

    return create_token(TOKEN_EOF, "");
}
