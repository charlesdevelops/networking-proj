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


def create_http_response(source_station, destination_station):
    html_content = f"""
<!DOCTYPE html>
<html>
<head>
    <title>Route Finder</title>
</head>
<body>
    <h1>Route Finder</h1>
    <form action="/" method="GET">
        <label for="source">Source Station:</label>
        <input type="text" id="source" name="source" value="{source_station}"><br>

        <label for="destination">Destination Station:</label>
        <input type="text" id="destination" name="destination" value="{destination_station}"><br>

        <input type="submit" value="Find Route">
    </form>
"""

    http_response = f"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: {len(html_content)}\r\n\r\n{html_content}"

    if source_station and destination_station:
        # Initiate the route-finding process
        init_payload = f"0 1 1 {station_name} localhost:{PORT_UDP} BASE 00:00 {destination_station} {source_station};"
        for neighbor in NEIGHBORS:
            server_socket_udp.sendto(init_payload.encode(), neighbor)

    return http_response.encode()

def handle_tcp_connection(client_socket_tcp, client_address_tcp):
    try:
        print(f'Connection from {client_address_tcp}')

        # Receive the HTTP request from the web browser
        request = client_socket_tcp.recv(1024).decode()
        print(f'Received request: {request}')

        # Parse the request to extract the query parameters
        request_lines = request.split('\r\n')
        request_line = request_lines[0]
        method, url, protocol = request_line.split()

        # Extract the source and destination stations from the query parameters
        query_params = url.split('?')
        if len(query_params) > 1:
            params = dict(param.split('=') for param in query_params[1].split('&'))
            source_station = params.get('source', '')
            destination_station = params.get('destination', '')
        else:
            source_station = ''
            destination_station = ''

        # Generate the HTTP/1.1 response
        response = create_http_response(source_station, destination_station)

        # Send the HTTP/1.1 response back to the web browser
        client_socket_tcp.sendall(response)

    finally:
        # Clean up the connection
        client_socket_tcp.close()

def handle_udp_datagram(data, address):
    print(f'Received UDP datagram from {address}: {data.decode()}')

    # Parse the received payload
    payload_str = data.decode().rstrip(';')
    found, hops, current, stations, addresses, routes, times, destination, source = parse_payload(payload_str)
    print(f"Found: {found}, Hops: {hops}, Current: {current}, Stations: {stations}, Addresses: {addresses}, Routes: {routes}, Times: {times}, Destination: {destination}, Source: {source}")

    # Check if this station is the destination
    if station_name == destination:
        print(f"This station is the destination ({station_name})")
        if found == 1:
            print("Found is 1, updating routes and times...")
            # Update routes and times
            updated_routes = []
            updated_times = []
            for i in range(current - 1, -1, -1):
                updated_routes.append(routes[i])
                updated_times.append(times[i])

            # Construct the updated payload
            updated_payload = " ".join([
                str(3), str(hops), str(current),
                *stations, *addresses,
                *updated_routes, *updated_times,
                destination, source
            ]) + ";"

            # Send the updated payload back to the source
            server_socket_udp.sendto(updated_payload.encode(), address)

        elif found == 3:
            print("Found is 3, generating HTML response...")
            # Route found successfully
            print(f"Route found from {source} to {destination}")
            print(f"Number of hops: {hops}")
            print(f"Station names: {stations}")
            print(f"Station addresses: {addresses}")
            print(f"Routes: {routes}")
            print(f"Departure times: {times}")

            # Generate the HTML response
            html_content = f"""
<!DOCTYPE html>
<html>
<head>
    <title>Route Found</title>
</head>
<body>
    <h1>Route Found</h1>
    <p>Route from {source} to {destination}:</p>
    <ul>
"""
            for i in range(hops):
                html_content += f"<li>{stations[i]} ({routes[i]}) - Depart: {times[i]}</li>"

            html_content += """
    </ul>
</body>
</html>
"""

            # http_response = f"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: {len(html_content)}\r\n\r\n{html_content}"
            # server_socket_tcp.sendall(http_response.encode())
            print(f"HTML response: {html_content}")
            http_response = f"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: {len(html_content)}\r\n\r\n{html_content}"
            print(f"HTTP response: {http_response}")
            server_socket_tcp.sendall(http_response.encode())
            print("Sent HTTP response to the web browser")

    else:
        print(f"This station ({station_name}) is not the destination, forwarding datagram...")
        # Forward the datagram to the appropriate neighbor station
        for neighbor_address in NEIGHBORS:
            if neighbor_address != address:
                server_socket_udp.sendto(data, neighbor_address)

        # Backtrace to the previous station
        if found == 1:
            print("Found is 1, updating routes and times...")
            updated_routes = []
            updated_times = []
            for i in range(current - 1, -1, -1):
                updated_route = routes[i]
                updated_time = times[i]
                updated_routes.append(updated_route)
                updated_times.append(updated_time)
                print(f"Appended route: {updated_route}, time: {updated_time}")

            # Construct the updated payload
            updated_payload = " ".join([
                str(3), str(hops), str(current),
                *stations, *addresses,
                *updated_routes, *updated_times,
                destination, source
            ]) + ";"
            print(f"Updated payload: {updated_payload}")

            # Send the updated payload back to the source
            server_socket_udp.sendto(updated_payload.encode(), address)
            print("Sent updated payload back to the source")


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