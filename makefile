build:
	gcc src/config.c src/main.c src/static.c -g -o main
run:
	gcc src/config.c src/main.c src/static.c -g -o main && ./main

clean:
	rm main
