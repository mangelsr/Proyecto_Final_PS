todo: servidor listener

servidor: ./src/servidor.c
	gcc -Wall ./src/servidor.c -o ./bin/servidor

listener: ./src/listener.c
	gcc -Wall ./src/listener.c -ludev -o ./bin/listener