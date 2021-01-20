

main: clean mail.o
	gcc main.c -o main mail.o -lcurl

mail.o: mail.c
	gcc -c mail.c -lcurl

clean:
	rm -rf main *.out *.o
