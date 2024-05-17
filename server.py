# STUDENT_1 ID = 23262455 (Harry Komah)
# STUDENT_2 ID = 23196696 (Charles Aidan Watson)

import socket
import struct
import select
import sys
import re
import fcntl
import os
import time
import signal

tcp_socket = None
udp_socket = None
def signal_handler(sig, frame):
    print('Signal caught, closing sockets...')
    if tcp_socket:
        tcp_socket.close()
    if udp_socket:
        udp_socket.close()
    sys.exit(0)

def current_time():
    # Get current time as a struct_time object
    current_struct_time = time.localtime()
    
    # Format hours and minutes as strings with leading zeros if needed
    current_time_str = time.strftime("%H:%M", current_struct_time)
    
    return current_time_str

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
            self.routes.append("...")
            self.times.append("...")
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

class Query:
  def __init__(self, destination, start_time, answers=None):
    self.start_time = start_time
    self.destination = destination
    self.answers = answers
  
class ClientNode:
  def __init__(self, fd, query):
    self.fd = fd
    self.query = query
    self.next = None

class ClientList:
  def __init__(self):
    self.head = None

  def add_client(self, fd, query):
    new_node = ClientNode(fd, query)
    new_node.next = self.head
    self.head = new_node

  def find_client(self, query):
    current = self.head
    while current is not None:
      if (current.query.destination == query.destination) and (current.query.start_time == query.start_time):
        return current
      current = current.next
    return None
  
  def remove_client(self, fd):
    prev = None
    current = self.head
    while current is not None:
      if current.fd == fd:
        if prev != None:
            prev.next = current.next
      prev = current
      current = current.next

class QueryManager:
  def __init__(self, max_queries=100):
    self.queries = []
    self.max_queries = max_queries
    self.query_index = 0

  def add_query(self, new_query):
    if len(self.queries) < self.max_queries:
      self.queries.append(new_query)
    else:
      self.queries[self.query_index % self.max_queries]
    self.query_index += 1

  def search_query(self, destination, start_time):
    for q in self.queries:
       if q.destination == destination and q.start_time == start_time:
          return q
    return None

def time_to_minutes(time_str):
    # print("timestring being decoded: " + time_str + "\n")
    if (":" in time_str):   
        hours, minutes = map(int, time_str.split(':'))
        return hours * 60 + minutes
    elif ("-" in time_str):
        hours, minutes = map(int, time_str.split('-'))
        return hours * 60 + minutes
    else:
        return 0

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

    def search_timetable(self, destination, time_earlier):
      for entry in self.entries[1:]:
        # departure-time,route-name,departing-from,arrival-time,arrival-station
        # if the time is now earlier, then go to the next if.
        print(entry)
        if time_to_minutes(time_earlier) > time_to_minutes(entry[0]): continue
        if destination == entry[4]:
          return entry
        
      # search the timetable from 00:00 past midnight, considering the timetable stays the same the next day.
      # for entry in self.entries[1:]:
      #   # departure-time,route-name,departing-from,arrival-time,arrival-station
      #   # if the time is now earlier, then go to the next if.
      #   print(entry)
      #   if 0 > time_to_minutes(entry[0]): continue
      #   if destination == entry[4]:
      #     return entry
      return None
      


def set_socket_timeout(sock, timeout_sec, timeout_usec=0):
  timeval = struct.pack('ll', timeout_sec, timeout_usec)
  try:
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVTIMEO, timeval)
  except socket.error as e:
    print(f"Error setting timeout: {e}")
    exit(1)

def is_socket_open(sock):
    try:
      flags = fcntl.fcntl(sock.fileno(), fcntl.F_GETFL)
      return True
    except OSError:
      return False

class TCPSocket:
    def __init__(self, ip_addr, tcp_port):
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.bind((ip_addr, tcp_port))
        self.socket.listen()
        set_socket_timeout(self.socket, 1, 500)

    def accept(self):
        client, addr = self.socket.accept()
        return client, addr
    
    def sendall(self, message):
        self.socket.sendall(message.encode('utf-8'))

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

def parse_query(query):
    # Regex pattern to match "GET /?to=someDestination HTTP/1.1"
    pattern1 = r"GET /\?to=([^ &]+) HTTP/1.1"
    # Regex pattern to match "GET /?to=someDestination&time=someTime HTTP/1.1"
    pattern2 = r"GET /\?to=([^ &]+)&time=([^ &]+) HTTP/1.1"

    # Function to decode specific URL-encoded characters
    def decode_url(encoded_string):
        return encoded_string.replace("-", ":")

    def decode_url_2(encoded_string):
        return encoded_string.replace("%3A", ":")

    # Try to match the first pattern
    match1 = re.match(pattern1, query)
    if match1:
        destination = decode_url(match1.group(1))
        # destination = decode_url2(destination)
        time = current_time()  # Assuming 'current_time' is a function you've defined earlier
        print(f"Using time: {time}")
        return (destination, time)

    # Try to match the second pattern
    match2 = re.match(pattern2, query)
    if match2:
        destination = decode_url(match2.group(1))
        time = decode_url(match2.group(2))
        print(f"Using time: {time}")
        return (destination, time)

    # If no patterns match, return None
    return None

