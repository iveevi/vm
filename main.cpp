#include <bitset>
#include <iostream>
#include <sstream>
#include <ncurses.h>
#include <string>

#include "registers.h"

using namespace std;

// Computer
byte pc;
byte *ram;
byte regs[REGS];

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

	switch (opc)
	{
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

	return asmb[opc] + "\tr" + to_string(r_one)
		+ ",\tr" + to_string(r_two) + ",\tr"
		+ to_string(r_thr);
}

WINDOW *redraw_reg_win()
{
	WINDOW *subwindow = newwin(REGS + 4, 19, 1, 2);

        box(subwindow, 0, 0);

	mvwprintw(subwindow, 1, 1, " REGISTERS");
	wmove(subwindow, 2, 1);
	whline(subwindow, ACS_HLINE, 18);
	for (size_t i = 0; i < REGS; i++) {
		ostringstream oss;

		oss << bitset <8> (regs[i]);

        	mvwprintw(subwindow, i + 3, 1, " $%x\t %s ", i, oss.str().c_str());
	}
	wmove(subwindow, 3, 7);
	wvline(subwindow, ACS_VLINE, REGS);

	mvwaddch(subwindow, 2, 0, ACS_LTEE);
	mvwaddch(subwindow, 2, 18, ACS_RTEE);
	mvwaddch(subwindow, 2, 7, ACS_TTEE);
	mvwaddch(subwindow, REGS + 3, 7, ACS_BTEE);

        wrefresh(subwindow);

	return subwindow;
}

WINDOW *redraw_ram_win(size_t start)
{
	WINDOW *subwindow = newwin(RAMS + 4, 19, 9, 2);

        box(subwindow, 0, 0);

	mvwprintw(subwindow, 1, 1, " RAM");
	wmove(subwindow, 2, 1);
	whline(subwindow, ACS_HLINE, 18);
	for (size_t i = start; i < RAMS + start; i++) {
		ostringstream oss;

		oss << bitset <8> (ram[i]);

        	mvwprintw(subwindow, i - start + 3, 1, " 0x%x\t %s ", i, oss.str().c_str());
	}
	wmove(subwindow, 3, 7);
	wvline(subwindow, ACS_VLINE, RAMS);

	mvwaddch(subwindow, 2, 0, ACS_LTEE);
	mvwaddch(subwindow, 2, 18, ACS_RTEE);
	mvwaddch(subwindow, 2, 7, ACS_TTEE);
	mvwaddch(subwindow, RAMS + 3, 7, ACS_BTEE);

        wrefresh(subwindow);

	return subwindow;
}

WINDOW *redraw_pc_win()
{
	WINDOW *pc_win = newwin(RAMS + 5, 52, 17, 2);

        box(pc_win, 0, 0);

	mvwprintw(pc_win, 1, 1, " PC");

	wmove(pc_win, 2, 1);
	whline(pc_win, ACS_HLINE, 50);
	mvwaddch(pc_win, 2, 0, ACS_LTEE);
	mvwaddch(pc_win, 2, 51, ACS_RTEE);

	wmove(pc_win, 4, 1);
	whline(pc_win, ACS_HLINE, 50);
	mvwaddch(pc_win, 4, 0, ACS_LTEE);
	mvwaddch(pc_win, 4, 51, ACS_RTEE);

	wmove(pc_win, 6, 1);
	whline(pc_win, ACS_HLINE, 50);
	mvwaddch(pc_win, 6, 0, ACS_LTEE);
	mvwaddch(pc_win, 6, 51, ACS_RTEE);

	ostringstream oss;

	oss << bitset <8> (pc);

	mvwprintw(pc_win, 3, 1, " Current Address\t %s ", oss.str().c_str());

	oss.str("");
	oss.clear();

	oss << bitset <8> (ram[pc]);

	mvwprintw(pc_win, 5, 1, " Current Instruction\t %s ", oss.str().c_str());

	mvwprintw(pc_win, 7, 1, " Assembly\t\t %s ", assembly(ram[pc]).c_str());

	mvwaddch(pc_win, 2, 23, ACS_TTEE);
	mvwaddch(pc_win, 3, 23, ACS_VLINE);
	mvwaddch(pc_win, 4, 23, ACS_PLUS);
	mvwaddch(pc_win, 5, 23, ACS_VLINE);
	mvwaddch(pc_win, 6, 23, ACS_PLUS);
	mvwaddch(pc_win, 7, 23, ACS_VLINE);
	mvwaddch(pc_win, 8, 23, ACS_BTEE);

	wrefresh(pc_win);

	return pc_win;
}

