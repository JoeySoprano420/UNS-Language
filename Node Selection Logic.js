let selectedNode = null;

function selectNode(nodeElement) {
    if (selectedNode) {
        selectedNode.classList.remove('selected'); // Remove previous selection
    }
    selectedNode = nodeElement;
    selectedNode.classList.add('selected'); // Highlight selected node

    // Handle commands via hashwords or summoning commands
    // Example: parseCommand(selectedNode.innerHTML);
}

// Add to the existing node creation logic
node.onclick = () => selectNode(node);
