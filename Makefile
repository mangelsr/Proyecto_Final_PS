daemon: ./src/daemon.c
	gcc -Wall ./src/daemon.c -o -ludev ./bin/daemon

servidor: ./src/servidor.c
	gcc -Wall ./src/servidor.c -o ./bin/servidor

listener: ./src/listener.c
	gcc -Wall ./src/listener.c -o -ludev ./bin/listener