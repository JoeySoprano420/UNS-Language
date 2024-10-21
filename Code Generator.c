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
