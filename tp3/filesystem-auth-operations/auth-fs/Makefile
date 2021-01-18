FS=auth-fs
MOUNT_PLACE=/tmp/$(FS)

FLAGS=-Wall -Wextra -g

INC_GLIB=`pkg-config --cflags --libs glib-2.0`
INC_FUSE=`pkg-config fuse3 --cflags --libs`

create_env: compile
	mkdir -p $(MOUNT_PLACE)
	./$(FS) -s -f $(MOUNT_PLACE)
	
compile: storage.o
	gcc $(FLAGS) $(FS).c $(INC_GLIB) $(INC_FUSE) -o $(FS) storage.o

storage.o: storage.c
	gcc -c storage.c $(INC_GLIB)

clear_env:
	umount $(MOUNT_PLACE)

clean:
	rm -rf *.o
	find . -type f -executable | xargs rm
