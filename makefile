run:
	gcc src/config.c src/main.c -o main && ./main

clean:
	rm main *.o
