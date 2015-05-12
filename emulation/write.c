#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define PROC 1024

int main() {
	int i,j;
	int is_taken[PROC];
	int send_to[PROC];
	int receive_from[PROC];
	int temp;

	for (i=0; i<PROC; i++) is_taken[i] = 0;

	srand(time(NULL));
	j=PROC/2;

	for (i=0; i<PROC; i++) {
		/*temp = rand() % PROC;
		while (is_taken[temp] != 0 || temp == i) {
			temp++;
			if (temp == PROC) temp = 0;
		}
		is_taken[temp] = 1;
		send_to[i] = temp;
		receive_from[temp] = i;*/
		
		send_to[i] = j;
		receive_from[j] = i;
		j++;
		if (j == PROC) j = 0;
	}
	//receive_from[0] = PROC-1;
	//send_to[PROC-1] = 0;

	printf("\nproc main() is\n    var out;\n    var in;\n    network {\n");

	for (i=0; i<PROC-1; i++) {
		if (send_to[i] < 16) printf("        { %d ! 1023; %d ? in } &\n", send_to[i], receive_from[i]);
		else if (send_to[i] < 256) printf("        { %d ! 255; %d ? in } &\n", send_to[i], receive_from[i]);
		else printf("        { %d ! 0; %d ? in } &\n", send_to[i], receive_from[i]);
	}
	if (send_to[PROC-1] < 16) printf("        { %d ! 1023; %d ? in }\n    }\n", send_to[PROC-1], receive_from[PROC-1]);
	else if (send_to[PROC-1] < 256) printf("        { %d ! 255; %d ? in }\n    }\n", send_to[PROC-1], receive_from[PROC-1]);
	else printf("        { %d ! 0; %d ? in }\n    }\n", send_to[PROC-1], receive_from[PROC-1]);
	//printf("        { %d ! 1024; %d ? in }\n    }\n", send_to[PROC-1], receive_from[PROC-1]);

	return 0;
}