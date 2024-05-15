python3 -m server JunctionA 4001 4002 localhost:4004 localhost:4006 &
python3 -m server BusportB 4003 4004 localhost:4002 &
python3 -m server StationC 4005 4006 localhost:4002 &
