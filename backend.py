# ML-Plus backend code
class Node:
    def __init__(self, name, process_func):
        self.name = name
        self.process_func = process_func
        self.connections = []

    def connect(self, other_node):
        self.connections.append(other_node)

    def execute(self, input_data):
        output_data = self.process_func(input_data)
        for connection in self.connections:
            connection.execute(output_data)

# Example processing function
def process_data(data):
    # Processing logic here
    return processed_data

# Create nodes
data_node = Node("Data Input", lambda x: x)  # Identity for input
process_node = Node("Data Processor", process_data)

# Connect nodes
data_node.connect(process_node)

# Example invocation from frontend
input_data = get_input_data()  # Function to get data from HTML-C
data_node.execute(input_data)
