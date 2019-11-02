//============================================================================
// Created on  : October 29, 2019
// Finished on : November 1, 2019
// Name        : Jongoh (Andy) Jeong
// Description : Checkers AI Program
//============================================================================

#include "board.h"

using std::endl;
using std::cin;
using std::cout;
using std::getline;
using std::string;
using std::ifstream;
using std::stringstream;
using std::toupper;
using std::tolower;
using std::list;
using std::regex;
using std::vector;

std::string invalid_region_color = "48;5;0"; // fg/bg;color;
std::string valid_region_color = "48;5;2";
std::string player_1_color = "31;5;10";
std::string player_2_color = "37;5;20";

bool check_tie = false;
bool jumpAvail = false;
int board::timelimit = 0;
int num_not_jumps;
char color;
bool board::AIPlayer1 = false;
bool board::AIPlayer2 = false;
char colIndex[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'};

// ---------constructors
board::board() { }

board::board(const board& board): color(board.color) {// @suppress("Class members should be properly initialized")
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 4; ++j) {
			pieces[i][j] = board.pieces[i][j];
		}
	}
}
// ---------end constructors

// ---------destructors
board::~board() {
	while (!moveList.empty()) 	{
		delete moveList.front();
		moveList.pop_front();
	}
}

move::~move() {
	for (list<jump*>::iterator it = jumpPoints.begin(); it != jumpPoints.end(); ++it) {
		--(*it)->num_jumps;
		if ((*it)->num_jumps == 0) delete (*it);
	}
}
// ---------end destructors

// initialize (entry point from game.cpp)
void board::initialize() {
	bool player1set = false;
	bool player2set = false;
	bool boardfileset = false;
	char player1yn;
	char player2yn;
	char boardfileyn;
	player1yn = player2yn = boardfileyn = ' ';
	string filename;
	string str;
    regex regex_pattern_yn("[a-zA-Z]");

	// ask for player 1
	do {
		// invalid input character check with regex
	    do {
			cout << "Will player 1 be a computer? (Y/N): ";
	        cin >> str;
	    } while(!std::regex_match(str,regex_pattern_yn));

	    player1yn = str[0];
		if (tolower(player1yn) == 'y') {
			board::AIPlayer1 = true;
			player1set = true;
		}
		else if (tolower(player1yn) == 'n') {
			board::AIPlayer1 = false;
			player1set = true;
		}
	} while (!player1set);

	// ask for player 2
	do	{
		// invalid input character check with regex
	    do {
			cout << "Will player 2 be a computer? (Y/N): ";
	        cin >> str;
	    } while(!std::regex_match(str,regex_pattern_yn));

	    player2yn = str[0];
		if (tolower(player2yn) == 'y') {
			board::AIPlayer2 = true;
			player2set = true;
		}
		else if (tolower(player2yn) == 'n')	{
			board::AIPlayer2 = false;
			player2set = true;
		}
	} while (!player2set);

	// if one of the players is computer
	if (board::AIPlayer1 == true || board::AIPlayer2 == true) {
		// invalid input character check with regex
	    do {
			cout << "Enter a time limit for the computer in seconds: (3 <= time <= 60 sec): ";
			cin >> timelimit;
			if(cin.fail()) {
				cin.clear();  // correct the stream.
				cin.ignore(); // skip the left over stream data.
			}
			if (timelimit <= 60 && timelimit >= 3) {
				break;
			}
	    } while(!(timelimit <= 60 && timelimit >= 3));
	}
	// ask for file load for board
	do	{
		cout << "Load board from a local file? (Y/N): ";
		cin >> boardfileyn;
		if (tolower(boardfileyn) == 'y' || tolower(boardfileyn) == 'n') boardfileset = true;
		cout << endl;
	} while (!boardfileset);
	if (boardfileset) {
		if (tolower(boardfileyn) == 'y') {
			ifstream fin;
			cout << "Enter a valid filename: ";
			string temp;
			cin >> temp;
			filename = temp.c_str();
			fin.open(filename);
			if (!fin) {
				std::cerr << "File open failed (Invalid Filename)." << endl;
				cout << "Enter a valid filename: ";
				cin >> temp;
				fin.open(temp.c_str());
				cout << endl;
			}
			if (!fin) exit(EXIT_FAILURE);
			board::resetfromfile(fin);
		} else {
			board::defaultreset();
		}
	}
	check_tie = false;
}
// ---------end initializer

