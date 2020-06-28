#include <bitset>
#include <cassert>
#include <iostream>
#include <ncurses.h>
#include <sstream>
#include <stack>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <sys/ioctl.h>

#include "registers.h"

using namespace std;

// Initializing Global Variables
size_t RAMS = 100;

// Computer
byte pc;
byte *ram;
byte regs[REGS];

// State preserving structures

// Only one register or one
// memory address can be changed
// in an instruction
struct state {
	size_t ri;
	byte rval;

	size_t mi;
	byte mval;
};

stack <state> history;

// Opcodes
enum opcodes {
	op_add,
	op_sub,
	op_and,
	op_or
};

string asmb[] = {
	"add",
	"sub",
	"and",
	"or"
};

void decode(byte line)
{
	// Opcode
	byte opc = (line & 0b11000000) >> 6;

	// Registers
	byte r_one = (line & 0b00110000) >> 4;
	byte r_two = (line & 0b00001100) >> 2;
	byte r_thr = (line & 0b00000011);
	
	history.push({r_one, regs[r_one]});

	switch (opc) {
	case op_add:
		regs[r_one] = regs[r_two] + regs[r_thr];
		break;
	case op_sub:
		regs[r_one] = regs[r_two] - regs[r_thr];
		break;
	case op_and:
		regs[r_one] = regs[r_two] & regs[r_thr];
		break;
	case op_or:
		regs[r_one] = regs[r_two] | regs[r_thr];
		break;
	}
}

string assembly(byte line)
{
	// Opcode
	byte opc = (line & 0b11000000) >> 6;

	// Registers
	byte r_one = (line & 0b00110000) >> 4;
	byte r_two = (line & 0b00001100) >> 2;
	byte r_thr = (line & 0b00000011);

	return asmb[opc] + "\t$" + to_string(r_one)
		+ ", $" + to_string(r_two) + ", $"
		+ to_string(r_thr);
}

WINDOW *redraw_reg_win()
{
	WINDOW *reg_win = newwin(REGS + 6, 15, 1, 55);

        box(reg_win, 0, 0);

	init_pair(1, COLOR_BLUE, COLOR_BLACK);
	
	wattrset(reg_win, COLOR_PAIR(1));

	wattron(reg_win, A_BOLD);
	mvwprintw(reg_win, 1, 1, " REGISTERS");
	wattroff(reg_win, A_BOLD);

	wstandend(reg_win);

	wmove(reg_win, 2, 1);
	whline(reg_win, ACS_HLINE, 13);
	for (size_t i = 0; i < REGS; i++) {
		ostringstream oss;

		oss << bitset <8> (regs[i]);

        	// mvwprintw(reg_win, i + 3, 1, " $%x\t %s ", i, oss.str().c_str());
        	mvwprintw(reg_win, i + 3, 1, " $%d\t 0x%.2x ", i, regs[i]);
	}
	
	mvwprintw(reg_win, REGS + 4, 1, " $pc\t 0x%.2x ", pc);
	
	wmove(reg_win, 3, 7);
	wvline(reg_win, ACS_VLINE, REGS + 2);
	wmove(reg_win, REGS + 3, 1);
	whline(reg_win, ACS_HLINE, 13);

	mvwaddch(reg_win, 2, 0, ACS_LTEE);
	mvwaddch(reg_win, 2, 14, ACS_RTEE);
	mvwaddch(reg_win, REGS + 3, 0, ACS_LTEE);
	mvwaddch(reg_win, REGS + 3, 14, ACS_RTEE);
	mvwaddch(reg_win, 2, 7, ACS_TTEE);
	mvwaddch(reg_win, REGS + 3, 7, ACS_PLUS);
	mvwaddch(reg_win, REGS + 5, 7, ACS_BTEE);

        wrefresh(reg_win);

	return reg_win;
}

