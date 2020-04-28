file?=test

all:
	gcc main.c -o main.o

run:
#	sudo dmesg clean
	sudo ./main.o < $(file).txt 1> $(file)_stdout.txt
	dmesg | grep Project1 > $(file)_dmesg.txt

debug:
	gcc main.c -DDEBUG -o main.o
	sudo ./main.o < $(file).txt 1> $(file)_debug.txt 2> $(file)_error.txt

clean:
	rm main.o $(file)_*