// ---------setting up board
void board::defaultreset() {
	color = P1PawnColor;
	// upper 4 rows: player 1
	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 4; ++j)
			pieces[i][j] = P1PawnColor;
	// middle 2 rows
	for (int i = 3; i < 5; ++i)
		for (int j = 0; j < 4; ++j)
			pieces[i][j] = '-';
	// lower 4 rows: player 2
	for (int i = 5; i < 8; ++i)
		for (int j = 0; j < 4; ++j)
			pieces[i][j] = P2PawnColor;
}
// ---------end setting up board

// ---------custom setting up board
bool endsWith(const std::string& s, const std::string& suffix)
{
    return s.size() >= suffix.size() &&
           s.substr(s.size() - suffix.size()) == suffix;
}

std::vector<std::string> split(std::string& s, std::string& delimiter, const bool& removeEmptyEntries = false)
{
    std::vector<std::string> tokens;

    for (size_t start = 0, end; start < s.length(); start = end + delimiter.length())
    {
         size_t position = s.find(delimiter, start);
         end = position != string::npos ? position : s.length();

         std::string token = s.substr(start, end - start);
         if (!removeEmptyEntries || !token.empty())
         {
             tokens.push_back(token);
         }
    }

    if (!removeEmptyEntries &&
        (s.empty() || endsWith(s, delimiter)))
    {
        tokens.push_back("");
    }

    return tokens;
}
void board::resetfromfile(ifstream &fname) {
	string line;
	std::vector<std::string> tokens;
	string delimiter = " -";
	char temp;
	const char str3 = '1';
	const char str4 = '2';
	const char str5 = '3';
	const char str6 = '4';
	int row = 0;
	bool mode; // true: even (player 2), false: odd (player 1)
	while (getline(fname, line) && row < 8) {
 		tokens = split(line, delimiter, true);
		mode = !(row % 2);
		if (mode) { // player 2: 1,3,5,7
			int realcol = 0;
			for (int col = 1; col < 8; ) {
				char str1 = tokens[0][col];
				switch (str1) {
					case str3: // 1
						temp = P1PawnColor;
						break;
					case str4: // 2
						temp = P2PawnColor;
						break;
					case str5: // 3
						temp = P1KingColor;
						break;
					case str6: // 4
						temp = P2KingColor;
						break;
					default:
						temp = '-';
				}
				pieces[row][realcol] = temp;
				col += 2;
				realcol += 1;
			}
		} else if (!mode) { // player 1: 0,2,4,6
			int realcol = 0;
			for (int col = 0; col < 8; ) {
				char str1 = tokens[0][col];
				switch (str1) {
					case str3:
						temp = P1PawnColor;
						break;
					case str4:
						temp = P2PawnColor;
						break;
					case str5:
						temp = P1KingColor;
						break;
					case str6:
						temp = P2KingColor;
						break;
					default:
						temp = '-';
				}
				pieces[row][realcol] = temp;
				col += 2;
				realcol += 1;
			}
		}
		if (row == 7) break;
		row += 1;
	}
	cout << endl;
	// player 1 or 2
	getline(fname, line);
	// verify player input
	int p = atoi(line.c_str());
	cout << "From file:: Starting player: " << p;
	if (p == 1) color = P1PawnColor;
	else if (p == 2) color = P2PawnColor;

	cout <<  " / ";
	// timelimit
	getline(fname, line);
	timelimit = atoi(line.c_str());

	// verify timelimit
	cout << "Timelimit: " << timelimit << " seconds."<< endl;
	if (board::AIPlayer1) cout << "Player 1: AI";
	else cout << "Player 1: person";
	cout <<  " / ";
	if (board::AIPlayer2) cout << "Player 2: AI";
	else cout << "Player 2: person";
	cout << endl;
	assert(color == P2PawnColor || color == P1PawnColor);
}
// ---------end custom setting up board

