import socket
import select
import sys
import re


class Payload:
    def __init__(self, found=0, hops=0, current=0, stations=None, addresses=None, routes=None, times=None, destination="", source="", starting_time="00:00"):
        self.found = found
        self.hops = hops
        self.current = current
        self.stations = stations if stations is not None else []
        self.addresses = addresses if addresses is not None else []
        self.routes = routes if routes is not None else []
        self.times = times if times is not None else []
        self.destination = destination
        self.source = source
        self.starting_time = starting_time

    def serialize(self):
        # Serialize the payload into a string
        payload_str = f"{self.found} {self.hops} {self.current} "
        payload_str += " ".join(self.stations) + " "
        payload_str += " ".join(self.addresses) + " "
        payload_str += " ".join(self.routes) + " "
        payload_str += " ".join(self.times) + " "
        payload_str += f"{self.destination} {self.source} {self.starting_time};"
        return payload_str

    def deserialize(self, payload_str):
        # Deserialize the payload from a string
        pattern = r"(\d+) (\d+) (\d+) (.+?);"
        match = re.match(pattern, payload_str)
        if match:
            self.found = int(match.group(1))
            self.hops = int(match.group(2))
            self.current = int(match.group(3))
            data = match.group(4).split()

            # Assuming order is preserved and known size for each segment
            print(data)
            hops = self.hops
            self.stations = data[:hops]
            self.addresses = data[hops:2*hops]
            self.routes = data[2*hops:3*hops]
            self.times = data[3*hops:4*hops]
            self.destination = data[-3]
            self.source = data[-2]
            self.starting_time = data[-1]

    def update_on_arrival(self, new_station, new_address):
        if self.found == 0:
            self.hops += 1
            self.current += 1
            self.stations.append(new_station)
            self.addresses.append(new_address)
            # Placeholder for the other fields
            self.routes.append("New_Route")
            self.times.append("New_Time")
        elif self.found == 1:
            # Implement backtrace logic
            self.current -= 1  # decrement current to simulate backtrace

    def print_payload(self):
        # Print payload attributes in a formatted way
        print("Deserialized Payload:")
        print("Found:", self.found)
        print("Hops:", self.hops)
        print("Current:", self.current)
        print("Stations:", self.stations)
        print("Addresses:", self.addresses)
        print("Routes:", self.routes)
        print("Times:", self.times)
        print("Destination:", self.destination)
        print("Source:", self.source)
        print("Starting time:", self.starting_time)

    def __str__(self):
        return self.serialize()

class Neighbours:
    def __init__(self, ipaddr, udpport):
        self.ipaddr = ipaddr
        self.udpport = udpport

class Timetable:
    def __init__(self, filename):
        self.filename = filename
        self.entries = []
        self.load_timetable()

    def load_timetable(self):
        try:
            with open(self.filename, 'r') as file:
                for line in file:
                    line = line.strip()
                    if not line.startswith('#') and line:
                        self.entries.append(line.split(','))
        except FileNotFoundError:
            print(f"Error: File '{self.filename}' not found.")
        except Exception as e:
            print(f"An error occurred: {e}")

    def print_timetable(self):
        for entry in self.entries:
            print(entry)

class TCPSocket:
    def __init__(self, ip_addr, tcp_port):
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.bind((ip_addr, tcp_port))
        self.socket.listen()

    def accept(self):
        client, addr = self.socket.accept()
        return client, addr

    def close(self):
        self.socket.close()


class UDPSocket:
    def __init__(self, ip_addr, udp_port):
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.socket.bind((ip_addr, udp_port))

    def send_to(self, message, ip_addr, udp_port):
        self.socket.sendto(message.encode('utf-8'), (ip_addr, udp_port))

    def receive(self):
        try:
            data, addr = self.socket.recvfrom(1024)
            return data, addr
        except socket.error:
            return None, None

    def close(self):
        self.socket.close()

def run_server(ip_addr, tcp_port, udp_port, timetable, neighbours, STATION_NAME):
    tcp_socket = TCPSocket(ip_addr, tcp_port)
    udp_socket = UDPSocket(ip_addr, udp_port)

    sockets_list = [tcp_socket.socket, udp_socket.socket]
    clients = {}

    try:
        while True:
            # Wait for ready sockets
            read_sockets, _, exception_sockets = select.select(sockets_list, [], sockets_list)

            for notified_socket in read_sockets:
                if notified_socket == tcp_socket.socket:
                    client_socket, client_address = tcp_socket.accept()
                    print(f"TCP connection established from {client_address}")
                    sockets_list.append(client_socket)
                    clients[client_socket] = client_address

                    request = client_socket.recv(1024).decode('utf-8')
                    print(f"Request received: {request}")

                    if request.startswith('GET'):
                        match = re.search(r"GET /\?to=([^ ]+) HTTP/1.1", request)
                        if match:
                            station_requested = match.group(1)
                            print(f"Station requested: {station_requested}")

                    init_payload = Payload(
                            destination=station_requested,
                            source=STATION_NAME
                            )
                    message = init_payload.serialize()
                    print(message)

                    
                    udp_socket.send_to(message, "localhost", udp_port)


                elif notified_socket == udp_socket.socket:
                    data, addr = udp_socket.receive()
                    data = data.decode('utf-8')
                    print(data)
                    p = Payload()
                    p.deserialize(data)
                    p.print_payload()

                    if p.found == 4:
                      pass
                    elif p.found == 3:
                      pass
                    elif p.found == 2:
                      pass
                    elif p.found == 1:
                      pass
                    else:
                      p.update_on_arrival(STATION_NAME, "localhost:" + str(udp_port))
                      message = p.serialize()
                      for n in neighbours:
                        if f"{n.ipaddr}:{n.udpport}" not in p.addresses:
                          udp_socket.send_to(message, n.ipaddr, n.udpport)
                      

                    # if data:
                    #     print(f"Received UDP data from {addr}: {data}")
                    

            for notified_socket in exception_sockets:
                sockets_list.remove(notified_socket)
                if notified_socket in clients:
                    del clients[notified_socket]

    finally:
        tcp_socket.close()
        udp_socket.close()
        for sock in sockets_list:
            sock.close()

if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("Usage: python -m server <STATION_NAME> <TCPPORT> <UDPPORT>")
        exit()
    IP_ADDRESS = "127.0.0.1"  # Localhost
    STATION_NAME = sys.argv[1]
    TCP_PORT = int(sys.argv[2])
    UDP_PORT = int(sys.argv[3])

    neighbours = []
    # Parse additional neighbour arguments
    for neighbour in sys.argv[4:]:
        try:
            ip, port = neighbour.split(':')
            port = int(port)

            print(ip,port)
            neighbours.append(Neighbours(ip, port))
        except ValueError:
            print(f"Error parsing neighbour data: {neighbour}")
            sys.exit()

    filename = "files/tt-" + STATION_NAME
    timetable = Timetable(filename)
    timetable.print_timetable()
    print("Neighbours:")
    print(len(neighbours))
    for n in neighbours:
        print(f"IP: {n.ipaddr}, UDP Port: {n.udpport}")


    run_server(IP_ADDRESS, TCP_PORT, UDP_PORT, timetable, neighbours, STATION_NAME)

