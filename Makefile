todo: libjsmn servidor listener  

servidor: objects
	gcc -Wall -I./include/ ./obj/servidor.o ./lib/libjsmn.a -lmicrohttpd -o ./bin/servidor

listener: objects
	gcc -Wall ./obj/listener.o -ludev -o ./bin/listener
	gcc -Wall ./obj/daemon.o -ludev -o ./bin/daemon

libjsmn: objects
	ar rcs ./lib/libjsmn.a ./obj/jsmn.o

objects:
	gcc -Wall -fPIC -c -g -Iinclude/ src/*.c
	mv -f *.o obj/

.PHONY: clean
clean:
	rm -f lib/*.so obj/*.o bin/* lib/*.a