// ---------handle command for the selected move input
void board::user_move_input() {
	print_moves();
	int curr_max_move_size = moveList.size();

	// take number choice input for the next move
	char str[256];
	// invalid input character check with regex
    regex regex_pattern("[1-9]+");
    do {
		std::cout << "Choose a move (in number key): ";
        cin >> str;
        cout << endl;
    } while(!std::regex_match(str,regex_pattern));
    // str - > int character conversion
	int chosen_move = atoi(str);

	// input number greater than how many available moves there are at the turn
	while (chosen_move > curr_max_move_size) {
		std::cout << "Choice out of range.\nChoose a move: ";
		std::cin >> chosen_move;
	}
	list<move*>::iterator it = moveList.begin();
	std::advance(it, chosen_move - 1);
	make_move(*it);
}

void board::print_moves() {
	// check jump condition
	if (!jumpAvail) num_not_jumps++;
	else num_not_jumps = 0;

	// check tie game condition
	if (num_not_jumps > 65) check_tie = true;

	// check who's turn it is
	if (color == P2PawnColor) cout << "Player 2's turn" << endl;
	else if (color == P1PawnColor) cout << "Player 1's turn" << endl;

	cout << endl;
	cout << "Available moves: " << endl;
	list<move*>::const_iterator it = moveList.begin();
	for (int i = 1; it != moveList.end(); ++it)	{
		cout << "Move " << i << ": ";
		// convertMove
		string::const_iterator itr = (*it)->command.begin();
		int temp = (*itr)-'0';
		itr += 2;
		cout << colIndex[(*itr)-'0'] << temp+1;
		itr += 2;
		while (*itr != '-') {
			temp = (*itr)-'0';
			cout << " --> ";
			itr += 2;
			cout << colIndex[(*itr)-'0'] << temp+1;
			itr += 2;
		}
		cout << endl;
		i++;
	}
	cout << endl;
}
// ---------end handle command for the selected move input

// ---------handle next turn
bool board::check_ai_turn() {
	if ((color == P1PawnColor) && board::AIPlayer1) return true;
	if ((color == P2PawnColor) && board::AIPlayer2) return true;
	return false;
}

void board::switch_color_on_next_turn() {
	// default: player 1 starts
	if (color == P1PawnColor) color = P2PawnColor;
	else color = P1PawnColor;
}

// called in 'game.cpp' as 2nd entry point, after calling initialize()
bool board::check_next_available_move() {
	if (check_multiple_jumps_available()) return true;
	if (list_move()) return true;

	return false; // no more moves --> terminate
}
// ---------end handle next turn

void board::make_king(const int& x, const int& y) {
	// make the pawn a king if in row 0 or 7
	if (x == 7 && pieces[x][y] == P1PawnColor) pieces[x][y] = P1KingColor;
	else if (x == 0 && pieces[x][y] == P2PawnColor) pieces[x][y] = P2KingColor;
}

bool board::check_valid_position(int x, int y) {
	if (x > -1 && x < 8 && y > -1 && y < 4) return true;
	else return false;
}

// utility function for display (print_board)
char *board::color_code_piece(char piece) {
	char *buf = (char*) malloc(30);
	if (buf == NULL) { // malloc fail
		std::cerr << "Malloc failed. Not enough memory?" << endl;
		exit(EXIT_FAILURE);
	}
	if (piece == P1PawnColor)      sprintf(buf, "\033[%s;1;%sm %c \033[0m", valid_region_color.c_str(), player_1_color.c_str(), P1PawnColor);
	else if (piece == P1KingColor) sprintf(buf, "\033[%s;1;%sm %c \033[0m", valid_region_color.c_str(), player_1_color.c_str(), P1KingColor);
	else if (piece == P2PawnColor) sprintf(buf, "\033[%s;1;%sm %c \033[0m", valid_region_color.c_str(), player_2_color.c_str(), P2PawnColor);
	else if (piece == P2KingColor) sprintf(buf, "\033[%s;1;%sm %c \033[0m", valid_region_color.c_str(), player_2_color.c_str(), P2KingColor);
	else sprintf(buf, "\033[%s;1;92m %c \033[0m", valid_region_color.c_str(), EmptyColor);
	return buf;
}

