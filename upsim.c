
#include "stdio.h"

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



unsigned int mem[200000];
unsigned char *pmem = (unsigned char *) mem;

unsigned int pc;
unsigned int sp;

unsigned int areg;
unsigned int breg;
unsigned int oreg;

unsigned int inst;

unsigned int running;

main() 
{
		
printf("\n");
	
load();
	
running = true;	
	
oreg = 0;	
	
while (running) 

{ 
  inst = pmem[pc]; 
  pc = pc + 1;	

  oreg = oreg | (inst & 0xf);	
	
  switch ((inst >> 4) & 0xf)
  {
	case i_ldwsp:  breg = areg; areg = mem[sp + oreg]; oreg = 0; break;
	case i_stwsp:  mem[sp + oreg] = areg; areg = breg; oreg = 0; break;
	case i_ldawsp: breg = areg; areg = sp + oreg; oreg = 0; break;   
		  
	case i_ldc:    breg = areg; areg = oreg; oreg = 0; break;
	case i_ldwcp:  breg = areg; areg = mem[oreg]; oreg = 0; break;
	case i_ldap:   breg = areg; areg = pc + oreg; oreg = 0; break;
		  
	case i_ldwi:   areg = mem[areg + oreg]; oreg = 0; break;
	case i_stwi:   mem[areg + oreg] = breg; oreg = 0; break;
		  
	case i_br:     pc = pc + oreg; oreg = 0; break;
	case i_brf:    if (areg == 0) pc = pc + oreg; oreg = 0; break;
	case i_adj:    sp = sp + oreg; oreg = 0; break; 
		  
	case i_eqc:    if (areg == oreg) areg = true; else areg = false; oreg = 0; break;
	case i_addc:   areg = areg + oreg; oreg = 0; break; 
		  
	case i_pfix:   oreg = oreg << 4; break;
	case i_nfix:   oreg = 0xFFFFFF00 | (oreg << 4); break;
		  
	case i_opr:
	switch (oreg)
	  {
		case o_add:  areg = areg + breg; break;
		case o_sub:  areg = breg - areg; break;
		case o_eq:   if (areg == breg) areg = true; else areg = false; break;
		case o_lss:  if (((int)breg) < ((int)areg)) areg = true; else areg = false; break;
		  
		case o_and:  areg = areg & breg; break;
		case o_or:   areg = areg | breg; break;
		case o_xor:  areg = areg ^ breg; break;
		case o_not:  areg = ~ areg; break;
		  
		case o_shl:  areg = breg << areg; break;
		case o_shr:  areg = breg >> areg; break;
		  
		case o_brx:  pc = areg; areg = breg; break;
		case o_call: mem[sp] = pc; pc = areg; areg = breg; break;
		case o_ret:  pc = mem[sp]; break;
		case o_setsp: sp = areg; areg = breg; break;		  
		  
		  case o_svc:  svc(); break;
	  };
	oreg = 0; break;		  
  
  };
	
}
	
}	
		  		  		  
load()
{ int low;
  int length;	
  int n;
  codefile = fopen("a.bin", "rb");
  low = inbin();	
  length = ((inbin() << 16) | low) << 2;
  low = inbin();	
  pc = ((inbin() << 16) | low) << 2;	
  for (n = 0; n < length; n++)
    pmem[n] = fgetc(codefile);
};

inbin(d) 
{ int lowbits;
  int highbits;
  lowbits = fgetc(codefile);
  highbits = fgetc(codefile);
  return (highbits << 8) | lowbits;
};

svc() 
{ switch (areg)
  { case 0: running = false; break;
	case 1: simout(mem[sp + 2], mem[sp + 3]); break;
	case 2: areg = simin(mem[sp + 2]) & 0xFF; break;
  }			
}

simout(b, s)
{ char fname[] = {'s', 'i', 'm', ' ', 0};
  int f;
  if (s < 256)
	putchar(b);
  else 
  { f = (s >> 8) & 7;
	if (! connected[f])
	{ fname[3] = f + '0';
	  simio[f] = fopen(fname, "w");
	  connected[f] = true;
	};	
	fputc(b, simio[f]);
  };	  
};

simin(s)
{ char fname[] = {'s', 'i', 'm', ' ', 0};
  int f;
  if (s < 256)
	return getchar();
  else 
  { f = (s >> 8) & 7;
	fname[3] = f + '0';
	if (! connected[f])
	{ simio[f] = fopen(fname, "r");
	  connected[f] = true;
	}	
	return fgetc(simio[f]) ;	  
  };
};


