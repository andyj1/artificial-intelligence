all:
	g++ train.cpp -std=c++11 -o train.exe
	./train.exe
	g++ test.cpp -std=c++11 -o test.exe
	./test.exe

train:
	g++ train.cpp -std=c++11 -o train.exe
	./train.exe

test:
	g++ test.cpp -std=c++11 -o test.exe
	./test.exe

debug_train:
	g++ -std=c++11 -g -o trainDebug.exe train.cpp

debug_test:
	g++ -std=c++11 -g -o testDebug.exe test.cpp

clean:
	rm -f *.exe *.stackdump *.out
