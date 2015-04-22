
#include <stdio.h>
#include <stdlib.h>

#define NUM_PROCESSORS 2

#define true     -1
#define false    0


FILE *codefile;

FILE *simio[8];

char connected[] = {0, 0, 0, 0, 0, 0, 0, 0};

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



unsigned int global_running;
unsigned int i;

unsigned int mem[NUM_PROCESSORS][200000];
unsigned char *pmem[NUM_PROCESSORS];

unsigned int pc[NUM_PROCESSORS];
unsigned int sp[NUM_PROCESSORS];

unsigned int areg[NUM_PROCESSORS];
unsigned int breg[NUM_PROCESSORS];
unsigned int oreg[NUM_PROCESSORS];

unsigned int inst[NUM_PROCESSORS];
unsigned int running[NUM_PROCESSORS];

main() {

    for (i=0; i<NUM_PROCESSORS; i++) {
        //pmem[i] = (unsigned char *) malloc (200000*sizeof(unsigned char));
        pmem[i] = (unsigned char *) mem[i];
    }
		
	printf("\n");
	global_running = true;

	for (i=0; i<NUM_PROCESSORS; i++) {
        load();
        running[i] = true;
        oreg[i] = 0;
    }	
	
    for (i=0; i<NUM_PROCESSORS; i++) {	
	//while (global_running) {
        while (running[i]) {

        //for (i=0; i<NUM_PROCESSORS; i++) {

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
            				  
            				case o_svc:  svc(); break;

                            case o_in: break;
                            case o_out: break;
            		  	};

            			oreg[i] = 0; break;		  
        	    };
            }
        }

        // check for finish
        global_running = false;
        for (i=0; i<NUM_PROCESSORS; i++) {
            global_running |= running[i];
        }
	}
}	
		  		  		  
load() { 
    int low;
    int length;	
    int n;
    codefile = fopen("a.bin", "rb");
    low = inbin();	
    length = ((inbin() << 16) | low) << 2;
    printf("length: %d\n", length);
    low = inbin();	
    pc[i] = ((inbin() << 16) | low) << 2;
    for (n = 0; n < length; n++) {
        pmem[i][n] = fgetc(codefile);
    }
    fclose(codefile);
};

inbin(d) {
    int lowbits;
    int highbits;
    lowbits = fgetc(codefile);
    highbits = fgetc(codefile);
    return (highbits << 8) | lowbits;
};

svc() { 
    switch (areg[i]) {
        case 0: running[i] = false; break;
    	case 1: simout(mem[i][sp[i] + 2], mem[i][sp[i] + 3]); break;
    	case 2: areg[i] = simin(mem[i][sp[i] + 2]) & 0xFF; break;
    };		
};

simout(b, s) { 
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
	    };	
	    fputc(b, simio[f]);
    };	  
};

simin(s) { 
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
	    return fgetc(simio[f]) ;	  
    };
};


