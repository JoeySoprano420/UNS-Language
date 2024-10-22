#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_VARIABLES 256

typedef struct {
    char* name;
    int value;
} Variable;

typedef struct {
    Variable variables[MAX_VARIABLES];
    size_t var_count;
} RuntimeEnvironment;

RuntimeEnvironment* create_runtime_environment() {
    RuntimeEnvironment* env = malloc(sizeof(RuntimeEnvironment));
    env->var_count = 0;
    return env;
}

void set_variable(RuntimeEnvironment* env, const char* name, int value) {
    for (size_t i = 0; i < env->var_count; i++) {
        if (strcmp(env->variables[i].name, name) == 0) {
            env->variables[i].value = value; // Update existing variable
            return;
        }
    }
    // Add new variable
    if (env->var_count < MAX_VARIABLES) {
        env->variables[env->var_count].name = strdup(name);
        env->variables[env->var_count].value = value;
        env->var_count++;
    }
}

int get_variable(RuntimeEnvironment* env, const char* name) {
    for (size_t i = 0; i < env->var_count; i++) {
        if (strcmp(env->variables[i].name, name) == 0) {
            return env->variables[i].value; // Return variable value
        }
    }
    fprintf(stderr, "Variable '%s' not found!\n", name);
    exit(EXIT_FAILURE);
}

void execute_node(RuntimeEnvironment* env, ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case NODE_NUMBER:
            // Simply return the value, though it's a simple representation.
            printf("%s\n", node->number_value);
            break;
        case NODE_IDENTIFIER:
            printf("%s\n", node->identifier);
            break;
        case NODE_ASSIGNMENT:
            {
                int value = get_variable(env, node->assignment.value->identifier); // Assume it's an identifier for simplicity
                set_variable(env, node->assignment.identifier, value);
                break;
            }
        case NODE_BINARY_EXPR:
            {
                int left_value = evaluate_expression(env, node->binary.left);
                int right_value = evaluate_expression(env, node->binary.right);
                int result = 0;
                switch (node->binary.op) {
                    case '+': result = left_value + right_value; break;
                    case '-': result = left_value - right_value; break;
                    case '*': result = left_value * right_value; break;
                    case '/': result = left_value / right_value; break;
                }
                return result; // Return the computed value
            }
        case NODE_IF:
            {
                int condition = execute_node(env, node->if_node.condition);
                if (condition) {
                    execute_node(env, node->if_node.then_branch);
                } else if (node->if_node.else_branch) {
                    execute_node(env, node->if_node.else_branch);
                }
                break;
            }
        case NODE_WHILE:
            while (execute_node(env, node->while_node.condition)) {
                execute_node(env, node->while_node.body);
            }
            break;
        case NODE_RETURN:
            return execute_node(env, node->return_node.value);
        case NODE_BLOCK:
            for (size_t i = 0; i < node->block.size; i++) {
                execute_node(env, node->block.statements[i]);
            }
            break;
        default:
            fprintf(stderr, "Unknown AST Node Type!\n");
            break;
    }
}

int evaluate_expression(RuntimeEnvironment* env, ASTNode* node) {
    switch (node->type) {
        case NODE_NUMBER:
            return atoi(node->number_value);
        case NODE_BINARY_EXPR:
            {
                int left_value = evaluate_expression(env, node->binary.left);
                int right_value = evaluate_expression(env, node->binary.right);
                switch (node->binary.op) {
                    case '+': return left_value + right_value;
                    case '-': return left_value - right_value;
                    case '*': return left_value * right_value;
                    case '/': return left_value / right_value;
                }
            }
            break;
        // Handle more cases as necessary
    }
    return 0;
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


                lexer->index++;
            }
            if (lexer->source[lexer->index] == '"') {
                size_t length = lexer->index - start;
                char* string_literal = strndup(lexer->source + start, length);
                lexer->index++; // Skip the closing quote
                return create_token(TOKEN_STRING, string_literal);
            }
            fprintf(stderr, "Unterminated string literal!\n");
            exit(EXIT_FAILURE);
        }

        if (isalpha(current) || current == '_') { // Handle identifiers
            size_t start = lexer->index;
            while (isalnum(lexer->source[lexer->index]) || lexer->source[lexer->index] == '_') {
                lexer->index++;
            }
            char* identifier = strndup(lexer->source + start, lexer->index - start);
            return create_token(TOKEN_IDENTIFIER, identifier);
        }

        switch (current) {
            case '+': lexer->index++; return create_token(TOKEN_PLUS, NULL);
            case '-': lexer->index++; return create_token(TOKEN_MINUS, NULL);
            case '*': lexer->index++; return create_token(TOKEN_STAR, NULL);
            case '/': lexer->index++; return create_token(TOKEN_SLASH, NULL);
            case '=': lexer->index++; return create_token(TOKEN_EQUAL, NULL);
            case '(': lexer->index++; return create_token(TOKEN_LPAREN, NULL);
            case ')': lexer->index++; return create_token(TOKEN_RPAREN, NULL);
            case '{': lexer->index++; return create_token(TOKEN_LBRACE, NULL);
            case '}': lexer->index++; return create_token(TOKEN_RBRACE, NULL);
            case ';': lexer->index++; return create_token(TOKEN_SEMICOLON, NULL);
            default:
                fprintf(stderr, "Unexpected character: '%c'\n", current);
                exit(EXIT_FAILURE);
        }
    }

    return create_token(TOKEN_EOF, NULL);
}

Token* create_token(TokenType type, char* value) {
    Token* token = malloc(sizeof(Token));
    token->type = type;
    token->value = value;
    return token;
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

int main() {
    const char* source_code = "a + b * 3";
    Lexer* lexer = create_lexer(source_code);
    Parser* parser = create_parser(lexer);
    
    ASTNode* root = parse_expression(parser);
    printf("Generated Code:\n");
    generate_code(root);
    
    // Cleanup
    // Free all allocated memory (not shown here for brevity)
    
    return 0;
}
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
typedef enum {
    TOKEN_NUMBER,
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_ASSIGN,
    TOKEN_SEMICOLON,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_COMMA,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_RETURN,
    TOKEN_EOF
} TokenType;
void generate_code(ASTNode* node) {
    if (node == NULL) {
        return;
    }

    generate_code(node->left);
    generate_code(node->right);

    switch (node->type) {
        case TOKEN_NUMBER:
            printf("PUSH %s\n", node->left->value); // Value would be stored
            break;
        case TOKEN_IDENTIFIER:
            printf("LOAD %s\n", node->left->value); // Load variable
            break;
        case TOKEN_PLUS:
            printf("ADD\n");
            break;
        case TOKEN_MINUS:
            printf("SUB\n");
            break;
        case TOKEN_MULTIPLY:
            printf("MUL\n");
            break;
        case TOKEN_DIVIDE:
            printf("DIV\n");
            break;
        default:
            break;
    }
}

void generate_code(ASTNode* node) {
    switch (node->type) {
        case NODE_NUMBER:
            printf("%s ", node->number_value);
            break;
        case NODE_IDENTIFIER:
            printf("%s ", node->identifier);
            break;
        case NODE_STRING:
            printf("\"%s\" ", node->string_value);
            break;
        case NODE_BINARY_EXPR:
            generate_code(node->binary.left);
            printf("%c ", node->binary.op);
            generate_code(node->binary.right);
            break;
        case NODE_ASSIGNMENT:
            printf("%s = ", node->assignment.identifier);
            generate_code(node->assignment.value);
            break;
        case NODE_IF:
            printf("if (");
            generate_code(node->if_node.condition);
            printf(") {\n");
            generate_code(node->if_node.then_branch);
            if (node->if_node.else_branch) {
                printf("} else {\n");
                generate_code(node->if_node.else_branch);
            }
            printf("}\n");
            break;
        case NODE_WHILE:
            printf("while (");
            generate_code(node->while_node.condition);
            printf(") {\n");
            generate_code(node->while_node.body);
            printf("}\n");
            break;
        case NODE_RETURN:
            printf("return ");
            generate_code(node->return_node.value);
            break;
        case NODE_BLOCK:
            for (size_t i = 0; i < node->block.size;
void generate_code(ASTNode* node) {
    if (!node) return; // Handle null nodes

    switch (node->type) {
        case NODE_NUMBER:
            printf("%s ", node->number_value);
            break;
        case NODE_IDENTIFIER:
            printf("%s ", node->identifier);
            break;
        case NODE_STRING:
            printf("\"%s\" ", node->string_value);
            break;
        case NODE_BINARY_EXPR:
            printf("(");
            generate_code(node->binary.left);
            printf(" %c ", node->binary.op);
            generate_code(node->binary.right);
            printf(")");
            break;
        case NODE_ASSIGNMENT:
            printf("%s = ", node->assignment.identifier);
            generate_code(node->assignment.value);
            printf(";\n");
            break;
        case NODE_IF:
            printf("if (");
            generate_code(node->if_node.condition);
            printf(") {\n");
            generate_code(node->if_node.then_branch);
            if (node->if_node.else_branch) {
                printf("} else {\n");
                generate_code(node->if_node.else_branch);
            }
            printf("}\n");
            break;
        case NODE_WHILE:
            printf("while (");
            generate_code(node->while_node.condition);
            printf(") {\n");
            generate_code(node->while_node.body);
            printf("}\n");
            break;
        case NODE_RETURN:
            printf("return ");
            generate_code(node->return_node.value);
            printf(";\n");
            break;
        case NODE_BLOCK:
            printf("{\n");
            for (size_t i = 0; i < node->block.size; i++) {
                generate_code(node->block.statements[i]);
            }
            printf("}\n");
            break;
        default:
            fprintf(stderr, "Unknown AST Node Type!\n");
            break;
    }
}

void generate_code(ASTNode* node) {
    switch (node->type) {
        case NODE_ASSIGNMENT:
            printf("%s = ", node->assignment.identifier);
            generate_code(node->assignment.expression);
            printf(";\n");
            break;
        case NODE_CALL:
            printf("call %s {\n", node->call.identifier);
            generate_code(node->); // consume 'while'
    advance(parser); // consume '('
    ASTNode* condition = parse_expression(parser);
    advance(parser); // consume ')'
    ASTNode* body = parse_statement(parser);

    return create_while_node(condition, body);
}

ASTNode* parse_block(Parser* parser) {
    advance(parser); // consume '{'
    ASTNode** statements = malloc(sizeof(ASTNode*) * 256); // Assuming max 256 statements per block for simplicity
    size_t count = 0;

    while (parser->current_token->type != TOKEN_RBRACE) {
        statements[count++] = parse_statement(parser);
    }

    advance(parser); // consume '}'

    ASTNode* block_node = create_block_node(statements, count);
    return block_node;
}

ASTNode* parse_statement(Parser* parser) {
    if (parser->current_token->type == TOKEN_IF) {
        return parse_if_statement(parser);
    }

    if (parser->current_token->type == TOKEN_WHILE) {
        return parse_while_statement(parser);
    }

    if (parser->current_token->type == TOKEN_LBRACE) {
        return parse_block(parser);
    }

    // Handle expressions or assignments
    ASTNode* expr = parse_expression(parser);
    advance(parser); // Consume ';' after the statement
    return expr;
}

void generate_code(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case NODE_BINARY_EXPR:
            generate_code(node->binary.left);
            printf(" %c ", node->binary.op);
            generate_code(node->binary.right);
            break;

        case NODE_NUMBER:
            printf("%s", node->string_value);
            break;

        case NODE_IDENTIFIER:
            printf("%s", node->identifier);
            break;

        case NODE_IF:
            printf("if (");
            generate_code(node->if_node.condition);
            printf(") ");
            generate_code(node->if_node.then_branch);
            if (node->if_node.else_branch) {
                printf(" else ");
                generate_code(node->if_node.else_branch);
            }
            break;

        case NODE_WHILE:
            printf("while (");
            generate_code(node->while_node.condition);
            printf(") ");
            generate_code(node->while_node.body);
            break;

        case NODE_BLOCK:
            printf("{\n");
            for (size_t i = 0; i < node->block.size; i++) {
                generate_code(node->block.statements[i]);
                printf(";\n");
            }
            printf("}\n");
            break;

        default:
            fprintf(stderr, "Unknown node type!\n");
            break;
    }
}

int main() {
    const char* source_code = "if (a > b) { a = b + 3; }";
    Lexer* lexer = create_lexer(source_code);
    Parser* parser = create_parser(lexer);

    ASTNode* root = parse_statement(parser);
    printf("Generated Code:\n");
    generate_code(root);

    // Cleanup code not shown for brevity

    return 0;
}
// Define a function structure to hold the function name, arguments, and body
typedef struct Function {
    char* name;
    char** arguments;
    size_t arg_count;
    ASTNode* body;
} Function;

#define MAX_FUNCTIONS 256
typedef struct {
    Function functions[MAX_FUNCTIONS];
    size_t func_count;
} FunctionTable;

// Create a global function table
FunctionTable function_table = { .func_count = 0 };

Function* create_function(const char* name, char** arguments, size_t arg_count, ASTNode* body) {
    if (function_table.func_count >= MAX_FUNCTIONS) {
        fprintf(stderr, "Maximum number of functions reached!\n");
        exit(EXIT_FAILURE);
    }
    Function* func = &function_table.functions[function_table.func_count++];
    func->name = strdup(name);
    func->arguments = arguments;
    func->arg_count = arg_count;
    func->body = body;
    return func;
}

Function* find_function(const char* name) {
    for (size_t i = 0; i < function_table.func_count; i++) {
        if (strcmp(function_table.functions[i].name, name) == 0) {
            return &function_table.functions[i];
        }
    }
    return NULL;
}

// Modify the ASTNode structure to support function calls
typedef struct ASTNode {
    ASTNodeType type;
    union {
        // Other node types...
        struct {
            char* function_name;
            ASTNode** arguments;
            size_t arg_count;
        } function_call;
        struct {
            char* function_name;
            char** parameters;
            ASTNode* body;
        } function_def;
    };
} ASTNode;

// Add a new node type for function definitions and calls
#define NODE_FUNCTION_DEF 100
#define NODE_FUNCTION_CALL 101

// Parsing functions for function definition and calls
ASTNode* parse_function_definition(Parser* parser) {
    // Assuming you have tokens for 'function', '(', ')', '{', '}'
    advance(parser); // consume 'function'
    char* function_name = parser->current_token->value;
    advance(parser); // consume function name

    advance(parser); // consume '('
    char** arguments = malloc(sizeof(char*) * MAX_VARIABLES);
    size_t arg_count = 0;
    while (parser->current_token->type != TOKEN_RPAREN) {
        arguments[arg_count++] = parser->current_token->value;
        advance(parser);
        if (parser->current_token->type == TOKEN_COMMA) {
            advance(parser); // consume ','
        }
    }
    advance(parser); // consume ')'

    advance(parser); // consume '{'
    ASTNode* body = parse_block(parser); // Parse the function body
    advance(parser); // consume '}'

    return create_function_def_node(function_name, arguments, arg_count, body);
}

ASTNode* parse_function_call(Parser* parser) {
    char* function_name = parser->current_token->value;
    advance(parser); // consume function name

    advance(parser); // consume '('
    ASTNode** arguments = malloc(sizeof(ASTNode*) * MAX_VARIABLES);
    size_t arg_count = 0;
    while (parser->current_token->type != TOKEN_RPAREN) {
        arguments[arg_count++] = parse_expression(parser);
        if (parser->current_token->type == TOKEN_COMMA) {
            advance(parser); // consume ','
        }
    }
    advance(parser); // consume ')'

    return create_function_call_node(function_name, arguments, arg_count);
}

// AST Node creation functions
ASTNode* create_function_def_node(const char* name, char** params, size_t param_count, ASTNode* body) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_FUNCTION_DEF;
    node->function_def.function_name = strdup(name);
    node->function_def.parameters = params;
    node->function_def.body = body;
    return node;
}

ASTNode* create_function_call_node(const char* name, ASTNode** arguments, size_t arg_count) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_FUNCTION_CALL;
    node->function_call.function_name = strdup(name);
    node->function_call.arguments = arguments;
    node->function_call.arg_count = arg_count;
    return node;
}

// Execution of function calls
void execute_function_call(RuntimeEnvironment* env, ASTNode* node) {
    Function* function = find_function(node->function_call.function_name);
    if (!function) {
        fprintf(stderr, "Undefined function '%s'\n", node->function_call.function_name);
        exit(EXIT_FAILURE);
    }

    // Handle arguments (for simplicity, assuming positional arguments)
    RuntimeEnvironment* local_env = create_runtime_environment();
    for (size_t i = 0; i < function->arg_count; i++) {
        int arg_value = evaluate_expression(env, node->function_call.arguments[i]);
        set_variable(local_env, function->arguments[i], arg_value);
    }

    // Execute the function body in the new local environment
    execute_node(local_env, function->body);
}

// Modify the execute_node function to handle function definitions and calls
void execute_node(RuntimeEnvironment* env, ASTNode* node) {
    switch (node->type) {
        // Existing cases...
        case NODE_FUNCTION_CALL:
            execute_function_call(env, node);
            break;
        case NODE_FUNCTION_DEF:
            create_function(node->function_def.function_name, node->function_def.parameters, node->function_def.arg_count, node->function_def.body);
            break;
        // Other cases...
    }
}

// Update ASTNode to support return values in functions
typedef struct ASTNode {
    ASTNodeType type;
    union {
        // Existing fields...
        struct {
            ASTNode* return_value; // Return value of the function
        } return_node;
        // Function call and definition already exist...
    };
} ASTNode;

#define NODE_RETURN 102 // Add a node type for return statements

// Function for creating return nodes
ASTNode* create_return_node(ASTNode* value) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_RETURN;
    node->return_node.return_value = value;
    return node;
}

// Parsing function for return statements
ASTNode* parse_return_statement(Parser* parser) {
    advance(parser); // consume 'return'
    ASTNode* value = parse_expression(parser); // Parse the return value
    advance(parser); // consume the semicolon
    return create_return_node(value);
}

// Modify function call execution to handle return values
int execute_function(RuntimeEnvironment* env, Function* function, ASTNode** arguments, size_t arg_count) {
    if (arg_count != function->arg_count) {
        fprintf(stderr, "Function '%s' expected %zu arguments but got %zu\n", function->name, function->arg_count, arg_count);
        exit(EXIT_FAILURE);
    }

    // Create a new local environment for the function
    RuntimeEnvironment* local_env = create_runtime_environment();
    for (size_t i = 0; i < arg_count; i++) {
        set_variable(local_env, function->arguments[i], evaluate_expression(env, arguments[i]));
    }

    // Execute the function body and look for return
    int result = 0;
    ASTNode* current_node = function->body;
    for (size_t i = 0; i < function->body->block.size; i++) {
        ASTNode* statement = function->body->block.statements[i];
        if (statement->type == NODE_RETURN) {
            result = evaluate_expression(local_env, statement->return_node.return_value);
            break; // Exit the function after a return statement
        }
        execute_node(local_env, statement); // Execute each statement
    }

    return result;
}

// Modify the parser to recognize the 'return' keyword
ASTNode* parse_statement(Parser* parser) {
    if (parser->current_token->type == TOKEN_RETURN) {
        return parse_return_statement(parser);
    }
    // Other statement types...
    return NULL;
}

// Standard library with a built-in print function
void stdlib_print(RuntimeEnvironment* env, ASTNode** arguments, size_t arg_count) {
    for (size_t i = 0; i < arg_count; i++) {
        int value = evaluate_expression(env, arguments[i]);
        printf("%d ", value);
    }
    printf("\n");
}

// Add built-in functions to the environment
void initialize_stdlib() {
    char* print_arguments[] = { "value" }; // Print takes one argument
    Function* print_function = create_function("print", print_arguments, 1, NULL);
    // Register 'print' function in the function table
    function_table.functions[function_table.func_count++] = *print_function;
}

// Modify function call execution to handle standard library calls
void execute_function_call(RuntimeEnvironment* env, ASTNode* node) {
    if (strcmp(node->function_call.function_name, "print") == 0) {
        stdlib_print(env, node->function_call.arguments, node->function_call.arg_count);
    } else {
        Function* function = find_function(node->function_call.function_name);
        if (!function) {
            fprintf(stderr, "Undefined function '%s'\n", node->function_call.function_name);
            exit(EXIT_FAILURE);
        }
        execute_function(env, function, node->function_call.arguments, node->function_call.arg_count);
    }
}

// Example code snippet showing usage in main function
int main() {
    const char* source_code = "function myFunc(a, b) { return a + b; } print(3 + 2);";
    Lexer* lexer = create_lexer(source_code);
    Parser* parser = create_parser(lexer);

    // Initialize standard library
    initialize_stdlib();

    // Parse and execute the code
    ASTNode* root = parse_block(parser);
    execute_node(NULL, root); // Assuming root is the main program node

    return 0;
}
typedef enum { TRUE = 1, FALSE = 0, NEUTRAL = 0.5 } Trinary;

Trinary is_trinary(double value) {
    if (value == 1) return TRUE;
    else if (value == 0) return FALSE;
    else return NEUTRAL;
}

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

void check_trinary(double value) {
    Trinary result = is_trinary(value);
    if (result == TRUE) {
        printf("The value is True.\n");
    } else if (result == FALSE) {
        printf("The value is False.\n");
    } else {
        printf("The value is Neutral.\n");
    }
}
// Pattern matching via conditional checks in C
int sum(int n, int acc) {
    if (n == 0) {
        return acc;  // Base case, returning the accumulated sum.
    } else {
        return sum(n - 1, acc + n);  // Recursive case, tail recursion.
    }
}

// Function to initiate sum computation
int start_sum(int n) {
    return sum(n, 0);  // Initial call with an accumulator set to 0.
}

void execute_tail_recursive_sum() {
    int n = 10; // Example value for summation
    printf("Sum from 1 to %d: %d\n", n, start_sum(n));
}

int main() {
    execute_tail_recursive_sum();  // Running the tail recursive function
    return 0;
}
#include <stdio.h>
#include <stdlib.h>

// Higher-order function: Apply a function to each element of an array
int* map(int* array, size_t size, int (*func)(int)) {
    int* result = malloc(size * sizeof(int));  // Dynamically allocate memory for result array
    for (size_t i = 0; i < size; i++) {
        result[i] = func(array[i]);  // Apply the function to each element
    }
    return result;
}

// Example function to square each element
int square(int x) {
    return x * x;
}

void execute_map_function() {
    int array[] = {1, 2, 3, 4, 5};
    size_t size = sizeof(array) / sizeof(array[0]);
    
    // Map the 'square' function over the array
    int* squared_array = map(array, size, square);
    
    // Print the results
    printf("Squared array:\n");
    for (size_t i = 0; i < size; i++) {
        printf("%d ", squared_array[i]);
    }
    printf("\n");

    // Free dynamically allocated memory
    free(squared_array);
}

int main() {
    execute_map_function();
    return 0;
}
#include <stdio.h>
#include <stdlib.h>

// Define a lazy evaluated structure
typedef struct {
    int (*evaluate)(void);  // Function pointer for lazy evaluation
} LazyInt;

// Function to create a LazyInt object
LazyInt* lazy_create(int (*func)(void)) {
    LazyInt* lazy_value = (LazyInt*)malloc(sizeof(LazyInt));
    lazy_value->evaluate = func;
    return lazy_value;
}

// Example of a lazily evaluated function
int compute_value() {
    printf("Computing value...\n");
    return 42;  // Example: expensive computation result
}

int execute_lazy(LazyInt* lazy_value) {
    return lazy_value->evaluate();
}

int main() {
    // Creating a lazily evaluated object with a lambda function
    LazyInt* lazy_value = lazy_create(compute_value);

    // Value will only be computed when needed
    printf("Lazy evaluation result: %d\n", execute_lazy(lazy_value));

    // Free allocated memory
    free(lazy_value);
    return 0;
}
#include <stdio.h>
#include <stdlib.h>

typedef void (*EventHandler)(void);  // Define a type for event handler functions

typedef struct {
    EventHandler* handlers;  // Array of event handlers
    size_t handler_count;     // Number of registered handlers
} EventManager;

// Create a new EventManager
EventManager* create_event_manager() {
    EventManager* manager = malloc(sizeof(EventManager));
    manager->handlers = NULL;
    manager->handler_count = 0;
    return manager;
}

// Register an event handler
void register_event_handler(EventManager* manager, EventHandler handler) {
    manager->handlers = realloc(manager->handlers, (manager->handler_count + 1) * sizeof(EventHandler));
    manager->handlers[manager->handler_count] = handler;
    manager->handler_count++;
}

// Trigger all registered event handlers
void trigger_events(EventManager* manager) {
    for (size_t i = 0; i < manager->handler_count; i++) {
        manager->handlers[i]();  // Call each handler
    }
}

// Example event handler function
void on_event_occurred() {
    printf("Event occurred! Handling it now...\n");
}

int main() {
    EventManager* manager = create_event_manager();

    // Register an event handler
    register_event_handler(manager, on_event_occurred);

    // Simulate an event occurring
    printf("Simulating an event...\n");
    trigger_events(manager);

    // Clean up
    free(manager->handlers);
    free(manager);
    return 0;
}
#include <stdio.h>
#include <stdlib.h>

// Define an enumeration for data types
typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_NONE
} DataType;

