#include "network.h"

typedef struct {
	int count;
	char[LAYERS-1] rout;
	char[LAYERS] addr;
	int data;
} packet;

typedef struct {
	packet *core;
	packet *edge;
} link;

typedef struct {
	link *core0;
	link *core1;
	link *edge0;
	link *edge1;
	packet **buffer;
} switch;

int main (int argc,  char* argv[]) {

	return 0;
}