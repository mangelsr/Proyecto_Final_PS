todo: servidor listener libjsmn 

servidor: objects
	gcc -Wall ./obj/servidorEjemplo.o -lmicrohttpd -o ./bin/servidorEjemplo
	gcc -Wall ./obj/ejemploPost.o -lmicrohttpd -o ./bin/ejemploPost

listener: objects
	gcc -Wall ./obj/listener.o -ludev -o ./bin/listener

libjsmn: objects
	ar rcs ./lib/libjsmn.a ./obj/jsmn.o

objects:
	gcc -Wall -fPIC -c -g -Iinclude/ src/*.c
	mv -f *.o obj/

.PHONY: clean
clean:
	rm -f lib/*.so obj/*.o bin/* lib/*.a