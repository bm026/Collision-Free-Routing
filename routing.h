#ifndef __ROUTING_H
#define __ROUTING_H

#include <stdio.h>
#include <stdlib.h>

Packet **offline_route_planner();
Packet *create_packet(int source, int destination, int data);

#endif
