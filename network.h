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

// data structures
typedef struct {
	int count;
	char rout[NUM_LAYERS];
	char addr[NUM_LAYERS];
	int data;
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
	Buffer buffer;
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
Packet *create_packet(Packet *p, int data, int count, char *edge_route, char *core_route);


#endif