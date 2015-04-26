#include "network.h"

int main (int argc,  char* argv[]) {

	int i, j, k;
	Network *n = NULL;
	Packet **p;

	char *active_cores = (char *) malloc ((NUM_CORES+2)*sizeof(char));
	int **active_core_data = (int **) malloc (NUM_CORES*sizeof(int *));
	for (i=0; i<NUM_CORES; i++) {
		active_core_data[i] = (int *) malloc (4*sizeof(int));
	}
	char is_packets;

	// set random seed
	srand(time(NULL));

	// initialise network
	n = init_network(n);

	// initialise processor memory
	for (i=0; i<NUM_CORES; i++) {
        pmem[i] = (unsigned char *) mem[i];
    }
	
	// load binary program
	printf("\n");
	global_running = true;
    load();

    // ready processors
	for (i=0; i<NUM_CORES; i++) {
        running[i] = true;
        oreg[i] = 0;
    }	
		
	// execute program on parallel processors
	while (global_running) {

		is_packets = false;

        for (i=0; i<NUM_CORES; i++) {

        	active_cores[i] = 0;

            if (running[i]) {

        	    inst[i] = pmem[i][pc[i]];
        	    pc[i] = pc[i] + 1;
        	    oreg[i] = oreg[i] | (inst[i] & 0xf);
        		
        	    switch ((inst[i] >> 4) & 0xf) {

            		case i_ldwsp:  breg[i] = areg[i]; areg[i] = mem[i][sp[i] + oreg[i]]; oreg[i] = 0; break;
            		case i_stwsp:  mem[i][sp[i] + oreg[i]] = areg[i]; areg[i] = breg[i]; oreg[i] = 0; break;
            		case i_ldawsp: breg[i] = areg[i]; areg[i] = sp[i] + oreg[i]; oreg[i] = 0; break;   
            			  
            		case i_ldc:    breg[i] = areg[i]; areg[i] = oreg[i]; oreg[i] = 0; break;
            		case i_ldwcp:  breg[i] = areg[i]; areg[i] = mem[i][oreg[i]]; oreg[i] = 0; break;
            		case i_ldap:   breg[i] = areg[i]; areg[i] = pc[i] + oreg[i]; oreg[i] = 0; break;
            			  
            		case i_ldwi:   areg[i] = mem[i][areg[i] + oreg[i]]; oreg[i] = 0; break;
            		case i_stwi:   mem[i][areg[i] + oreg[i]] = breg[i]; oreg[i] = 0; break;
            			  
            		case i_br:     pc[i] = pc[i] + oreg[i]; oreg[i] = 0; break;
            		case i_brf:    if (areg[i] == 0) pc[i] = pc[i] + oreg[i]; oreg[i] = 0; break;
            		case i_adj:    sp[i] = sp[i] + oreg[i]; oreg[i] = 0; break; 
            			  
            		case i_eqc:    if (areg[i] == oreg[i]) areg[i] = true; else areg[i] = false; oreg[i] = 0; break;
            		case i_addc:   areg[i] = areg[i] + oreg[i]; oreg[i] = 0; break; 
            			  
            		case i_pfix:   oreg[i] = oreg[i] << 4; break;
            		case i_nfix:   oreg[i] = 0xFFFFFF00 | (oreg[i] << 4); break;
            			  
            		case i_opr:

            			switch (oreg[i]) {

            				case o_add:  areg[i] = areg[i] + breg[i]; break;
            				case o_sub:  areg[i] = breg[i] - areg[i]; break;
            				case o_eq:   if (areg[i] == breg[i]) areg[i] = true; else areg[i] = false; break;
            				case o_lss:  if (((int)breg[i]) < ((int)areg[i])) areg[i] = true; else areg[i] = false; break;
            				  
            				case o_and:  areg[i] = areg[i] & breg[i]; break;
            				case o_or:   areg[i] = areg[i] | breg[i]; break;
            				case o_xor:  areg[i] = areg[i] ^ breg[i]; break;
            				case o_not:  areg[i] = ~ areg[i]; break;
            				  
            				case o_shl:  areg[i] = breg[i] << areg[i]; break;
            				case o_shr:  areg[i] = breg[i] >> areg[i]; break;
            				  
            				case o_brx:  pc[i] = areg[i]; areg[i] = breg[i]; break;
            				case o_call: mem[i][sp[i]] = pc[i]; pc[i] = areg[i]; areg[i] = breg[i]; break;
            				case o_ret:  pc[i] = mem[i][sp[i]]; break;
            				case o_setsp: sp[i] = areg[i]; areg[i] = breg[i]; break;		  
            				  
            				case o_svc:  svc(i); break;

                            case o_in: 

                            	if (n -> cores[i] -> ports[areg[i]] == NULL) {
                            		pc[i] = pc[i] - 1;
                            	}
                            	else {
                            		breg[i] = areg[i];
                            		areg[i] = n -> cores[i] -> ports[breg[i]] -> data;
                            		n -> cores[i] -> ports[breg[i]] == NULL;
                            	}
                            	break;
                            
                            case o_out:

                            	// indicate core has packet to send
                            	is_packets = true;
                            	active_cores[i] = 1;
                            	// set packet source
                            	active_core_data[i][0] = i;
                            	// set packet destination
                            	active_core_data[i][1] = areg[i];
                            	// set destination port
                            	active_core_data[i][2] = 0;
                            	// set packet data
                            	active_core_data[i][3] = breg[i];
                            	break;
            		  	}

            			oreg[i] = 0; break;		  
        	    }
            }
        }

        // add new data packets to network
        if (is_packets) {

        	// run routing algorithm
        	p = offline_route_planner(active_cores, active_core_data);
			//p = tprr_route_planner(active_cores, active_core_data);

        	// add packets to cores
			for (i=0; i<NUM_CORES; i++) {
				if (active_cores[i] == 1) {
					add_packet_to_core(n, p[i], i);

					// print all routes
					// out route
					printf("Packet %d. Out: ", i);
					for (k=0; k<p[i] -> count; k++) {
						if (p[i] -> rout[k] == 0) printf("0");
						else printf("1");
					}
					// in route
					printf(". In: ");
					for (k=p[i] -> count-1; k >= 0; k--) {
						if (p[i] -> addr[k] == 0) printf("0");
						else printf("1");
					}
					printf(".\n");
				}
			}
        }

        // carry out one timestep of network routing
        network_timesteps(n, 2);

        // check for finish
        global_running = false;
        for (i=0; i<NUM_CORES; i++) {
            global_running |= running[i];
        }
	}

	/*//p = offline_route_planner();
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
	for (i=0; i<13; i++) {
		network_timestep(n, 1);
		print_network_state(n);
	}*/

	return 0;
}


