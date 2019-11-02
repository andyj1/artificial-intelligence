//============================================================================
// Created on  : October 29, 2019
// Finished on : November 1, 2019
// Name        : Jongoh (Andy) Jeong
// Description : Checkers AI Program
//============================================================================

// game.cpp --> board.cpp
// entry points: initialize(), check_next_available_move(), check_ai_turn()
#include "game.h"

using std::endl;
using std::cin;
using std::cout;

const int game::maxIterDepth = 25;

int main() {
	game new_game;
	new_game.start();
}

game::game() :
		curBoard(nodeptr<board>(new board())), bestMove(NULL), tempBestMove(NULL), maxdepth(0),
		curDepth(0), timeUp(false), gameOver(false), reachedEnd(false), startTime(0), endTime(0),
		startTimeD(0), endTimeD(0) { }

void game::start() {
	gameOver = false;
	curBoard->initialize();
	while (!gameOver) get_going();
}

void game::terminate() {
	gameOver = true;
	cout << "The game is over." << endl;
	cout << endl;
	// winner declared
	if (curBoard->color == P1PawnColor) cout << "Player 2 wins." << endl;
	else cout << "Player 1 wins." << endl;

	// another game
	char new_game_yn;
	bool ask_another_game = false;
	cout << "Play again? (Y/N):" << endl;
	cin >> new_game_yn;

	while (!ask_another_game) {
		if (tolower(new_game_yn) == 'y') {
			ask_another_game = true;
			start();
		}
		else if (tolower(new_game_yn) == 'n') ask_another_game = true;
		else { // any other character than 'y' or 'n'
			cout << "Play again? (Y/N):" << endl;
			cin >> new_game_yn;
		}
	}
}
void game::tie_game() {
	char answer;
	bool loop = true;
	gameOver = true;
	cout << "The game is over." << endl;
	cout << endl;
	cout << "Game ended in a tie" << '\n';
	cout << "Do you want to play again? (Y/N):" << endl;
	cin >> answer;
	while (loop) {
		if (tolower(answer) == 'y') {
			loop = false;
			start();
		}
		else if (tolower(answer) == 'n')
			loop = false;
		else {
			cout << "Do you want to play again? (Y/N):" << endl;
			cin >> answer;
		}
	}
}

void game::get_going() {
	// if tie is returned, proceed with tie game procedure
	if (curBoard->check_tie) tie_game();
	// display board
	curBoard->print_board();

	// if not tie, then proceed accordingly
	if (!curBoard->check_next_available_move()) terminate();
	else if (curBoard->check_ai_turn()) ai_search();
	else curBoard->user_move_input();
}

void game::ai_search() {
	curBoard->print_moves();
	cout << "AI is making a move..." << endl;
	if (curBoard->moveList.size() == 1) {
		bestMove = curBoard->moveList.front();
		time(&startTime);
		time(&endTime);
		curDepth = 0;
	}
	else {
		time(&startTime);
		for (int i = 1; i < maxIterDepth; ++i)	{
			time(&startTimeD);
			maxdepth = i;
			alpha_beta_search(curBoard, i, std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
			time(&endTimeD);
			if (difftime(endTimeD, startTimeD) >= ((board::timelimit) / 2)) {
				time(&endTime);
				timeUp = true;
				break;
			}
			if (timeUp)	break;
			else bestMove = tempBestMove;

			if (reachedEnd)	break;
		}
	}
	assert(bestMove != NULL);
	ai_search_message();
}

void game::ai_search_message() {
	curBoard->make_move(bestMove);
	cout << "Completed search to depth " << curDepth << "." << endl;
	if (timeUp && curDepth != maxIterDepth && !reachedEnd) cout << "Ran out of time searching to depth " << curDepth + 1 << "." << endl;
	cout << "Search duration: " << difftime(endTime, startTime) << " seconds" << endl;
	cout << "Move done: ";

	std::string::const_iterator itr = bestMove->command.begin();
	int temp = (*itr) - '0';
	itr += 2;
	char columnHeaders[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'};
	cout << columnHeaders[(*itr) - '0'] << temp + 1;
	itr += 2;
	while (*itr != '-')	{
		temp = (*itr) - '0';
		cout << " -> ";
		itr += 2;
		cout << columnHeaders[(*itr) - '0'] << temp + 1;
		itr += 2;
	}
	timeUp = false;
	reachedEnd = false;
	bestMove = NULL;
	tempBestMove = NULL;
	cout << endl;
}

int game::alpha_beta_search(nodeptr<board> &b, int depth, int alpha, int beta) {
	if (depth != maxdepth && !b->check_next_available_move()) {
		curDepth = maxdepth;
		reachedEnd = true;
		if (b->color == P1PawnColor) return std::numeric_limits<int>::max();
		else return std::numeric_limits<int>::min();
	}
	reachedEnd = false;
	if (depth == 0) return b->evaluate();
	std::list<move *>::iterator iter = b->moveList.begin();

	int localalpha = std::numeric_limits<int>::min();
	int localbeta = std::numeric_limits<int>::max();

	if (b->color == P2PawnColor) { // player 1 (alpha)
		for (; iter != b->moveList.end(); ++iter) {
			time(&endTime);
			if (difftime(endTime, startTime) >= (board::timelimit - 1))	{
				timeUp = true;
				break;
			}
			b->make_move(*iter);
			nodeptr<board> newB(new board(*b));
			int value = alpha_beta_search(newB, depth - 1, alpha, std::min(localbeta, beta));
			b->undo_move(*iter);
			b->switch_color_on_next_turn();
			if (value > alpha) {
				alpha = value;
				if (depth == maxdepth) tempBestMove = (*iter);
			}
			if (alpha >= beta && depth < maxdepth) return alpha;
		}
		if (!timeUp && depth == maxdepth) curDepth = depth;
		return alpha;
	} else if (b->color == P1PawnColor) { // player 2 (beta)
		for (; iter != b->moveList.end(); ++iter) {
			time(&endTime);
			if (difftime(endTime, startTime) >= (board::timelimit - 1))	{
				timeUp = true;
				break;
			}
			b->make_move(*iter);
			nodeptr<board> newB(new board(*b));
			int value = alpha_beta_search(newB, depth - 1, std::max(localalpha, alpha), beta);
			b->undo_move(*iter);
			b->switch_color_on_next_turn();
			if (value < beta) {
				beta = value;
				if (depth == maxdepth) tempBestMove = (*iter);
			}
			if (alpha >= beta) return beta;
		}
		if (!timeUp && depth == maxdepth) curDepth = depth;
		return beta;
	}
}
