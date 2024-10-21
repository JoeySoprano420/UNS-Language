function processNode(node) {
    fetch('/process_node', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify(node),
    })
    .then(response => response.json())
    .then(data => {
        console.log('Processing result:', data.result);
        // Update the UI with the result
    })
    .catch((error) => {
        console.error('Error:', error);
    });
}