def http_response(message):
   return """\
HTTP/1.1 200 OK
Content-Type: text/html

<html>
<body>
    <h1>From Py:</h1>
    <p>""" + message + """
</p>
</body>
</html>
"""

latest_time = None

def update_file_mtime(filepath):
    global latest_time
    # Retrieve the modification time of the file
    mtime = os.path.getmtime(filepath)
    # Update the global variable
    latest_time = mtime

def check_and_update(filepath, timetable):
   global latest_time
   mtime = os.path.getmtime(filepath)
   if mtime > latest_time:
      print(f"{filepath} is newer, updating timetable...")
      timetable = Timetable(filepath)
   else:
      print("Current timetable is the newest!")

def run_server(ip_addr, tcp_port, udp_port, timetable, neighbours, STATION_NAME):
    global tcp_socket, udp_socket
    tcp_socket = TCPSocket(ip_addr, tcp_port)
    udp_socket = UDPSocket(ip_addr, udp_port)

    sockets_list = [tcp_socket.socket, udp_socket.socket]
    queries = QueryManager()
    clients = ClientList()

    try:
        while True:
            # Wait for ready sockets
            print("Py: back to select\n")
            filepath = "files/tt-" + STATION_NAME
            check_and_update(filepath, timetable)
            
            read_sockets, _, exception_sockets = select.select(sockets_list, [], sockets_list, 15)
            if not (read_sockets or exception_sockets):
              current = clients.head
              while current is not None:
                 if current.query is None:
                    # bad client
                    current.fd.close()
                    clients.remove_client(current.fd)
                 q = queries.search_query(current.query.destination, current.query.start_time)
                 
                 if q is not None:
                    if q.answers is not None:
                        try:
                            current.fd.sendall(http_response(q.answers).encode('utf-8'))
                        except Exception:
                            print("bad file descriptor")
                        finally:
                            clients.remove_client(current.fd)
                            current.fd.close()
                    else:
                       init_payload = Payload(
                        destination=q.destination,
                        source=STATION_NAME,
                        starting_time=q.start_time
                       )
                       message = init_payload.serialize()
                       udp_socket.send_to(message, "localhost", udp_port)
                 else:
                    # queries are asked but not put in the list
                    queries.add_query(current.query)
                    qu = current.query
                    new_p = Payload(destination=qu.destination, source=STATION_NAME, starting_time=qu.start_time)
                    message = new_p.serialize()
                    udp_socket.send_to(message, "localhost", udp_port)
              continue

            for notified_socket in read_sockets:
                if notified_socket == tcp_socket.socket:
                    client_socket, client_address = tcp_socket.accept()
                    print(f"TCP connection established from {client_address}")
                    try:
                        # client_socket.settimeout(1.5)  # Set a timeout of 1.5 seconds
                        request = client_socket.recv(1024).decode('utf-8')
                        # client_socket.settimeout(None)  # Remove timeout after successful reception
                        # Process the request
                        if request:
                            print("Received data:", request)
                            
                        else:
                            print("No data received, possibly connection was closed by client.")
                            continue
                        # Your code to handle the request here
                    except socket.timeout:
                        print(f"Timeout occurred for {client_socket.getpeername()}, no data received")
                        continue
                        # Optionally, handle timeout-specific logic here
                    except ConnectionResetError:
                        print("Connection reset by peer, removing client")
                        client_socket.close()
                        continue
                    except Exception as e:
                        print(f"An error occurred: {e}")
                        client_socket.close()
                        continue
                    
                    parse_result = parse_query(request)
                    if parse_result != None:
                       (dest, tm) = parse_result
                       q = Query(dest, tm)
                       init_payload = Payload(
                        destination=q.destination,
                        source=STATION_NAME,
                        starting_time=q.start_time
                       )
                       same_query = queries.search_query(dest, tm)
                       clients.remove_client(client_socket) # Make sure no duplicates, remove and adding it again.
                       if same_query is None:
                          clients.add_client(client_socket, q)
                          queries.add_query(q)  
                          message = init_payload.serialize()
                          print(message)
                          udp_socket.send_to(message, "localhost", udp_port)
                       else:
                          if same_query.answers == None:
                             message = init_payload.serialize()
                             print("query has been asked before, but no answer yet. Asking...")
                             clients.add_client(client_socket, q)
                             udp_socket.send_to(message, "localhost", udp_port)
                          else:
                             print("Query has been asked before, sending the fastest route")
                             try:
                                 client_socket.sendall(http_response(message).encode('utf-8'))
                             except Exception:
                                 print("Might be a bad file descriptor")
                             finally:
                                 client_socket.close()
                    else:
                       print("Not meaningful") # Browser might send some unnecessary stuffs.
                       # Don't close it otherwise the previous meaningful request get lost.
                       continue

                elif notified_socket == udp_socket.socket:
                    data, addr = udp_socket.receive()
                    data = data.decode('utf-8')
                    print(data)
                    p = Payload()
                    p.deserialize(data)
                    p.print_payload()

                    if p.destination == STATION_NAME:
                      if p.found == 0:
                        print("found: 0=>1")
                        p.update_on_arrival(STATION_NAME, "localhost:" + str(udp_port))
                        p.found = 1
                        p.print_payload()
                      
                      if p.found == 2:
                        print("found: 2=>3")
                        p.found = 3
                        p.print_payload()
                    
                    if p.source == STATION_NAME:
                      if p.found == 1:
                        print("found: 1=>2")
                        t = timetable.search_timetable(p.stations[1], p.starting_time) # check next station, in which starting_time is earlier than departure time
                        if t is None:
                           p.times[0] = "99:99" # too late even for the first route
                        else:
                           p.times[0] = t[0]
                        p.found = 2
                        p.print_payload()
                      
                      if p.found == 3:
                        print("found: 3=>4")
                        p.found = 4

                    if p.found == 4:
                      print("\n****\npy: DONE!\n\n")
                      target_query = queries.search_query(p.destination, p.starting_time)

                      message = p.serialize()
                      q = Query(p.destination, p.starting_time)
                      if target_query is None:
                         q.answers = message
                         queries.add_query(q)
                      elif target_query.answers == None:
                         target_query.answers = message
                      else:
                         temp = Payload()
                         temp.deserialize(target_query.answers)
                         # Update the answer to the newest
                         if time_to_minutes(temp.times[temp.hops-1]) > time_to_minutes(p.times[p.hops-1]):
                            target_query.answers = message
                        # if not, then it's already better route.

                      # Simple HTTP response
                      http_resp = http_response(target_query.answers)
                      c = clients.find_client(q)
                      if c:
                        try:
                            c.fd.sendall(http_resp.encode("utf-8"))
                        except OSError as e:
                            # Handle the OSError
                            print(f"An OSError occurred: {e}")
                        finally:
                            clients.remove_client(c.fd)
                            c.fd.close()
                      else:
                        continue # lost client or has been answered.
                    elif p.found == 3:
                      p.current -=1
                      message = p.serialize()
                      pattern = r'^(.*):(\d+)$'
                      match = re.match(pattern, p.addresses[p.current-1])
                      udp_socket.send_to(message, match.group(1), int(match.group(2)))
                    elif p.found == 2:
                      cur = p.current
                      p.current += 1
                      t = timetable.search_timetable(p.stations[cur], p.times[cur-1]) # check next stations, where departure time is later than arrival_time
                      if t is None:
                         p.times[cur] = "99:99"
                         p.routes[cur] = "NOTFOUND" # This will get replaced if there is a found route, see logic above.
                      else:
                         p.times[cur] = t[3]
                         p.routes[cur] = t[1]
                      message = p.serialize()
                      pattern = r'^(.*):(\d+)$'
                      match = re.match(pattern, p.addresses[p.current-1])
                      udp_socket.send_to(message, match.group(1), int(match.group(2)))
                    elif p.found == 1:
                      print("BACKTRACE!")
                      p.current -=1
                      message = p.serialize()
                      pattern = r'^(.*):(\d+)$'
                      match = re.match(pattern, p.addresses[p.current-1])
                      udp_socket.send_to(message, match.group(1), int(match.group(2)))
                    else:
                      p.update_on_arrival(STATION_NAME, "localhost:" + str(udp_port))
                      message = p.serialize()
                      print(message)
                      for n in neighbours:
                        if f"{n.ipaddr}:{n.udpport}" not in p.addresses:
                          udp_socket.send_to(message, n.ipaddr, n.udpport)
                      

                    # if data:
                    #     print(f"Received UDP data from {addr}: {data}")
                    

            # for notified_socket in exception_sockets:
            #     sockets_list.remove(notified_socket)
            #     if notified_socket in clients:
            #         del clients[notified_socket]

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
    
    # Signal handler.
    signal.signal(signal.SIGINT, signal_handler)

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
    update_file_mtime(filename)
    timetable.print_timetable()
    print("Neighbours:")
    print(len(neighbours))
    for n in neighbours:
        print(f"IP: {n.ipaddr}, UDP Port: {n.udpport}")


    run_server(IP_ADDRESS, TCP_PORT, UDP_PORT, timetable, neighbours, STATION_NAME)
