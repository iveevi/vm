#ifndef REGISTERS_H_
#define REGISTERS_H_

#define REGS 4

typedef unsigned char byte;

extern byte regs[];

#include <bitset>
#include <sstream>

void update(WINDOW *&rwin)
{
        for (size_t i = 0; i < REGS; i++) {
		std::ostringstream oss;

		oss << std::bitset <8> (regs[i]);

        	mvwprintw(rwin, i + 3, 1, "%x   %s", i, oss.str().c_str());
	}
}

#define RAMS    REGS

#endif