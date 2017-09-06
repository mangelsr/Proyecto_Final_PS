Proyecto final programacion de sistemas
======================

Requerimentos:
---------------
Librerias de C:
	libudev-dev
	sudo apt-get install libudev*

	libmicrohttpd
	sudo apt-get install microhttpd*

Librerias de python3:
	requests
	sudo pip3 install requests
Tener disponible los puertos 8888 y 4545, que son los que usa para conectar los componentes
El puerto 8888 sirve para conectar el cliente con el servidor REST, mientras que el 4545 es para
el servidor REST con el proceso daemon.


Instrucciones:
---------------
	Correr el daemon ./bin/daemon
	Correr el servidor ./bin/servidor
	Correr el cliente ./src/cliente.py


