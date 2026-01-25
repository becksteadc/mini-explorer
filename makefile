default: main.c
	gcc -o mexplorer.exe main.c -luser32 -lgdi32 -lshell32

release:
	gcc -O2 -o mexplorer.exe main.c -luser32 -lgdi32 -lshell32

debug:
	gcc -g -o mexplorer.exe main.c -luser32 -lgdi32 -lshell32

clean:
	rm mexplorer.exe
