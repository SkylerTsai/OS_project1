file?=test

all:
	gcc main.c -o main.o

run:
	sudo dmesg --clear
	sudo ./main.o < $(file).txt 1> $(file)_stdout.txt
	dmesg | grep project1 > $(file)_dmesg.txt

debug:
	gcc main.c -DDEBUG -o main.o
	sudo ./main.o < $(file).txt 1> $(file)_debug.txt 2> $(file)_error.txt

demo:
	sudo dmesg --clear
	sudo ./main.o < $(file).txt
	dmesg | grep project1

clean:
	rm main.o $(file)_*
