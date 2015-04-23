#include "network.h"

int main (int argc,  char* argv[]) {

	int i, j, k;
	Network *n = NULL;
	Packet **p;

	srand(time(NULL));

	n = init_network(n);

	//p = offline_route_planner();
	p = tprr_route_planner();

	for (i=0; i<NUM_CORES; i++) {
		add_packet_to_core(n, p[i], i);

		// print all routes
		// out route
		printf("Packet %d. Out: ", i);
		for (k=0; k<p[i] -> count; k++) {
			if (p[i] -> rout[k] == 0) printf("0");
			else printf("1");
		}
		printf(". In: ");
		for (k=p[i] -> count-1; k >= 0; k--) {
			if (p[i] -> addr[k] == 0) printf("0");
			else printf("1");
		}
		printf(".\n");
	}



	//p = create_packet(p, 41, 1, "1", "1");
	//add_packet_to_core(n, p, 0);

	//network_timesteps(n, 3);

	//p = create_packet(p, 42, 1, "1", "0");
	//add_packet_to_core(n, p, 1);

	//print_network_state(n);



	network_timesteps(n, 20);

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
	|   LAYOUT OF LINK/SWITCH IDS                        |
	|   -------------------------                        |
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

/*Packet *create_packet(Packet *p, int data, int count, char *edge_route, char *core_route) {

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
}*/

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
		//print_network_state(n);
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
	printf("Packet added to core %d\n", core_num);
}

