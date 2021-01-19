FS=auth-fs
MOUNT_PLACE=/tmp/$(FS)

FLAGS=-Wall -Wextra -g

INC_GLIB=`pkg-config --cflags --libs glib-2.0`
INC_CFL=`pkg-config --cflags fuse3 glib-2.0`
INC_LIB=`pkg-config --libs fuse3 glib-2.0`

env: clean compile
	mkdir -p $(MOUNT_PLACE)
	./$(FS) -f $(MOUNT_PLACE)
	
compile: storage.o
	gcc $(FLAGS) -o $(FS) storage.o $(INC_CFL) $(FS).c $(INC_LIB)

storage.o: storage.c
	gcc -c storage.c $(INC_GLIB)

drop_env:
	umount $(MOUNT_PLACE)

clean:
	rm -rf *.o
	find . -type f -executable | xargs rm -rf
