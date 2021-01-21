FS=auth-fs
MOUNT_PLACE=/tmp/$(FS)

FLAGS=-Wall -Wextra -g

INC_GLIB=`pkg-config --cflags --libs glib-2.0`
INC_CFL=`pkg-config --cflags fuse3 glib-2.0 json-c`
INC_LIB=`pkg-config --libs fuse3 glib-2.0 json-c`
INC_CURL=-lcurl
JSONC=`pkg-config --cflags --libs json-c`

env: clean compile
	mkdir -p $(MOUNT_PLACE)
	./$(FS) -f -o allow_other $(MOUNT_PLACE)
	
compile: storage.o fs-tools.o mail.o auth-api.o
	gcc $(FLAGS) -o $(FS) storage.o fs-tools.o $(INC_CFL) mail.o auth-api.o $(INC_CURL) $(FS).c $(INC_LIB)

storage.o: storage.c
	gcc -c storage.c $(INC_GLIB)

fs-tools.o: fs-tools.c
	gcc -c fs-tools.c

mail.o: mail.c
	gcc -c mail.c $(INC_CURL)

auth-api.o: auth-api.c
	gcc -c auth-api.c $(INC_CURL)

drop_env:
	umount $(MOUNT_PLACE)

clean:
	rm -rf *.o
	find . -type f -executable | xargs rm -rf