int main()
{
	ram = new byte[256];

	srand(clock());
	for (size_t i = 0; i < REGS; i++)
		regs[i] = rand() % UINT8_MAX;

	for (size_t i = 0; i < 256; i++)
		ram[i] = rand() % UINT8_MAX;

	// Start windows
	initscr();

	// Begin Setup
	noecho();
	cbreak();

	curs_set(0);

	keypad(stdscr, TRUE);
        box(stdscr, 0, 0);

        refresh();

	// REGISTERS
	WINDOW *reg_win = newwin(REGS + 4, 19, 1, 2);

        box(reg_win, 0, 0);

	mvwprintw(reg_win, 1, 1, " REGISTERS");
	wmove(reg_win, 2, 1);
	whline(reg_win, ACS_HLINE, 17);
	for (size_t i = 0; i < REGS; i++) {
		ostringstream oss;

		oss << bitset <8> (regs[i]);

        	mvwprintw(reg_win, i + 3, 1, " $%x\t %s ", i, oss.str().c_str());
	}
	wmove(reg_win, 3, 7);
	wvline(reg_win, ACS_VLINE, REGS);

	mvwaddch(reg_win, 2, 0, ACS_LTEE);
	mvwaddch(reg_win, 2, 18, ACS_RTEE);
	mvwaddch(reg_win, 2, 7, ACS_TTEE);
	mvwaddch(reg_win, REGS + 3, 7, ACS_BTEE);

        wrefresh(reg_win);

	// RAM
	WINDOW *ram_win = newwin(RAMS + 4, 19, 9, 2);

        box(ram_win, 0, 0);

	mvwprintw(ram_win, 1, 1, " RAM");
	wmove(ram_win, 2, 1);
	whline(ram_win, ACS_HLINE, 17);
	for (size_t i = 0; i < RAMS; i++) {
		ostringstream oss;

		oss << bitset <8> (ram[i]);

        	mvwprintw(ram_win, i + 3, 1, " 0x%x\t %s ", i, oss.str().c_str());
	}
	wmove(ram_win, 3, 7);
	wvline(ram_win, ACS_VLINE, RAMS);

	mvwaddch(ram_win, 2, 0, ACS_LTEE);
	mvwaddch(ram_win, 2, 18, ACS_RTEE);
	mvwaddch(ram_win, 2, 7, ACS_TTEE);
	mvwaddch(ram_win, RAMS + 3, 7, ACS_BTEE);

        wrefresh(ram_win);

	// PC
	WINDOW *pc_win = newwin(RAMS + 5, 52, 17, 2);

        box(pc_win, 0, 0);

	mvwprintw(pc_win, 1, 1, " PC");

	wmove(pc_win, 2, 1);
	whline(pc_win, ACS_HLINE, 50);
	mvwaddch(pc_win, 2, 0, ACS_LTEE);
	mvwaddch(pc_win, 2, 51, ACS_RTEE);

	wmove(pc_win, 4, 1);
	whline(pc_win, ACS_HLINE, 50);
	mvwaddch(pc_win, 4, 0, ACS_LTEE);
	mvwaddch(pc_win, 4, 51, ACS_RTEE);

	wmove(pc_win, 6, 1);
	whline(pc_win, ACS_HLINE, 50);
	mvwaddch(pc_win, 6, 0, ACS_LTEE);
	mvwaddch(pc_win, 6, 51, ACS_RTEE);

	ostringstream oss;

	oss << bitset <8> (pc);

	mvwprintw(pc_win, 3, 1, " Current Address\t %s ", oss.str().c_str());

	oss.str("");
	oss.clear();

	oss << bitset <8> (ram[pc]);

	mvwprintw(pc_win, 5, 1, " Current Instruction\t %s ", oss.str().c_str());

	mvwprintw(pc_win, 7, 1, " Assembly\t\t %s ", assembly(ram[pc]).c_str());

	mvwaddch(pc_win, 2, 23, ACS_TTEE);
	mvwaddch(pc_win, 3, 23, ACS_VLINE);
	mvwaddch(pc_win, 4, 23, ACS_PLUS);
	mvwaddch(pc_win, 5, 23, ACS_VLINE);
	mvwaddch(pc_win, 6, 23, ACS_PLUS);
	mvwaddch(pc_win, 7, 23, ACS_VLINE);
	mvwaddch(pc_win, 8, 23, ACS_BTEE);

	wrefresh(pc_win);

	// Finishing touches
	move(0, 0);
	scrollok(stdscr, TRUE);
        refresh();

	int cursor = 1;

	size_t start = 0;

	int c;
	while (true) {
		c = getch();

		if (c == 'q' || c == 27)
			break;

		if (c == KEY_UP) {
			if (start > 0)
				start -= 4;
			// start = max(start - 4, (size_t) 0);
			delwin(ram_win);
			ram_win = redraw_ram_win(start);
			continue;
		}
		
		if (c == KEY_DOWN) {
			if (start < 252)
				start += 4;
			// start = min(start + 4, (size_t) 256);
			delwin(ram_win);
			ram_win = redraw_ram_win(start);
			continue;
		}

		/* Save for later backspacing
		 * if (c == KEY_BACKSPACE && cmd.length() > 0) {
			addch('\b');
			addch(' ');
			addch('\b');

			cmd = cmd.substr(0, cmd.length() - 1);
			move(cmd.length() - 1);
			refresh();
		} */

		if (c == 10) {
			decode(ram[pc]);
		
			if (pc < 255)
				pc++;

			delwin(reg_win);
			reg_win = redraw_reg_win();

			delwin(pc_win);
			pc_win = redraw_pc_win();
			continue;
		}
	}

	// Release Resources
        delwin(reg_win);
	delwin(ram_win);
        endwin();

	return 0;
}
