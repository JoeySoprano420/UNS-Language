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

// AST Node Types
typedef enum {
    NODE_BINARY_EXPR,
    NODE_NUMBER,
    NODE_IDENTIFIER,
    NODE_STRING,
    NODE_ASSIGNMENT,
    NODE_IF,
    NODE_WHILE,
    NODE_RETURN,
    NODE_BLOCK
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    union {
        struct { struct ASTNode* left; struct ASTNode* right; TokenType op; } binary;
        char* identifier; // For identifier and string nodes
        char* string_value; // For string nodes
        struct { struct ASTNode* condition; struct ASTNode* then_branch; struct ASTNode* else_branch; } if_node;
        struct { struct ASTNode* condition; struct ASTNode* body; } while_node;
        struct { struct ASTNode** statements; size_t size; } block; // For blocks of statements
    };
} ASTNode;

ASTNode* parse_expression(Parser* parser);
ASTNode* parse_statement(Parser* parser);
ASTNode* parse_block(Parser* parser);

ASTNode* parse_if_statement(Parser* parser) {
    advance(parser); // consume 'if'
    advance(parser); // consume '('
    ASTNode* condition = parse_expression(parser);
    advance(parser); // consume ')'
    ASTNode* then_branch = parse_statement(parser);
    
    ASTNode* else_branch = NULL;
    if (parser->current_token->type == TOKEN_ELSE) {
        advance(parser); // consume 'else'
        else_branch = parse_statement(parser);
    }
    
    return create_if_node(condition, then_branch, else_branch);
}

ASTNode* parse_while_statement(Parser* parser) {
    advance(parser); // consume 'while'
    advance(parser); // consume '('
    ASTNode* condition = parse_expression(parser);
    advance(parser); // consume ')'
    ASTNode* body = parse_statement(parser);
    
    return create_while_node(condition, body);
}

ASTNode* parse_return_statement(Parser* parser) {
    advance(parser); // consume 'return'
    ASTNode* value = parse_expression(parser);
    advance(parser); // consume ';'
    
    return create_return_node(value);
}

ASTNode* parse_statement(Parser* parser) {
    if (parser->current_token->type == TOKEN_IF) {
        return parse_if_statement(parser);
    }
    if (parser->current_token->type == TOKEN_WHILE) {
        return parse_while_statement(parser);
    }
    if (parser->current_token->type == TOKEN_RETURN) {
        return parse_return_statement(parser);
    }
    
    // Add more statement types as necessary
    // For assignment
    if (parser->current_token->type == TOKEN_IDENTIFIER) {
        char* identifier = parser->current_token->value;
        advance(parser); // consume identifier
        advance(parser); // consume '='
        ASTNode* value = parse_expression(parser);
        advance(parser); // consume ';'
        return create_assignment_node(identifier, value);
    }

    return NULL;
}

ASTNode* parse_block(Parser* parser) {
    advance(parser); // consume '{'
    ASTNode* block_node = create_block_node();
    
    while (parser->current_token->type != TOKEN_RBRACE && parser->current_token->type != TOKEN_EOF) {
        ASTNode* statement = parse_statement(parser);
        add_statement_to_block(block_node, statement);
    }
    
    advance(parser); // consume '}'
    return block_node;
}

typedef struct ASTNode {
    enum {
        NODE_ASSIGNMENT,
        NODE_CALL,
        NODE_IF,
        NODE_WHILE,
        NODE_BLOCK,
        NODE_IDENTIFIER,
        NODE_NUMBER,
        NODE_STRING,
        NODE_BINARY_EXPR,
        NODE_NODE_DEF,
        NODE_EXPORT,
        NODE_IMPORT,
        NODE_ERROR_HANDLING
    } type;
    union {
        struct {
            char identifier[256];
            struct ASTNode* expression;
        } assignment;
        struct {
            char identifier[256];
            struct ASTNode* parameters;
            struct ASTNode* response;
        } call;
        struct {
            struct ASTNode* condition;
            struct ASTNode* then_branch;
            struct ASTNode* else_branch;
        } if_node;
        struct {
            struct ASTNode* condition;
            struct ASTNode* body;
        } while_node;
        struct {
            struct ASTNode* statements[1024];
            int count;
        } block;
        char identifier[256];
        char string_value[256];
        int number_value;
        struct {
            struct ASTNode* left;
            struct ASTNode* right;
            char operator;
        } binary;
        struct {
            char name[256];
            struct ASTNode* input;
            struct ASTNode* output;
            struct ASTNode* process;
        } node_def;
        struct {
            char identifier[256];
            char metadata[256];
        } export_node;
        struct {
            char identifier[256];
        } import_node;
        struct {
            char identifier[256];
            struct ASTNode* action;
        } error_handling;
    };
} ASTNode;

ASTNode* parse_statement(Token** tokens, int* pos);
ASTNode* parse_expression(Token** tokens, int* pos);

// Example of parsing logic for assignment
ASTNode* parse_assignment(Token** tokens, int* pos) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_ASSIGNMENT;

    if (tokens[*pos]->type == TOKEN_IDENTIFIER) {
        strcpy(node->assignment.identifier, tokens[*pos]->value);
        (*pos)++;
    }

    if (tokens[*pos]->type == TOKEN_OPERATOR && strcmp(tokens[*pos]->value, ":=") == 0) {
        (*pos)++;
    }

    node->assignment.expression = parse_expression(tokens, pos);

    if (tokens[*pos]->type == TOKEN_SEMICOLON) {
        (*pos)++;
    }

    return node;
}

// Implement parse_expression and other parsing functions...
