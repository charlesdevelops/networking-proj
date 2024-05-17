python3 -m server TerminalA 4001 4002 localhost:4006 localhost:4008 localhost:4010 &
python3 -m server JunctionB 4003 4004 localhost:4006 &
python3 -m server BusportC 4005 4006 localhost:4002 localhost:4004 localhost:4010 &
python3 -m server StationD 4007 4008 localhost:4002 &
python3 -m server TerminalE 4009 4010 localhost:4002 localhost:4006 &
