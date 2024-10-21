typedef struct ASTNode {
    TokenType type;
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

ASTNode* create_node(TokenType type, ASTNode* left, ASTNode* right) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = type;
    node->left = left;
    node->right = right;
    return node;
}

typedef struct {
    Lexer* lexer;
    Token* current_token;
} Parser;

Parser* create_parser(Lexer* lexer) {
    Parser* parser = (Parser*)malloc(sizeof(Parser));
    parser->lexer = lexer;
    parser->current_token = lexer_next_token(lexer);
    return parser;
}

void advance(Parser* parser) {
    free(parser->current_token);
    parser->current_token = lexer_next_token(parser->lexer);
}

ASTNode* parse_expression(Parser* parser);

ASTNode* parse_factor(Parser* parser) {
    Token* token = parser->current_token;

    if (token->type == TOKEN_NUMBER) {
        advance(parser);
        return create_node(TOKEN_NUMBER, NULL, NULL);
    }

    if (token->type == TOKEN_IDENTIFIER) {
        advance(parser);
        return create_node(TOKEN_IDENTIFIER, NULL, NULL);
    }

    return NULL;
}

ASTNode* parse_term(Parser* parser) {
    ASTNode* node = parse_factor(parser);

    while (parser->current_token->type == TOKEN_MULTIPLY || parser->current_token->type == TOKEN_DIVIDE) {
        TokenType operation = parser->current_token->type;
        advance(parser);
        node = create_node(operation, node, parse_factor(parser));
    }

    return node;
}

ASTNode* parse_expression(Parser* parser) {
    ASTNode* node = parse_term(parser);

    while (parser->current_token->type == TOKEN_PLUS || parser->current_token->type == TOKEN_MINUS) {
        TokenType operation = parser->current_token->type;
        advance(parser);
        node = create_node(operation, node, parse_term(parser));
    }

    return node;
}
