ayane: *.cc *.h
	g++ -DDBG -Og -fmax-errors=1 -fno-exceptions -g -oayane -pipe -std=c++17 *.cc -lgmp

prof:
	g++ -fno-exceptions -oayane -pg -pipe -std=c++17 *.cc -lgmp

release:
	g++ -O3 -fno-exceptions -oayane -pipe -s -std=c++17 *.cc -lgmp

clean:
	rm ayane

install:
	mv ayane /usr/local/bin

uninstall:
	rm /usr/local/bin/ayane