// Union to hold different types of values
typedef union {
    int int_value;
    float float_value;
    char* string_value;
} Value;

// Define a structure to hold a value and its type
typedef struct {
    DataType type;
    Value value;
} Data;

// Function to print the value based on its type
void print_data(Data data) {
    switch (data.type) {
        case TYPE_INT:
            printf("Integer: %d\n", data.value.int_value);
            break;
        case TYPE_FLOAT:
            printf("Float: %.2f\n", data.value.float_value);
            break;
        case TYPE_STRING:
            printf("String: %s\n", data.value.string_value);
            break;
        default:
            printf("No value\n");
    }
}

int main() {
    // Create different data types
    Data int_data = {TYPE_INT, .value.int_value = 42};
    Data float_data = {TYPE_FLOAT, .value.float_value = 3.14f};
    Data string_data = {TYPE_STRING, .value.string_value = "Hello, UNS-Language!"};

    // Print values using pattern matching
    print_data(int_data);
    print_data(float_data);
    print_data(string_data);

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 256

// Function to write data to a file
void write_to_file(const char* filename, const char* data) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file for writing");
        return;
    }
    fprintf(file, "%s", data);
    fclose(file);
}

// Function to read data from a file
char* read_from_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file for reading");
        return NULL;
    }
    
    char* buffer = malloc(BUFFER_SIZE);
    if (buffer == NULL) {
        perror("Error allocating memory");
        fclose(file);
        return NULL;
    }
    
    fread(buffer, sizeof(char), BUFFER_SIZE, file);
    fclose(file);
    return buffer;
}

int main() {
    const char* filename = "example.txt";
    const char* data_to_write = "Hello, UNS-Language! This is a file I/O example.";

    // Write to file
    write_to_file(filename, data_to_write);
    printf("Data written to %s\n", filename);

    // Read from file
    char* data_read = read_from_file(filename);
    if (data_read != NULL) {
        printf("Data read from file: %s\n", data_read);
        free(data_read);  // Don't forget to free the allocated memory!
    }

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_EOF,
    TOKEN_INVALID
} TokenType;

typedef struct {
    TokenType type;
    char value[32]; // Buffer to hold the value as a string
} Token;

typedef struct {
    int error_code;
    char error_message[256];
} ErrorHandler;

// Function prototypes
Token get_next_token(const char** input);
int evaluate_expression(const char* input, ErrorHandler* err);
void append_to_file(const char* filename, const char* content);
void handle_error(ErrorHandler* err);
void process_input(ErrorHandler* err);
void priority_event_handler(const char* event);
void recursive_function(int n, int* result);
int trinary_logic(int a, int b, int c);

void append_to_file(const char* filename, const char* content) {
    FILE* file = fopen(filename, "a");
    if (!file) {
        perror("File opening failed");
        return;
    }
    fprintf(file, "%s\n", content);
    fclose(file);
}

Token get_next_token(const char** input) {
    while (isspace(**input)) (*input)++; // Skip whitespace

    Token token;
    if (isdigit(**input)) {
        size_t i = 0;
        while (isdigit(**input) && i < 31) {
            token.value[i++] = **input;
            (*input)++;
        }
        token.value[i] = '\0';
        token.type = TOKEN_NUMBER;
        return token;
    }

    if (strchr("+-*/", **input)) {
        token.value[0] = **input;
        token.value[1] = '\0';
        token.type = TOKEN_OPERATOR;
        (*input)++;
        return token;
    }

    token.type = TOKEN_EOF;
    return token;
}

int evaluate_expression(const char* input, ErrorHandler* err) {
    Token current_token = get_next_token(&input);
    if (current_token.type != TOKEN_NUMBER) {
        err->error_code = 1;
        snprintf(err->error_message, sizeof(err->error_message), "Expected a number but found '%s'", current_token.value);
        return 0; // Return 0 in case of error
    }
    
    int result = atoi(current_token.value);

    while (current_token.type != TOKEN_EOF) {
        Token operator_token = get_next_token(&input);
        if (operator_token.type != TOKEN_OPERATOR) {
            err->error_code = 2;
            snprintf(err->error_message, sizeof(err->error_message), "Invalid operator: '%s'", operator_token.value);
            return result;
        }
        
        current_token = get_next_token(&input);
        if (current_token.type != TOKEN_NUMBER) {
            err->error_code = 1;
            snprintf(err->error_message, sizeof(err->error_message), "Expected a number after operator '%s' but found '%s'", operator_token.value, current_token.value);
            return result;
        }
        
        int next_value = atoi(current_token.value);
        
        switch (operator_token.value[0]) {
            case '+': result += next_value; break;
            case '-': result -= next_value; break;
            case '*': result *= next_value; break;
            case '/': 
                if (next_value == 0) {
                    err->error_code = 3;
                    snprintf(err->error_message, sizeof(err->error_message), "Division by zero!");
                    return result; 
                }
                result /= next_value; 
                break;
            default:
                err->error_code = 4;
                snprintf(err->error_message, sizeof(err->error_message), "Unknown operator: '%s'", operator_token.value);
                return result;
        }
    }

    return result;
}

void handle_error(ErrorHandler* err) {
    if (err->error_code != 0) {
        fprintf(stderr, "Error: %s\n", err->error_message);
    }
}

void priority_event_handler(const char* event) {
    printf("Event handled: %s\n", event);
    // Here you can expand event handling logic
}

void process_input(ErrorHandler* err) {
    char expression[256];
    printf("Enter an expression (or 'exit' to quit): ");
    while (fgets(expression, sizeof(expression), stdin)) {
        if (strcmp(expression, "exit\n") == 0) break;
        
        // Remove newline character
        expression[strcspn(expression, "\n")] = '\0';

        // Evaluate expression
        int result = evaluate_expression(expression, err);
        handle_error(err);
        if (err->error_code == 0) {
            printf("Result: %d\n", result);
            append_to_file("calculations.log", expression);
            priority_event_handler("Expression Evaluated");
        }
        printf("Enter an expression (or 'exit' to quit): ");
    }
}

void recursive_function(int n, int* result) {
    if (n <= 0) return;
    *result += n;
    recursive_function(n - 1, result);
}

int trinary_logic(int a, int b, int c) {
    return (a > b) ? ((a > c) ? a : c) : ((b > c) ? b : c);
}

int main() {
    ErrorHandler err = {0, ""}; // Initialize error handler
    process_input(&err);

    // Example usage of recursive function
    int sum = 0;
    recursive_function(5, &sum);
    printf("Sum from 5 to 1: %d\n", sum);

    // Example of trinary logic
    int max_value = trinary_logic(10, 20, 15);
    printf("Max value among 10, 20, 15: %d\n", max_value);

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    TOKEN_COMMAND,
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_EOF,
    TOKEN_INVALID
} TokenType;

typedef struct {
    TokenType type;
    char value[32]; // Buffer for command, number, or operator
} Token;

typedef struct {
    int error_code;
    char error_message[256];
} ErrorHandler;

// Function prototypes
Token get_next_token(const char** input);
int evaluate_expression(const char* input, ErrorHandler* err);
void append_to_file(const char* filename, const char* content);
void handle_error(ErrorHandler* err);
void process_command(const char* command);
void process_input(ErrorHandler* err);

// Function to append results to a log file
void append_to_file(const char* filename, const char* content) {
    FILE* file = fopen(filename, "a");
    if (!file) {
        perror("File opening failed");
        return;
    }
    fprintf(file, "%s\n", content);
    fclose(file);
}

// Tokenizer function for handling commands and expressions
Token get_next_token(const char** input) {
    while (isspace(**input)) (*input)++; // Skip whitespace

    Token token;
    if (strncmp(*input, "log", 3) == 0) {
        strcpy(token.value, "log");
        token.type = TOKEN_COMMAND;
        *input += 3;
        return token;
    }

    if (isdigit(**input)) {
        size_t i = 0;
        while (isdigit(**input) && i < 31) {
            token.value[i++] = **input;
            (*input)++;
        }
        token.value[i] = '\0';
        token.type = TOKEN_NUMBER;
        return token;
    }

    if (strchr("+-*/", **input)) {
        token.value[0] = **input;
        token.value[1] = '\0';
        token.type = TOKEN_OPERATOR;
        (*input)++;
        return token;
    }

    token.type = TOKEN_EOF;
    return token;
}

// Function to evaluate mathematical expressions
int evaluate_expression(const char* input, ErrorHandler* err) {
    Token current_token = get_next_token(&input);
    if (current_token.type != TOKEN_NUMBER) {
        err->error_code = 1;
        snprintf(err->error_message, sizeof(err->error_message), "Expected a number but found '%s'", current_token.value);
        return 0; // Return 0 in case of error
    }

    int result = atoi(current_token.value);

    while (current_token.type != TOKEN_EOF) {
        Token operator_token = get_next_token(&input);
        if (operator_token.type != TOKEN_OPERATOR) {
            err->error_code = 2;
            snprintf(err->error_message, sizeof(err->error_message), "Invalid operator: '%s'", operator_token.value);
            return result;
        }

        current_token = get_next_token(&input);
        if (current_token.type != TOKEN_NUMBER) {
            err->error_code = 1;
            snprintf(err->error_message, sizeof(err->error_message), "Expected a number after operator '%s' but found '%s'", operator_token.value, current_token.value);
            return result;
        }

        int next_value = atoi(current_token.value);

        switch (operator_token.value[0]) {
            case '+': result += next_value; break;
            case '-': result -= next_value; break;
            case '*': result *= next_value; break;
            case '/':
                if (next_value == 0) {
                    err->error_code = 3;
                    snprintf(err->error_message, sizeof(err->error_message), "Division by zero!");
                    return result;
                }
                result /= next_value;
                break;
            default:
                err->error_code = 4;
                snprintf(err->error_message, sizeof(err->error_message), "Unknown operator: '%s'", operator_token.value);
                return result;
        }
    }

    return result;
}

