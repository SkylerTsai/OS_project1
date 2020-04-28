file?=test

all:
	gcc main.c -o main.out

run:
	sudo dmesg clean
	sudo ./main.out < $(file).txt 1> $(file)_stdout.txt
	dmesg | grep Project1 > $(file)_dmesg.txt

clean:
	rm main.out
