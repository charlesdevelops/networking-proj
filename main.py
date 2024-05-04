import socket

HOST = ''  # Listen on all available interfaces
PORT = 2401  # Port number to listen on (from the command-line arguments)

# Create a TCP/IP socket
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Allow reuse of the socket address
server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

# Bind the socket to the host and port
server_socket.bind((HOST, PORT))

# Listen for incoming connections
server_socket.listen(5)  # Queue up to 5 connection requests

print(f'Server listening on {HOST}:{PORT}')

def response_html():
    html_content = """
<!DOCTYPE html>
<html>
<head>
    <title>Hello, World!</title>
</head>
<body>
    <h1>Hello, World!</h1>
    <p>Served via Python <3</p>
</body>
</html>
"""
    http_response = f"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: {len(html_content)}\r\n\r\n{html_content}"
    return http_response

while True:
    # Wait for a connection
    print('Waiting for a connection...')
    client_socket, client_address = server_socket.accept()

    try:
        print(f'Connection from {client_address}')

        # Receive the HTTP request from the web browser
        request = client_socket.recv(1024)
        print(f'Received request: {request.decode()}')

        # Generate the HTML response
        html_response = response_html()

        # Send the HTML response back to the web browser
        client_socket.sendall(html_response.encode())

    finally:
        # Clean up the connection
        client_socket.close()
