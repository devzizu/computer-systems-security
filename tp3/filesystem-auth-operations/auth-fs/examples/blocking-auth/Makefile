
JSONC=`pkg-config --cflags --libs json-c`

main: clean auth-api.o
	gcc main.c -o main auth-api.o -lcurl $(JSONC)

auth-api.o: auth-api.c
	gcc -c auth-api.c -lcurl

clean:
	rm -rf *.o *.out main
