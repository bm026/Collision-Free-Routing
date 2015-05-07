#ifndef __NETWORK_H
#define __NETWORK_H

// libraries
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

// network definitions
#define BUFF_SIZE 4
#define NUM_LAYERS 1
#define NUM_CORES 2
#define PORT_DEPTH 128

// processor definitions
#define true     -1
#define false    0
#define i_ldwsp  0x0
#define i_stwsp  0x1
#define i_ldawsp 0x2
#define i_ldc    0x3
#define i_ldwcp  0x4
#define i_ldap   0x5
#define i_ldwi   0x6 
#define i_stwi   0x7
#define i_br     0x8
#define i_brf    0x9 
#define i_adj    0xA
#define i_eqc    0xB
#define i_addc   0xC
#define i_pfix   0xD
#define i_nfix   0xE
#define i_opr    0xF
#define o_add    0x0
#define o_sub    0x1
#define o_eq     0x2
#define o_lss    0x3
#define o_and    0x4
#define o_or     0x5
#define o_xor    0x6
#define o_not    0x7
#define o_shl    0x8
#define o_shr    0x9
#define o_brx    0xA
#define o_call   0xB
#define o_ret    0xC
#define o_setsp  0xD
#define o_svc    0xE
#define o_in     0x10
#define o_out    0x11

// processor global variables
unsigned int global_running;
unsigned int mem[NUM_CORES][200000];
unsigned char *pmem[NUM_CORES];
unsigned int pc[NUM_CORES];
unsigned int sp[NUM_CORES];
unsigned int areg[NUM_CORES];
unsigned int breg[NUM_CORES];
unsigned int oreg[NUM_CORES];
unsigned int inst[NUM_CORES];
unsigned int running[NUM_CORES];
unsigned int is_wait[NUM_CORES];
FILE *codefile;
FILE *simio[8];
char connected[] = {0, 0, 0, 0, 0, 0, 0, 0};

// network global variables
int timestep_count = 0;
int collision_count = 0;
int packet_count = 0;

// user changeable variables
unsigned int verbose = true;
unsigned int routing_code = 1; // see reference below

// Routing codes and their corresponding algorithms:
// 0: developed algorithm
// 1: two-phase randomised routing

// link enumerator
typedef enum {
	EDGE,
	CORE
} Direction;

// network data structures
typedef struct {
	int source;
	int destination;
	int port;
	int count;
	char rout[NUM_LAYERS];
	char addr[NUM_LAYERS];
	int data;
	Direction direction;
} Packet;

typedef struct {
	//Packet *queue[BUFF_SIZE];
	Packet **queue;
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
	//Packet *recv;
	//Packet *ports[NUM_PORTS];
	Buffer *ports[NUM_CORES];
} Core;

typedef struct {
	Core *cores[NUM_CORES];
	Link *links[NUM_LAYERS][2*NUM_CORES];
	Switch *switches[NUM_LAYERS][NUM_CORES];
} Network;

// processor functions
void load();
int inbin();
void svc(int i);
void simout(int b, int s);
int simin(int s);

// network functions
Network *init_network (Network *n);
Buffer *new_buffer(int size);
void network_timesteps(Network *n, int iterations);
Packet *buffer_read(Buffer *buffer, int size);
int buffer_write(Buffer *buffer, Packet *p, int max_size);
void add_packet_to_core(Network *n, Packet *p, int core_num);
void send_packets_from_switches(Network *n);
void check_switches_for_received_packets(Network *n);
void send_packets_from_cores(Network *n);
void check_cores_for_received_packets(Network *n);
void link_cleanup(Network *n);
void print_network_state(Network *n);
Packet *create_packet(int source, int destination, int port, int data);

// routing functions
Packet **offline_route_planner(char *active_cores, int **active_core_data);
Packet **tprr_route_planner(char *active_cores, int **active_core_data);

#endif
