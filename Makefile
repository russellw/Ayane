ayane: src/*
	g++ -DDBG -Og -fmax-errors=1 -fno-exceptions -fsanitize=address -g3 -oayane -pipe -std=c++17 src/*.cc -lgmp

prof:
	g++ -fno-exceptions -oayane -pg -pipe -std=c++17 src/*.cc -lgmp

release:
	g++ -O3 -fno-exceptions -oayane -pipe -std=c++17 src/*.cc -lgmp

clean:
	rm ayane

install:
	mv ayane /usr/local/bin

uninstall:
	rm /usr/local/bin/ayane
