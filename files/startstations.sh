./station-server StationA 4001 4002 localhost:4006 &
./station-server TerminalB 4003 4004 localhost:4006 &
./station-server JunctionC 4005 4006 localhost:4002 localhost:4004 &
