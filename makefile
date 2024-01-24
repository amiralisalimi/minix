run:
	gcc src/config.c src/main.c src/static.c -o main && ./main

clean:
	rm main
