#include <sys/types.h>          /* some systems still require this */
#include <sys/stat.h>
#include <stdio.h>              /* for convenience */
#include <stdlib.h>             /* for convenience */
#include <stddef.h>             /* for offsetof */
#include <string.h>             /* for convenience */
#include <unistd.h>             /* for convenience */
#include <signal.h>             /* for SIG_ERR */ 
#include <netdb.h> 
#include <errno.h> 
#include <syslog.h> 
#include <sys/socket.h> 
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <libudev.h>
#include <pthread.h>

/*
    while(1){
        //obtener usb hacerlo en un hilo (con sleep)
        recive()/read() //conexion con el daemon_server
    }
*/
#define PORT 4545 //para comunicarse con el servidor
#define BUFLEN 100 //para lo que recibe del servidor
#define MAXDEVICES 10
char* ip = "127.0.0.1";

struct dispositivo{
  char *nodo;       // /dev/...
  char *puntoMontaje; // /media/...
  char *id;         // idVendor:idProduct
  char *nombre;     // cadena vacia inicialmente
};

struct dispositivo usbs[MAXDEVICES];

struct udev_device* obtener_hijo(struct udev* udev, struct udev_device* padre, const char* subsistema)
{
    struct udev_device* hijo = NULL;
    struct udev_enumerate* enumerar = udev_enumerate_new(udev);

    udev_enumerate_add_match_parent(enumerar, padre);
    udev_enumerate_add_match_subsystem(enumerar, subsistema);
    udev_enumerate_scan_devices(enumerar);

    struct udev_list_entry* dispositivos = udev_enumerate_get_list_entry(enumerar);
    struct udev_list_entry* entrada;

    udev_list_entry_foreach(entrada, dispositivos) {
        const char* ruta = udev_list_entry_get_name(entrada);
        hijo = udev_device_new_from_syspath(udev, ruta);
    }

    udev_enumerate_unref(enumerar);
    return hijo;
}

static void enumerar_disp_alm_masivo(struct udev* udev)
{
    struct udev_enumerate* enumerar = udev_enumerate_new(udev);

    udev_enumerate_add_match_subsystem(enumerar, "scsi");
    udev_enumerate_add_match_property(enumerar, "DEVTYPE", "scsi_device");
    udev_enumerate_scan_devices(enumerar);

    struct udev_list_entry* dispositivos = udev_enumerate_get_list_entry(enumerar);
    struct udev_list_entry* entrada;

    int i = 0; 

    udev_list_entry_foreach(entrada, dispositivos) {
        const char* ruta = udev_list_entry_get_name(entrada);
        struct udev_device* scsi = udev_device_new_from_syspath(udev, ruta);
        
        struct udev_device* block = obtener_hijo(udev, scsi, "block");
        struct udev_device* scsi_disk = obtener_hijo(udev, scsi, "scsi_disk");

        struct udev_device* usb 
            = udev_device_get_parent_with_subsystem_devtype(scsi, "usb", "usb_device");
        
        if (block && scsi_disk && usb){
            printf("block = %s, usb=%s:%s, scsi=%s\n",
                udev_device_get_devnode(block),
                udev_device_get_sysattr_value(usb, "idVendor"),
                udev_device_get_sysattr_value(usb, "idProduct"),
                udev_device_get_sysattr_value(scsi, "vendor"));
            struct dispositivo *d = malloc(sizeof(struct dispositivo));
            d->nodo = (char *)udev_device_get_devnode(block);
            d->nombre = (char *)malloc(1024*sizeof(char));
            char id[1024];
            snprintf(id, sizeof(id), "%s:%s", 
                        (char *)udev_device_get_sysattr_value(usb, "idVendor"), 
                        (char *)udev_device_get_sysattr_value(usb, "idProduct"));
            d->id = id;
            //d->puntoMonje = ;
            usbs[i] = *d;
        }
        if (block){
            udev_device_unref(block);
        }
        if (scsi_disk){
            udev_device_unref(scsi_disk);
        }
        
        udev_device_unref(scsi);
    }
    udev_enumerate_unref(enumerar);
}

