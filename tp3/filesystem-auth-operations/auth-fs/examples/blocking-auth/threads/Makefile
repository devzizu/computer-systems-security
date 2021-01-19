
JSONC=`pkg-config --cflags --libs json-c`

main: processor-thread.o auth-api.o
	gcc main.c -lpthread -o main processor-thread.o auth-api.o -lcurl $(JSONC)

auth-api.o: auth-api.c
	gcc -c auth-api.c -lcurl

processor-thread.o: processor-thread.c
	gcc -c processor-thread.c $(JSONC)

clean:
	rm -rf *.o *.out main
