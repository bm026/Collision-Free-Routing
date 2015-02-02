#include "network.h"

int main (int argc,  char* argv[]) {

	int i;
	Network *n = NULL;
	Packet *p = NULL;

	n = init_network(n);

	p = create_packet(p, 41, 1, "1", "1");
	add_packet_to_core(n, p, 0);

	network_timesteps(n, 3);

	p = create_packet(p, 42, 1, "1", "0");
	add_packet_to_core(n, p, 1);

	print_network_state(n);

	network_timesteps(n, 1);

	// testing each timestep
	/*for (i=0; i<13; i++) {
		network_timestep(n, 1);
		print_network_state(n);
	}*/

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

void network_timesteps(Network *n, int iterations) {

	int i;

	for (i=0; i<iterations; i++) {

		// sends all packets from cores
		send_packets_from_cores(n);

		// send all packets from switches
		send_packets_from_switches(n);

		// check all cores for received packets
		check_cores_for_received_packets(n);

		// check all switches for received packets
		check_switches_for_received_packets(n);

		// move temp values to links, completing parallel emulation
		link_cleanup(n);

		// for testing
		print_network_state(n);
	}
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

void send_packets_from_switches(Network *n) {

	int i, j;
	Switch *s;

	for (i=0; i<NUM_LAYERS; i++) {
		for (j=0; j<NUM_CORES; j++) {

			// current switch pointer
			s = n -> switches[i][j];

			// core0
			if (s -> core0 -> temp == NULL) {
				s -> core0 -> temp = buffer_read(s -> c0buffer);
			}
			// if the link is full and the buffer wants to send another packet down it, a
			// collision occurs 
			else if (s -> c0buffer -> count != 0) {
				printf("COLLISION\n");
			}

			// core1
			if (s -> core1 -> temp == NULL) {
				s -> core1 -> temp = buffer_read(s -> c1buffer);
			}
			else if (s -> c1buffer -> count != 0) {
				printf("COLLISION\n");
			}

			// exclude top layer
			if (i != NUM_LAYERS-1) {
				// edge0
				if (s -> edge0 -> temp == NULL) {
					s -> edge0 -> temp = buffer_read(s -> e0buffer);
				}
				else if (s -> e0buffer -> count != 0) {
					printf("COLLISION\n");
				}

				// edge1
				if (s -> edge1 -> temp == NULL) {
					s -> edge1 -> temp = buffer_read(s -> e1buffer);
				}
				else if (s -> e1buffer -> count != 0) {
					printf("COLLISION\n");
				}
			}
		}
	}
}

void check_switches_for_received_packets(Network *n) {

	int i, j;
	int full;
	Switch *s;

	for (i=0; i<NUM_LAYERS; i++) {
		for (j=0; j<NUM_CORES; j++) {

			// current switch pointer
			s = n -> switches[i][j];

			// core0: if there is a packet in the link ... 
			if (s -> core0 -> comm != NULL) {
				// ... going in the correct direction ...
				if (s -> core0 -> comm -> direction == EDGE) {
					s -> core0 -> comm -> count--;
					// ... if it is to be forwarded in the same direction ...
					if (s -> core0 -> comm -> count != 0) {
						// ... down which link ...
						if (s -> core0 -> comm -> rout[i+1] == 0) {
							full = buffer_write(s -> e0buffer, s -> core0 -> comm);
						}
						else {
							full = buffer_write(s -> e1buffer, s -> core0 -> comm);
						}
					}
					// ... or if the direction is to be changed ...
					else {
						s -> core0 -> comm -> direction = CORE;
						// ... down which link ...
						if (s -> core0 -> comm -> addr[i] == 0) {
							full = buffer_write(s -> c0buffer, s -> core0 -> comm);
						}
						else {
							full = buffer_write(s -> c1buffer, s -> core0 -> comm);
						}
					}
					// tidy up old packet from link
					if (full == 0) {
						s -> core0 -> comm = NULL;
					}
				}
			}

			// core1 
			if (s -> core1 -> comm != NULL) {
				if (s -> core1 -> comm -> direction == EDGE) {
					s -> core1 -> comm -> count--;
					if (s -> core1 -> comm -> count != 0) {
						if (s -> core1 -> comm -> rout[i+1] == 0) {
							full = buffer_write(s -> e0buffer, s -> core1 -> comm);
						}
						else {
							full = buffer_write(s -> e1buffer, s -> core1 -> comm);
						}
					}
					else {
						s -> core1 -> comm -> direction = CORE;
						if (s -> core1 -> comm -> addr[i] == 0) {
							full = buffer_write(s -> c0buffer, s -> core1 -> comm);
						}
						else {
							full = buffer_write(s -> c1buffer, s -> core1 -> comm);
						}
					}
					// tidy up old packet from link
					if (full == 0) {
						s -> core1 -> comm = NULL;
					}
				}
			}

			// exclude top layer
			if (i != NUM_LAYERS-1) {
			// edge0
				if (n -> switches[i][j] -> edge0 -> comm != NULL) {
					if (n -> switches[i][j] -> edge0 -> comm -> direction == CORE) {
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
					if (n -> switches[i][j] -> edge1 -> comm -> direction == CORE) {
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
}

void send_packets_from_cores(Network *n) {

	int i;

	for (i=0; i<NUM_CORES; i++) {
		// checks for packet to send
		if (n -> cores[i] -> send != NULL) {
			// checks which link to send down
			if (n -> cores[i] -> send -> rout[0] == 0) {
				// checks link is free
				if (n -> cores[i] -> io0 -> temp == NULL) {
					n -> cores[i] -> io0 -> temp = n -> cores[i] -> send;
				}
				// if link is not free, collision
				else {
					printf("COLLISION\n");
				}
			}
			else {
				if (n -> cores[i] -> io1 -> temp == NULL) {
					n -> cores[i] -> io1 -> temp = n -> cores[i] -> send;
				}
				else {
					printf("COLLISION\n");
				}
			}	

			// reset packet status
			n -> cores[i] -> send = NULL;
		}
	}
}

void check_cores_for_received_packets(Network *n) {

	int i;

	for (i=0; i<NUM_CORES; i++) {
		// checks for packet in link0
		if (n -> cores[i] -> io0 -> comm != NULL) {
			// checks for correct direction
			if (n -> cores[i] -> io0 -> comm -> direction == CORE) {
				n -> cores[i] -> recv = n -> cores[i] -> io0 -> comm;
				printf("Packet received on core %d with data: %d\n", i, n -> cores[i] -> recv -> data);
				n -> cores[i] -> io0 -> comm = NULL;
			}
		}

		// checks for packet in link1
		if (n -> cores[i] -> io1 -> comm != NULL) {
			// checks for correct direction
			if (n -> cores[i] -> io1 -> comm -> direction == CORE) {
				n -> cores[i] -> recv = n -> cores[i] -> io1 -> comm;
				printf("Packet received on core %d with data: %d\n", i, n -> cores[i] -> recv -> data);
				n -> cores[i] -> io1 -> comm = NULL;
			}
		}
	}
}

void link_cleanup(Network *n) {

	int i, j;
	Link *l;

	// move temp values to links, completing parallel emulation
	for (i=0; i<NUM_LAYERS; i++) {
		for (j=0; j<2*NUM_CORES; j++) {

			// current link pointer
			l = n -> links[i][j];

			if (l -> comm == NULL) {
				l -> comm = l -> temp;
				l -> temp = NULL;
			}
			else if (l -> temp != NULL) {
				printf("COLLISION\n");
			}
		}
	}
}

void print_network_state(Network *n) {

	int i, j;

	// print each network layer
	for (i=NUM_LAYERS-1; i>=0; i--) {

		printf("\nLAYER %d\n----------------------\n\nActive switch buffers:\n\n", i);

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
