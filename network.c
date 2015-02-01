#include "network.h"

int main (int argc,  char* argv[]) {

	int i;
	Network *n = NULL;
	Packet *p = NULL;

	n = init_network(n);
	p = create_packet(p, 42, 3, "101", "010");
	add_packet_to_core(n, p, 3);

	for (i=0; i<30; i++) {
		n = network_timestep(n);
	}

	print_network_state(n);

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
			n -> switches[i][j] -> c0buffer = (Buffer *)malloc(sizeof(Buffer));
			n -> switches[i][j] -> c1buffer = (Buffer *)malloc(sizeof(Buffer));
			n -> switches[i][j] -> e0buffer = (Buffer *)malloc(sizeof(Buffer));
			n -> switches[i][j] -> e1buffer = (Buffer *)malloc(sizeof(Buffer));
			n -> switches[i][j] -> c0buffer -> nextRead = 0;
			n -> switches[i][j] -> c0buffer -> nextWrite = 0;
			n -> switches[i][j] -> c0buffer -> count = 0;
			n -> switches[i][j] -> c1buffer -> nextRead = 0;
			n -> switches[i][j] -> c1buffer -> nextWrite = 0;
			n -> switches[i][j] -> c1buffer -> count = 0;
			n -> switches[i][j] -> e0buffer -> nextRead = 0;
			n -> switches[i][j] -> e0buffer -> nextWrite = 0;
			n -> switches[i][j] -> e0buffer -> count = 0;
			n -> switches[i][j] -> e1buffer -> nextRead = 0;
			n -> switches[i][j] -> e1buffer -> nextWrite = 0;
			n -> switches[i][j] -> e1buffer -> count = 0;
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
	+---------------------------------------------------*/

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

Packet *create_packet(Packet *p, int data, int count, char *edge_route, char *core_route) {

	int i;

	// initialise packet memory
	p = (Packet *)malloc(sizeof(Packet));

	// add packet data
	p -> data = data;
	p -> count = count;
	p -> direction = EDGE;

	// interpret route
	for (i=0; i<count; i++) {
		
		// edge route
		if (edge_route[i] == '1') {
			p -> rout[i] = 1; 
		}
		else {
			p -> rout[i] = 0;
		}

		// core route
		if (core_route[i] == '1') {
			p -> addr[i] = 1;
		}
		else {
			p -> addr[i] = 0;
		}
	}

	return p;
}

Network *network_timestep(Network *n) {

	int i,j;
	int full;

	// sends all packets from cores
	for (i=0; i<NUM_CORES; i++) {
		send_packet_from_core(n, i);
	}

	for (i=0; i<NUM_LAYERS; i++) {
		for (j=0; j<NUM_CORES; j++) {

			// send packets first (emulating parallelism)

			// core0
			if (n -> switches[i][j] -> core0 -> temp == NULL) {
				n -> switches[i][j] -> core0 -> temp = buffer_read(n -> switches[i][j] -> c0buffer);
			}
			// if the link is full and the buffer wants to send another packet down it, a
			// collision occurs 
			else if (n -> switches[i][j] -> c0buffer -> count != 0) {
				printf("COLLISION\n");
			}

			// core1
			if (n -> switches[i][j] -> core1 -> temp == NULL) {
				n -> switches[i][j] -> core1 -> temp = buffer_read(n -> switches[i][j] -> c1buffer);
			}
			else if (n -> switches[i][j] -> c1buffer -> count != 0) {
				printf("COLLISION\n");
			}

			// exclude top layer
			if (i != NUM_LAYERS-1) {
				// edge0
				if (n -> switches[i][j] -> edge0 -> temp == NULL) {
					n -> switches[i][j] -> edge0 -> temp = buffer_read(n -> switches[i][j] -> e0buffer);
				}
				else if (n -> switches[i][j] -> e0buffer -> count != 0) {
					printf("COLLISION\n");
				}

				// edge1
				if (n -> switches[i][j] -> edge1 -> temp == NULL) {
					n -> switches[i][j] -> edge1 -> temp = buffer_read(n -> switches[i][j] -> e1buffer);
				}
				else if (n -> switches[i][j] -> e1buffer -> count != 0) {
					printf("COLLISION\n");
				}
			}

			// receive packages

			// core0: if there is a packet in the link ... 
			if (n -> switches[i][j] -> core0 -> comm != NULL) {
				// ... going in the correct direction ...
				if (n -> switches[i][j] -> core0 -> comm -> direction = EDGE) {
					n -> switches[i][j] -> core0 -> comm -> count--;
					// ... if it is to be forwarded in the same direction ...
					if (n -> switches[i][j] -> core0 -> comm -> count != 0) {
						// ... down which link ...
						if (n -> switches[i][j] -> core0 -> comm -> rout[i] == 0) {
							full = buffer_write(n -> switches[i][j] -> e0buffer, n -> switches[i][j] -> core0 -> comm);
						}
						else {
							full = buffer_write(n -> switches[i][j] -> e1buffer, n -> switches[i][j] -> core0 -> comm);
						}
					}
					// ... or if the direction is to be changed ...
					else {
						n -> switches[i][j] -> core0 -> comm -> direction = CORE;
						// ... down which link ...
						if (n -> switches[i][j] -> core0 -> comm -> addr[i] == 0) {
							full = buffer_write(n -> switches[i][j] -> c0buffer, n -> switches[i][j] -> core0 -> comm);
						}
						else {
							full = buffer_write(n -> switches[i][j] -> c1buffer, n -> switches[i][j] -> core0 -> comm);
						}
					}
					// tidy up old packet from link
					if (full == 0) {
						n -> switches[i][j] -> core0 -> comm = NULL;
					}
				}
			}

			// core1 
			if (n -> switches[i][j] -> core1 -> comm != NULL) {
				if (n -> switches[i][j] -> core1 -> comm -> direction = EDGE) {
					n -> switches[i][j] -> core1 -> comm -> count--;
					if (n -> switches[i][j] -> core1 -> comm -> count != 0) {
						if (n -> switches[i][j] -> core1 -> comm -> rout[i] == 0) {
							full = buffer_write(n -> switches[i][j] -> e0buffer, n -> switches[i][j] -> core1 -> comm);
						}
						else {
							full = buffer_write(n -> switches[i][j] -> e1buffer, n -> switches[i][j] -> core1 -> comm);
						}
					}
					else {
						n -> switches[i][j] -> core1 -> comm -> direction = CORE;
						if (n -> switches[i][j] -> core1 -> comm -> addr[i] == 0) {
							full = buffer_write(n -> switches[i][j] -> c0buffer, n -> switches[i][j] -> core1 -> comm);
						}
						else {
							full = buffer_write(n -> switches[i][j] -> c1buffer, n -> switches[i][j] -> core1 -> comm);
						}
					}
					// tidy up old packet from link
					if (full == 0) {
						n -> switches[i][j] -> core1 -> comm = NULL;
					}
				}
			}

			// exclude top layer
			if (i != NUM_LAYERS-1) {
			// edge0
				if (n -> switches[i][j] -> edge0 -> comm != NULL) {
					if (n -> switches[i][j] -> edge0 -> comm -> direction = CORE) {
						if (n -> switches[i][j] -> edge0 -> comm -> addr[i] == 0) {
							full = buffer_write(n -> switches[i][j] -> c0buffer, n -> switches[i][j] -> edge0 -> comm);
						}
						else {
							full = buffer_write(n -> switches[i][j] -> c1buffer, n -> switches[i][j] -> edge0 -> comm);
						}
						// tidy up old packet from link
						if (full == 0) {
							n -> switches[i][j] -> edge0 -> comm = NULL;
						}
					}
				}

				// edge1
				if (n -> switches[i][j] -> edge1 -> comm != NULL) {
					if (n -> switches[i][j] -> edge1 -> comm -> direction = CORE) {
						if (n -> switches[i][j] -> edge1 -> comm -> addr[i] == 0) {
							full = buffer_write(n -> switches[i][j] -> c0buffer, n -> switches[i][j] -> edge1 -> comm);
						}
						else {
							full = buffer_write(n -> switches[i][j] -> c1buffer, n -> switches[i][j] -> edge1 -> comm);
						}
						// tidy up old packet from link
						if (full == 0) {
							n -> switches[i][j] -> edge1 -> comm = NULL;
						}
					}
				}
			}
		}
	}

	// move temp values to links, completing parallel emulation
	for (i=0; i<NUM_LAYERS; i++) {
		for (j=0; j<2*NUM_CORES; j++) {
			if (n -> links[i][j] -> comm == NULL) {
				n -> links[i][j] -> comm = n -> links[i][j] -> temp;
				n -> links[i][j] -> temp = NULL;
			}
			else if (n -> links[i][j] -> temp != NULL) {
				printf("COLLISION\n");
			}
		}
	}

	// check cores for received packets
	check_for_received_packets(n);

	return n;
}

Packet *buffer_read(Buffer *buffer) {

	Packet *p = NULL;

	if (buffer -> count != 0) {
		p = buffer -> queue[buffer -> nextRead]; 
		buffer -> nextRead = (buffer -> nextRead + 1) % BUFF_SIZE;
		buffer -> count--;
	}

	return p;
}

int buffer_write(Buffer *buffer, Packet *p) {

	if (buffer -> count < BUFF_SIZE) {
		buffer -> queue[buffer -> nextWrite] = p;
		buffer -> nextWrite = (buffer -> nextWrite + 1) % BUFF_SIZE;
		buffer -> count++;
	}
	else {
		printf("ERROR: FULL BUFFER\n");
		return 1;
	}

	return 0;
}

void add_packet_to_core(Network *n, Packet *p, int core_num) {
	n -> cores[core_num] -> send = p;
}

void send_packet_from_core(Network *n, int core_num) {

	// checks for packet to send
	if (n -> cores[core_num] -> send != NULL) {
		// checks which link to send down
		if (n -> cores[core_num] -> send -> rout[0] == 0) {
			// checks link is free
			if (n -> cores[core_num] -> io0 -> temp == NULL) {
				n -> cores[core_num] -> io0 -> temp = n -> cores[core_num] -> send;
			}
			// if link is not free, collision
			else {
				printf("COLLISION\n");
			}
		}
		else {
			if (n -> cores[core_num] -> io1 -> temp == NULL) {
				n -> cores[core_num] -> io1 -> temp = n -> cores[core_num] -> send;
			}
			else {
				printf("COLLISION\n");
			}
		}

		// reset packet status
		n -> cores[core_num] -> send = NULL;
	}
}

void check_for_received_packets(Network *n) {

	int i;

	for (i=0; i<NUM_CORES; i++) {
		// checks for packet in link0
		if (n -> cores[i] -> io0 -> comm != NULL) {
			// checks for correct direction
			if (n -> cores[i] -> io0 -> comm -> direction == CORE) {
				n -> cores[i] -> recv = n -> cores[i] -> io0 -> comm;
				printf("Packet received on core %d with data: %d\n", i, n -> cores[i] -> recv -> data);
			}
		}

		// checks for packet in link1
		if (n -> cores[i] -> io1 -> comm != NULL) {
			// checks for correct direction
			if (n -> cores[i] -> io1 -> comm -> direction == CORE) {
				n -> cores[i] -> recv = n -> cores[i] -> io1 -> comm;
				printf("Packet received on core %d with data: %d\n", i, n -> cores[i] -> recv -> data);
			}
		}
	}
}

void print_network_state(Network *n) {

	int i, j;

	// print each network layer
	for (i=NUM_LAYERS-1; i>=0; i--) {

		printf("LAYER %d\n---------------------\n\nActive switch buffers:\n\n", i);

		// print each active switch buffer
		for (j=0; j<NUM_CORES; j++) {
			if (n -> switches[i][j] -> c0buffer -> count != 0) {
				printf("Core %d c0buffer[%d]\n", j, n -> switches[i][j] -> c0buffer -> count);
			}
			if (n -> switches[i][j] -> c1buffer -> count != 0) {
				printf("Core %d c1buffer[%d]\n", j, n -> switches[i][j] -> c1buffer -> count);
			}
			// exclude top layer
			if (i != NUM_LAYERS-1) {
				if (n -> switches[i][j] -> e0buffer -> count != 0) {
					printf("Core %d e0buffer[%d]\n", j, n -> switches[i][j] -> e0buffer -> count);
				}
				if (n -> switches[i][j] -> e1buffer -> count != 0) {
					printf("Core %d e1buffer[%d]\n", j, n -> switches[i][j] -> e1buffer -> count);
				}
			}
		}

		printf("\nActive links:\n\n");

		// print each active link
		for (j=0; j<2*NUM_CORES; j++) {
			if (n -> links[i][j] -> comm != NULL) {
				printf("Link %d occupied\n", j);
			}
		}
	}

	printf("\nActive cores:\n\n");

	// print each active core
	for (j=0; j<NUM_CORES; j++) {
		if (n -> cores[j] -> send != NULL) {
			printf("Core %d data to send: %d\n", j, n -> cores[j] -> send -> data);
		}
		if (n -> cores[j] -> recv != NULL) {
			printf("Core %d data received: %d\n", j, n -> cores[j] -> recv -> data);
		}
	}
}