// display board
void board::print_board() {
	// top line
	cout << "   \e(0\x6C\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x77\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x77\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x77\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x77\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x77\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x77\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x77\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x6B\e(B" << endl;
	// loop over each row item
	for (int i = 0; i < 8; i++) {
		// row number on the left
		cout << " \033[1;92m" << i+1 << "\033[0m ";

		// even num. rectangle is invalid
		if (i % 2 == 0) cout << ("\e(0\x78\e(B\033[" + invalid_region_color + ";1;92m   \033[0m");

		// rows 0,1,2 --> pieces can lie
		for (int j = 0; j < 3; j++) {
			char *buf = color_code_piece(pieces[i][j]);
			cout << "\e(0\x78\e(B" << buf << "\e(0\x78\e(B" << ("\033[" + invalid_region_color + ";1;92m   \033[0m");
			free(buf);
		}
		char *buf = color_code_piece(pieces[i][3]);
		cout << "\e(0\x78\e(B" << buf << "\e(0\x78\e(B";
		free (buf);

		// odd num. rectangle is invalid
		if (i % 2 == 1) cout << ("\033[" + invalid_region_color + ";1;92m   \033[0m\e(0\x78\e(B");
		cout << endl;

		if (i==7) cout << "   \e(0\x6D\e(B";
		else cout << "   \e(0\x74\e(B";
		if (i < 7) cout << "\e(0\x71\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x6E\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x6E\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x6E\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x6E\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x6E\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x6E\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x6E\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x71\e(B";
		else cout << "\e(0\x71\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x76\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x76\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x76\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x76\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x76\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x76\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x76\e(B\e(0\x71\e(B\e(0\x71\e(B\e(0\x71\e(B";
		if (i==7) cout << "\e(0\x6A\e(B";
		else cout << "\e(0\x75\e(B";

		cout << endl;
	}
	// each column
	cout << "\033[1;92m     A   B   C   D   E   F   G   H  \033[0m\n" << endl;
	char regPiece[30], kingPiece[30];
	sprintf(regPiece, "\033[1;%sm%c\033[0m", player_1_color.c_str(), P1PawnColor);
	sprintf(kingPiece, "\033[1;%sm%c\033[0m", player_1_color.c_str(), P1KingColor);
	cout << "Player 1 -- Pawn: " << regPiece << " / King: " << kingPiece << endl;
	sprintf(regPiece, "\033[1;%sm%c\033[0m", player_2_color.c_str(), P2PawnColor);
	sprintf(kingPiece, "\033[1;%sm%c\033[0m", player_2_color.c_str(), P2KingColor);
	cout << "Player 2 -- Pawn: " << regPiece << " / King: " << kingPiece << endl;
}

// make a move
void board::make_move(move* move) {
	if (!move->jumpPoints.empty()) {
		list<jump*>::iterator it;
		for (it  = move->jumpPoints.begin(); it != move->jumpPoints.end(); ++it) pieces[(*it)->xcurr][(*it)->ycurr] = '-';
	}
	char c = pieces[move->x1][move->y1];
	pieces[move->x1][move->y1] = '-';
	pieces[move->x2][move->y2] = c;

	make_king(move->x2, move->y2);
	switch_color_on_next_turn();
}

// undo a move for rolling back after recursion
void board::undo_move(move* m) {
	if (!m->jumpPoints.empty()) {
		for (list<jump*>::iterator it = m->jumpPoints.begin(); it != m->jumpPoints.end(); ++it) {
			pieces[(*it)->xstart][(*it)->ystart] = '-';
			pieces[(*it)->xcurr][(*it)->ycurr] = (*it)->c;
			pieces[(*it)->xend][(*it)->yend] = '-';
		}
	}
	pieces[m->x2][m->y2] = '-';
	pieces[m->x1][m->y1] = m->mPiece;
}

