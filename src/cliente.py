#!/usr/bin/env python3
import requests
import json

def printmenu():
    print("\t### Menu ### ")
    print("1. Listar dispositivos")
    print("2. Nombrar dispositivo")
    print("3. Leer archivo")
    print("4. Escribir archivo")
    print("5. Salir")


print("Cliente")

ip = input("Ingrese la ip del servidor: ")
port = input("Ingrese el puerto a usar: ")
url = "http://" + ip + ":" + port

op = ""

while (op!="5"):
    printmenu()
    op = input("Seleccione una opcion: ")
    
    while (op!="1" and op!="2" and op!="3" and op!="4" and op!="5"):
        op = input("Opcion invalida, seleccione una opcion: ")
    
    if (op == "1"):
        r = requests.get(url + "/listar_dispositivos")
        print(r.status_code)
        print(r.text)
    
    elif (op == "2"):
        info = {}
        info["solicitud"] = "nombrar_dispositivo"
        info["nodo"] = input("Ingrese direccion del nodo: ")
        info["nombre"] = input("Ingrese el nombre del dispositivo: ")
        r = requests.post(url + "/nombrar_dispositivo", data = info)
        print(json.dumps(info))
        print(len(json.dumps(info)))
        print(r.status_code)
        print(r.text)
    
    elif (op == "3"):
        info = {}
        info["solicitud"] = "leer_archivo"
        info["nombre"] = input("Ingrese el nombre del dispositivo: ")
        info["nombre_archivo"] = input("Ingrese el nombre del archivo: ")
        r = requests.post(url + "/leer_archivo", data = info)
        print(json.dumps(info))
        print(len(json.dumps(info)))
        print(r.status_code)
        print(r.text)
    
    elif (op == "4"):
        info = {}
        info["solicitud"] = "escribir_archivo"
        info["nombre"] = input("Ingrese el nombre del dispositivo: ")
        info["nombre_archivo"] = input("Ingrese el nombre del archivo: ")
        info["contenido"] = input("Ingrese el nombre del archivo: ")
        info["tamano_contenido"] = len(info["contenido"])
        json1 = {}
        json1["json"] = str(json.dumps(info))
        print(json1)
        r = requests.post(url + "/escribir_archivo", data = json1)
        print(json.dumps(info))
        print(len(json.dumps(info)))
        print(r.status_code)
        print(r.text)
    
    elif (op == "5"):
        print("Salio del sistema")