void load() { 
    int low;
    int length;	
    int n, i;
    codefile = fopen("a.bin", "rb");
    
    for (i=0; i<NUM_CORES; i++) {
        low = inbin();	
        length = ((inbin() << 16) | low);// << 2;
        //printf("length[%d]: %d\n", i, length);
        low = inbin();	
        pc[i] = ((inbin() << 16) | low) << 2;
        //printf("pc[%d]: %d\n", i, pc[i]);
        for (n = 0; n < length; n++) {
            pmem[i][n] = fgetc(codefile);
        }
    }
    fclose(codefile);
}

int inbin() {
    int lowbits;
    int highbits;
    lowbits = fgetc(codefile);
    highbits = fgetc(codefile);
    return (highbits << 8) | lowbits;
}

void svc(int i) { 
    switch (areg[i]) {
        case 0: running[i] = false; break;
    	case 1: simout(mem[i][sp[i] + 2], mem[i][sp[i] + 3]); break;
    	case 2: areg[i] = simin(mem[i][sp[i] + 2]) & 0xFF; break;
    }
}

void simout(int b, int s) { 
    char fname[] = {'s', 'i', 'm', ' ', 0};
    int f;
    if (s < 256)
	    putchar(b);
    else {
        f = (s >> 8) & 7;
	    if (! connected[f]) { 
            fname[3] = f + '0';
	        simio[f] = fopen(fname, "w");
	        connected[f] = true;
	    }
	    fputc(b, simio[f]);
    }
}

