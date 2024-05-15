python3 -m server StationA 4001 4002 localhost:4008 &
python3 -m server TerminalB 4003 4004 localhost:4006 &
python3 -m server JunctionC 4005 4006 localhost:4004 localhost:4008 localhost:4010 &
python3 -m server BusportD 4007 4008 localhost:4002 localhost:4006 &
python3 -m server StationE 4009 4010 localhost:4006 &
