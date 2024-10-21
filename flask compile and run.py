from flask import Flask, request, jsonify
import subprocess

app = Flask(__name__)

@app.route('/compile', methods=['POST'])
def compile_code():
    language = request.json['language']
    code = request.json['code']
    
    if language == 'python':
        compiled_code = PythonGenerator(UNSParser(code).get_nodes()).generate_py_code()
    elif language == 'c':
        compiled_code = CCodeGenerator(UNSParser(code).get_nodes()).generate_c_code()
    elif language == 'javascript':
        compiled_code = JavaScriptGenerator(UNSParser(code).get_nodes()).generate_js_code()
    
    # Return the generated code
    return jsonify({"compiled_code": compiled_code})

if __name__ == '__main__':
    app.run(debug=True)