WINDOW *redraw_ram_win(size_t start)
{
	WINDOW *subwindow = newwin(RAMS + 4, 15, 1, 2);

        box(subwindow, 0, 0);

	init_pair(1, COLOR_BLUE, COLOR_BLACK);
	
	wattrset(subwindow, COLOR_PAIR(1));
	
	wattron(subwindow, A_BOLD);
	mvwprintw(subwindow, 1, 1, " MEMORY");
	wattroff(subwindow, A_BOLD);

	wstandend(subwindow);

	wmove(subwindow, 2, 1);
	whline(subwindow, ACS_HLINE, 13);
	for (size_t i = start; i < RAMS + start; i++) {
		ostringstream oss;

		oss << bitset <8> (ram[i]);

        	mvwprintw(subwindow, i - start + 3, 1, " 0x%.2x\t 0x%.2x ", i, ram[i]);
	}
	wmove(subwindow, 3, 7);
	wvline(subwindow, ACS_VLINE, RAMS);

	mvwaddch(subwindow, 2, 0, ACS_LTEE);
	mvwaddch(subwindow, 2, 14, ACS_RTEE);
	mvwaddch(subwindow, 2, 7, ACS_TTEE);
	mvwaddch(subwindow, RAMS + 3, 7, ACS_BTEE);

        wrefresh(subwindow);

	return subwindow;
}

WINDOW *redraw_pc_win(size_t start)
{
	WINDOW *pc_win = newwin(RAMS + 4, 36, 1, 18);

        box(pc_win, 0, 0);

	init_pair(1, COLOR_BLUE, COLOR_BLACK);

	wattrset(pc_win, COLOR_PAIR(1));

	wattron(pc_win, A_BOLD);
	mvwprintw(pc_win, 1, 1, " DISASSEMBLY");
	wattroff(pc_win, A_BOLD);
	
	wstandend(pc_win);

	wmove(pc_win, 2, 1);
	whline(pc_win, ACS_HLINE, 34);
	mvwaddch(pc_win, 2, 0, ACS_LTEE);
	mvwaddch(pc_win, 2, 35, ACS_RTEE);
	
	for (size_t i = start; i < RAMS + start; i++) {
		ostringstream oss;

		oss << bitset <8> (ram[i]);

		if (i == pc)
			wattron(pc_win, A_STANDOUT);

        	mvwprintw(pc_win, i - start + 3, 1, " 0x%.2x   0x%.2x   %s ", i, ram[i], assembly(ram[i]).c_str());

		if (i == pc)
			wattroff(pc_win, A_STANDOUT);
	}
	
	wmove(pc_win, 3, 7);
	wvline(pc_win, ACS_VLINE, RAMS);
	
	wmove(pc_win, 3, 14);
	wvline(pc_win, ACS_VLINE, RAMS);

	mvwaddch(pc_win, 2, 7, ACS_TTEE);
	mvwaddch(pc_win, 2 + RAMS + 1, 7, ACS_BTEE);

	mvwaddch(pc_win, 2, 14, ACS_TTEE);
	mvwaddch(pc_win, 2 + RAMS + 1, 14, ACS_BTEE);

	wrefresh(pc_win);

	return pc_win;
}

void execute(vector <string> wds, size_t &pc_start)
{
	string cmd = wds[0];

	if (cmd == "set") {
		assert(wds.size() > 2);

		size_t tmp;

		string addr = wds[1];

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
		
		byte address = tmp;

		string inst = wds[2];

		string conc;
		for (size_t i = 3; i < wds.size(); i++) {
			conc += wds[i];

			if (i < wds.size() - 1)
				conc += " ";
		}

		byte rd;
		byte rs;
		byte rt;

		sscanf(conc.c_str(), "$%hhd, $%hhd, $%hhd", &rd, &rs, &rt);
		
		byte opc = 0;
		for (opc = 0; opc < 4; opc++) {
			if (inst == asmb[opc])
				break;
		}

		byte bin = (opc << 6) + (rd << 4) + (rs << 2) + rt;

		ram[address] = bin;
	} else if (cmd == "reset") {
		state st;

		while (!history.empty()) {
			st = history.top();
			history.pop();

			regs[st.ri] = st.rval;
			ram[st.mi] = st.mval;
		}

		pc_start = 0;
		pc = 0;
	} else if (cmd == "load") {
		assert(wds.size() > 1);

		auto prs = assembler(wds[1]);
		for (auto pr : prs)
			ram[pr.first] = pr.second;
	}
}

