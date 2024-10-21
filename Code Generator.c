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
