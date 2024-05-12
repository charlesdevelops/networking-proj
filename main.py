import socket
import argparse
import select
import re

MAX_NAME_LENGTH = 100

def parse_neighbor_port(arg):
    try:
        hostname, port = arg.split(':')
        return hostname, int(port)
    except ValueError:
        raise argparse.ArgumentTypeError("Neighbor port must be in the format 'hostname:port'")

def parse_args():
    parser = argparse.ArgumentParser(description='Station Server')

    parser.add_argument('station_name', type=str, help='Name of the station')
    parser.add_argument('browser_port', type=int, help='Port number for browser (TCP)')
    parser.add_argument('query_port', type=int, help='Port number for queries (UDP)')
    parser.add_argument('neighbour_ports', nargs='+', type=parse_neighbor_port, help='List of neighbor ports in the format hostname:port')

    return parser.parse_args()

args = parse_args()

station_name = args.station_name
PORT_TCP = args.browser_port
PORT_UDP = args.query_port
NEIGHBORS = args.neighbour_ports


# debugging
print("Station Name:", station_name)
print("Browser Port:", PORT_TCP)
print("Query Port:", PORT_UDP)
print("Neighbour Ports:", NEIGHBORS)

# TCP/IP socket for web browser communication
HOST_TCP = ''

# UDP/IP socket for station-to-station communication
HOST_UDP = ''

# Create a TCP/IP socket
server_socket_tcp = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket_tcp.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server_socket_tcp.bind((HOST_TCP, PORT_TCP))
server_socket_tcp.listen(5)

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

def parse_payload(payload_str):
    parts = payload_str.split()
    found = int(parts[0])
    hops = int(parts[1])
    current = int(parts[2])
    stations = parts[3:3+hops]
    addresses = parts[3+hops:3+hops*2]
    routes = parts[3+hops*2:3+hops*3]
    times = parts[3+hops*3:3+hops*4]
    destination = parts[3+hops*4]
    source = parts[3+hops*4+1]
    return found, hops, current, stations, addresses, routes, times, destination, source

def handle_udp_datagram(data, address):
    print(f'Received UDP datagram from {address}: {data.decode()}')

    # Parse the received payload
    payload_str = data.decode().rstrip(';')
    found, hops, current, stations, addresses, routes, times, destination, source = parse_payload(payload_str)

    # Check if this station is the destination
    if station_name == destination:
        if found == 1:
            # Update routes and times
            updated_routes = []
            updated_times = []
            for i in range(current-1, -1, -1):
                updated_routes.append(routes[i])
                updated_times.append(times[i])

            # Construct the updated payload
            updated_payload = " ".join([
                str(2), str(hops), str(current),
                *stations, *addresses,
                *updated_routes, *updated_times,
                destination, source
            ]) + ";"

            # Send the updated payload back to the source
            server_socket_udp.sendto(updated_payload.encode(), address)

        elif found == 3:
            # Route found successfully
            print(f"Route found from {source} to {destination}")
            print(f"Number of hops: {hops}")
            print(f"Station names: {stations}")
            print(f"Station addresses: {addresses}")
            print(f"Routes: {routes}")
            print(f"Departure times: {times}")

    else:
        # Forward the datagram to the appropriate neighbor station
        for neighbor_address in NEIGHBORS:
            if neighbor_address != address:
                server_socket_udp.sendto(data, neighbor_address)

        # Backtrace to the previous station
        if found == 1:
            current -= 1
            neighbor_ip, neighbor_port = re.match(r"([^:]+):(\d+)", addresses[current-1]).groups()
            neighbor_address = (neighbor_ip, int(neighbor_port))

            # Construct the updated payload
            updated_payload = " ".join([
                str(found), str(hops), str(current),
                *stations, *addresses,
                *routes, *times,
                destination, source
            ]) + ";"

            # Send the updated payload to the previous station
            server_socket_udp.sendto(updated_payload.encode(), neighbor_address)

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

# while True:
#     # Handle incoming TCP connections from web browsers
#     client_socket_tcp, client_address_tcp = server_socket_tcp.accept()
#     handle_tcp_connection(client_socket_tcp, client_address_tcp)

#     # Handle incoming UDP datagrams from other station servers
#     data, address = server_socket_udp.recvfrom(1024)
#     handle_udp_datagram(data, address)

while True:
    read_sockets, _, _ = select.select([server_socket_tcp, server_socket_udp], [], [])

    for sock in read_sockets:
        if sock == server_socket_tcp:
            # Handle incoming TCP connection
            client_socket_tcp, client_address_tcp = sock.accept()
            handle_tcp_connection(client_socket_tcp, client_address_tcp)

        elif sock == server_socket_udp:
            # Handle incoming UDP datagram
            data, address = sock.recvfrom(1024)
            handle_udp_datagram(data, address)