int main()
{
	ram = new byte[256];

	srand(clock());
	/* for (size_t i = 0; i < REGS; i++)
		regs[i] = rand() % UINT8_MAX;

	for (size_t i = 0; i < 256; i++)
		ram[i] = rand() % UINT8_MAX; */

	// Start windows
	initscr();

	int mx, my;

	winsize size;

	ioctl(0, TIOCGWINSZ, (char *) &size);

	RAMS = size.ws_row - 9;

	// Begin Setup
	noecho();
	cbreak();

	curs_set(0);

	start_color();

	keypad(stdscr, TRUE);
        box(stdscr, 0, 0);

        refresh();

	// REGISTERS
	WINDOW *reg_win = newwin(REGS + 6, 15, 1, 55);

        box(reg_win, 0, 0);

	init_pair(1, COLOR_BLUE, COLOR_BLACK);
	
	wattrset(reg_win, COLOR_PAIR(1));

	wattron(reg_win, A_BOLD);
	mvwprintw(reg_win, 1, 1, " REGISTERS");
	wattroff(reg_win, A_BOLD);

	wstandend(reg_win);

	wmove(reg_win, 2, 1);
	whline(reg_win, ACS_HLINE, 13);
	for (size_t i = 0; i < REGS; i++) {
		ostringstream oss;

		oss << bitset <8> (regs[i]);

        	// mvwprintw(reg_win, i + 3, 1, " $%x\t %s ", i, oss.str().c_str());
        	mvwprintw(reg_win, i + 3, 1, " $%d\t 0x%.2x ", i, regs[i]);
	}
	
	mvwprintw(reg_win, REGS + 4, 1, " $pc\t 0x%.2x ", pc);
	
	wmove(reg_win, 3, 7);
	wvline(reg_win, ACS_VLINE, REGS + 2);
	wmove(reg_win, REGS + 3, 1);
	whline(reg_win, ACS_HLINE, 13);

	mvwaddch(reg_win, 2, 0, ACS_LTEE);
	mvwaddch(reg_win, 2, 14, ACS_RTEE);
	mvwaddch(reg_win, REGS + 3, 0, ACS_LTEE);
	mvwaddch(reg_win, REGS + 3, 14, ACS_RTEE);
	mvwaddch(reg_win, 2, 7, ACS_TTEE);
	mvwaddch(reg_win, REGS + 3, 7, ACS_PLUS);
	mvwaddch(reg_win, REGS + 5, 7, ACS_BTEE);

        wrefresh(reg_win);

	// MEMORY
	WINDOW *ram_win = newwin(RAMS + 4, 15, 1, 2);

        box(ram_win, 0, 0);

	init_pair(1, COLOR_BLUE, COLOR_BLACK);
	
	wattrset(ram_win, COLOR_PAIR(1));
	
	wattron(ram_win, A_BOLD);
	mvwprintw(ram_win, 1, 1, " MEMORY");
	wattroff(ram_win, A_BOLD);

	wstandend(ram_win);

	wmove(ram_win, 2, 1);
	whline(ram_win, ACS_HLINE, 13);
	for (size_t i = 0; i < RAMS; i++) {
		ostringstream oss;

		oss << bitset <8> (ram[i]);

        	mvwprintw(ram_win, i + 3, 1, " 0x%.2x\t 0x%.2x ", i, ram[i]);
	}
	wmove(ram_win, 3, 7);
	wvline(ram_win, ACS_VLINE, RAMS);

	mvwaddch(ram_win, 2, 0, ACS_LTEE);
	mvwaddch(ram_win, 2, 14, ACS_RTEE);
	mvwaddch(ram_win, 2, 7, ACS_TTEE);
	mvwaddch(ram_win, RAMS + 3, 7, ACS_BTEE);

        wrefresh(ram_win);

	// DISASSEMBLY
	WINDOW *pc_win = newwin(RAMS + 4, 36, 1, 18);

        box(pc_win, 0, 0);

	init_pair(1, COLOR_BLUE, COLOR_BLACK);

	wattrset(pc_win, COLOR_PAIR(1));

	wattron(pc_win, A_BOLD);
	mvwprintw(pc_win, 1, 1, " DISASSEMBLY");
	wattroff(pc_win, A_BOLD);
	
	wstandend(pc_win);

	wmove(pc_win, 2, 1);
	whline(pc_win, ACS_HLINE, 34);
	mvwaddch(pc_win, 2, 0, ACS_LTEE);
	mvwaddch(pc_win, 2, 35, ACS_RTEE);
	
	for (size_t i = 0; i < RAMS; i++) {
		ostringstream oss;

		oss << bitset <8> (ram[i]);

		if (i == pc)
			wattron(pc_win, A_STANDOUT);

        	mvwprintw(pc_win, i + 3, 1, " 0x%.2x   0x%.2x   %s ", i, ram[i], assembly(ram[i]).c_str());

		if (i == pc)
			wattroff(pc_win, A_STANDOUT);
	}
	
	wmove(pc_win, 3, 7);
	wvline(pc_win, ACS_VLINE, RAMS);
	
	wmove(pc_win, 3, 14);
	wvline(pc_win, ACS_VLINE, RAMS);

	mvwaddch(pc_win, 2, 7, ACS_TTEE);
	mvwaddch(pc_win, 2 + RAMS + 1, 7, ACS_BTEE);

	mvwaddch(pc_win, 2, 14, ACS_TTEE);
	mvwaddch(pc_win, 2 + RAMS + 1, 14, ACS_BTEE);

	wrefresh(pc_win);

	// COMMAND PANEL
	WINDOW *cmd_win = newwin(3, size.ws_col - 4, RAMS + 5, 2);

        box(cmd_win, 0, 0);

	wmove(cmd_win, 1, 1);
	wrefresh(cmd_win);

	// Finishing touches
	scrollok(stdscr, TRUE);
        refresh();

	size_t start = 0;
	size_t pc_win_start = 0;

	bool play = false;
	
	thread th([&]() {
		while (true) {
			if (!play)
				continue;

			decode(ram[pc]);
		
			if (pc < 255)
				pc++;
			else
				play = false;

			if (pc_win_start < 256 - RAMS)
				pc_win_start++;

			delwin(reg_win);
			reg_win = redraw_reg_win();

			delwin(pc_win);
			pc_win = redraw_pc_win(pc_win_start);
			
			this_thread::sleep_for(0.1s);
		}
	});
	
	size_t cursor = 1;
	string cmd;

	int c;
	while (true) {
		c = getch();

		if (c == 'q' || c == 27)
			break;

		if (c == KEY_UP) {
			if (start > 0)
				start--;
			// start = max(start - 4, (size_t) 0);
			delwin(ram_win);
			ram_win = redraw_ram_win(start);
			continue;
		}
		
		if (c == KEY_DOWN) {
			if (start < 256 - RAMS)
				start++;
			// start = min(start + 4, (size_t) 256);
			delwin(ram_win);
			ram_win = redraw_ram_win(start);
			continue;
		}

		if (isprint(c)) {
			waddch(cmd_win, c);
			wrefresh(cmd_win);

			cmd += c;
		}

		if (c == 10) {
			for (size_t i = 0; i < cmd.length(); i++) {
				waddch(cmd_win, '\b');
				waddch(cmd_win, ' ');
				waddch(cmd_win, '\b');
			}

			istringstream iss(cmd);

			vector <string> toks;

			string wd;
			while (iss >> wd)
				toks.push_back(wd);

			execute(toks, pc_win_start);

			//cout << "ram: " << (size_t) ram[0] << endl;

			delwin(ram_win);
			ram_win = redraw_ram_win(start);

			delwin(reg_win);
			reg_win = redraw_reg_win();

			delwin(pc_win);
			pc_win = redraw_pc_win(pc_win_start);

			cmd.clear();
			
			wrefresh(cmd_win);
		}

		 if (c == KEY_BACKSPACE && cmd.length() > 0) {
			waddch(cmd_win, '\b');
			waddch(cmd_win, ' ');
			waddch(cmd_win, '\b');

			cmd = cmd.substr(0, cmd.length() - 1);
			wrefresh(cmd_win);
		}

		if (c == KEY_F(3)) {
			decode(ram[pc]);
		
			if (pc < 255)
				pc++;

			if (pc_win_start < 256 - RAMS)
				pc_win_start++;

			delwin(reg_win);
			reg_win = redraw_reg_win();

			delwin(pc_win);
			pc_win = redraw_pc_win(pc_win_start);
			continue;
		}

		/* if (c == KEY_F(2) && pc > 0) {
			pc--;

			pc_win_start--;

			state st = history.top();
			history.pop();

			regs[st.ri] = st.rval;
			ram[st.mi] = st.mval;

			delwin(ram_win);
			ram_win = redraw_ram_win(start);

			delwin(reg_win);
			reg_win = redraw_reg_win();

			delwin(pc_win);
			pc_win = redraw_pc_win(pc_win_start);
			continue;
		} */

		if (c == KEY_F(5))
			play = !play;
	}

	th.detach();

	// Release Resources
        delwin(reg_win);
	delwin(ram_win);
	delwin(pc_win);
	delwin(cmd_win);
        endwin();

	return 0;
}
