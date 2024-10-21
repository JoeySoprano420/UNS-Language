import re

# Define the UNS Node structure
class Node:
    def __init__(self, name, inputs, outputs, process):
        self.name = name
        self.inputs = inputs
        self.outputs = outputs
        self.process = process

# Parser to extract UNS syntax
class UNSParser:
    def __init__(self, code):
        self.code = code
        self.nodes = []

    def parse(self):
        node_pattern = re.compile(r'node\s+(\$\w+)\s*\{\s*input:\s*(.*)\s*output:\s*(.*)\s*process:\s*(.*)\s*\}')
        matches = node_pattern.findall(self.code)

        for match in matches:
            name = match[0].strip()
            inputs = [inp.strip() for inp in match[1].split(',')]
            outputs = [out.strip() for out in match[2].split(',')]
            process = match[3].strip()
            node = Node(name, inputs, outputs, process)
            self.nodes.append(node)

    def get_nodes(self):
        return self.nodes

# Example UNS code
uns_code = """
node $CalculateAverage {
    input: $value1, $value2, $value3
    output: $average
    process: $average = ($value1 + $value2 + $value3) / 3
}
"""

parser = UNSParser(uns_code)
parser.parse()
nodes = parser.get_nodes()

# For demonstration, print parsed nodes
for node in nodes:
    print(f"Node: {node.name}, Inputs: {node.inputs}, Outputs: {node.outputs}, Process: {node.process}")