int simin(int s) { 
    char fname[] = {'s', 'i', 'm', ' ', 0};
    int f;
    if (s < 256)
	    return getchar();
    else {
        f = (s >> 8) & 7;
	    fname[3] = f + '0';
	    if (! connected[f]) { 
            simio[f] = fopen(fname, "r");
	        connected[f] = true;
	    }	
	    return fgetc(simio[f]);  
    }
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
		for (j=0; j<NUM_PORTS; j++) n -> cores[i] -> ports[j] = NULL;
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
				c -> ports[c -> io0 -> comm -> port] = c -> io0 -> comm;
				printf("Packet received on core %d (port %d) with data: %d\n", i, 
					c -> io0 -> comm -> port, c -> io0 -> comm -> data);
				c -> io0 -> comm = NULL;
			}
		}

		// checks for packet in link1
		if (c -> io1 -> comm != NULL) {
			// checks for correct direction
			if (c -> io1 -> comm -> direction == CORE) {
				c -> ports[c -> io1 -> comm -> port] = c -> io1 -> comm;
				printf("Packet received on core %d (port %d) with data: %d\n", i, 
					c -> io1 -> comm -> port, c -> io1 -> comm -> data);
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
		for (i=0; i<NUM_PORTS; i++) {
			if (n -> cores[j] -> ports[i] != NULL) {
				printf("Core %d data received on port %d: %d\n", j, i, n -> cores[j] -> ports[i] -> data);
			}
		}
	}
}

Packet *create_packet(int source, int destination, int port, int data) {

	Packet *p;

	// initialise packet memory
	p = (Packet *)malloc(sizeof(Packet));

	// add packet data
	p -> source = source;
	p -> destination = destination;
	p -> port = port;
	p -> data = data;
	p -> count = 0;
	p -> direction = EDGE;

	return p;
}


Packet **offline_route_planner(char *active_cores, int **active_core_data) {

	int i, j, base;
	char switch_state[NUM_CORES*NUM_LAYERS];
	int route_positions[2][NUM_CORES];
	Packet **packets = (Packet **) malloc (NUM_CORES*sizeof(Packet *));

	for (i=0; i<NUM_CORES*NUM_LAYERS; i++) {
		switch_state[i] = 0;
	}

	// example permutations
	/*packets[0] = create_packet(0, 2, 0, 2);
	packets[1] = create_packet(1, 4, 0, 14);
	packets[2] = create_packet(2, 6, 0, 26);
	packets[3] = create_packet(3, 1, 0, 31);
	packets[4] = create_packet(4, 7, 0, 47);
	packets[5] = create_packet(5, 0, 0, 50);
	packets[6] = create_packet(6, 5, 0, 65);
	packets[7] = create_packet(7, 3, 0, 73);*/

	// create packets
	for (i=0; i<NUM_CORES; i++) {
		if (active_cores[i] == 1) {
			packets[i] = create_packet(active_core_data[i][0], active_core_data[i][1],
				active_core_data[i][2], active_core_data[i][3]);
		}
		else packets[i] = NULL;
	}

	for (i=1; i<=NUM_LAYERS; i++) {
		base = (int)pow(2,i);
		for (j=0; j<NUM_CORES; j++) {

			if (packets[j] != NULL) {
				if (packets[j] -> count == 0) {
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
	}

	return packets;
}

Packet **tprr_route_planner(char *active_cores, int **active_core_data) {

	int i, j, base;
	Packet **packets = (Packet **) malloc (NUM_CORES*sizeof(Packet *));
	int packet_pos;

	// example permutations
	/*packets[0] = create_packet(0, 2, 0, 2);
	packets[1] = create_packet(1, 4, 1, 14);
	packets[2] = create_packet(2, 6, 2, 26);
	packets[3] = create_packet(3, 1, 3, 31);
	packets[4] = create_packet(4, 7, 0, 47);
	packets[5] = create_packet(5, 0, 1, 50);
	packets[6] = create_packet(6, 5, 2, 65);
	packets[7] = create_packet(7, 3, 3, 73);*/

	// create packets
	for (i=0; i<NUM_CORES; i++) {
		if (active_cores[i] == 1) {
			packets[i] = create_packet(active_core_data[i][0], active_core_data[i][1],
				active_core_data[i][2], active_core_data[i][3]);
		}
		else packets[i] = NULL;
	}


	for (i=0; i<NUM_CORES; i++) {

		if (packets[i] != NULL) {

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

	}

	return packets;
}