// part of heuristic
// - account for two left-most / two right-most columns near corners
int board::corner_diagonal(char color1, char color2) {
	int value = 0;
	if (tolower(pieces[0][0]) == color1 || tolower(pieces[1][0]) == color1) {
		value += 9;
		if (tolower(pieces[0][0]) == color2) value -= 3;
		if (tolower(pieces[1][0]) == color2) value -= 3;
		if (tolower(pieces[1][1]) == color2) value -= 1;
		if (tolower(pieces[2][0]) == color2) value -= 1;
		if (tolower(pieces[2][1]) == color2) value -= 1;
		if (tolower(pieces[3][1]) == color2) value -= 1;
	}
	if (tolower(pieces[6][3]) == color1 || tolower(pieces[7][3]) == color1) {
		value += 9;
		if (tolower(pieces[4][2]) == color2) value -= 1;
		if (tolower(pieces[5][2]) == color2) value -= 1;
		if (tolower(pieces[5][3]) == color2) value -= 1;
		if (tolower(pieces[6][2]) == color2) value -= 1;
		if (tolower(pieces[6][3]) == color2) value -= 3;
		if (tolower(pieces[7][3]) == color2) value -= 3;
	}
	return value;
}

// heuristic function
int board::evaluate() {
	int a1 = 0, a2 = 0, b = 0, c = 0, d = 0;
	for (int i = 0; i != 8; ++i) { // row
		for (int j = 0; j != 4; ++j) { // jth filled column
			if (pieces[i][j] == P2PawnColor) { // player 2 pawn
				a1 += 2;			// reward two units
				if (i == 0) b += 9; // reward if reached top row
				else b += i; 	 	// reward by how far from top row
				c += 1;				// reward one unit
			}
			else if (pieces[i][j] == P1PawnColor) { // player 1 pawn
				a2 -=2;				// penalize two units
				if (i == 7) b -= 9; // penalize if reached bottom row
				else b -= (7 - i);  // penalize by how far from bottom row
				c -= 1;				// penalize one unit
			}
			else if (pieces[i][j] == P2KingColor) { // P2 reached king status
				a1 += 3;							// reward three units
				c += 1;								// reward one unit
			}
			else if (pieces[i][j] == P1KingColor) { // P1 reached king status
				a2 -= 3;							// penalize three units
				c -= 1;								// penalize one unit
			}
		}
	}
	if (c > 0 && a2 >= -8)  //
		d -= corner_diagonal(P1PawnColor, P2PawnColor);
	else if (c < 0 && a1 <= 8) 
		d += corner_diagonal(P2PawnColor, P1PawnColor);
	a1 *= 100000000; 	  				// P2 pawn/king
	a2 *= 100000000; 	  				// P1 pawn/king
	b *= 1000000; 	 	  				// distance to becoming a king
	c *= 10000;			  				// P2 pawn/king vs. P1 pawn/king
	d *= 100;			  				// corner diagonal statistic
	int e = rand() % 100; 				// random variance
	if (color == P1PawnColor) e = -e; 	// negate if P1
	return a1 + a2 + b + c + d + e;		// sum up
}

inline int board::createkey(int xstart, int ystart, int xj, int yj, int xe, int ye) { return ye + xe*10 + yj*100 + xj*1000 + ystart*10000 + xstart*100000; }

int board::reverse(int i) {
    int num = 0;
    while (i/100 > 0) {
        num += i % 100;
        num *= 100;
        i /= 100;
    }
    num += i;
    return num;

}

bool board::check_multiple_jumps_available() {
	while (!moveList.empty()) {
		delete moveList.front();
		moveList.pop_front();
	}
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 4; ++j) {
			if (color == P1PawnColor) {
				if (pieces[i][j] == P1PawnColor) {
					list<jump*> jlist1;
					check_jump(jlist1, P1PawnColor, i, j, NULL);
					make_jump_move(jlist1);
				}
				else if (pieces[i][j] == P1KingColor) {
					list<jump*> jlist2;
					check_jump(jlist2, P1KingColor, i, j, NULL);
					make_jump_move(jlist2);
				}
			} else {
				if (pieces[i][j] == P2PawnColor) {
					list<jump*> jlist1;
					check_jump(jlist1, P2PawnColor, i, j, NULL);
					make_jump_move(jlist1);
				}
				else if (pieces[i][j] == P2KingColor) {
					list<jump*> jlist2;
					check_jump(jlist2, P2KingColor, i, j, NULL);
					make_jump_move(jlist2);
				}
			}
		}
	}
	if (moveList.empty()) return (jumpAvail = false);
	return (jumpAvail = true);
}

