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

to talk to UDP, ``telnet localhost UDP_PORT``.
get to your web browser, go to ``localhost:PORT``.
