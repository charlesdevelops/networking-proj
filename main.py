import socket

# TCP/IP socket for web browser communication
HOST_TCP = ''
PORT_TCP = 2401

# UDP/IP socket for station-to-station communication
HOST_UDP = ''
PORT_UDP = 2408
NEIGHBORS = [('host2', 2560), ('host3', 2566)]  # Neighbor station addresses

# Create a TCP/IP socket
server_socket_tcp = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket_tcp.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server_socket_tcp.bind((HOST_TCP, PORT_TCP))
server_socket_tcp.listen(20)

# Create a UDP/IP socket
server_socket_udp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
server_socket_udp.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server_socket_udp.bind((HOST_UDP, PORT_UDP))

print(f'Server listening on TCP {HOST_TCP}:{PORT_TCP} and UDP {HOST_UDP}:{PORT_UDP}')

def handle_tcp_connection(client_socket_tcp, client_address_tcp):
    try:
        print(f'Connection from {client_address_tcp}')

        # Receive the HTTP request from the web browser
        request = client_socket_tcp.recv(1024).decode()
        print(f'Received request: {request}')

        # Generate the HTTP/1.1 response
        response = create_http_response()

        # Send the HTTP/1.1 response back to the web browser
        client_socket_tcp.sendall(response)

    finally:
        # Clean up the connection
        client_socket_tcp.close()

def handle_udp_datagram(data, address):
    print(f'Received UDP datagram from {address}: {data.decode()}')
    # Process the received UDP datagram
    # ...

    # Send a response back to the neighbor station (if needed)
    response = b'UDP response from this station'
    server_socket_udp.sendto(response, address)

    # Optionally, forward the datagram to other neighbor stations
    for neighbor in NEIGHBORS:
        if neighbor != address:
            server_socket_udp.sendto(data, neighbor)

def create_http_response():
    html_content = """
<!DOCTYPE html>
<html>
<head>
    <title>Hello, World!</title>
</head>
<body>
    <h1>Hello, World!</h1>
    <p>Served from Python <3</p>
</body>
</html>
"""

    http_response = f"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: {len(html_content)}\r\n\r\n{html_content}"
    return http_response.encode()

while True:
    # Handle incoming TCP connections from web browsers
    client_socket_tcp, client_address_tcp = server_socket_tcp.accept()
    handle_tcp_connection(client_socket_tcp, client_address_tcp)

    # Handle incoming UDP datagrams from other station servers
    data, address = server_socket_udp.recvfrom(1024)
    handle_udp_datagram(data, address)