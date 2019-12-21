#ifndef BOARD_H_
#define BOARD_H_

#include <cctype>
#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <string.h>
#include <vector>
#include <unistd.h>
#include <assert.h>
#include <sstream>
#include <cstdlib>
#include <limits>
#include <regex>

#define P1PawnColor	'o'
#define P2PawnColor	'*'
#define P1KingColor	'%'
#define P2KingColor	'@'
#define EmptyColor	' '
extern int num_not_jumps;

class jump {
	int num_jumps;
	int xstart, ystart;
	int xcurr, ycurr;
	int xend, yend;
	int key;
	char jumpingPiece;
	bool noNextMove;
	char c;
	jump* prev;

	// constructor, destructor
	jump(char jpingp, char piece, int xstart, int ystart, int xcurr, int ycurr, int xend, int yend, jump* p, int k) :
		prev(p), jumpingPiece(jpingp), noNextMove(true), num_jumps(0), c(piece), xstart(xstart), ystart(ystart),xcurr(xcurr), ycurr(ycurr), xend(xend), yend(yend), key(k) { }
//	~jump() { }

	// shared
	friend class move;
	friend class board;
	friend class game;
};

class move {
	char mPiece;
	int x1, y1;	// initial position
	int x2, y2; // final position
	std::string command;
	std::list<jump*> jumpPoints;

	// constructor, destructor
	move(char c, int xstart, int ystart, int xend, int yend) :
		mPiece(c), x1(xstart), y1(ystart), x2(xend), y2(yend) { }
	~move();

	// shared
//	friend class move;
	friend class board;
	friend class game;
};

class board {
	char pieces[8][4]; // records all markers of pieces on board
	char color; // color of piece as defined above
	static bool AIPlayer1;
	static bool AIPlayer2;
	bool check_tie;
	bool jumpAvail;
	static int timelimit;

	board();
	~board();
	board(const board&);

	std::list<move*> moveList;	// list of moves queued
	void initialize();
	void defaultreset();
	void resetfromfile(std::ifstream&);

	// check turns
	bool check_ai_turn();
	void switch_color_on_next_turn();

	// take moves
	void user_move_input();
	bool check_next_available_move();
	bool check_valid_position(int, int);
	void insert_move_to_queue(const int&, const int&, int, int); //generates a code for each move which is easily parsed
	void make_move(move *);
	void undo_move(move *);
	bool list_move();

	// display
	void print_board();
	void print_moves();

	// utility
	char *color_code_piece(char);
	static void convertMoves(const std::string&);
	int createkey(int, int, int, int, int, int);
	int reverse(int);

	// jump check
	bool check_multiple_jumps_available();
	void check_jump(std::list<jump*>&, char c, int, int, jump*);
	bool check_jump_valid(int, int, int, int, int, int);

	void make_king(const int&, const int&);
	void insert_jump_move_to_list(std::list<jump*>&, char, int, int, int, int, int, int, jump*);
	void make_jump_move(std::list<jump*>&);
	void check_neighbor(int&, int&);
	void convert(const int&, const int&, std::string&);

	// heuristics
	int corner_diagonal(char, char);
	int evaluate();

	// shared
	template <class T> friend class nodeptr;
	friend class game;
};

#endif /* BOARD_H_ */
