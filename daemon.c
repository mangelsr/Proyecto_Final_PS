#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <libudev.h>
#include <pthread.h>

/*
    while(1){
        //obtener usb hacerlo en un hilo (con sleep)
        recive()/read() //conexion con el servidor
    }
*/

struct udev *p = udev_new();

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

    pthread_t hiloActualizacion;



    while(1){
        //conexion con el servidor...

    }

    return (0);
}


void* monitorear(void* arg){
    while(1){
        enumerar_disp_alm_masivo(p);
        sleep(3);
    }
}


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
                udev_device_get_sysattr_value(usb, "vendor"),
                udev_device_get_sysattr_value(usb, "idProduct"),
                udev_device_get_sysattr_value(scsi, "vendor"));
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