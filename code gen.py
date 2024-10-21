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

class CCodeGenerator:
    def __init__(self, nodes):
        self.nodes = nodes

    def generate_c_code(self):
        c_code = ""
        for node in self.nodes:
            inputs = ', '.join([f"float {inp}" for inp in node.inputs])
            outputs = ', '.join([f"float {out}" for out in node.outputs])
            process = node.process.replace("$", "")
            c_code += f"void {node.name.replace('$', '')}({inputs}, {outputs}) {{\n"
            c_code += f"    {process};\n"
            c_code += "}\n\n"
        return c_code

# Generate C code from parsed nodes
c_generator = CCodeGenerator(nodes)
print(c_generator.generate_c_code())

class PythonGenerator:
    def __init__(self, nodes):
        self.nodes = nodes

    def generate_py_code(self):
        py_code = ""
        for node in self.nodes:
            inputs = ', '.join(node.inputs)
            outputs = ', '.join(node.outputs)
            process = node.process
            py_code += f"def {node.name.replace('$', '')}({inputs}):\n"
            py_code += f"    {outputs} = None\n"
            py_code += f"    {process}\n"
            py_code += f"    return {outputs}\n\n"
        return py_code

# Generate Python code from parsed nodes
py_generator = PythonGenerator(nodes)
print(py_generator.generate_py_code())

