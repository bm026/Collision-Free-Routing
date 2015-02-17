#include "routing.h"
//#include "network.h"

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
	packets[6] = create_packet(6, 7, 65);
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
						switch_state[(i-2)*NUM_CORES+j] = 1;
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