void send_packets_from_switches(Network *n) {

	int i, j;
	int base;
	Switch *s;

	for (i=0; i<NUM_LAYERS; i++) {
		for (j=0; j<NUM_CORES; j++) {

			// current switch pointer
			s = n -> switches[i][j];

			// core0
			if (s -> core0 -> temp == NULL && s -> core0 -> comm == NULL) {
				s -> core0 -> temp = buffer_read(s -> c0buffer);
			}
			// if the link is full and the buffer wants to send another packet down it, a
			// collision occurs 
			else if (s -> c0buffer -> count != 0) {
				// get link number and report collision
				base = (int) pow(2, i+1);
				if (j % base < base/2) {
					printf("COLLISION on layer:%d, link:%d\n", i, 2*j);
				}
				else {
					printf("COLLISION on layer:%d, link:%d\n", i, 2*j+1-base);
				}
			}

			// core1
			if (s -> core1 -> temp == NULL && s -> core1 -> comm == NULL) {
				s -> core1 -> temp = buffer_read(s -> c1buffer);
			}
			else if (s -> c1buffer -> count != 0) {
				// get link number and report collision
				base = (int) pow(2, i+1);
				if (j % base < base/2) {
					printf("COLLISION on layer:%d, link:%d\n", i, 2*j+base);
				}
				else {
					printf("COLLISION on layer:%d, link:%d\n", i, 2*j+1);
				}
			}

			// exclude top layer
			if (i != NUM_LAYERS-1) {
				// edge0
				if (s -> edge0 -> temp == NULL && s -> edge0 -> comm == NULL) {
					s -> edge0 -> temp = buffer_read(s -> e0buffer);
				}
				else if (s -> e0buffer -> count != 0) {
					printf("COLLISION on layer:%d, link:%d\n", i+1, 2*j);
				}

				// edge1
				if (s -> edge1 -> temp == NULL && s -> edge1 -> comm == NULL) {
					s -> edge1 -> temp = buffer_read(s -> e1buffer);
				}
				else if (s -> e1buffer -> count != 0) {
					printf("COLLISION on layer:%d, link:%d\n", i+1, 2*j+1);
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
				if (s -> edge0 -> comm != NULL) {
					if (s -> edge0 -> comm -> direction == CORE) {
						if (s -> edge0 -> comm -> addr[i] == 0) {
							full = buffer_write(s -> c0buffer, s -> edge0 -> comm);
						}
						else {
							full = buffer_write(s -> c1buffer, s -> edge0 -> comm);
						}
						// tidy up old packet from link
						if (full == 0) {
							s -> edge0 -> comm = NULL;
						}
					}
				}

				// edge1
				if (s -> edge1 -> comm != NULL) {
					if (s -> edge1 -> comm -> direction == CORE) {
						if (s -> edge1 -> comm -> addr[i] == 0) {
							full = buffer_write(s -> c0buffer, s -> edge1 -> comm);
						}
						else {
							full = buffer_write(s -> c1buffer, s -> edge1 -> comm);
						}
						// tidy up old packet from link
						if (full == 0) {
							s -> edge1 -> comm = NULL;
						}
					}
				}
			}
		}
	}
}

void send_packets_from_cores(Network *n) {

	int i;
	Core *c;

	for (i=0; i<NUM_CORES; i++) {

		// current core pointer
		c = n -> cores[i];

		// checks for packet to send
		if (c -> send != NULL) {
			// checks which link to send down
			if (c -> send -> rout[0] == 0) {
				// checks link is free
				if (c -> io0 -> temp == NULL && c -> io0 -> comm == NULL) {
					c -> io0 -> temp = c -> send;
					printf("Packet sent from core %d with data: %d\n", i, c -> send -> data);
					c -> send = NULL;
				}
				// if link is not free, collision
				else {
					printf("COLLISION on layer:0, link:%d\n", 2*i);
				}
			}
			else {
				if (c -> io1 -> temp == NULL && c -> io1 -> comm == NULL) {
					c -> io1 -> temp = c -> send;
					printf("Packet sent from core %d with data: %d\n", i, c -> send -> data);
					c -> send = NULL;
				}
				else {
					printf("COLLISION on layer:0, link:%d\n", 2*i+1);
				}
			}	
		}
	}
}

void check_cores_for_received_packets(Network *n) {

	int i;
	Core *c;

	for (i=0; i<NUM_CORES; i++) {

		// current core pointer
		c = n -> cores[i];
		// checks for packet in link0
		if (c -> io0 -> comm != NULL) {
			// checks for correct direction
			if (c -> io0 -> comm -> direction == CORE) {
				c -> recv = c -> io0 -> comm;
				printf("Packet received on core %d with data: %d\n", i, c -> recv -> data);
				c -> io0 -> comm = NULL;
			}
		}

		// checks for packet in link1
		if (c -> io1 -> comm != NULL) {
			// checks for correct direction
			if (c -> io1 -> comm -> direction == CORE) {
				c -> recv = c -> io1 -> comm;
				printf("Packet received on core %d with data: %d\n", i, c -> recv -> data);
				c -> io1 -> comm = NULL;
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

			// collision already checked?
			//if (l -> comm == NULL) {
			//	l -> comm = l -> temp;
			//	l -> temp = NULL;
			//}
			//else if (l -> temp != NULL) {
			//	printf("COLLISION\n");
			//}

			l -> comm = l -> temp;
			l -> temp = NULL;
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

Packet **offline_route_planner() {

	int i, j, base;
	//Packet *packets[NUM_CORES];
	char switch_state[NUM_CORES*NUM_LAYERS];
	int route_positions[2][NUM_CORES];
	Packet **packets = (Packet **) malloc (NUM_CORES*sizeof(Packet *));

	for (i=0; i<NUM_CORES*NUM_LAYERS; i++) {
		switch_state[i] = 0;
	}

	// example permutations
	packets[0] = create_packet(0, 2, 2);
	packets[1] = create_packet(1, 4, 14);
	packets[2] = create_packet(2, 6, 26);
	packets[3] = create_packet(3, 1, 31);
	packets[4] = create_packet(4, 7, 47);
	packets[5] = create_packet(5, 0, 50);
	packets[6] = create_packet(6, 5, 65);
	packets[7] = create_packet(7, 3, 73);

	for (i=1; i<=NUM_LAYERS; i++) {
		base = (int)pow(2,i);
		for (j=0; j<NUM_CORES; j++) {

			if (packets[j] -> count == 0 && packets[j] != NULL) {
				// for first link, pick random
				if (i == 1) {
					packets[j] -> rout[0] = rand() % 2;
					
					if (packets[j] -> rout[0] == 0) {
						// update route position and set back route
						if (packets[j] -> source % 2 == 0) {
							route_positions[0][j] = packets[j] -> source;
						}
						else {
							route_positions[0][j] = packets[j] -> source-1;
						}
						if (packets[j] -> destination % 2 == 0) {
							route_positions[1][j] = packets[j] -> destination;
							packets[j] -> addr[0] = 0;
						}
						else {
							route_positions[1][j] = packets[j] -> destination-1;
							packets[j] -> addr[0] = 1;
						}

					}
					else {
						// update route position and set back route
						if (packets[j] -> source % 2 == 0) {
							route_positions[0][j] = packets[j] -> source+1;
						}
						else {
							route_positions[0][j] = packets[j] -> source;
						}
						if (packets[j] -> destination % 2 == 0) {
							route_positions[1][j] = packets[j] -> destination+1;
							packets[j] -> addr[0] = 0;
						}
						else {
							route_positions[1][j] = packets[j] -> destination;
							packets[j] -> addr[0] = 1;
						}
					}
				}

				// main network traversal
				else {
					// switch is unused, take link 0
					if (switch_state[(i-2)*NUM_CORES+route_positions[0][j]] == 0) {
						// change switch state
						switch_state[(i-2)*NUM_CORES+route_positions[0][j]] = 1;
						// route left
						packets[j] -> rout[i-1] = 0;
						// update route position and set back route
						if (route_positions[0][j] % base < base/2) {
							// position stays same	
						}
						else {
							route_positions[0][j] -= (base/2);
						}
						if (route_positions[1][j] % base < base/2) {
							packets[j] -> addr[i-1] = 0;
						}
						else {
							packets[j] -> addr[i-1] = 1;
							route_positions[1][j] -= (base/2);
						}
					}

					// if switch has already been used, take link 1
					else {
						// route right
						packets[j] -> rout[i-1] = 1;
						// update route position and set back route
						if (route_positions[0][j] % base < base/2) {
							route_positions[0][j] += (base/2);	
						}
						if (route_positions[1][j] % base < base/2) {
							packets[j] -> addr[i-1] = 0;
							route_positions[1][j] += (base/2);
						}
						else {
							packets[j] -> addr[i-1] = 1;
						}
					}
				}

				// check for completed path
				if (route_positions[0][j] == route_positions[1][j]) {
					packets[j] -> count = i;
				}
			}
		}
	}

	return packets;
}

Packet **tprr_route_planner() {

	int i, j, base;
	Packet **packets = (Packet **) malloc (NUM_CORES*sizeof(Packet *));
	int packet_pos;

	// example permutations
	packets[0] = create_packet(0, 2, 2);
	packets[1] = create_packet(1, 4, 14);
	packets[2] = create_packet(2, 6, 26);
	packets[3] = create_packet(3, 1, 31);
	packets[4] = create_packet(4, 7, 47);
	packets[5] = create_packet(5, 0, 50);
	packets[6] = create_packet(6, 5, 65);
	packets[7] = create_packet(7, 3, 73);


	for (i=0; i<NUM_CORES; i++) {

		packet_pos = packets[i] -> source;
		packets[i] -> count = NUM_LAYERS;

		// out route
		for (j=1; j<=NUM_LAYERS; j++) {
			base = (int) pow(2,j);

			// pick random route
			packets[i] -> rout[j-1] = rand() % 2;

			// if route is 0
			if (packets[i] -> rout[j-1] == 0) {
				if (packet_pos % base < base/2) {
					// packet position stays same
				}
				else {
					packet_pos -= (base/2);
				}
			}
			// if route is 1
			else {
				if (packet_pos % base < base/2) {
					packet_pos += (base/2);
				}
				else {
					// packet position stays same
				}
			}
		}

		// in route
		for (j=NUM_LAYERS; j>0; j--) {
			base = (int) pow(2,j);

			if (packets[i] -> destination % base < base/2) {
				packets[i] -> addr[j-1] = 0;
			}
			else {
				packets[i] -> addr[j-1] = 1;
			}
		}

	}

	return packets;
}

Packet *create_packet(int source, int destination, int data) {

	//int i;
	Packet *p;

	// initialise packet memory
	p = (Packet *)malloc(sizeof(Packet));

	// add packet data
	p -> source = source;
	p -> destination = destination;
	p -> data = data;
	p -> count = 0;
	p -> direction = EDGE;

	/*// interpret route
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
	}*/

	return p;
}