class JavaScriptGenerator:
    def __init__(self, nodes):
        self.nodes = nodes

    def generate_js_code(self):
        js_code = ""
        for node in self.nodes:
            inputs = ', '.join(node.inputs)
            outputs = ', '.join(node.outputs)
            process = node.process
            js_code += f"function {node.name.replace('$', '')}({inputs}) {{\n"
            js_code += f"    let {outputs};\n"
            js_code += f"    {process};\n"
            js_code += f"    return {outputs};\n"
            js_code += "}\n\n"
        return js_code

# Generate JavaScript code from parsed nodes
js_generator = JavaScriptGenerator(nodes)
print(js_generator.generate_js_code())
