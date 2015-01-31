#include "network.h"

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


Network *init_network (Network *n);


int main (int argc,  char* argv[]) {

	Network *n = NULL;
	n = init_network(n);

	printf("Network initialised\n");

	return 0;
}

Network *init_network (Network *n) {

	int i,j;
	int base;

	// initialise network
	n = (Network *)malloc(sizeof(Network));

	// initialise core memory
	for (i=0; i<NUM_CORES; i++) {
		n -> cores[i] = (Core *)malloc(sizeof(Core));
		n -> cores[i] -> send = NULL;
		n -> cores[i] -> recv = NULL;
	}

	// initialise link memory
	for (i=0; i<NUM_LAYERS; i++) {
		for (j=0; j<2*NUM_CORES; j++) {
			n -> links[i][j] = (Link *)malloc(sizeof(Link));
			n -> links[i][j] -> comm = NULL;
			n -> links[i][j] -> temp = NULL;
		}
	}

	// initialise switch memory
	for (i=0; i<NUM_LAYERS; i++) {
		for (j=0; j<NUM_CORES; j++) {
			n -> switches[i][j] = (Switch *)malloc(sizeof(Switch));
			n -> switches[i][j] -> buffer.nextWrite = 0;
			n -> switches[i][j] -> buffer.count = 0;
		}
	}

	// connect links to cores
	for (i=0; i<NUM_CORES; i++) {
		n -> cores[i] -> io0 = n -> links[0][2*i];
		n -> cores[i] -> io1 = n -> links[0][2*i+1];	
	}

	/*---------------------------------------------------+
	|                                                    |
	|   LAYOUT OF LINK/SWITCH LAYERS                     |
	|   ----------------------------                     |
	|                                                    |
	|   0   1   2   3   4   5   6   7     : link id      |
	|                                                    |
	|   l   l   l   l   l   l   l   l                    |
	|    \ /     \ /     \ /     \ /      ...            |
	|     s       s       s       s                      |
	|                                                    |
	|     0       1       2       3       : switch id    |
	|                                                    |
	+----------------------------------------------------*/

	// connect links to switches
	for (i=1; i<=NUM_LAYERS; i++) {

		base = (int)pow(2,i);

		for (j=0; j<NUM_CORES; j++) {

			// connect core side links
			if (j % base < base/2) {
				n -> switches[i-1][j] -> core0 = n -> links[i-1][2*j];
				n -> switches[i-1][j] -> core1 = n -> links[i-1][2*j+base];
			} 
			else {
				n -> switches[i-1][j] -> core0 = n -> links[i-1][2*j+1-base];
				n -> switches[i-1][j] -> core1 = n -> links[i-1][2*j+1];
			}

			// connect edge side links
			if (i != NUM_LAYERS) {
				n -> switches[i-1][j] -> edge0 = n -> links[i][2*j];
				n -> switches[i-1][j] -> edge1 = n -> links[i][2*j+1];
			}
			// set null for top layer
			else {
				n -> switches[i-1][j] -> edge0 = NULL;
				n -> switches[i-1][j] -> edge1 = NULL;
			}
		}
	}

	return n;
}
