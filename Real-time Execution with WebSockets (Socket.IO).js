// Frontend
socket.on('compile-line', function(code) {
  fetch('/compile-line', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json'
    },
    body: JSON.stringify({code: code})
  }).then(response => response.json())
    .then(data => {
      // Output the compiled result or error
      if (data.error) {
        // Display the error
        displayError(data.error);
      } else {
        // Display the compiled output
        displayOutput(data.output);
      }
    });
});
