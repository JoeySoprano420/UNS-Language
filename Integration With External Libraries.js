const socket = new WebSocket('ws://yourserver.com/socket');

socket.onmessage = function(event) {
    const data = JSON.parse(event.data);
    // Update UI based on real-time data received
};
