default: main.c
	gcc -O2 -o mexplorer.exe main.c -luser32 -lgdi32 -lshell32

clean:
	rm mexplorer.exe