// Error handling function
void handle_error(ErrorHandler* err) {
    if (err->error_code != 0) {
        fprintf(stderr, "Error: %s\n", err->error_message);
    }
}

// Function to process user commands like logging results
void process_command(const char* command) {
    if (strcmp(command, "log") == 0) {
        printf("Logging results...\n");
        // Here, integrate with your logging system or file management
    } else {
        printf("Unknown command: %s\n", command);
    }
}

// Main function for interactive input processing
void process_input(ErrorHandler* err) {
    char expression[256];
    printf("Enter an expression (or 'exit' to quit): ");
    while (fgets(expression, sizeof(expression), stdin)) {
        if (strcmp(expression, "exit\n") == 0) break;

        // Remove newline character
        expression[strcspn(expression, "\n")] = '\0';

        // Check for commands
        if (strncmp(expression, "log", 3) == 0) {
            process_command(expression);
            continue;
        }

        // Evaluate expression
        int result = evaluate_expression(expression, err);
        handle_error(err);
        if (err->error_code == 0) {
            printf("Result: %d\n", result);
            append_to_file("calculations.log", expression);
        }
        printf("Enter an expression (or 'exit' to quit): ");
    }
}

int main() {
    ErrorHandler err = {0}; // Initialize error handler
    process_input(&err); // Start processing input
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>

typedef enum {
    TOKEN_COMMAND,
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_FUNCTION,
    TOKEN_IDENTIFIER,
    TOKEN_EOF,
    TOKEN_INVALID
} TokenType;

typedef struct {
    TokenType type;
    char value[32];
} Token;

typedef struct {
    int error_code;
    char error_message[256];
} ErrorHandler;

typedef struct {
    char function_name[32];
    char function_body[256];
} UserFunction;

// Global function registry
UserFunction function_registry[10];
int function_count = 0;

// Function prototypes
Token get_next_token(const char** input);
int evaluate_expression(const char* input, ErrorHandler* err);
void define_function(const char* definition, ErrorHandler* err);
void append_to_file(const char* filename, const char* content);
void handle_error(ErrorHandler* err);
void process_command(const char* command);
void process_input(ErrorHandler* err);

// Tokenizer function for handling commands and expressions
Token get_next_token(const char** input) {
    while (isspace(**input)) (*input)++; // Skip whitespace

    Token token;
    if (strncmp(*input, "def", 3) == 0) {
        strcpy(token.value, "def");
        token.type = TOKEN_COMMAND;
        *input += 3;
        return token;
    }

    if (isdigit(**input)) {
        size_t i = 0;
        while (isdigit(**input) && i < 31) {
            token.value[i++] = **input;
            (*input)++;
        }
        token.value[i] = '\0';
        token.type = TOKEN_NUMBER;
        return token;
    }

    if (isalpha(**input)) {
        size_t i = 0;
        while (isalnum(**input) && i < 31) {
            token.value[i++] = **input;
            (*input)++;
        }
        token.value[i] = '\0';
        token.type = TOKEN_IDENTIFIER;
        return token;
    }

    if (strchr("+-*/()", **input)) {
        token.value[0] = **input;
        token.value[1] = '\0';
        token.type = TOKEN_OPERATOR;
        (*input)++;
        return token;
    }

    token.type = TOKEN_EOF;
    return token;
}

// Function to define user-defined functions
void define_function(const char* definition, ErrorHandler* err) {
    char function_name[32];
    char function_body[256];

    // Example format: def func_name (args) { body }
    if (sscanf(definition, "def %s (%*[^)]) { %[^\n] }", function_name, function_body) == 2) {
        strcpy(function_registry[function_count].function_name, function_name);
        strcpy(function_registry[function_count].function_body, function_body);
        function_count++;
        printf("Function '%s' defined.\n", function_name);
    } else {
        err->error_code = 5;
        snprintf(err->error_message, sizeof(err->error_message), "Invalid function definition: '%s'", definition);
    }
}

// Function to evaluate mathematical expressions
int evaluate_expression(const char* input, ErrorHandler* err) {
    Token current_token = get_next_token(&input);
    if (current_token.type != TOKEN_NUMBER) {
        err->error_code = 1;
        snprintf(err->error_message, sizeof(err->error_message), "Expected a number but found '%s'", current_token.value);
        return 0; // Return 0 in case of error
    }

    int result = atoi(current_token.value);

    while (current_token.type != TOKEN_EOF) {
        Token operator_token = get_next_token(&input);
        if (operator_token.type != TOKEN_OPERATOR) {
            err->error_code = 2;
            snprintf(err->error_message, sizeof(err->error_message), "Invalid operator: '%s'", operator_token.value);
            return result;
        }

        current_token = get_next_token(&input);
        if (current_token.type != TOKEN_NUMBER) {
            err->error_code = 1;
            snprintf(err->error_message, sizeof(err->error_message), "Expected a number after operator '%s' but found '%s'", operator_token.value, current_token.value);
            return result;
        }

        int next_value = atoi(current_token.value);

        switch (operator_token.value[0]) {
            case '+': result += next_value; break;
            case '-': result -= next_value; break;
            case '*': result *= next_value; break;
            case '/':
                if (next_value == 0) {
                    err->error_code = 3;
                    snprintf(err->error_message, sizeof(err->error_message), "Division by zero!");
                    return result;
                }
                result /= next_value;
                break;
            default:
                err->error_code = 4;
                snprintf(err->error_message, sizeof(err->error_message), "Unknown operator: '%s'", operator_token.value);
                return result;
        }
    }

    return result;
}

// Error handling function
void handle_error(ErrorHandler* err) {
    if (err->error_code != 0) {
        fprintf(stderr, "Error: %s\n", err->error_message);
    }
}

// Function to process user commands
void process_command(const char* command) {
    if (strncmp(command, "def", 3) == 0) {
        ErrorHandler err = {0};
        define_function(command, &err);
        handle_error(&err);
    } else {
        printf("Unknown command: %s\n", command);
    }
}

// Main function for interactive input processing
void process_input(ErrorHandler* err) {
    char expression[256];
    printf("Enter an expression (or 'exit' to quit): ");
    while (fgets(expression, sizeof(expression), stdin)) {
        if (strcmp(expression, "exit\n") == 0) break;

        if (strncmp(expression, "def", 3) == 0) {
            process_command(expression);
        } else {
            int result = evaluate_expression(expression, err);
            handle_error(err);
            printf("Result: %d\n", result);
        }
        printf("Enter an expression (or 'exit' to quit): ");
    }
}

// Entry point of the program
int main() {
    ErrorHandler err = {0};
    process_input(&err);
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>

typedef enum {
    TOKEN_COMMAND,
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_FUNCTION,
    TOKEN_IDENTIFIER,
    TOKEN_EOF,
    TOKEN_INVALID
} TokenType;

typedef struct {
    TokenType type;
    char value[32];
} Token;

typedef struct {
    int error_code;
    char error_message[256];
} ErrorHandler;

// Event structure to handle user-defined events
typedef struct {
    char event_name[32];
    void (*event_handler)(const char* args);
} UserEvent;

UserEvent event_registry[10];
int event_count = 0;

// Function prototypes
Token get_next_token(const char** input);
int evaluate_expression(const char* input, ErrorHandler* err);
void define_event(const char* definition, ErrorHandler* err);
void handle_error(ErrorHandler* err);
void process_command(const char* command);
void interactive_calculator();

// Function to retrieve the next token from the input
Token get_next_token(const char** input) {
    while (isspace(**input)) (*input)++; // Skip whitespace

    Token token;
    if (strncmp(*input, "event", 5) == 0) {
        strcpy(token.value, "event");
        token.type = TOKEN_COMMAND;
        *input += 5;
        return token;
    }

    if (isdigit(**input)) {
        size_t i = 0;
        while (isdigit(**input) && i < 31) {
            token.value[i++] = **input;
            (*input)++;
        }
        token.value[i] = '\0';
        token.type = TOKEN_NUMBER;
        return token;
    }

    if (isalpha(**input)) {
        size_t i = 0;
        while (isalnum(**input) && i < 31) {
            token.value[i++] = **input;
            (*input)++;
        }
        token.value[i] = '\0';
        token.type = TOKEN_IDENTIFIER;
        return token;
    }

    if (strchr("+-*/()", **input)) {
        token.value[0] = **input;
        token.value[1] = '\0';
        token.type = TOKEN_OPERATOR;
        (*input)++;
        return token;
    }

    token.type = TOKEN_EOF;
    return token;
}

// Function to define user events
void define_event(const char* definition, ErrorHandler* err) {
    char event_name[32];
    char event_handler[256];

    // Example format: event event_name { handler }
    if (sscanf(definition, "event %s { %[^\n] }", event_name, event_handler) == 2) {
        strcpy(event_registry[event_count].event_name, event_name);
        event_registry[event_count].event_handler = (void (*)(const char*)) event_handler; // Cast to a function pointer
        event_count++;
        printf("Event '%s' defined.\n", event_name);
    } else {
        err->error_code = 6;
        snprintf(err->error_message, sizeof(err->error_message), "Invalid event definition: '%s'", definition);
    }
}

// Function to evaluate expressions
int evaluate_expression(const char* input, ErrorHandler* err) {
    Token current_token = get_next_token(&input);
    if (current_token.type != TOKEN_NUMBER) {
        err->error_code = 1;
        snprintf(err->error_message, sizeof(err->error_message), "Expected a number but found '%s'", current_token.value);
        return 0; // Return 0 in case of error
    }

    int result = atoi(current_token.value);
    
    // Further evaluation logic...
    
    return result;
}

// Main interactive calculator function
void interactive_calculator() {
    char command[256];
    ErrorHandler err = {0, ""};

    printf("Enter commands (type 'exit' to quit):\n");
    while (1) {
        printf("> ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0; // Remove newline

        if (strcmp(command, "exit") == 0) {
            break;
        }

        process_command(command);
    }
}

// Process commands and invoke corresponding functions
void process_command(const char* command) {
    ErrorHandler err = {0, ""};
    
    if (strncmp(command, "def", 3) == 0) {
        // Define function or event
        if (strncmp(command, "event", 5) == 0) {
            define_event(command, &err);
        } else {
            // Other command processing
        }
    } else {
        // Evaluate expression
        evaluate_expression(command, &err);
    }

    handle_error(&err);
}

// Error handling function
void handle_error(ErrorHandler* err) {
    if (err->error_code != 0) {
        printf("Error (%d): %s\n", err->error_code, err->error_message);
    }
}

int main() {
    interactive_calculator();
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>

typedef enum {
    TOKEN_COMMAND,
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_FUNCTION,
    TOKEN_IDENTIFIER,
    TOKEN_EOF,
    TOKEN_INVALID
} TokenType;

typedef struct {
    TokenType type;
    char value[64];  // Increased buffer size for function names
} Token;

typedef struct {
    int error_code;
    char error_message[256];
} ErrorHandler;

// Function prototype for user-defined functions
typedef struct {
    char name[64];
    char body[256]; // Store function body as a string for simplicity
} UserFunction;

// Global variables for storing functions and events
UserFunction function_registry[10];
int function_count = 0;

UserEvent event_registry[10];
int event_count = 0;

// Function prototypes
Token get_next_token(const char** input);
int evaluate_expression(const char* input, ErrorHandler* err);
void define_function(const char* definition, ErrorHandler* err);
void handle_error(ErrorHandler* err);
void interactive_calculator();
void process_command(const char* command);

// Function to retrieve the next token from the input
Token get_next_token(const char** input) {
    while (isspace(**input)) (*input)++; // Skip whitespace

    Token token;
    if (strncmp(*input, "def", 3) == 0) {
        strcpy(token.value, "def");
        token.type = TOKEN_COMMAND;
        *input += 3;
        return token;
    }

    if (isdigit(**input)) {
        size_t i = 0;
        while (isdigit(**input) && i < 63) {
            token.value[i++] = **input;
            (*input)++;
        }
        token.value[i] = '\0';
        token.type = TOKEN_NUMBER;
        return token;
    }

    if (isalpha(**input)) {
        size_t i = 0;
        while (isalnum(**input) && i < 63) {
            token.value[i++] = **input;
            (*input)++;
        }
        token.value[i] = '\0';
        token.type = TOKEN_IDENTIFIER;
        return token;
    }

    if (strchr("+-*/()", **input)) {
        token.value[0] = **input;
        token.value[1] = '\0';
        token.type = TOKEN_OPERATOR;
        (*input)++;
        return token;
    }

    token.type = TOKEN_EOF;
    return token;
}

// Function to define user functions
void define_function(const char* definition, ErrorHandler* err) {
    char func_name[64];
    char func_body[256];

    // Example format: def func_name { body }
    if (sscanf(definition, "def %s { %[^\n] }", func_name, func_body) == 2) {
        strcpy(function_registry[function_count].name, func_name);
        strcpy(function_registry[function_count].body, func_body);
        function_count++;
        printf("Function '%s' defined.\n", func_name);
    } else {
        err->error_code = 6;
        snprintf(err->error_message, sizeof(err->error_message), "Invalid function definition: '%s'", definition);
    }
}

// Function to evaluate expressions
int evaluate_expression(const char* input, ErrorHandler* err) {
    Token current_token = get_next_token(&input);
    if (current_token.type != TOKEN_NUMBER) {
        err->error_code = 1;
        snprintf(err->error_message, sizeof(err->error_message), "Expected a number but found '%s'", current_token.value);
        return 0; // Return 0 in case of error
    }

    int result = atoi(current_token.value);
    
    // Further evaluation logic...
    
    return result;
}

// Main interactive calculator function
void interactive_calculator() {
    char command[256];
    ErrorHandler err = {0, ""};

    printf("Enter commands (type 'exit' to quit):\n");
    while (1) {
        printf("> ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0; // Remove newline

        if (strcmp(command, "exit") == 0) {
            break;
        }

        process_command(command);
    }
}

// Process commands and invoke corresponding functions
void process_command(const char* command) {
    ErrorHandler err = {0, ""};
    
    if (strncmp(command, "def", 3) == 0) {
        // Define function or event
        define_function(command, &err);
    } else {
        // Evaluate expression
        evaluate_expression(command, &err);
    }

    handle_error(&err);
}

// Error handling function
void handle_error(ErrorHandler* err) {
    if (err->error_code != 0) {
        printf("Error (%d): %s\n", err->error_code, err->error_message);
    }
}

int main() {
    interactive_calculator();
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    TOKEN_COMMAND,
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_FUNCTION,
    TOKEN_IDENTIFIER,
    TOKEN_EOF,
    TOKEN_INVALID
} TokenType;

typedef struct {
    TokenType type;
    char value[64];  // Increased buffer size for function names
} Token;

typedef struct {
    int error_code;
    char error_message[256];
} ErrorHandler;

typedef struct {
    char name[64];
    char body[256]; // Store function body as a string for simplicity
} UserFunction;

UserFunction function_registry[10];
int function_count = 0;

// Function prototypes
Token get_next_token(const char** input);
int evaluate_expression(const char* input, ErrorHandler* err);
void define_function(const char* definition, ErrorHandler* err);
void handle_error(ErrorHandler* err);
void interactive_calculator();
void process_command(const char* command);
int calculate(int a, int b, char operator);
void append_to_file(const char* filename, const char* content);
void load_binary_data(const char* filename);

// Main interactive calculator function
void interactive_calculator() {
    char command[256];
    ErrorHandler err = {0, ""};

    printf("Welcome to the Interactive Calculator. Type 'exit' to quit.\n");
    while (1) {
        printf("> ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0; // Remove newline

        if (strcmp(command, "exit") == 0) {
            break;
        }

        process_command(command);
    }
}

// Function to evaluate expressions
int evaluate_expression(const char* input, ErrorHandler* err) {
    Token current_token = get_next_token(&input);
    if (current_token.type != TOKEN_NUMBER) {
        err->error_code = 1;
        snprintf(err->error_message, sizeof(err->error_message), "Expected a number but found '%s'", current_token.value);
        return 0; // Return 0 in case of error
    }

    int result = atoi(current_token.value);
    current_token = get_next_token(&input); // Get the operator

    while (current_token.type == TOKEN_OPERATOR) {
        char operator = current_token.value[0];
        current_token = get_next_token(&input); // Get the next number
        if (current_token.type != TOKEN_NUMBER) {
            err->error_code = 1;
            snprintf(err->error_message, sizeof(err->error_message), "Expected a number after operator '%c', but found '%s'", operator, current_token.value);
            return 0; // Return 0 in case of error
        }

        result = calculate(result, atoi(current_token.value), operator);
        current_token = get_next_token(&input); // Get the next operator
    }

    return result;
}

// Calculate the result based on the operator
int calculate(int a, int b, char operator) {
    switch (operator) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return (b != 0) ? a / b : 0; // Simple error handling for division by zero
        default: return a; // No change on unknown operator
    }
}

// Function to define user functions
void define_function(const char* definition, ErrorHandler* err) {
    char func_name[64];
    char func_body[256];

    // Example format: def func_name { body }
    if (sscanf(definition, "def %s { %[^\n] }", func_name, func_body) == 2) {
        strcpy(function_registry[function_count].name, func_name);
        strcpy(function_registry[function_count].body, func_body);
        function_count++;
        printf("Function '%s' defined.\n", func_name);
    } else {
        err->error_code = 6;
        snprintf(err->error_message, sizeof(err->error_message), "Invalid function definition: '%s'", definition);
    }
}

// Process commands and invoke corresponding functions
void process_command(const char* command) {
    ErrorHandler err = {0, ""};
    
    if (strncmp(command, "def", 3) == 0) {
        // Define function
        define_function(command, &err);
    } else if (strncmp(command, "loadbin", 7) == 0) {
        // Load binary data
        char filename[64];
        sscanf(command, "loadbin %s", filename);
        load_binary_data(filename);
    } else if (strncmp(command, "append", 6) == 0) {
        // Append to a file
        char filename[64], content[128];
        sscanf(command, "append %s %[^\n]", filename, content);
        append_to_file(filename, content);
    } else {
        // Evaluate expression
        int result = evaluate_expression(command, &err);
        if (err.error_code == 0) {
            printf("Result: %d\n", result);
        }
    }

    handle_error(&err);
}

// Append content to a file
void append_to_file(const char* filename, const char* content) {
    FILE* file = fopen(filename, "a");
    if (file) {
        fprintf(file, "%s\n", content);
        fclose(file);
        printf("Appended '%s' to '%s'.\n", content, filename);
    } else {
        printf("Error: Unable to open file '%s'.\n", filename);
    }
}

// Load binary data from a file
void load_binary_data(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file) {
        fseek(file, 0, SEEK_END);
        long length = ftell(file);
        fseek(file, 0, SEEK_SET);
        unsigned char* buffer = (unsigned char*)malloc(length);
        fread(buffer, 1, length, file);
        fclose(file);

        // Process binary data (for demonstration)
        printf("Loaded %ld bytes from '%s'.\n", length, filename);
        free(buffer);
    } else {
        printf("Error: Unable to open binary file '%s'.\n", filename);
    }
}

// Error handling function
void handle_error(ErrorHandler* err) {
    if (err->error_code != 0) {
        printf("Error (%d): %s\n", err->error_code, err->error_message);
    }
}

// Function to retrieve the next token from the input
Token get_next_token(const char** input) {
    while (isspace(**input)) (*input)++; // Skip whitespace

    Token token;
    if (strncmp(*input, "def", 3) == 0) {
        strcpy(token.value, "def");
        token.type = TOKEN_COMMAND;
        *input += 3;
        return token;
    }

    if (isdigit(**input)) {
        size_t i = 0;
        while (isdigit(**input) && i < 63) {
            token.value[i++] = **input;
            (*input)++;
        }
        token.value[i] = '\0';
        token.type = TOKEN_NUMBER;
        return token;
    }

    if (isalpha(**input)) {
        size_t i = 0;
        while (isalnum(**input) && i < 63) {
            token.value[i++] = **input;
            (*input)++;
        }
        token.value[i] = '\0';
        token.type = TOKEN_IDENTIFIER;
        return token;
    }

    if (strchr("+-*/()", **input)) {
        token.value[0] = **input;
        token.value[1] = '\0';
        token.type = TOKEN_OPERATOR;
        (*input)++;
        return token;
    }

    token.type = TOKEN_EOF;
    return token;
}

int main() {
    interactive_calculator(); // Start the interactive calculator
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    TOKEN_COMMAND,
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_FUNCTION,
    TOKEN_IDENTIFIER,
    TOKEN_EOF,
    TOKEN_INVALID
} TokenType;

typedef struct {
    TokenType type;
    char value[64];
} Token;

typedef struct {
    int error_code;
    char error_message[256];
} ErrorHandler;

typedef struct {
    char name[64];
    char body[256]; // Store function body as a string
} UserFunction;

UserFunction function_registry[10];
int function_count = 0;

// Function prototypes
Token get_next_token(const char** input);
int evaluate_expression(const char* input, ErrorHandler* err);
void define_function(const char* definition, ErrorHandler* err);
void handle_error(ErrorHandler* err);
void interactive_calculator();
void process_command(const char* command);
int calculate(int a, int b, char operator);
void append_to_file(const char* filename, const char* content);
void load_binary_data(const char* filename);
void error_log(const char* filename, const char* message);
void prioritize_event_handling();
void binary_data_operations();

void interactive_calculator() {
    char command[256];
    ErrorHandler err = {0, ""};

    printf("Welcome to the Enhanced Interactive Calculator. Type 'exit' to quit.\n");
    while (1) {
        printf("> ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0; // Remove newline

        if (strcmp(command, "exit") == 0) {
            break;
        }

        process_command(command);
    }
}

Token get_next_token(const char** input) {
    // Implementation for tokenization...
}

int evaluate_expression(const char* input, ErrorHandler* err) {
    Token current_token = get_next_token(&input);
    if (current_token.type != TOKEN_NUMBER) {
        err->error_code = 1;
        snprintf(err->error_message, sizeof(err->error_message), "Expected a number but found '%s'", current_token.value);
        return 0; // Return 0 in case of error
    }

    int result = atoi(current_token.value);
    current_token = get_next_token(&input); // Get the operator

    while (current_token.type == TOKEN_OPERATOR) {
        char operator = current_token.value[0];
        current_token = get_next_token(&input); // Get the next number
        if (current_token.type != TOKEN_NUMBER) {
            err->error_code = 1;
            snprintf(err->error_message, sizeof(err->error_message), "Expected a number after operator '%c', but found '%s'", operator, current_token.value);
            return 0; // Return 0 in case of error
        }

        result = calculate(result, atoi(current_token.value), operator);
        current_token = get_next_token(&input); // Get the next operator
    }

    return result;
}

int calculate(int a, int b, char operator) {
    switch (operator) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return (b != 0) ? a / b : 0; // Simple error handling for division by zero
        default: return a; // No change on unknown operator
    }
}

void define_function(const char* definition, ErrorHandler* err) {
    char func_name[64];
    char func_body[256];

    // Example format: def func_name { body }
    if (sscanf(definition, "def %s { %[^\n] }", func_name, func_body) == 2) {
        strcpy(function_registry[function_count].name, func_name);
        strcpy(function_registry[function_count].body, func_body);
        function_count++;
        printf("Function '%s' defined.\n", func_name);
    } else {
        err->error_code = 6;
        snprintf(err->error_message, sizeof(err->error_message), "Invalid function definition: '%s'", definition);
    }
}

void process_command(const char* command) {
    ErrorHandler err = {0, ""};

    if (strncmp(command, "def", 3) == 0) {
        // Define function
        define_function(command, &err);
    } else if (strncmp(command, "loadbin", 7) == 0) {
        // Load binary data
        char filename[64];
        sscanf(command, "loadbin %s", filename);
        load_binary_data(filename);
    } else if (strncmp(command, "append", 6) == 0) {
        // Append to a file
        char filename[64], content[128];
        sscanf(command, "append %s %[^\n]", filename, content);
        append_to_file(filename, content);
    } else {
        // Evaluate expression
        int result = evaluate_expression(command, &err);
        if (err.error_code != 0) {
            handle_error(&err);
        } else {
            printf("Result: %d\n", result);
        }
    }
}

void handle_error(ErrorHandler* err) {
    printf("Error: %s\n", err->error_message);
    error_log("error_log.txt", err->error_message); // Log the error
}

void error_log(const char* filename, const char* message) {
    FILE* file = fopen(filename, "a");
    if (file) {
        fprintf(file, "Error: %s\n", message);
        fclose(file);
    }
}

void append_to_file(const char* filename, const char* content) {
    FILE* file = fopen(filename, "a");
    if (file) {
        fprintf(file, "%s\n", content);
        fclose(file);
        printf("Content appended to '%s'.\n", filename);
    } else {
        printf("Failed to open file '%s'.\n", filename);
    }
}

void load_binary_data(const char* filename) {
    // Implementation for loading binary data...
    printf("Binary data loaded from '%s'.\n", filename);
}

void prioritize_event_handling() {
    // Implementing a priority-based event handling system
    printf("Priority-based event handling system initialized.\n");
}

void binary_data_operations() {
    // Additional functions for binary data processing
    printf("Binary data operations available.\n");
}

// Entry point
int main() {
    interactive_calculator();
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    TOKEN_COMMAND,
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_FUNCTION,
    TOKEN_IDENTIFIER,
    TOKEN_EOF,
    TOKEN_INVALID
} TokenType;

typedef struct {
    TokenType type;
    char value[64];
} Token;

typedef struct {
    int error_code;
    char error_message[256];
} ErrorHandler;

typedef struct {
    int priority;
    void (*taskFunction)(void);
} Task;

#define MAX_TASKS 10
Task taskQueue[MAX_TASKS];
int taskCount = 0;

// Function prototypes
Token get_next_token(const char** input);
int evaluate_expression(const char* input, ErrorHandler* err);
void define_function(const char* definition, ErrorHandler* err);
void load_binary_data(const char* filename);
void append_to_file(const char* filename, const char* content);
void handle_error(ErrorHandler* err);
void prioritize_event_handling();
void execute_tasks();
void add_task(void (*taskFunction)(void), int priority);
void binary_data_operations();

void load_binary_data(const char* filename) {
    FILE *file = fopen(filename, "rb"); // Open for reading in binary mode
    if (file == NULL) {
        perror("Error opening binary file");
        return;
    }
    
    int data;
    while (fread(&data, sizeof(data), 1, file)) {
        printf("Data read from %s: %d\n", filename, data);
    }
    
    fclose(file);
}

void append_to_file(const char* filename, const char* content) {
    FILE *file = fopen(filename, "ab"); // Open for appending in binary mode
    if (file == NULL) {
        perror("Error opening file for appending");
        return;
    }
    
    fwrite(content, sizeof(char), strlen(content), file);
    fclose(file);
}

void add_task(void (*taskFunction)(void), int priority) {
    if (taskCount < MAX_TASKS) {
        taskQueue[taskCount].taskFunction = taskFunction;
        taskQueue[taskCount].priority = priority;
        taskCount++;
    }
}

void execute_tasks() {
    // Sort tasks by priority (simple bubble sort for demonstration)
    for (int i = 0; i < taskCount - 1; i++) {
        for (int j = 0; j < taskCount - i - 1; j++) {
            if (taskQueue[j].priority < taskQueue[j + 1].priority) {
                Task temp = taskQueue[j];
                taskQueue[j] = taskQueue[j + 1];
                taskQueue[j + 1] = temp;
            }
        }
    }
    // Execute tasks
    for (int i = 0; i < taskCount; i++) {
        taskQueue[i].taskFunction();
    }
}

void task1() {
    printf("Executing Task 1\n");
}

void task2() {
    printf("Executing Task 2\n");
}

void binary_data_operations() {
    // Example of binary data operations
    int data = 12345;
    append_to_file("data.bin", (char*)&data);
    load_binary_data("data.bin");
}

int main() {
    // Define and add tasks
    add_task(task1, 2); // Higher priority
    add_task(task2, 1); // Lower priority
    
    // Execute prioritized tasks
    execute_tasks();
    
    // Perform binary data operations
    binary_data_operations();

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    TOKEN_COMMAND,
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_FUNCTION,
    TOKEN_IDENTIFIER,
    TOKEN_EOF,
    TOKEN_INVALID
} TokenType;

typedef struct {
    TokenType type;
    char value[64];
} Token;

typedef struct {
    int error_code;
    char error_message[256];
} ErrorHandler;

typedef struct EventListener {
    void (*callback)(void);
    struct EventListener* next;
} EventListener;

typedef struct {
    int priority;
    void (*taskFunction)(void);
} Task;

#define MAX_TASKS 10
Task taskQueue[MAX_TASKS];
int taskCount = 0;
EventListener* eventListeners = NULL;

// Function prototypes
Token get_next_token(const char** input);
int evaluate_expression(const char* input, ErrorHandler* err);
void define_function(const char* definition, ErrorHandler* err);
void load_binary_data(const char* filename);
void append_to_file(const char* filename, const char* content);
void handle_error(ErrorHandler* err);
void prioritize_event_handling();
void execute_tasks();
void add_task(void (*taskFunction)(void), int priority);
void trigger_event();
void add_event_listener(void (*callback)(void));
void binary_data_operations();

// Function to dynamically add a task at runtime
void add_task(void (*taskFunction)(void), int priority) {
    if (taskCount < MAX_TASKS) {
        taskQueue[taskCount].taskFunction = taskFunction;
        taskQueue[taskCount].priority = priority;
        taskCount++;
    } else {
        printf("Error: Task queue is full!\n");
    }
}

// Function to execute tasks based on their priority
void execute_tasks() {
    // Sort tasks by priority
    for (int i = 0; i < taskCount - 1; i++) {
        for (int j = 0; j < taskCount - i - 1; j++) {
            if (taskQueue[j].priority < taskQueue[j + 1].priority) {
                Task temp = taskQueue[j];
                taskQueue[j] = taskQueue[j + 1];
                taskQueue[j + 1] = temp;
            }
        }
    }
    // Execute tasks
    for (int i = 0; i < taskCount; i++) {
        taskQueue[i].taskFunction();
    }
}

// Function to trigger all event listeners
void trigger_event() {
    EventListener* current = eventListeners;
    while (current) {
        current->callback();
        current = current->next;
    }
}

// Function to add an event listener
void add_event_listener(void (*callback)(void)) {
    EventListener* new_listener = (EventListener*)malloc(sizeof(EventListener));
    new_listener->callback = callback;
    new_listener->next = eventListeners;
    eventListeners = new_listener;
}

// Sample task functions
void task1() {
    printf("Executing Task 1\n");
}

void task2() {
    printf("Executing Task 2\n");
}

void load_binary_data(const char* filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Error opening binary file");
        return;
    }
    
    int data;
    while (fread(&data, sizeof(data), 1, file)) {
        printf("Data read from %s: %d\n", filename, data);
    }
    
    fclose(file);
}

void append_to_file(const char* filename, const char* content) {
    FILE *file = fopen(filename, "ab");
    if (file == NULL) {
        perror("Error opening file for appending");
        return;
    }
    
    fwrite(content, sizeof(char), strlen(content), file);
    fclose(file);
}

void binary_data_operations() {
    int data = 12345;
    append_to_file("data.bin", (char*)&data);
    load_binary_data("data.bin");
}

// Example listener callback
void on_event_trigger() {
    printf("Event Triggered!\n");
}

int main() {
    // Adding event listener
    add_event_listener(on_event_trigger);
    
    // Defining and adding tasks
    add_task(task1, 2);
    add_task(task2, 1);

    // Execute tasks
    execute_tasks();

    // Triggering an event
    trigger_event();

    // Perform binary data operations
    binary_data_operations();

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <unistd.h>

typedef enum {
    TOKEN_COMMAND,
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_FUNCTION,
    TOKEN_IDENTIFIER,
    TOKEN_EOF,
    TOKEN_INVALID
} TokenType;

typedef struct {
    TokenType type;
    char value[64];
} Token;

typedef struct {
    int error_code;
    char error_message[256];
} ErrorHandler;

typedef struct EventListener {
    void (*callback)(void);
    struct EventListener* next;
} EventListener;

typedef struct {
    int priority;
    void (*taskFunction)(void);
} Task;

#define MAX_TASKS 10
Task taskQueue[MAX_TASKS];
int taskCount = 0;
EventListener* eventListeners = NULL;
pthread_mutex_t taskMutex = PTHREAD_MUTEX_INITIALIZER;

// Function prototypes
void add_task(void (*taskFunction)(void), int priority);
void execute_tasks();
void trigger_event();
void add_event_listener(void (*callback)(void));
void remove_event_listener(void (*callback)(void));
void load_binary_data(const char* filename);
void append_to_file(const char* filename, const char* content);
void handle_error(ErrorHandler* err);
void interactive_cli();

// Function to dynamically add a task at runtime
void add_task(void (*taskFunction)(void), int priority) {
    pthread_mutex_lock(&taskMutex);
    if (taskCount < MAX_TASKS) {
        taskQueue[taskCount].taskFunction = taskFunction;
        taskQueue[taskCount].priority = priority;
        taskCount++;
    } else {
        printf("Error: Task queue is full!\n");
    }
    pthread_mutex_unlock(&taskMutex);
}

// Function to execute tasks based on their priority
void execute_tasks() {
    pthread_mutex_lock(&taskMutex);
    // Sort tasks by priority
    for (int i = 0; i < taskCount - 1; i++) {
        for (int j = 0; j < taskCount - i - 1; j++) {
            if (taskQueue[j].priority < taskQueue[j + 1].priority) {
                Task temp = taskQueue[j];
                taskQueue[j] = taskQueue[j + 1];
                taskQueue[j + 1] = temp;
            }
        }
    }
    // Execute tasks
    for (int i = 0; i < taskCount; i++) {
        taskQueue[i].taskFunction();
    }
    taskCount = 0; // Reset task count after execution
    pthread_mutex_unlock(&taskMutex);
}

// Function to trigger all event listeners
void trigger_event() {
    EventListener* current = eventListeners;
    while (current) {
        current->callback();
        current = current->next;
    }
}

// Function to add an event listener
void add_event_listener(void (*callback)(void)) {
    EventListener* new_listener = (EventListener*)malloc(sizeof(EventListener));
    new_listener->callback = callback;
    new_listener->next = eventListeners;
    eventListeners = new_listener;
    printf("Event listener added.\n");
}

// Function to remove an event listener
void remove_event_listener(void (*callback)(void)) {
    EventListener* current = eventListeners;
    EventListener* previous = NULL;

    while (current) {
        if (current->callback == callback) {
            if (previous) {
                previous->next = current->next;
            } else {
                eventListeners = current->next;
            }
            free(current);
            printf("Event listener removed.\n");
            return;
        }
        previous = current;
        current = current->next;
    }
    printf("Event listener not found.\n");
}

// Sample task functions
void task1() {
    printf("Executing Task 1\n");
}

void task2() {
    printf("Executing Task 2\n");
}

void load_binary_data(const char* filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Error opening binary file");
        return;
    }

    int data;
    while (fread(&data, sizeof(data), 1, file)) {
        printf("Data read from %s: %d\n", filename, data);
    }

    fclose(file);
}

void append_to_file(const char* filename, const char* content) {
    FILE *file = fopen(filename, "ab");
    if (file == NULL) {
        perror("Error opening file for appending");
        return;
    }

    fwrite(content, sizeof(char), strlen(content), file);
    fclose(file);
}

// Error handling
void handle_error(ErrorHandler* err) {
    if (err->error_code != 0) {
        fprintf(stderr, "Error [%d]: %s\n", err->error_code, err->error_message);
    }
}

// Interactive Command-Line Interface
void interactive_cli() {
    char command[64];
    while (1) {
        printf("Enter command (add_task, load_data, append_data, exit): ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0; // Remove newline

        if (strcmp(command, "exit") == 0) {
            break;
        } else if (strcmp(command, "add_task") == 0) {
            add_task(task1, 1);
            printf("Added task1 with priority 1.\n");
        } else if (strcmp(command, "load_data") == 0) {
            load_binary_data("data.bin");
        } else if (strcmp(command, "append_data") == 0) {
            append_to_file("data.bin", "12345");
            printf("Appended data to file.\n");
        } else {
            printf("Invalid command.\n");
        }
        trigger_event(); // Trigger any events after executing commands
    }
}

// Example listener callback
void on_event_trigger() {
    printf("Event Triggered!\n");
}

int main() {
    // Add event listeners
    add_event_listener(on_event_trigger);

    // Start interactive command line interface
    interactive_cli();

    // Execute all tasks in the task queue
    execute_tasks();

    // Cleanup: free event listeners
    EventListener* current = eventListeners;
    while (current) {
        EventListener* to_free = current;
        current = current->next;
        free(to_free);
    }

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <unistd.h>

typedef enum {
    TOKEN_COMMAND,
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_FUNCTION,
    TOKEN_IDENTIFIER,
    TOKEN_EOF,
    TOKEN_INVALID
} TokenType;

typedef struct {
    TokenType type;
    char value[64];
} Token;

typedef struct {
    int error_code;
    char error_message[256];
} ErrorHandler;

typedef struct EventListener {
    void (*callback)(void);
    struct EventListener* next;
} EventListener;

typedef struct {
    int priority;
    void (*taskFunction)(void);
} Task;

#define MAX_TASKS 100
Task* taskQueue[MAX_TASKS];
int taskCount = 0;
EventListener* eventListeners = NULL;
pthread_mutex_t taskMutex = PTHREAD_MUTEX_INITIALIZER;

// Function prototypes
void add_task(void (*taskFunction)(void), int priority);
void execute_tasks();
void trigger_event();
void add_event_listener(void (*callback)(void));
void remove_event_listener(void (*callback)(void));
void load_binary_data(const char* filename);
void append_to_file(const char* filename, const char* content);
void handle_error(ErrorHandler* err);
void interactive_cli();
void* execute_task(void* arg);

// Function to dynamically add a task at runtime
void add_task(void (*taskFunction)(void), int priority) {
    pthread_mutex_lock(&taskMutex);
    if (taskCount < MAX_TASKS) {
        Task* newTask = (Task*)malloc(sizeof(Task));
        newTask->taskFunction = taskFunction;
        newTask->priority = priority;
        taskQueue[taskCount++] = newTask;
    } else {
        printf("Error: Task queue is full!\n");
    }
    pthread_mutex_unlock(&taskMutex);
}

// Function to execute tasks based on their priority
void execute_tasks() {
    pthread_mutex_lock(&taskMutex);
    // Sort tasks by priority using bubble sort (for simplicity)
    for (int i = 0; i < taskCount - 1; i++) {
        for (int j = 0; j < taskCount - i - 1; j++) {
            if (taskQueue[j]->priority < taskQueue[j + 1]->priority) {
                Task* temp = taskQueue[j];
                taskQueue[j] = taskQueue[j + 1];
                taskQueue[j + 1] = temp;
            }
        }
    }
    
    // Execute tasks in separate threads
    for (int i = 0; i < taskCount; i++) {
        pthread_t thread;
        pthread_create(&thread, NULL, execute_task, taskQueue[i]);
        pthread_detach(thread); // Allow thread to run independently
    }
    
    taskCount = 0; // Reset task count after execution
    pthread_mutex_unlock(&taskMutex);
}

// Thread function to execute a task
void* execute_task(void* arg) {
    Task* task = (Task*)arg;
    task->taskFunction();
    free(task); // Free task after execution
    return NULL;
}

// Function to trigger all event listeners
void trigger_event() {
    EventListener* current = eventListeners;
    while (current) {
        current->callback();
        current = current->next;
    }
}

// Function to add an event listener
void add_event_listener(void (*callback)(void)) {
    EventListener* new_listener = (EventListener*)malloc(sizeof(EventListener));
    new_listener->callback = callback;
    new_listener->next = eventListeners;
    eventListeners = new_listener;
    printf("Event listener added.\n");
}

// Function to remove an event listener
void remove_event_listener(void (*callback)(void)) {
    EventListener* current = eventListeners;
    EventListener* previous = NULL;

    while (current) {
        if (current->callback == callback) {
            if (previous) {
                previous->next = current->next;
            } else {
                eventListeners = current->next;
            }
            free(current);
            printf("Event listener removed.\n");
            return;
        }
        previous = current;
        current = current->next;
    }
    printf("Event listener not found.\n");
}

// Sample task functions
void task1() {
    printf("Executing Task 1\n");
}

void task2() {
    printf("Executing Task 2\n");
}

void load_binary_data(const char* filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Error opening binary file");
        return;
    }

    int data;
    while (fread(&data, sizeof(data), 1, file)) {
        printf("Data read from %s: %d\n", filename, data);
    }

    fclose(file);
}

void append_to_file(const char* filename, const char* content) {
    FILE *file = fopen(filename, "ab");
    if (file == NULL) {
        perror("Error opening file for appending");
        return;
    }

    fwrite(content, sizeof(char), strlen(content), file);
    fclose(file);
}

// Error handling
void handle_error(ErrorHandler* err) {
    fprintf(stderr, "Error [%d]: %s\n", err->error_code, err->error_message);
}

// Interactive CLI to manage tasks and events
void interactive_cli() {
    char input[256];
    printf("Interactive CLI: Type commands to manage tasks or event listeners.\n");

    while (1) {
        printf("> ");
        fgets(input, sizeof(input), stdin);
        if (strncmp(input, "add_task", 8) == 0) {
            add_task(task1, 1); // Example task with priority 1
        } else if (strncmp(input, "add_listener", 12) == 0) {
            add_event_listener(task2); // Example listener
        } else if (strncmp(input, "remove_listener", 15) == 0) {
            remove_event_listener(task2); // Example to remove listener
        } else if (strncmp(input, "trigger_event", 13) == 0) {
            trigger_event(); // Trigger all event listeners
        } else if (strncmp(input, "load_data", 9) == 0) {
            load_binary_data("data.bin"); // Load data from a binary file
        } else if (strncmp(input, "append_data", 12) == 0) {
            append_to_file("data.bin", "New Data"); // Append data to a binary file
        } else if (strncmp(input, "exit", 4) == 0) {
            break; // Exit the loop
        } else {
            printf("Invalid command.\n");
        }
    }
}

int main() {
    interactive_cli(); // Start interactive command-line interface
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <unistd.h>

typedef enum {
    TOKEN_COMMAND,
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_FUNCTION,
    TOKEN_IDENTIFIER,
    TOKEN_EOF,
    TOKEN_INVALID
} TokenType;

typedef struct {
    TokenType type;
    char value[64];
} Token;

typedef struct {
    int error_code;
    char error_message[256];
} ErrorHandler;

typedef struct EventListener {
    void (*callback)(void);
    int ref_count;  // Reference count for memory management
    struct EventListener* next;
} EventListener;

typedef struct {
    int priority;
    void (*taskFunction)(void);
    int ref_count;  // Reference count for memory management
} Task;

#define MAX_TASKS 100
Task* taskQueue[MAX_TASKS];
int taskCount = 0;
EventListener* eventListeners = NULL;
pthread_mutex_t taskMutex = PTHREAD_MUTEX_INITIALIZER;

// Error Codes
typedef enum {
    SUCCESS,
    ERROR_TASK_QUEUE_FULL,
    ERROR_MEMORY_ALLOCATION,
    ERROR_FILE_ACCESS,
    ERROR_INVALID_INPUT,
    ERROR_EVENT_LISTENER_NOT_FOUND
} ErrorCode;

// Function prototypes
void add_task(void (*taskFunction)(void), int priority, ErrorHandler* err);
void execute_tasks();
void trigger_event();
void add_event_listener(void (*callback)(void), ErrorHandler* err);
void remove_event_listener(void (*callback)(void), ErrorHandler* err);
void load_binary_data(const char* filename, ErrorHandler* err);
void append_to_file(const char* filename, const char* content, ErrorHandler* err);
void handle_error(ErrorHandler* err);
void interactive_cli();
void* execute_task(void* arg);
void parse_command(const char* input, ErrorHandler* err);

// Function to dynamically add a task at runtime
void add_task(void (*taskFunction)(void), int priority, ErrorHandler* err) {
    pthread_mutex_lock(&taskMutex);
    if (taskCount < MAX_TASKS) {
        Task* newTask = (Task*)malloc(sizeof(Task));
        if (!newTask) {
            err->error_code = ERROR_MEMORY_ALLOCATION;
            snprintf(err->error_message, sizeof(err->error_message), "Memory allocation failed for task.");
            pthread_mutex_unlock(&taskMutex);
            return;
        }
        newTask->taskFunction = taskFunction;
        newTask->priority = priority;
        newTask->ref_count = 1; // Initialize ref count
        taskQueue[taskCount++] = newTask;
    } else {
        err->error_code = ERROR_TASK_QUEUE_FULL;
        snprintf(err->error_message, sizeof(err->error_message), "Task queue is full.");
    }
    pthread_mutex_unlock(&taskMutex);
}

// Function to execute tasks based on their priority
void execute_tasks() {
    pthread_mutex_lock(&taskMutex);
    for (int i = 0; i < taskCount - 1; i++) {
        for (int j = 0; j < taskCount - i - 1; j++) {
            if (taskQueue[j]->priority < taskQueue[j + 1]->priority) {
                Task* temp = taskQueue[j];
                taskQueue[j] = taskQueue[j + 1];
                taskQueue[j + 1] = temp;
            }
        }
    }
    
    for (int i = 0; i < taskCount; i++) {
        pthread_t thread;
        pthread_create(&thread, NULL, execute_task, taskQueue[i]);
        pthread_detach(thread);
        taskQueue[i]->ref_count++; // Increment ref count for executing task
    }
    
    taskCount = 0; // Reset task count after execution
    pthread_mutex_unlock(&taskMutex);
}

// Thread function to execute a task
void* execute_task(void* arg) {
    Task* task = (Task*)arg;
    task->taskFunction();
    if (--task->ref_count == 0) { // Decrement ref count and free if it reaches zero
        free(task);
    }
    return NULL;
}

// Function to trigger all event listeners
void trigger_event() {
    EventListener* current = eventListeners;
    while (current) {
        current->callback();
        current = current->next;
    }
}

// Function to add an event listener
void add_event_listener(void (*callback)(void), ErrorHandler* err) {
    EventListener* new_listener = (EventListener*)malloc(sizeof(EventListener));
    if (!new_listener) {
        err->error_code = ERROR_MEMORY_ALLOCATION;
        snprintf(err->error_message, sizeof(err->error_message), "Memory allocation failed for event listener.");
        return;
    }
    new_listener->callback = callback;
    new_listener->ref_count = 1; // Initialize ref count
    new_listener->next = eventListeners;
    eventListeners = new_listener;
    printf("Event listener added.\n");
}

// Function to remove an event listener
void remove_event_listener(void (*callback)(void), ErrorHandler* err) {
    EventListener* current = eventListeners;
    EventListener* previous = NULL;

    while (current) {
        if (current->callback == callback) {
            if (previous) {
                previous->next = current->next;
            } else {
                eventListeners = current->next;
            }
            if (--current->ref_count == 0) {
                free(current);
            }
            printf("Event listener removed.\n");
            return;
        }
        previous = current;
        current = current->next;
    }
    err->error_code = ERROR_EVENT_LISTENER_NOT_FOUND;
    snprintf(err->error_message, sizeof(err->error_message), "Event listener not found.");
}

// Sample task functions
void task1() {
    printf("Executing Task 1\n");
}

void task2() {
    printf("Executing Task 2\n");
}

// Function to load binary data with error handling
void load_binary_data(const char* filename, ErrorHandler* err) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        err->error_code = ERROR_FILE_ACCESS;
        snprintf(err->error_message, sizeof(err->error_message), "Failed to open file: %s", filename);
        return;
    }
    // ... Load data from file ...
    fclose(file);
}

// Function to append content to a binary file with error handling
void append_to_file(const char* filename, const char* content, ErrorHandler* err) {
    FILE* file = fopen(filename, "ab");
    if (!file) {
        err->error_code = ERROR_FILE_ACCESS;
        snprintf(err->error_message, sizeof(err->error_message), "Failed to open file for appending: %s", filename);
        return;
    }
    // ... Append content to file ...
    fclose(file);
}

// Function to handle errors
void handle_error(ErrorHandler* err) {
    if (err->error_code != SUCCESS) {
        fprintf(stderr, "Error: %s\n", err->error_message);
        err->error_code = SUCCESS; // Reset error code after handling
    }
}

// Function to parse user commands
void parse_command(const char* input, ErrorHandler* err) {
    // ... Simple command parsing implementation ...
    if (strcmp(input, "task1") == 0) {
        add_task(task1, 1, err);
    } else if (strcmp(input, "task2") == 0) {
        add_task(task2, 2, err);
    } else {
        err->error_code = ERROR_INVALID_INPUT;
        snprintf(err->error_message, sizeof(err->error_message), "Invalid command: %s", input);
    }
}

// Interactive command-line interface
void interactive_cli() {
    char input[256];
    ErrorHandler err = {SUCCESS, ""};
    while (1) {
        printf("Enter command: ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0; // Remove newline character
        parse_command(input, &err);
        handle_error(&err);
        if (strcmp(input, "exit") == 0) {
            break;
        }
    }
}

// Main function to run the CLI
int main() {
    interactive_cli();
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <unistd.h>

#define MAX_COMMAND_LENGTH 128
#define MAX_TASKS 100
#define MAX_EVENT_LISTENERS 50

typedef enum {
    TOKEN_COMMAND,
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_IDENTIFIER,
    TOKEN_EOF,
    TOKEN_INVALID
} TokenType;

typedef struct {
    TokenType type;
    char value[64];
} Token;

typedef struct {
    int error_code;
    char error_message[256];
} ErrorHandler;

typedef struct EventListener {
    void (*callback)(void);
    int ref_count;  // Reference count for memory management
    struct EventListener* next;
} EventListener;

typedef struct {
    int priority;
    void (*taskFunction)(void);
    int ref_count;  // Reference count for memory management
} Task;

Task* taskQueue[MAX_TASKS];
int taskCount = 0;
EventListener* eventListeners = NULL;
pthread_mutex_t taskMutex = PTHREAD_MUTEX_INITIALIZER;

// Error Codes
typedef enum {
    SUCCESS,
    ERROR_TASK_QUEUE_FULL,
    ERROR_MEMORY_ALLOCATION,
    ERROR_FILE_ACCESS,
    ERROR_INVALID_INPUT,
    ERROR_EVENT_LISTENER_NOT_FOUND
} ErrorCode;

// Function prototypes
void add_task(void (*taskFunction)(void), int priority, ErrorHandler* err);
void execute_tasks();
void trigger_event();
void add_event_listener(void (*callback)(void), ErrorHandler* err);
void remove_event_listener(void (*callback)(void), ErrorHandler* err);
void interactive_cli();
void parse_command(const char* input, ErrorHandler* err);
void print_error(ErrorHandler* err);
void* execute_task(void* arg);
Token* tokenize(const char* input, int* tokenCount);
void free_tokens(Token* tokens, int tokenCount);

// Function to add a task dynamically
void add_task(void (*taskFunction)(void), int priority, ErrorHandler* err) {
    pthread_mutex_lock(&taskMutex);
    if (taskCount < MAX_TASKS) {
        Task* newTask = (Task*)malloc(sizeof(Task));
        if (!newTask) {
            err->error_code = ERROR_MEMORY_ALLOCATION;
            snprintf(err->error_message, sizeof(err->error_message), "Memory allocation failed for task.");
            pthread_mutex_unlock(&taskMutex);
            return;
        }
        newTask->taskFunction = taskFunction;
        newTask->priority = priority;
        newTask->ref_count = 1; // Initialize ref count
        taskQueue[taskCount++] = newTask;
    } else {
        err->error_code = ERROR_TASK_QUEUE_FULL;
        snprintf(err->error_message, sizeof(err->error_message), "Task queue is full.");
    }
    pthread_mutex_unlock(&taskMutex);
}

// Function to execute tasks based on their priority
void execute_tasks() {
    pthread_mutex_lock(&taskMutex);
    // Sort tasks by priority (simple bubble sort for demonstration)
    for (int i = 0; i < taskCount - 1; i++) {
        for (int j = 0; j < taskCount - i - 1; j++) {
            if (taskQueue[j]->priority < taskQueue[j + 1]->priority) {
                Task* temp = taskQueue[j];
                taskQueue[j] = taskQueue[j + 1];
                taskQueue[j + 1] = temp;
            }
        }
    }
    
    for (int i = 0; i < taskCount; i++) {
        pthread_t thread;
        pthread_create(&thread, NULL, execute_task, taskQueue[i]);
        pthread_detach(thread);
        taskQueue[i]->ref_count++; // Increment ref count for executing task
    }
    
    taskCount = 0; // Reset task count after execution
    pthread_mutex_unlock(&taskMutex);
}

// Thread function to execute a task
void* execute_task(void* arg) {
    Task* task = (Task*)arg;
    task->taskFunction();
    if (--task->ref_count == 0) { // Decrement ref count and free if it reaches zero
        free(task);
    }
    return NULL;
}

// Function to trigger all event listeners
void trigger_event() {
    EventListener* current = eventListeners;
    while (current) {
        current->callback();
        current = current->next;
    }
}

// Function to add an event listener
void add_event_listener(void (*callback)(void), ErrorHandler* err) {
    EventListener* new_listener = (EventListener*)malloc(sizeof(EventListener));
    if (!new_listener) {
        err->error_code = ERROR_MEMORY_ALLOCATION;
        snprintf(err->error_message, sizeof(err->error_message), "Memory allocation failed for event listener.");
        return;
    }
    new_listener->callback = callback;
    new_listener->ref_count = 1; // Initialize ref count
    new_listener->next = eventListeners;
    eventListeners = new_listener;
}

// Function to remove an event listener
void remove_event_listener(void (*callback)(void), ErrorHandler* err) {
    EventListener** current = &eventListeners;
    while (*current) {
        if ((*current)->callback == callback) {
            EventListener* temp = *current;
            *current = (*current)->next;
            if (--temp->ref_count == 0) {
                free(temp);
            }
            return;
        }
        current = &((*current)->next);
    }
    err->error_code = ERROR_EVENT_LISTENER_NOT_FOUND;
    snprintf(err->error_message, sizeof(err->error_message), "Event listener not found.");
}

// Improved command parsing
void parse_command(const char* input, ErrorHandler* err) {
    int tokenCount;
    Token* tokens = tokenize(input, &tokenCount);
    if (!tokens) {
        err->error_code = ERROR_INVALID_INPUT;
        snprintf(err->error_message, sizeof(err->error_message), "Failed to tokenize input.");
        return;
    }

    // Command handling
    if (strcmp(tokens[0].value, "add_task") == 0 && tokenCount == 3) {
        // Assume tokens[1] is priority and tokens[2] is task function name
        int priority = atoi(tokens[1].value);
        // Here we would ideally resolve the function name to a real function
        void (*taskFunction)(void) = NULL; // Placeholder
        add_task(taskFunction, priority, err);
    } else if (strcmp(tokens[0].value, "add_listener") == 0 && tokenCount == 2) {
        // Assume tokens[1] is listener function name
        void (*listenerFunction)(void) = NULL; // Placeholder
        add_event_listener(listenerFunction, err);
    } else {
        err->error_code = ERROR_INVALID_INPUT;
        snprintf(err->error_message, sizeof(err->error_message), "Unknown command or incorrect number of arguments.");
    }

    free_tokens(tokens, tokenCount);
}

// Function to tokenize input command
Token* tokenize(const char* input, int* tokenCount) {
    Token* tokens = malloc(MAX_COMMAND_LENGTH * sizeof(Token));
    *tokenCount = 0;
    char* token = strtok(strdup(input), " ");
    while (token) {
        if (isdigit(token[0])) {
            tokens[*tokenCount].type = TOKEN_NUMBER;
        } else if (isalpha(token[0])) {
            tokens[*tokenCount].type = TOKEN_COMMAND;
        } else {
            tokens[*tokenCount].type = TOKEN_INVALID;
        }
        strncpy(tokens[*tokenCount].value, token, sizeof(tokens[*tokenCount].value) - 1);
        tokens[*tokenCount].value[sizeof(tokens[*tokenCount].value) - 1] = '\0';
        (*tokenCount)++;
        token = strtok(NULL, " ");
    }
    return tokens;
}

// Function to free allocated tokens
void free_tokens(Token* tokens, int tokenCount) {
    free(tokens);
}

// Error handling function
void print_error(ErrorHandler* err) {
    if (err->error_code != SUCCESS) {
        fprintf(stderr, "Error [%d]: %s\n", err->error_code, err->error_message);
        err->error_code = SUCCESS; // Reset after handling
    }
}

// Interactive command-line interface
void interactive_cli() {
    char input[MAX_COMMAND_LENGTH];
    ErrorHandler err = {SUCCESS, ""};

    while (1) {
        printf(">>> ");
        if (!fgets(input, sizeof(input), stdin)) break; // Exit on EOF
        input[strcspn(input, "\n")] = 0; // Remove newline

        parse_command(input, &err);
        print_error(&err);
    }
}

// Sample task function
void sample_task() {
    printf("Executing Sample Task\n");
}

// Sample listener function
void sample_listener() {
    printf("Sample Event Triggered\n");
}

int main() {
    ErrorHandler err = {SUCCESS, ""};

    add_task(sample_task, 1, &err);
    print_error(&err);

    add_event_listener(sample_listener, &err);
    print_error(&err);

    interactive_cli(); // Start command-line interface
    return 0;
}
    new_listener->ref_count = 1; // Initialize ref count
    new_listener->next = eventListeners; // Add to the front of the list
    eventListeners = new_listener;
}

// Function to remove an event listener
void remove_event_listener(void (*callback)(void), ErrorHandler* err) {
    EventListener** current = &eventListeners; // Pointer to the pointer for easy removal
    while (*current) {
        if ((*current)->callback == callback) {
            EventListener* to_remove = *current;
            *current = (*current)->next; // Remove from list
            if (--to_remove->ref_count == 0) {
                free(to_remove);
            }
            return;
        }
        current = &((*current)->next);
    }
    err->error_code = ERROR_EVENT_LISTENER_NOT_FOUND;
    snprintf(err->error_message, sizeof(err->error_message), "Event listener not found.");
}

// Example task functions
void task1() {
    printf("Executing Task 1\n");
}

void task2() {
    printf("Executing Task 2\n");
}

// Function to print errors
void print_error(ErrorHandler* err) {
    if (err->error_code != SUCCESS) {
        fprintf(stderr, "Error %d: %s\n", err->error_code, err->error_message);
        err->error_code = SUCCESS; // Reset error after handling
    }
}

// Command-line interface for user interaction
void interactive_cli() {
    char input[MAX_COMMAND_LENGTH];
    ErrorHandler err = {SUCCESS, ""};
    
    printf("Enter commands (add_task, add_listener, execute, exit):\n");
    while (1) {
        printf("> ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0; // Remove newline character
        
        if (strcmp(input, "exit") == 0) {
            break;
        }
        parse_command(input, &err);
        print_error(&err);
    }
}

// Function to parse user commands
void parse_command(const char* input, ErrorHandler* err) {
    Token* tokens = NULL;
    int tokenCount = 0;

    // Tokenization (dummy implementation for brevity)
    // A full implementation should split the input into tokens based on commands and parameters
    tokens = tokenize(input, &tokenCount);

    if (tokens[0].type == TOKEN_INVALID) {
        err->error_code = ERROR_INVALID_INPUT;
        snprintf(err->error_message, sizeof(err->error_message), "Invalid command.");
        free_tokens(tokens, tokenCount);
        return;
    }

    // Example command handling
    if (strcmp(tokens[0].value, "add_task") == 0 && tokenCount >= 2) {
        int priority = atoi(tokens[1].value);
        add_task(task1, priority, err); // For demonstration, task1 is added
    } else if (strcmp(tokens[0].value, "add_listener") == 0) {
        add_event_listener(task2, err); // For demonstration, task2 is the callback
    } else if (strcmp(tokens[0].value, "execute") == 0) {
        execute_tasks();
    } else {
        err->error_code = ERROR_INVALID_INPUT;
        snprintf(err->error_message, sizeof(err->error_message), "Command not recognized.");
    }
    
    free_tokens(tokens, tokenCount);
}

// Function to tokenize input commands (simple implementation)
Token* tokenize(const char* input, int* tokenCount) {
    Token* tokens = malloc(10 * sizeof(Token)); // Dummy allocation
    char* token = strtok(input, " ");
    *tokenCount = 0;

    while (token != NULL) {
        if (*tokenCount >= 10) break; // Limit to 10 tokens for simplicity
        if (isdigit(token[0])) {
            tokens[*tokenCount].type = TOKEN_NUMBER;
        } else if (isalpha(token[0])) {
            tokens[*tokenCount].type = TOKEN_COMMAND;
        } else {
            tokens[*tokenCount].type = TOKEN_INVALID;
        }
        strcpy(tokens[*tokenCount].value, token);
        (*tokenCount)++;
        token = strtok(NULL, " ");
    }

    return tokens;
}

// Function to free tokens
void free_tokens(Token* tokens, int tokenCount) {
    free(tokens);
}

int main() {
    interactive_cli();
    return 0;
}
#include <pthread.h>

#define MAX_THREADS 10

typedef struct {
    void (*taskFunction)(void);
    int priority;
} ThreadTask;

ThreadTask taskPool[MAX_THREADS];
int taskPoolCount = 0;
pthread_mutex_t task_mutex = PTHREAD_MUTEX_INITIALIZER;

// Thread function to execute tasks
void* thread_function(void* arg) {
    ThreadTask* task = (ThreadTask*)arg;
    task->taskFunction(); // Execute the task
    return NULL;
}

// Function to add a task to the thread pool
void add_task_to_pool(void (*taskFunction)(void), int priority) {
    pthread_mutex_lock(&task_mutex);
    if (taskPoolCount < MAX_THREADS) {
        taskPool[taskPoolCount].taskFunction = taskFunction;
        taskPool[taskPoolCount].priority = priority;
        taskPoolCount++;
    }
    pthread_mutex_unlock(&task_mutex);
}

// Function to execute tasks concurrently
void execute_tasks_concurrently() {
    pthread_t threads[MAX_THREADS];
    int thread_count = 0;

    for (int i = 0; i < taskPoolCount; i++) {
        pthread_create(&threads[thread_count], NULL, thread_function, (void*)&taskPool[i]);
        thread_count++;
    }

    // Wait for all threads to finish
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
}

// Enhanced command parser to allow for more complex commands
void parse_command(const char* input, ErrorHandler* err) {
    Token* tokens = NULL;
    int tokenCount = 0;

    tokens = tokenize(input, &tokenCount);
    
    if (tokenCount == 0) {
        err->error_code = ERROR_INVALID_INPUT;
        snprintf(err->error_message, sizeof(err->error_message), "No command entered.");
        free_tokens(tokens, tokenCount);
        return;
    }

    // Command handling logic
    if (strcmp(tokens[0].value, "add_task") == 0 && tokenCount >= 2) {
        int priority = atoi(tokens[1].value);
        add_task_to_pool(task1, priority);
        printf("Task added to pool with priority %d\n", priority);
    } else if (strcmp(tokens[0].value, "execute_concurrent") == 0) {
        execute_tasks_concurrently();
    } else if (strcmp(tokens[0].value, "remove_listener") == 0) {
        // Assume that listener removal is implemented
        remove_event_listener(task2, err);
    } else {
        err->error_code = ERROR_INVALID_INPUT;
        snprintf(err->error_message, sizeof(err->error_message), "Command not recognized.");
    }

    free_tokens(tokens, tokenCount);
}

int main() {
    interactive_cli(); // Start the CLI for user input
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define MAX_TASKS 100

typedef enum {
    SCHED_FIFO,
    SCHED_RR,
    SCHED_CUSTOM
} SchedulingPolicy;

typedef struct {
    void (*taskFunction)(void);
    int priority;
    SchedulingPolicy policy;
} Task;

typedef struct {
    Task tasks[MAX_TASKS];
    int taskCount;
    pthread_mutex_t mutex;
} TaskScheduler;

TaskScheduler scheduler;

// Initialize the scheduler
void init_scheduler() {
    scheduler.taskCount = 0;
    pthread_mutex_init(&scheduler.mutex, NULL);
}

// Add task with customizable scheduling policy
void add_task(void (*taskFunction)(void), int priority, SchedulingPolicy policy) {
    pthread_mutex_lock(&scheduler.mutex);
    if (scheduler.taskCount < MAX_TASKS) {
        scheduler.tasks[scheduler.taskCount].taskFunction = taskFunction;
        scheduler.tasks[scheduler.taskCount].priority = priority;
        scheduler.tasks[scheduler.taskCount].policy = policy;
        scheduler.taskCount++;
    }
    pthread_mutex_unlock(&scheduler.mutex);
}

// Execute tasks based on scheduling policy
void execute_tasks() {
    for (int i = 0; i < scheduler.taskCount; i++) {
        // You can expand this section for more complex scheduling
        scheduler.tasks[i].taskFunction();
    }
}

// Example Task Function
void sample_task() {
    printf("Executing sample task...\n");
}

// Dynamic Listener Management
typedef struct Listener {
    void (*callback)(void);
    struct Listener* next;
} Listener;

Listener* listeners = NULL;

// Add a listener
void add_listener(void (*callback)(void)) {
    Listener* new_listener = (Listener*)malloc(sizeof(Listener));
    new_listener->callback = callback;
    new_listener->next = listeners;
    listeners = new_listener;
}

// Remove a listener
void remove_listener(void (*callback)(void)) {
    Listener** current = &listeners;
    while (*current) {
        if ((*current)->callback == callback) {
            Listener* temp = *current;
            *current = (*current)->next;
            free(temp);
            return;
        }
        current = &(*current)->next;
    }
}

// Notify all listeners
void notify_listeners() {
    Listener* current = listeners;
    while (current) {
        current->callback();
        current = current->next;
    }
}

// Memory Management (Garbage Collection Placeholder)
void gc_collect() {
    // Placeholder for a simple reference counting mechanism or other garbage collection methods
    // Implement your own logic here as needed
}

// Main Function
int main() {
    init_scheduler();

    // Adding tasks with customizable scheduling
    add_task(sample_task, 1, SCHED_FIFO);
    // Add more tasks with different scheduling policies as needed

    // Notify listeners
    notify_listeners();
    
    // Execute tasks
    execute_tasks();

    // Perform garbage collection
    gc_collect();

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define MAX_TASKS 100

typedef enum {
    SCHED_FIFO,
    SCHED_RR,
    SCHED_CUSTOM
} SchedulingPolicy;

typedef struct {
    void (*taskFunction)(void);
    int priority;
    SchedulingPolicy policy;
} Task;

typedef struct {
    Task tasks[MAX_TASKS];
    int taskCount;
    pthread_mutex_t mutex;
} TaskScheduler;

TaskScheduler scheduler;

// Initialize the scheduler
void init_scheduler() {
    scheduler.taskCount = 0;
    pthread_mutex_init(&scheduler.mutex, NULL);
}

// Add task with customizable scheduling policy
void add_task(void (*taskFunction)(void), int priority, SchedulingPolicy policy) {
    pthread_mutex_lock(&scheduler.mutex);
    if (scheduler.taskCount < MAX_TASKS) {
        scheduler.tasks[scheduler.taskCount].taskFunction = taskFunction;
        scheduler.tasks[scheduler.taskCount].priority = priority;
        scheduler.tasks[scheduler.taskCount].policy = policy;
        scheduler.taskCount++;
    }
    pthread_mutex_unlock(&scheduler.mutex);
}

// Execute tasks based on scheduling policy
void execute_tasks() {
    for (int i = 0; i < scheduler.taskCount; i++) {
        // You can expand this section for more complex scheduling
        scheduler.tasks[i].taskFunction();
    }
}

// Example Task Function
void sample_task() {
    printf("Executing sample task...\n");
}

// Dynamic Listener Management
typedef struct Listener {
    void (*callback)(void);
    struct Listener* next;
} Listener;

Listener* listeners = NULL;

// Add a listener
void add_listener(void (*callback)(void)) {
    Listener* new_listener = (Listener*)malloc(sizeof(Listener));
    new_listener->callback = callback;
    new_listener->next = listeners;
    listeners = new_listener;
}

// Remove a listener
void remove_listener(void (*callback)(void)) {
    Listener** current = &listeners;
    while (*current) {
        if ((*current)->callback == callback) {
            Listener* temp = *current;
            *current = (*current)->next;
            free(temp);
            return;
        }
        current = &(*current)->next;
    }
}

// Notify all listeners
void notify_listeners() {
    Listener* current = listeners;
    while (current) {
        current->callback();
        current = current->next;
    }
}

// Reference Counting for Memory Management
typedef struct RefCounted {
    int ref_count;
    void (*free_func)(void *);
} RefCounted;

void retain(RefCounted *obj) {
    if (obj) {
        obj->ref_count++;
    }
}

void release(RefCounted *obj) {
    if (obj && --obj->ref_count == 0) {
        if (obj->free_func) {
            obj->free_func(obj);
        }
        free(obj);
    }
}

typedef struct {
    RefCounted base;
    int data;
} MyObject;

void free_my_object(void *obj) {
    // Custom cleanup code for MyObject
}

MyObject *create_my_object(int data) {
    MyObject *obj = malloc(sizeof(MyObject));
    obj->base.ref_count = 1;
    obj->base.free_func = free_my_object;
    obj->data = data;
    return obj;
}

// Garbage Collection Placeholder
typedef struct Object {
    int marked;
    struct Object *next;
    // Other fields...
} Object;

Object *heap = NULL;

void mark(Object *obj) {
    if (obj && !obj->marked) {
        obj->marked = 1;
        // Recursively mark referenced objects
        // mark(obj->referenced_object);
    }
}

void sweep() {
    Object **obj = &heap;
    while (*obj) {
        if (!(*obj)->marked) {
            Object *unreached = *obj;
            *obj = unreached->next;
            free(unreached);
        } else {
            (*obj)->marked = 0;
            obj = &(*obj)->next;
        }
    }
}

void gc() {
    // Mark phase
    for (Object *obj = heap; obj; obj = obj->next) {
        mark(obj);
    }
    // Sweep phase
    sweep();
}

// Main Function
int main() {
    init_scheduler();

    // Adding tasks with customizable scheduling
    add_task(sample_task, 1, SCHED_FIFO);
    // Add more tasks with different scheduling policies as needed

    // Notify listeners
    notify_listeners();
    
    // Execute tasks
    execute_tasks();

    // Perform garbage collection
    gc();

    // Memory management example
    MyObject *obj = create_my_object(42);
    retain((RefCounted *)obj);
    release((RefCounted *)obj);
    release((RefCounted *)obj); // This will free the object

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define MAX_TASKS 100

typedef enum {
    SCHED_FIFO,
    SCHED_RR,
    SCHED_SJF, // Shortest Job First
    SCHED_PRIORITY // Priority Queue
} SchedulingPolicy;

typedef struct {
    void (*taskFunction)(void);
    int priority;
    int job_length; // For SJF
    SchedulingPolicy policy;
} Task;

typedef struct {
    Task tasks[MAX_TASKS];
    int taskCount;
    pthread_mutex_t mutex;
} TaskScheduler;

TaskScheduler scheduler;

// Initialize the scheduler
void init_scheduler() {
    scheduler.taskCount = 0;
    pthread_mutex_init(&scheduler.mutex, NULL);
}

// Add task with customizable scheduling policy
void add_task(void (*taskFunction)(void), int priority, int job_length, SchedulingPolicy policy) {
    pthread_mutex_lock(&scheduler.mutex);
    if (scheduler.taskCount < MAX_TASKS) {
        scheduler.tasks[scheduler.taskCount].taskFunction = taskFunction;
        scheduler.tasks[scheduler.taskCount].priority = priority;
        scheduler.tasks[scheduler.taskCount].job_length = job_length; // Length for SJF
        scheduler.tasks[scheduler.taskCount].policy = policy;
        scheduler.taskCount++;
    }
    pthread_mutex_unlock(&scheduler.mutex);
}

// Compare function for SJF
int compare_sjf(const void* a, const void* b) {
    return ((Task*)a)->job_length - ((Task*)b)->job_length;
}

// Execute tasks based on scheduling policy
void execute_tasks() {
    // Sort based on scheduling policy
    if (scheduler.taskCount > 0) {
        Task* task_array = scheduler.tasks;

        // SJF
        qsort(task_array, scheduler.taskCount, sizeof(Task), compare_sjf);

        for (int i = 0; i < scheduler.taskCount; i++) {
            task_array[i].taskFunction();
        }
    }
}

// Example Task Function
void sample_task() {
    printf("Executing sample task...\n");
}

// Dynamic Listener Management
typedef struct Listener {
    void (*callback)(void);
    struct Listener* next;
} Listener;

Listener* listeners = NULL;

// Add a listener with a user-defined callback
void add_listener(void (*callback)(void)) {
    Listener* new_listener = (Listener*)malloc(sizeof(Listener));
    new_listener->callback = callback;
    new_listener->next = listeners;
    listeners = new_listener;
}

// Remove a listener
void remove_listener(void (*callback)(void)) {
    Listener** current = &listeners;
    while (*current) {
        if ((*current)->callback == callback) {
            Listener* temp = *current;
            *current = (*current)->next;
            free(temp);
            return;
        }
        current = &(*current)->next;
    }
}

// Notify all listeners
void notify_listeners() {
    Listener* current = listeners;
    while (current) {
        current->callback();
        current = current->next;
    }
}

// Reference Counting for Memory Management
typedef struct RefCounted {
    int ref_count;
    void (*free_func)(void *);
} RefCounted;

void retain(RefCounted *obj) {
    if (obj) {
        obj->ref_count++;
    }
}

void release(RefCounted *obj) {
    if (obj && --obj->ref_count == 0) {
        if (obj->free_func) {
            obj->free_func(obj);
        }
        free(obj);
    }
}

// Garbage Collection with Cycle Detection
typedef struct Object {
    int marked;
    struct Object *next;
    struct Object **references; // Array of references to other objects
    int ref_count; // Reference count for cycle detection
} Object;

Object *heap = NULL;

void mark(Object *obj) {
    if (obj && !obj->marked) {
        obj->marked = 1;
        // Recursively mark referenced objects
        for (int i = 0; i < obj->ref_count; i++) {
            mark(obj->references[i]);
        }
    }
}

void sweep() {
    Object **obj = &heap;
    while (*obj) {
        if (!(*obj)->marked) {
            Object *unreached = *obj;
            *obj = unreached->next;
            free(unreached);
        } else {
            (*obj)->marked = 0;
            obj = &(*obj)->next;
        }
    }
}

void gc() {
    // Mark phase
    for (Object *obj = heap; obj; obj = obj->next) {
        mark(obj);
    }
    // Sweep phase
    sweep();
}

// Main Function
int main() {
    init_scheduler();

    // Adding tasks with customizable scheduling
    add_task(sample_task, 1, 5, SCHED_SJF); // Example task with job length 5
    // Add more tasks with different scheduling policies and lengths as needed

    // Notify listeners
    notify_listeners();
    
    // Execute tasks
    execute_tasks();

    // Perform garbage collection
    gc();

    return 0;
}
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef enum {
    CONDITION_IS,
    CONDITION_IS_NOT,
    CONDITION_IS_BOTH,
    CONDITION_IS_NEITHER,
    CONDITION_IS_FLEXIBLE,
    CONDITION_IS_NEUTRAL,
    CONDITION_IS_DEPENDENT,
    CONDITION_IS_INDEPENDENT,
    CONDITION_IS_INCOMPATIBLE,
    CONDITION_COMPLETE,
    CONDITION_MOST,
    CONDITION_IRRELEVANT,
    CONDITION_SUSPICIOUS,
    CONDITION_EXCEPTION,
    CONDITION_CUSTOM,
    CONDITION_UNRECOGNIZED,
    CONDITION_POTENTIALLY,
    CONDITION_PERIODIC,
    CONDITION_RANDOM,
    CONDITION_ON,
    CONDITION_OFF
} ConditionType;

typedef struct {
    char* variable;
    ConditionType type;
    char* condition1;
    char* condition2;
} ConditionCheck;

// Function to process the condition check
void process_condition_check(ConditionCheck* check) {
    switch (check->type) {
        case CONDITION_IS:
            printf("Check if %s is %s\n", check->variable, check->condition1);
            break;
        case CONDITION_IS_NOT:
            printf("Check if %s is not %s\n", check->variable, check->condition1);
            break;
        case CONDITION_IS_BOTH:
            printf("Check if %s is both %s and %s\n", check->variable, check->condition1, check->condition2);
            break;
        case CONDITION_IS_NEITHER:
            printf("Check if %s is neither %s nor %s\n", check->variable, check->condition1, check->condition2);
            break;
        case CONDITION_IS_FLEXIBLE:
            printf("Check if %s is flexible in range %s\n", check->variable, check->condition1);
            break;
        case CONDITION_IS_NEUTRAL:
            printf("Check if %s is neutral\n", check->variable);
            break;
        case CONDITION_IS_DEPENDENT:
            printf("Check if %s is dependent on %s\n", check->variable, check->condition1);
            break;
        case CONDITION_IS_INDEPENDENT:
            printf("Check if %s is independent of %s\n", check->variable, check->condition1);
            break;
        case CONDITION_IS_INCOMPATIBLE:
            printf("Check if %s is incompatible with %s\n", check->variable, check->condition1);
            break;
        case CONDITION_COMPLETE:
            printf("Check if process %s is complete\n", check->condition1);
            break;
        case CONDITION_MOST:
            printf("Check if %s is most of %s\n", check->variable, check->condition1);
            break;
        case CONDITION_IRRELEVANT:
            printf("Check if value %s is irrelevant\n", check->condition1);
            break;
        case CONDITION_SUSPICIOUS:
            printf("Check if value %s is suspicious\n", check->condition1);
            break;
        case CONDITION_EXCEPTION:
            printf("Check if condition %s is an exception\n", check->condition1);
            break;
        case CONDITION_CUSTOM:
            printf("Check with custom condition: %s\n", check->condition1);
            break;
        case CONDITION_UNRECOGNIZED:
            printf("Check unrecognized input\n");
            break;
        case CONDITION_POTENTIALLY:
            printf("Check potentially: %s\n", check->condition1);
            break;
        case CONDITION_PERIODIC:
            printf("Check periodic value: %s\n", check->condition1);
            break;
        case CONDITION_RANDOM:
            printf("Check random condition: %s\n", check->condition1);
            break;
        case CONDITION_ON:
            printf("Check if %s is on\n", check->condition1);
            break;
        case CONDITION_OFF:
            printf("Check if %s is off\n", check->condition1);
            break;
        default:
            printf("Unknown condition type.\n");
    }
}

// Function to parse a condition check string
ConditionCheck* parse_condition_check(const char* input) {
    ConditionCheck* check = (ConditionCheck*)malloc(sizeof(ConditionCheck));
    char command[256];
    sscanf(input, "check %s %s %s", command, check->variable, check->condition1);

    if (strcmp(command, ":IS:") == 0) {
        check->type = CONDITION_IS;
    } else if (strcmp(command, ":IS NOT:") == 0) {
        check->type = CONDITION_IS_NOT;
    } else if (strcmp(command, ":IS BOTH:") == 0) {
        check->type = CONDITION_IS_BOTH;
        sscanf(input, "check %s :IS BOTH: %[^,], %s", check->variable, check->condition1, check->condition2);
    }
    // Add more conditions as needed...
    else {
        check->type = CONDITION_UNRECOGNIZED;
    }

    return check;
}

int main() {
    // Example condition checks
    const char* input = "check $variable :IS: $condition";
    ConditionCheck* check = parse_condition_check(input);
    process_condition_check(check);
    free(check);

    return 0;
}
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

typedef enum {
    CONDITION_IS,
    CONDITION_IS_NOT,
    CONDITION_IS_BOTH,
    CONDITION_IS_NEITHER,
    CONDITION_IS_FLEXIBLE,
    CONDITION_IS_NEUTRAL,
    CONDITION_IS_DEPENDENT,
    CONDITION_IS_INDEPENDENT,
    CONDITION_IS_INCOMPATIBLE,
    CONDITION_COMPLETE,
    CONDITION_MOST,
    CONDITION_IRRELEVANT,
    CONDITION_SUSPICIOUS,
    CONDITION_EXCEPTION,
    CONDITION_CUSTOM,
    CONDITION_UNRECOGNIZED,
    CONDITION_POTENTIALLY,
    CONDITION_PERIODIC,
    CONDITION_RANDOM,
    CONDITION_ON,
    CONDITION_OFF
} ConditionType;

typedef struct {
    char* variable;
    ConditionType type;
    char* condition1;
    char* condition2; // For conditions that require two conditions
} ConditionCheck;

// Function to free the ConditionCheck memory
void free_condition_check(ConditionCheck* check) {
    if (check) {
        free(check->variable);
        free(check->condition1);
        free(check->condition2);
        free(check);
    }
}

// Function to process the condition check
void process_condition_check(const ConditionCheck* check) {
    switch (check->type) {
        case CONDITION_IS:
            printf("Check if %s is %s\n", check->variable, check->condition1);
            break;
        case CONDITION_IS_NOT:
            printf("Check if %s is not %s\n", check->variable, check->condition1);
            break;
        case CONDITION_IS_BOTH:
            printf("Check if %s is both %s and %s\n", check->variable, check->condition1, check->condition2);
            break;
        case CONDITION_IS_NEITHER:
            printf("Check if %s is neither %s nor %s\n", check->variable, check->condition1, check->condition2);
            break;
        case CONDITION_IS_FLEXIBLE:
            printf("Check if %s is flexible in range %s\n", check->variable, check->condition1);
            break;
        case CONDITION_IS_NEUTRAL:
            printf("Check if %s is neutral\n", check->variable);
            break;
        case CONDITION_IS_DEPENDENT:
            printf("Check if %s is dependent on %s\n", check->variable, check->condition1);
            break;
        case CONDITION_IS_INDEPENDENT:
            printf("Check if %s is independent of %s\n", check->variable, check->condition1);
            break;
        case CONDITION_IS_INCOMPATIBLE:
            printf("Check if %s is incompatible with %s\n", check->variable, check->condition1);
            break;
        case CONDITION_COMPLETE:
            printf("Check if process %s is complete\n", check->condition1);
            break;
        case CONDITION_MOST:
            printf("Check if %s is most of %s\n", check->variable, check->condition1);
            break;
        case CONDITION_IRRELEVANT:
            printf("Check if value %s is irrelevant\n", check->condition1);
            break;
        case CONDITION_SUSPICIOUS:
            printf("Check if value %s is suspicious\n", check->condition1);
            break;
        case CONDITION_EXCEPTION:
            printf("Check if condition %s is an exception\n", check->condition1);
            break;
        case CONDITION_CUSTOM:
            printf("Check with custom condition: %s\n", check->condition1);
            break;
        case CONDITION_UNRECOGNIZED:
            printf("Check unrecognized input\n");
            break;
        case CONDITION_POTENTIALLY:
            printf("Check potentially: %s\n", check->condition1);
            break;
        case CONDITION_PERIODIC:
            printf("Check periodic value: %s\n", check->condition1);
            break;
        case CONDITION_RANDOM:
            printf("Check random condition: %s\n", check->condition1);
            break;
        case CONDITION_ON:
            printf("Check if %s is on\n", check->condition1);
            break;
        case CONDITION_OFF:
            printf("Check if %s is off\n", check->condition1);
            break;
        default:
            printf("Unknown condition type.\n");
    }
}

// Function to parse a condition check string
ConditionCheck* parse_condition_check(const char* input) {
    ConditionCheck* check = (ConditionCheck*)malloc(sizeof(ConditionCheck));
    check->variable = (char*)malloc(50); // Adjust as necessary
    check->condition1 = (char*)malloc(50);
    check->condition2 = (char*)malloc(50); // For conditions that require two conditions

    // Initialize conditions
    check->condition2[0] = '\0'; // Default to empty

    char command[256];
    char temp[256];
    
    // Using sscanf to read from input
    int num_items = sscanf(input, "check %s %49s %49s", command, check->variable, check->condition1);

    if (num_items < 2) {
        check->type = CONDITION_UNRECOGNIZED; // Malformed input
        return check;
    }

    if (strcmp(command, ":IS:") == 0) {
        check->type = CONDITION_IS;
    } else if (strcmp(command, ":IS NOT:") == 0) {
        check->type = CONDITION_IS_NOT;
    } else if (strcmp(command, ":IS BOTH:") == 0) {
        check->type = CONDITION_IS_BOTH;
        sscanf(input, "check %s :IS BOTH: %49[^,], %49s", check->variable, check->condition1, check->condition2);
    } else if (strcmp(command, ":IS NEITHER:") == 0) {
        check->type = CONDITION_IS_NEITHER;
        sscanf(input, "check %s :IS NEITHER: %49[^,], %49s", check->variable, check->condition1, check->condition2);
    } else if (strcmp(command, ":IS FLEXIBLE:") == 0) {
        check->type = CONDITION_IS_FLEXIBLE;
    } else if (strcmp(command, ":IS NEUTRAL:") == 0) {
        check->type = CONDITION_IS_NEUTRAL;
    } else if (strcmp(command, ":IS DEPENDENT:") == 0) {
        check->type = CONDITION_IS_DEPENDENT;
    } else if (strcmp(command, ":IS INDEPENDENT:") == 0) {
        check->type = CONDITION_IS_INDEPENDENT;
    } else if (strcmp(command, ":IS INCOMPATIBLE:") == 0) {
        check->type = CONDITION_IS_INCOMPATIBLE;
    } else if (strcmp(command, ":COMPLETE:") == 0) {
        check->type = CONDITION_COMPLETE;
    } else if (strcmp(command, ":MOST:") == 0) {
        check->type = CONDITION_MOST;
    } else if (strcmp(command, ":IRRELEVANT:") == 0) {
        check->type = CONDITION_IRRELEVANT;
    } else if (strcmp(command, ":SUSPICIOUS:") == 0) {
        check->type = CONDITION_SUSPICIOUS;
    } else if (strcmp(command, ":EXCEPTION:") == 0) {
        check->type = CONDITION_EXCEPTION;
    } else if (strcmp(command, ":CUSTOM:") == 0) {
        check->type = CONDITION_CUSTOM;
    } else if (strcmp(command, ":POTENTIALLY:") == 0) {
        check->type = CONDITION_POTENTIALLY;
    } else if (strcmp(command, ":PERIODIC:") == 0) {
        check->type = CONDITION_PERIODIC;
    } else if (strcmp(command, ":RANDOM:") == 0) {
        check->type = CONDITION_RANDOM;
    } else if (strcmp(command, ":ON:") == 0) {
        check->type = CONDITION_ON;
    } else if (strcmp(command, ":OFF:") == 0) {
        check->type = CONDITION_OFF;
    } else {
        check->type = CONDITION_UNRECOGNIZED;
    }

    return check;
}

// Main function for demonstration
int main() {
    const char* input = "check myVar :IS: active";
    ConditionCheck* check = parse_condition_check(input);
    process_condition_check(check);
    free_condition_check(check); // Clean up memory
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef enum {
    CONDITION_IS,
    CONDITION_IS_NOT,
    CONDITION_IS_BOTH,
    CONDITION_IS_NEITHER,
    CONDITION_IS_FLEXIBLE,
    CONDITION_IS_NEUTRAL,
    CONDITION_IS_DEPENDENT,
    CONDITION_IS_INDEPENDENT,
    CONDITION_IS_INCOMPATIBLE,
    CONDITION_COMPLETE,
    CONDITION_MOST,
    CONDITION_IRRELEVANT,
    CONDITION_SUSPICIOUS,
    CONDITION_EXCEPTION,
    CONDITION_CUSTOM,
    CONDITION_UNRECOGNIZED,
    CONDITION_POTENTIALLY,
    CONDITION_PERIODIC,
    CONDITION_RANDOM,
    CONDITION_ON,
    CONDITION_OFF
} ConditionType;

typedef struct {
    char* variable;
    ConditionType type;
    char* condition1;
    char* condition2; // For conditions that require two conditions
} ConditionCheck;

// Function to free the ConditionCheck memory
void free_condition_check(ConditionCheck* check) {
    if (check) {
        free(check->variable);
        free(check->condition1);
        free(check->condition2);
        free(check);
    }
}

// Function to process the condition check
void process_condition_check(const ConditionCheck* check) {
    switch (check->type) {
        case CONDITION_IS:
            printf("Check if %s is %s\n", check->variable, check->condition1);
            break;
        case CONDITION_IS_NOT:
            printf("Check if %s is not %s\n", check->variable, check->condition1);
            break;
        case CONDITION_IS_BOTH:
            printf("Check if %s is both %s and %s\n", check->variable, check->condition1, check->condition2);
            break;
        case CONDITION_IS_NEITHER:
            printf("Check if %s is neither %s nor %s\n", check->variable, check->condition1, check->condition2);
            break;
        case CONDITION_IS_FLEXIBLE:
            printf("Check if %s is flexible in range %s\n", check->variable, check->condition1);
            break;
        case CONDITION_IS_NEUTRAL:
            printf("Check if %s is neutral\n", check->variable);
            break;
        case CONDITION_IS_DEPENDENT:
            printf("Check if %s is dependent on %s\n", check->variable, check->condition1);
            break;
        case CONDITION_IS_INDEPENDENT:
            printf("Check if %s is independent of %s\n", check->variable, check->condition1);
            break;
        case CONDITION_IS_INCOMPATIBLE:
            printf("Check if %s is incompatible with %s\n", check->variable, check->condition1);
            break;
        case CONDITION_COMPLETE:
            printf("Check if process %s is complete\n", check->condition1);
            break;
        case CONDITION_MOST:
            printf("Check if %s is most of %s\n", check->variable, check->condition1);
            break;
        case CONDITION_IRRELEVANT:
            printf("Check if value %s is irrelevant\n", check->condition1);
            break;
        case CONDITION_SUSPICIOUS:
            printf("Check if value %s is suspicious\n", check->condition1);
            break;
        case CONDITION_EXCEPTION:
            printf("Check if condition %s is an exception\n", check->condition1);
            break;
        case CONDITION_CUSTOM:
            printf("Check with custom condition: %s\n", check->condition1);
            break;
        case CONDITION_UNRECOGNIZED:
            printf("Check unrecognized input\n");
            break;
        case CONDITION_POTENTIALLY:
            printf("Check potentially: %s\n", check->condition1);
            break;
        case CONDITION_PERIODIC:
            printf("Check periodic value: %s\n", check->condition1);
            break;
        case CONDITION_RANDOM:
            printf("Check random condition: %s\n", check->condition1);
            break;
        case CONDITION_ON:
            printf("Check if %s is on\n", check->condition1);
            break;
        case CONDITION_OFF:
            printf("Check if %s is off\n", check->condition1);
            break;
        default:
            printf("Unknown condition type.\n");
    }
}

// Enhanced parsing function with error handling
ConditionCheck* parse_condition_check(const char* input) {
    ConditionCheck* check = (ConditionCheck*)malloc(sizeof(ConditionCheck));
    check->variable = (char*)malloc(50); // Adjust size as necessary
    check->condition1 = (char*)malloc(50);
    check->condition2 = (char*)malloc(50);
    check->condition2[0] = '\0'; // Default to empty for single condition checks

    char conditionType[30];
    char buffer[256];

    // Parse the input string
    int num_items = sscanf(input, "check %49s :%29s : %49s , %49s", check->variable, conditionType, check->condition1, check->condition2);
    if (num_items < 3) {
        fprintf(stderr, "Error: Malformed input. Expected format: 'check <variable> :<condition> : <condition1> [, <condition2>]'\n");
        free_condition_check(check);
        return NULL;
    }

    // Map condition types to enum values
    if (strcmp(conditionType, "IS") == 0) {
        check->type = CONDITION_IS;
    } else if (strcmp(conditionType, "IS NOT") == 0) {
        check->type = CONDITION_IS_NOT;
    } else if (strcmp(conditionType, "IS BOTH") == 0) {
        check->type = CONDITION_IS_BOTH;
    } else if (strcmp(conditionType, "IS NEITHER") == 0) {
        check->type = CONDITION_IS_NEITHER;
    } else if (strcmp(conditionType, "IS FLEXIBLE") == 0) {
        check->type = CONDITION_IS_FLEXIBLE;
    } else if (strcmp(conditionType, "IS NEUTRAL") == 0) {
        check->type = CONDITION_IS_NEUTRAL;
    } else if (strcmp(conditionType, "IS DEPENDENT") == 0) {
        check->type = CONDITION_IS_DEPENDENT;
    } else if (strcmp(conditionType, "IS INDEPENDENT") == 0) {
        check->type = CONDITION_IS_INDEPENDENT;
    } else if (strcmp(conditionType, "IS INCOMPATIBLE") == 0) {
        check->type = CONDITION_IS_INCOMPATIBLE;
    } else if (strcmp(conditionType, "COMPLETE") == 0) {
        check->type = CONDITION_COMPLETE;
    } else if (strcmp(conditionType, "MOST") == 0) {
        check->type = CONDITION_MOST;
    } else if (strcmp(conditionType, "IRRELEVANT") == 0) {
        check->type = CONDITION_IRRELEVANT;
    } else if (strcmp(conditionType, "SUSPICIOUS") == 0) {
        check->type = CONDITION_SUSPICIOUS;
    } else if (strcmp(conditionType, "EXCEPTION") == 0) {
        check->type = CONDITION_EXCEPTION;
    } else if (strcmp(conditionType, "CUSTOM") == 0) {
        check->type = CONDITION_CUSTOM;
    } else if (strcmp(conditionType, "UNRECOGNIZED") == 0) {
        check->type = CONDITION_UNRECOGNIZED;
    } else if (strcmp(conditionType, "POTENTIALLY") == 0) {
        check->type = CONDITION_POTENTIALLY;
    } else if (strcmp(conditionType, "PERIODIC") == 0) {
        check->type = CONDITION_PERIODIC;
    } else if (strcmp(conditionType, "RANDOM") == 0) {
        check->type = CONDITION_RANDOM;
    } else if (strcmp(conditionType, "ON") == 0) {
        check->type = CONDITION_ON;
    } else if (strcmp(conditionType, "OFF") == 0) {
        check->type = CONDITION_OFF;
    } else {
        fprintf(stderr, "Error: Unrecognized condition type: %s\n", conditionType);
        free_condition_check(check);
        return NULL;
    }

    return check;
}

// Main function to demonstrate the enhanced condition check
int main() {
    const char* input = "check myVar :IS: myCondition, myCondition2"; // Example input
    ConditionCheck* check = parse_condition_check(input);

    if (check) {
        process_condition_check(check);
        free_condition_check(check);
    }

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef enum {
    CONDITION_IS,
    CONDITION_IS_NOT,
    CONDITION_IS_BOTH,
    CONDITION_IS_NEITHER,
    CONDITION_IS_FLEXIBLE,
    CONDITION_IS_NEUTRAL,
    CONDITION_IS_DEPENDENT,
    CONDITION_IS_INDEPENDENT,
    CONDITION_IS_INCOMPATIBLE,
    CONDITION_COMPLETE,
    CONDITION_MOST,
    CONDITION_IRRELEVANT,
    CONDITION_SUSPICIOUS,
    CONDITION_EXCEPTION,
    CONDITION_CUSTOM,
    CONDITION_UNRECOGNIZED,
    CONDITION_POTENTIALLY,
    CONDITION_PERIODIC,
    CONDITION_RANDOM,
    CONDITION_ON,
    CONDITION_OFF
} ConditionType;

typedef struct {
    char* variable;
    ConditionType type;
    char* condition1;
    char* condition2; // For conditions that require two conditions
} ConditionCheck;

// Function to free the ConditionCheck memory
void free_condition_check(ConditionCheck* check) {
    if (check) {
        free(check->variable);
        free(check->condition1);
        free(check->condition2);
        free(check);
    }
}

// Function to process the condition check
void process_condition_check(const ConditionCheck* check) {
    if (!check) return; // Ensure check is valid

    switch (check->type) {
        case CONDITION_IS:
            printf("Check if %s is %s\n", check->variable, check->condition1);
            break;
        case CONDITION_IS_NOT:
            printf("Check if %s is not %s\n", check->variable, check->condition1);
            break;
        case CONDITION_IS_BOTH:
            printf("Check if %s is both %s and %s\n", check->variable, check->condition1, check->condition2);
            break;
        case CONDITION_IS_NEITHER:
            printf("Check if %s is neither %s nor %s\n", check->variable, check->condition1, check->condition2);
            break;
        case CONDITION_IS_FLEXIBLE:
            printf("Check if %s is flexible in range %s\n", check->variable, check->condition1);
            break;
        case CONDITION_IS_NEUTRAL:
            printf("Check if %s is neutral\n", check->variable);
            break;
        case CONDITION_IS_DEPENDENT:
            printf("Check if %s is dependent on %s\n", check->variable, check->condition1);
            break;
        case CONDITION_IS_INDEPENDENT:
            printf("Check if %s is independent of %s\n", check->variable, check->condition1);
            break;
        case CONDITION_IS_INCOMPATIBLE:
            printf("Check if %s is incompatible with %s\n", check->variable, check->condition1);
            break;
        case CONDITION_COMPLETE:
            printf("Check if process %s is complete\n", check->condition1);
            break;
        case CONDITION_MOST:
            printf("Check if %s is most of %s\n", check->variable, check->condition1);
            break;
        case CONDITION_IRRELEVANT:
            printf("Check if value %s is irrelevant\n", check->condition1);
            break;
        case CONDITION_SUSPICIOUS:
            printf("Check if value %s is suspicious\n", check->condition1);
            break;
        case CONDITION_EXCEPTION:
            printf("Check if condition %s is an exception\n", check->condition1);
            break;
        case CONDITION_CUSTOM:
            printf("Check with custom condition: %s\n", check->condition1);
            break;
        case CONDITION_UNRECOGNIZED:
            printf("Check unrecognized input\n");
            break;
        case CONDITION_POTENTIALLY:
            printf("Check potentially: %s\n", check->condition1);
            break;
        case CONDITION_PERIODIC:
            printf("Check periodic value: %s\n", check->condition1);
            break;
        case CONDITION_RANDOM:
            printf("Check random condition: %s\n", check->condition1);
            break;
        case CONDITION_ON:
            printf("Check if %s is on\n", check->condition1);
            break;
        case CONDITION_OFF:
            printf("Check if %s is off\n", check->condition1);
            break;
        default:
            printf("Unknown condition type.\n");
    }
}

// Enhanced parsing function with error handling
ConditionCheck* parse_condition_check(const char* input) {
    ConditionCheck* check = (ConditionCheck*)malloc(sizeof(ConditionCheck));
    if (!check) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL; // Early return on memory allocation failure
    }

    check->variable = (char*)malloc(50);
    check->condition1 = (char*)malloc(50);
    check->condition2 = (char*)malloc(50);
    if (!check->variable || !check->condition1 || !check->condition2) {
        fprintf(stderr, "Memory allocation failed for conditions\n");
        free_condition_check(check);
        return NULL;
    }

    // Initialize all conditions to empty
    memset(check->variable, 0, 50);
    memset(check->condition1, 0, 50);
    memset(check->condition2, 0, 50);
    check->condition2[0] = '\0'; // Default to empty for single condition checks

    char conditionType[30];
    // Parse the input string
    int num_items = sscanf(input, "check %49s : %29s : %49s , %49s", 
        check->variable, conditionType, check->condition1, check->condition2);

    // Determine the condition type
    if (num_items < 3) {
        fprintf(stderr, "Input error: not enough parameters\n");
        free_condition_check(check);
        return NULL;
    }

    if (strcmp(conditionType, "IS") == 0) {
        check->type = CONDITION_IS;
    } else if (strcmp(conditionType, "IS_NOT") == 0) {
        check->type = CONDITION_IS_NOT;
    } else if (strcmp(conditionType, "IS_BOTH") == 0) {
        check->type = CONDITION_IS_BOTH;
    } else if (strcmp(conditionType, "IS_NEITHER") == 0) {
        check->type = CONDITION_IS_NEITHER;
    } else if (strcmp(conditionType, "IS_FLEXIBLE") == 0) {
        check->type = CONDITION_IS_FLEXIBLE;
    } else if (strcmp(conditionType, "IS_NEUTRAL") == 0) {
        check->type = CONDITION_IS_NEUTRAL;
    } else if (strcmp(conditionType, "IS_DEPENDENT") == 0) {
        check->type = CONDITION_IS_DEPENDENT;
    } else if (strcmp(conditionType, "IS_INDEPENDENT") == 0) {
        check->type = CONDITION_IS_INDEPENDENT;
    } else if (strcmp(conditionType, "IS_INCOMPATIBLE") == 0) {
        check->type = CONDITION_IS_INCOMPATIBLE;
    } else if (strcmp(conditionType, "COMPLETE") == 0) {
        check->type = CONDITION_COMPLETE;
    } else if (strcmp(conditionType, "MOST") == 0) {
        check->type = CONDITION_MOST;
    } else if (strcmp(conditionType, "IRRELEVANT") == 0) {
        check->type = CONDITION_IRRELEVANT;
    } else if (strcmp(conditionType, "SUSPICIOUS") == 0) {
        check->type = CONDITION_SUSPICIOUS;
    } else if (strcmp(conditionType, "EXCEPTION") == 0) {
        check->type = CONDITION_EXCEPTION;
    } else if (strcmp(conditionType, "CUSTOM") == 0) {
        check->type = CONDITION_CUSTOM;
    } else if (strcmp(conditionType, "POTENTIALLY") == 0) {
        check->type = CONDITION_POTENTIALLY;
    } else if (strcmp(conditionType, "PERIODIC") == 0) {
        check->type = CONDITION_PERIODIC;
    } else if (strcmp(conditionType, "RANDOM") == 0) {
        check->type = CONDITION_RANDOM;
    } else if (strcmp(conditionType, "ON") == 0) {
        check->type = CONDITION_ON;
    } else if (strcmp(conditionType, "OFF") == 0) {
        check->type = CONDITION_OFF;
    } else {
        check->type = CONDITION_UNRECOGNIZED;
    }

    return check;
}

// Example main function to demonstrate usage
int main() {
    const char* input = "check variable1 : IS : condition1 , condition2";
    ConditionCheck* check = parse_condition_check(input);

    if (check) {
        process_condition_check(check);
        free_condition_check(check);
    }

    return 0;
}
