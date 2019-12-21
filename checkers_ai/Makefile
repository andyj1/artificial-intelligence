CXX=g++
CFLAGS = -std=c++0x -c -ggdb 
DEPS = board.h game.h

%.o: %.cpp $(DEPS)
	$(CXX) $(CFLAGS) -o $@ $<

game.exe: board.o game.o
	$(CXX) -o game.exe board.o game.o

debug:
	$(CXX) $(CFLAGS) -g gameDebug.exe board.cpp game.cpp

clean:
	rm *.exe *.o *.stackdump *~
