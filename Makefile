all:
	g++ -I src/include -L src/lib -o bouncy bouncy.cpp -lSDL2main -lSDL2