/*
void* monitorear(void* arg){
    while(1){
        enumerar_disp_alm_masivo((struct udev *)arg);
        sleep(3);
    }
    return (void *)0;
}
*/

int main(int argc, char** argv){
    
        pid_t process_id = 0;
    
        // Crea el proceso hijo
        process_id = fork();
        
        // Verifica retorno del fork()
        if (process_id < 0){
            printf("fork failed!\n");
            // Return failure in exit status
            exit(1);
        }
    
        // Mata el proceso del padre
        if (process_id > 0){
            printf("process_id of child process %d\n", process_id);
            exit(0);
        }
    
        umask(0);
    
        chdir("/");
    
        //pthread_t hiloActualizacion;
    
        struct udev *p = udev_new();
    
    ////////////////////////////////S E R V I D O R///////////////////////////////////
        int daemon_server;
        //Direccion del daemon_server
        struct sockaddr_in direccion_daemon_server;
        //ponemos en 0 la estructura direccion_daemon_server
        memset(&direccion_daemon_server, 0, sizeof(direccion_daemon_server));
    
        //llenamos los campos
        //IPv4
        direccion_daemon_server.sin_family = AF_INET;
        //Convertimos el numero de puerto al endianness de la red
        direccion_daemon_server.sin_port = htons(PORT);
        //Nos vinculamos a la interface localhost o podemos usar INADDR_ANY para ligarnos A TODAS las interfaces
        direccion_daemon_server.sin_addr.s_addr = inet_addr(ip);
    
        //Abrimos un socket para el daemon
        daemon_server = socket(((struct sockaddr *)&direccion_daemon_server)->sa_family, SOCK_STREAM, 0);
        if (daemon_server == -1)
        {
            printf("Error al abrir el socket\n");
            return -1;
        }
        //Para que no haya problemas debido a que el socket siga abierto_daemon_server
        int abierto_daemon_server = 1;
        setsockopt(daemon_server, SOL_SOCKET, SO_REUSEADDR, &abierto_daemon_server, sizeof(abierto_daemon_server));
    
        //Enlazamos el socket
        int enlace_daemon_server = bind(daemon_server, (struct sockaddr *)&direccion_daemon_server, sizeof(direccion_daemon_server));
        if(enlace_daemon_server != 0)
        {
            printf("Error!!!\n");
            printf("No se puede enlazar al puerto : dirección ya está en uso\n");
        return -1;
        }
    
        //Ponemos al socket del daemon en espera
        int escuchar = listen(daemon_server,100);
        if(escuchar == -1)
        {
            printf("No es posible escuchar en ese puerto\n");
            return -1;
        }
        printf("Enlazado al puerto.\n");
    
        struct sockaddr_in direccion_servidor;
        memset(&direccion_daemon_server, 0, sizeof(direccion_servidor));
        unsigned int tam = sizeof(direccion_servidor);
    
        enumerar_disp_alm_masivo(p);
    
        while(1)
        {
            int servidor = accept(daemon_server,(struct sockaddr *)&direccion_servidor,&tam);
      
            char *fromServidor = (char *)malloc(BUFLEN*sizeof(char *));
            recv(servidor, fromServidor, BUFLEN, 0);
            printf("%s\n",fromServidor);
            if (0==strcmp(fromServidor,"listar_dispositivos")){
                send(servidor, "{lista de dispositivos}", 100, 0);
            } else if (0==strcmp(fromServidor,"nombrar_dispositivo")){
                send(servidor, "{nombrar dispositivo}", 100, 0);
            } else{
                send(servidor, "{otra solicitud}", 100, 0);
            }

            close(servidor);
        }
    //////////////////////////////////////////////////////////////////////////////////
        return (0);
    }