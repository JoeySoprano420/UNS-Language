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
