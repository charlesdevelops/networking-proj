Details can be found [here](https://teaching.csse.uwa.edu.au/units/CITS3002/project2024/index.php)

# Socket

## SOCK_STREAM
The TCP socket will be responsible for the query between the browser and the station servers.


## SOCK_DGRAM
The UDP socket will be responsible for the internal process of finding out the answer of the query by relaying the message to the peers until it get to the destination. Need to figure out how, and what format the timetable every server station need to have.

## Here's the station server will be establishing connection on port -> TCP_PORT and open for UDP datagrams on port -> UDP_PORT
to run:
```bash
make
```

to clean:
```bash
make clean
```

After make, a ``station-server`` executable will show up, this is our station-server in c
example: `` ./station-server BusportB 4008 4010 localhost:4012 localhost:4020``

Follow the instruction on how to make the adjacency (basically setup automation) 
``cd files``
```bash
shell>  cc -Wall -Werror -o buildrandomtimetables buildrandomtimetables.c

shell>  ./buildrandomtimetables 6

shell>  chmod +x assignports.sh

shell>  ./assignports.sh adjacency startstations.sh

shell>  chmod +x makewebpage.sh

shell>  ./makewebpage.sh startstations.sh mywebpage.html
```
More infos on [Link](https://teaching.csse.uwa.edu.au/units/CITS3002/project2024/getting-started.php)

## The **PROTOCOL** and PAYLOAD struct
IMPORTANT: Both python and c program should use the same string format when sending UDP protocol (communicating with neighbours).
In C:
```C
typedef struct payload{
  int found; // 0 or 1
  int hops;
  int current;
  char **stations;
  char **address; // their respective address per routes.
  char **routes;
  char **time; // their times.
  char destination[MAX_NAME_LENGTH];
  char source[MAX_NAME_LENGTH];
  
} PAYLOAD;
```
Here, the payload will be crafted into string, by *craft_payload* function, but for initialization manually crafted payload would be neater.
The payload basically consists of all those members of ``struct payload`` in the same order they listed above. 
This can be easily explained with an example:
```C
char payload_string[] = "1 5 5 StationA StationB StationC StationD StationE localhost:1002 localhost:1003 localhost:1004 localhost:1005 localhost:1006 BASE A_B B_C C_D D_E 10:45 11:45 12:45 13:45 14:45 StationX StationA;";```
```
EVERY PACKET IS DELIMITED BY SPACE AND ENDED WITH SEMICOLON.
Here, ``1 5 5`` refers to ``found, hops, current``.
``found`` -> 0 (not found), 1 (found)
``hops`` -> how many hops
``current`` -> used to track where we are at the list of stations, address, or routes.
``stations`` -> name of station traversed, delimited by blank space. 
``address`` -> here, its mainly ``localhost:PORT``
``routes`` -> the route taken.
``time`` -> at what departure-time. (To be decided, if this is best way)
``destination`` -> final arrival.
``source`` -> where it comes from.

### Protocol
Now the protocol will work as follows:
Whenever the payload is initialized it will ALWAYS have this format:
init_payload = "0 1 1 ``source`` ``localhost:sourceport`` ``"BASE"`` ``start_time`` ``destination`` ``source``;"

When it arrives into **new** destination, hence ``found == 0``, 
the payload will get updated to supposed from init_payload to StationB
"0 2 2 ``source StationB`` ``localhost:sourceport localhost:StationBPort`` ``"BASE" ...`` ``start_time ...`` ``destination`` ``source``;"
Now, the three dots ``...``, means that to be filled on the second roundtrip after the first backtrace.

### How does it backtrace?
Now ``current`` plays an important role.
say that the payload arrived at ``destination`` from StationB
"1 3 3 ``source StationB destination`` ``localhost:sourceport localhost:StationBPort localhost:destinationport`` ``"BASE" ... ...`` ``start_time ... ...`` ``destination`` ``source``;"
First, it will flip the found bit into 1, and add the destination infos on to the payload. The current will point where the payload is at now. So, ``current`` is 3 now, points to ``destination`` also ``localhost:destinationport`` accordingly. Hence, backtrace to ``current - 1``. Now, this works on a lot of different implementations.

```C
// BACKTRACE! definitely a neighbour.
 printf("BACKTRACE!\n");
 char ip_target[INET6_ADDRSTRLEN];
 char port_target[MAX_PORT];
 int current =  --received_payload.current;
 sscanf(received_payload.address[current-1], "%45[^:]:%s", ip_target, port_target);
 printf("%s\n", received_payload.address[current-1]);
 char *payload_tosend = craft_payload(received_payload);
 printf("payload to send %s\n", payload_tosend);
 talk_to(ip_target, port_target, payload_tosend);
 ```

Here, the C code in ``main.c`` will decrement the ``current`` before sending it to the "traced back" neighbour.

### Updating ``...`` on ``routes`` and ``time``
Once, the payload got back to the source, with ``found == 1``, this time, we gonna track the whole address back, so referring to the last payload example, starting from ``localhost:sourceport -> localhost:StationBport -> localhost:destinationport``.

This time, ``found`` will be incremented, and the logic will work this way, whenever ``found == 2``, try to "what routes and departure-time to be updated". Since, neighbour can only know the port and IP address of its neighbour (NOT THE Station_Name). This might be the optimal solution.

Once, everything is updated, and it hits the ``destination`` again, ``found == 3``, **FOUND EVERYTHING!**