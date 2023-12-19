all:
	g++ main.cpp -ltbb -O3 -o main

run: all	
	./main > results.txt

clean:
	rm main results.txt