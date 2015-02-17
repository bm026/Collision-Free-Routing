#ifndef __NETWORK_H
#define __NETWORK_H

// libraries
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

// definitions
#define BUFF_SIZE 4
#define NUM_LAYERS 3
#define NUM_CORES 8

// enumerator
typedef enum {
	EDGE,
	CORE
} Direction;

// data structures
typedef struct {
	int source;
	int destination;
	int count;
	char rout[NUM_LAYERS];
	char addr[NUM_LAYERS];
	int data;
	Direction direction;
} Packet;

typedef struct {
	Packet *queue[BUFF_SIZE];
	int nextRead;
	int nextWrite;
	int count;
} Buffer;

typedef struct {
	Packet *comm;
	Packet *temp;
} Link;

typedef struct {
	Link *core0;
	Link *core1;
	Link *edge0;
	Link *edge1;
	Buffer *c0buffer;
	Buffer *c1buffer;
	Buffer *e0buffer;
	Buffer *e1buffer;
} Switch;

typedef struct {
	Link *io0;
	Link *io1;
	Packet *send;
	Packet *recv;
} Core;

typedef struct {
	Core *cores[NUM_CORES];
	Link *links[NUM_LAYERS][2*NUM_CORES];
	Switch *switches[NUM_LAYERS][NUM_CORES];
} Network;

// functions
Network *init_network (Network *n);
//Packet *create_packet(Packet *p, int data, int count, char *edge_route, char *core_route);
void network_timesteps(Network *n, int iterations);
Packet *buffer_read(Buffer *buffer);
int buffer_write(Buffer *buffer, Packet *p);
void add_packet_to_core(Network *n, Packet *p, int core_num);
void send_packets_from_switches(Network *n);
void check_switches_for_received_packets(Network *n);
void send_packets_from_cores(Network *n);
void check_cores_for_received_packets(Network *n);
void link_cleanup(Network *n);
void print_network_state(Network *n);
Packet **offline_route_planner();
Packet *create_packet(int source, int destination, int data);

#endif