void board::insert_jump_move_to_list(list<jump*>& jlist, char color, int xstart, int ystart, int xjump, int yjump, int xend, int yend, jump* jp) {
	pieces[xstart][ystart] = '-';

    int key = createkey(xstart+1, ystart+1, xjump+1, yjump+1, xend+1, yend+1);
    jump* jcheck = jp;
    while (jcheck != NULL) {
        if (key == jcheck->key || key == reverse(jcheck->key)) return;
        jcheck = jcheck->prev;
    }
	jump* j = new jump(color, pieces[xjump][yjump], xstart, ystart, xjump, yjump, xend, yend, jp, key);
	if (jp != NULL) jp->noNextMove = false;
	int initial = jlist.size();
	jlist.push_front(j);
	int final = jlist.size();
	if (final > initial) { // if a jump is added, handle positions
		pieces[xend][yend] = color;
		check_jump(jlist, color, xend, yend, j);
	}
}

// utility for check_jump_possible
bool board::check_jump_valid(int x, int y, int x_to_jump, int y_to_jump, int x_end, int y_end) {
	// jump condition:
	// (1) check for valid positions at jumping position and ending position
	// (2) check if the jumping position and ending position is not empty
	// (3) check if the jumping position has the opposite color (to take over)
	char curr_piece = pieces[x][y];
	if (check_valid_position(x_to_jump, y_to_jump) && check_valid_position(x_end, y_end)) {
		if (pieces[x_to_jump][y_to_jump] != '-' && pieces[x_end][y_end] == '-') {
			// consider for each (current) color piece
			if (color == P1PawnColor) {
				if (pieces[x_to_jump][y_to_jump] == P2PawnColor || pieces[x_to_jump][y_to_jump] == P2KingColor) {
					return true;
				}
			} else if (color == P2PawnColor) {
				if (pieces[x_to_jump][y_to_jump] == P1PawnColor || pieces[x_to_jump][y_to_jump] == P1KingColor) {
					return true;
				}
			}
		}
	}
	return false;
}

