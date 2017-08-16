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

    udev_enumerate_uref(enumerar);
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
        hijo = udev_device_new_from_syspath(udev, ruta);
        
        struct udev_device* block = obtener_hijo(udev, scsi, "block");
        struct udev_device* scsi = obtener_hijo(udev, scsi, "scsi_disk");

        struct udev_device* usb = 
            udev_device_get_parent_with_subsytem_devtype(scsi, "usb", "usb_device");
        
        if (block && scsi_disk && usb){
            printf("block = %s, usb=%s:%s, scsi=%s\n",
                udev_device_get_devnode(block);
                udev_device_get_value(usb, "vendor");
                udev_device_get_value(usb, "idProduct");
                udev_device_get_value(scsi, "vendor"));
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