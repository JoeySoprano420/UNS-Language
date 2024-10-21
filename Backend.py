from flask import Flask, request, jsonify

app = Flask(__name__)

@app.route('/process_node', methods=['POST'])
def process_node():
    node_data = request.json
    # Example: Check node type and execute the corresponding logic
    if node_data['type'] == 'ML':
        processed_data = execute_ml_node(node_data['parameters'])
    elif node_data['type'] == 'HTML':
        rendered_html = execute_html_node(node_data['parameters'])
    return jsonify({'result': processed_data or rendered_html})

def execute_ml_node(parameters):
    # Implement ML logic based on parameters
    return "ML Result"

def execute_html_node(parameters):
    # Implement HTML rendering logic based on parameters
    return "<div>Rendered HTML</div>"

if __name__ == '__main__':
    app.run(debug=True)