// input color: (e.g.) if current color is P1PawnColor, both pawn and king
void board::check_jump(list<jump*>& jlist, char color, int x, int y, jump* jp = NULL) {
	pieces[x][y] = color;
	// move
	if (tolower(color) == P1PawnColor) {
		if (x % 2 == 0) {
			if (check_jump_valid(x,y, x+1, y, x+2, y-1)) insert_jump_move_to_list(jlist, color, x, y, x+1, y, x+2, y-1, jp);
			if (check_jump_valid(x,y,x+1, y+1, x+2, y+1)) insert_jump_move_to_list(jlist, color, x, y, x+1, y+1, x+2, y+1, jp);
		}
		else {
			if (check_jump_valid(x,y,x+1, y-1, x+2, y-1)) insert_jump_move_to_list(jlist, color, x, y, x+1, y-1, x+2, y-1, jp);
			if (check_jump_valid(x,y,x+1, y, x+2, y+1)) insert_jump_move_to_list(jlist, color, x, y, x+1, y, x+2, y+1, jp);
		}
	}
	else if (tolower(color) == P2PawnColor) {
		if (x % 2 == 0) {
			if (check_jump_valid(x,y,x-1, y+1, x-2, y+1)) insert_jump_move_to_list(jlist, color, x, y, x-1, y+1, x-2, y+1, jp);
			if (check_jump_valid(x,y,x-1, y, x-2, y-1))	insert_jump_move_to_list(jlist, color, x, y, x-1, y, x-2, y-1, jp);
		}
		else {
			if (check_jump_valid(x,y,x-1, y-1, x-2, y-1)) insert_jump_move_to_list(jlist, color, x, y, x-1, y-1, x-2, y-1, jp);
			if (check_jump_valid(x,y,x-1, y, x-2, y+1)) insert_jump_move_to_list(jlist, color, x, y, x-1, y, x-2, y+1, jp);
		}
	}
	else if (tolower(color) == P1KingColor) {
		if (x % 2 == 0) {
			if (check_jump_valid(x,y,x-1, y+1, x-2, y+1)) insert_jump_move_to_list(jlist, color, x, y, x-1, y+1, x-2, y+1, jp);
			if (check_jump_valid(x,y,x-1, y, x-2, y-1))	insert_jump_move_to_list(jlist, color, x, y, x-1, y, x-2, y-1, jp);
			// regular p1pawncolor move
			if (check_jump_valid(x,y, x+1, y, x+2, y-1)) insert_jump_move_to_list(jlist, color, x, y, x+1, y, x+2, y-1, jp);
			if (check_jump_valid(x,y,x+1, y+1, x+2, y+1)) insert_jump_move_to_list(jlist, color, x, y, x+1, y+1, x+2, y+1, jp);
		}
		else {
			if (check_jump_valid(x,y,x-1, y-1, x-2, y-1)) insert_jump_move_to_list(jlist, color, x, y, x-1, y-1, x-2, y-1, jp);
			if (check_jump_valid(x,y,x-1, y, x-2, y+1)) insert_jump_move_to_list(jlist, color, x, y, x-1, y, x-2, y+1, jp);
			// regular p1pawncolor move
			if (check_jump_valid(x,y,x+1, y-1, x+2, y-1)) insert_jump_move_to_list(jlist, color, x, y, x+1, y-1, x+2, y-1, jp);
			if (check_jump_valid(x,y,x+1, y, x+2, y+1)) insert_jump_move_to_list(jlist, color, x, y, x+1, y, x+2, y+1, jp);
		}
	}
	else if (tolower(color) == P2KingColor) {
		if (x % 2 == 0) {
			if (check_jump_valid(x,y, x+1, y, x+2, y-1)) insert_jump_move_to_list(jlist, color, x, y, x+1, y, x+2, y-1, jp);
			if (check_jump_valid(x,y,x+1, y+1, x+2, y+1)) insert_jump_move_to_list(jlist, color, x, y, x+1, y+1, x+2, y+1, jp);
			// regular p2pawncolor move
			if (check_jump_valid(x,y,x-1, y+1, x-2, y+1)) insert_jump_move_to_list(jlist, color, x, y, x-1, y+1, x-2, y+1, jp);
			if (check_jump_valid(x,y,x-1, y, x-2, y-1))	insert_jump_move_to_list(jlist, color, x, y, x-1, y, x-2, y-1, jp);
		}
		else {
			if (check_jump_valid(x,y,x+1, y-1, x+2, y-1)) insert_jump_move_to_list(jlist, color, x, y, x+1, y-1, x+2, y-1, jp);
			if (check_jump_valid(x,y,x+1, y, x+2, y+1)) insert_jump_move_to_list(jlist, color, x, y, x+1, y, x+2, y+1, jp);
			// regular p2pawncolor move
			if (check_jump_valid(x,y,x-1, y-1, x-2, y-1)) insert_jump_move_to_list(jlist, color, x, y, x-1, y-1, x-2, y-1, jp);
			if (check_jump_valid(x,y,x-1, y, x-2, y+1)) insert_jump_move_to_list(jlist, color, x, y, x-1, y, x-2, y+1, jp);
		}
	}
}

void board::make_jump_move(list<jump*>& jlist) {
	if (!jlist.empty()) {
		list<jump*>::const_iterator it = jlist.begin();
		for (; it != jlist.end(); ++it)	{
			if ((*it)->noNextMove) {
				move* m = new move((*it)->jumpingPiece, -1, -1, -1, -1);
				jump* jp = (*it);
				while (jp != NULL) {
					m->jumpPoints.push_front(jp);
					++jp->num_jumps;
					jp = jp->prev;
				}
				m->x1 = m->jumpPoints.front()->xstart;
				m->y1 = m->jumpPoints.front()->ystart;
				convert(m->jumpPoints.front()->xstart, m->jumpPoints.front()->ystart, m->command);
				for (list<jump*>::iterator it = m->jumpPoints.begin(); it != m->jumpPoints.end(); ++it) {
					convert((*it)->xend, (*it)->yend, m->command);
					if ((*it)->noNextMove) {
						m->x2 = (*it)->xend;
						m->y2 = (*it)->yend;
					}
				}
				moveList.push_back(m);
				m->command += "-1";
				undo_move(m);
			}
		}
	}
}

