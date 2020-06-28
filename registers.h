#ifndef REGISTERS_H_
#define REGISTERS_H_

#define REGS 4

typedef unsigned char byte;

extern byte regs[];

#include <bitset>
#include <sstream>
#include <fstream>

void update(WINDOW *&rwin)
{
        for (size_t i = 0; i < REGS; i++) {
		std::ostringstream oss;

		oss << std::bitset <8> (regs[i]);

        	mvwprintw(rwin, i + 3, 1, "%x   %s", i, oss.str().c_str());
	}
}

extern size_t RAMS;

extern std::string asmb[];

// Assembler
using namespace std; // remove later

std::vector <std::pair <byte, byte>> assembler(std::string fpath)
{
	std::ifstream fin(fpath);

	std::vector  <std::pair <byte, byte>> out;

	byte address = 0;

	std::string line;
	while (getline(fin, line)) {
		if (line.empty())
			continue;

		std::vector <std::string> wds;

		std::istringstream iss(line);

		std::string wd;
		while (iss >> wd)
			wds.push_back(wd);
		
		// Remove later
		assert(wds.size() > 0);

		std::string inst = wds[0];
		
		byte opc = 0;
		for (opc = 0; opc < 4; opc++) {
			if (inst == asmb[opc])
				break;
		}

		if (opc == 4 && inst == ".orig") {
			string addr = wds[1];

			size_t tmp;

			if (addr[1] == 'x' || addr[1] == 'X') {
				addr = addr.substr(2);
				
				istringstream iss(addr);
				iss >> hex >> tmp;
			} else if (addr[1] == 'b' || addr[1] == 'B') {
				addr = addr.substr(2);

				tmp = bitset <8> (addr).to_ulong();
			} else {
				istringstream iss(addr);
				iss >> dec >> tmp;
			}

			address = tmp;
			continue;
		}

		std::string conc;
		for (size_t i = 1; i < wds.size(); i++) {
			conc += wds[i];

			if (i < wds.size() - 1)
				conc += " ";
		}

		byte rd;
		byte rs;
		byte rt;

		sscanf(conc.c_str(), "$%hhd, $%hhd, $%hhd", &rd, &rs, &rt);

		byte bin = (opc << 6) + (rd << 4) + (rs << 2) + rt;

		out.push_back({address, bin});

		address++;
	}

	return out;
}

#endif
