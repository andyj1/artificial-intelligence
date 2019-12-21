#ifndef GAME_H_
#define GAME_H_

#include <sys/types.h>
#include <vector>
#include <assert.h>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <iostream>
#include <limits>
#include <list>
#include <time.h>
#include <string>
#include <iterator>

#include "board.h"

template <class T>
class nodeptr {
	private:
		T* pData;
	public:
		nodeptr(T* pVal): pData(pVal) {}
		~nodeptr()	{
			delete pData;
		}
		T& operator*() {
			return *pData;
		}
		T* operator->() {
			return pData;
		}
};
class game {
	public:
		nodeptr<board> curBoard;
		static const int maxIterDepth;
		move* bestMove;
		move* tempBestMove;
		int maxdepth, curDepth;
		bool timeUp, gameOver, reachedEnd;
		time_t startTime, endTime, startTimeD, endTimeD;

		void get_going();
		void ai_search_message();
		void ai_search();
		int alpha_beta_search(nodeptr<board>&, int, int, int);
		void terminate();
		void tie_game();

		game();
		void start();
};

#endif /* GAME_H_ */