// converts x,y to string representation
void board::convert(const int &x, const int &y, string &s) {
	 char c1 = '0'+x;
	 char c2;
	 if (x % 2 == 0) c2 = '0' + (2*y + 1);
	 else c2 = '0'+(2*y);
	 s += c1;
	 s += ' ';
	 s += c2;
	 s += ' ';
}

void board::check_neighbor(int &x, int &y) {
	// player 1: move down the board
	if (tolower(pieces[x][y]) == P1PawnColor) {
		if (x % 2 == 0) {
			insert_move_to_queue(x, y, x+1, y);
			insert_move_to_queue(x, y, x+1, y+1);
		} else {
			insert_move_to_queue(x, y, x+1, y);
			insert_move_to_queue(x, y, x+1, y-1);
		}
	}
	// player 2: move up the board
	else if (tolower(pieces[x][y]) == P2PawnColor) {
		if (x % 2 == 0) {
			insert_move_to_queue(x, y, x-1, y);
			insert_move_to_queue(x, y, x-1, y+1);
		} else {
			insert_move_to_queue(x, y, x-1, y);
			insert_move_to_queue(x, y, x-1, y-1);
		}
	}
	// player 1 king: move up the board
	else if (tolower(pieces[x][y]) == P1KingColor) {
		if (x % 2 == 0) {
			insert_move_to_queue(x, y, x-1, y);
			insert_move_to_queue(x, y, x-1, y+1);
			// regular p1pawncolor moves
			insert_move_to_queue(x, y, x+1, y);
			insert_move_to_queue(x, y, x+1, y+1);
		} else {
			insert_move_to_queue(x, y, x-1, y);
			insert_move_to_queue(x, y, x-1, y-1);
			// regular p1pawncolor moves
			insert_move_to_queue(x, y, x+1, y);
			insert_move_to_queue(x, y, x+1, y-1);
		}
	}
	// player 2 king: move down the board
	else if (tolower(pieces[x][y]) == P2KingColor) {
		if (x % 2 == 0) {
			insert_move_to_queue(x, y, x+1, y);
			insert_move_to_queue(x, y, x+1, y+1);
			// regular p2pawncolor moves
			insert_move_to_queue(x, y, x-1, y);
			insert_move_to_queue(x, y, x-1, y+1);
		} else {
			insert_move_to_queue(x, y, x+1, y);
			insert_move_to_queue(x, y, x+1, y-1);
			// regular p2pawncolor moves
			insert_move_to_queue(x, y, x-1, y);
			insert_move_to_queue(x, y, x-1, y-1);
		}
	}
}


void board::insert_move_to_queue(const int &x1,const int &y1, int x2, int y2) {
	if (check_valid_position(x2, y2) && pieces[x2][y2] == '-') {
		move* m = new move(pieces[x1][y1], x1, y1, x2, y2);
		moveList.push_back(m);
		convert(x1, y1, m->command);
		convert(x2, y2, m->command);
		m->command += "-1";
	}
}

bool board::list_move() {
	// clear up move list
	while (!moveList.empty()) {
		delete moveList.front();
		moveList.pop_front();
	}
	// if movelist is not filled yet
	// player 1
	if (color == P2PawnColor) {
		for (int i = 0; i < 8; ++i) {
			for (int j = 0; j != 4; ++j) {
				if (pieces[i][j] == P2PawnColor || pieces[i][j] == P2KingColor) {
					check_neighbor(i, j);
				}
			}
		}
	// player 2
	} else if (color == P1PawnColor) {
		for (int i = 7; i > -1; --i) {
			for (int j = 0; j != 4; ++j) {
				if (pieces[i][j] == P1PawnColor || pieces[i][j] == P1KingColor) {
					check_neighbor(i, j);
				}
			}
		}
	}
	if (moveList.empty()) return false;
	return true;
}
