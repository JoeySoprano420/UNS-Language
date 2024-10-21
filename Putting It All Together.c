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
