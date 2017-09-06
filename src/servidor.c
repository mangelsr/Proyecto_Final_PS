/* Feel free to use this example code in any way
   you see fit (Public Domain) */

   #include <sys/types.h>
   #ifndef _WIN32
   #include <sys/select.h>
   #include <sys/socket.h>
   #else
   #include <winsock2.h>
   #endif
   #include <microhttpd.h>
   #include <stdio.h>
   #include <string.h>
   #include <stdlib.h>
   #include <arpa/inet.h>
   #include "jsmn.h"
   
   #if defined(_MSC_VER) && _MSC_VER+0 <= 1800
   /* Substitution is OK while return value is not used */
   #define snprintf _snprintf
   #endif
   
   #define PORT            8888
   #define POSTBUFFERSIZE  1024
   #define MAXNAMESIZE     1000
   #define MAXANSWERSIZE   1024
   
   #define GET             0
   #define POST            1
   
   struct connection_info_struct
   {
     int connectiontype;
     char *answerstring;
     struct MHD_PostProcessor *postprocessor;
   };

   static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
    if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
        strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
      return 0;
    }
    return -1;
  }
      
   static int send_page (struct MHD_Connection *connection, const char *page){
     int ret;
     struct MHD_Response *response;
       
     response =
       MHD_create_response_from_buffer (strlen (page), (void *) page,
                       MHD_RESPMEM_PERSISTENT);
     if (!response)
       return MHD_NO;
   
     ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
     MHD_destroy_response (response);
   
     return ret;
   }
      
      
   static int iterate_post (void *coninfo_cls, enum MHD_ValueKind kind, const char *key,
                    const char *filename, const char *content_type,
                    const char *transfer_encoding, const char *data, uint64_t off,
                    size_t size){
     struct connection_info_struct *con_info = coninfo_cls;
     if (0 == strcmp (key, "json")){

       printf("%s: %s\n", key, data);
       /*
        Usar jsmn para parsear el json que envio el cliente
        sacar la solicitud y dependiendo esta, comunicarse con el daemon
        por medio de sockets para cumplir lo que pide
        la respuesta amacenarla en answerstring para enviarsela al cliente
       */

       int i;
       int r;

       jsmn_parser p;
       jsmntok_t t[128]; /* We expect no more than 128 tokens */
     
       jsmn_init(&p);
       r = jsmn_parse(&p, data, strlen(data), t, sizeof(t)/sizeof(t[0]));
       if (r < 0) {
         printf("Failed to parse JSON: %d\n", r);
         return 1;
       }
     
       /* Assume the top-level element is an object */
       if (r < 1 || t[0].type != JSMN_OBJECT) {
         printf("Object expected\n");
         return 1;
       }

       char* solicitud = (char *)malloc(40*sizeof(char));
       char* nodo = (char *)malloc(50*sizeof(char));
       char* nombre = (char *)malloc(50*sizeof(char));
       char* nombre_archivo = (char *)malloc(50*sizeof(char));
       char* contenido = (char *)malloc(20480*sizeof(char));
       char* tcontenido = (char *)malloc(1024*sizeof(char));
       int tamano_contenido;
     
       /* Loop over all keys of the root object */
       for (i = 1; i < r; i++) {
         if (jsoneq(data, &t[i], "solicitud") == 0) {
           printf("- Solicitud: %.*s\n", t[i+1].end-t[i+1].start,
                    data + t[i+1].start);
           sprintf(solicitud, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
           fflush(stdout);
           i++;
         } else if (jsoneq(data, &t[i], "nodo") == 0) {
           printf("- Nodo: %.*s\n", t[i+1].end-t[i+1].start,
                    data + t[i+1].start);
           sprintf(nodo, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
           fflush(stdout);
           i++;
         } else if (jsoneq(data, &t[i], "nombre") == 0) {
           printf("- Nombre: %.*s\n", t[i+1].end-t[i+1].start,
                    data + t[i+1].start);
           sprintf(nombre, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
           fflush(stdout);
           i++;
         } else if (jsoneq(data, &t[i], "nombre_archivo") == 0) {
           printf("- Nombre del archivo: %.*s\n", t[i+1].end-t[i+1].start,
                    data + t[i+1].start);
           sprintf(nombre_archivo, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
           fflush(stdout);
           i++;
         } else if (jsoneq(data, &t[i], "contenido") == 0) {
           printf("- Contenido del archivo: %.*s\n", t[i+1].end-t[i+1].start,
                    data + t[i+1].start);
           sprintf(contenido, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
           fflush(stdout);
           i++;
         } else if (jsoneq(data, &t[i], "tamano_contenido") == 0) {
           printf("- Tamano del Contenido: %.*s\n", t[i+1].end-t[i+1].start,
                    data + t[i+1].start);
           sprintf(tcontenido, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
           tamano_contenido = atoi(tcontenido);
           i++;
         } else {
           printf("Unexpected key: %.*s\n", t[i].end-t[i].start,
                    data + t[i].start);
         }
       }
       printf("%s\n", solicitud);
      
       if (0==strcmp(solicitud,"nombrar_dispositivo")){
         printf(" %s\n", nodo);
         printf(" %s\n", nombre);
       }
       else if (0==strcmp(solicitud,"escribir_archivo")){
        printf(" %s\n", contenido);
        printf(" %d\n", tamano_contenido);
      }
       
       char *answerstring;
       answerstring = malloc(10);
       if (!answerstring)
         return MHD_NO;
       con_info->answerstring = answerstring;
   
       return MHD_NO;
     }
     return MHD_YES;
   }
   
   static void request_completed (void *cls, struct MHD_Connection *connection,
                     void **con_cls, enum MHD_RequestTerminationCode toe){
     struct connection_info_struct *con_info = *con_cls;
   
     if (NULL == con_info)
       return;
   
     if (con_info->connectiontype == POST)
       {
         MHD_destroy_post_processor (con_info->postprocessor);
         if (con_info->answerstring)
           free (con_info->answerstring);
       }
   
     free (con_info);
     *con_cls = NULL;
   }
      
   static int answer_to_connection (void *cls, struct MHD_Connection *connection,
                            const char *url, const char *method,
                            const char *version, const char *upload_data,
                            size_t *upload_data_size, void **con_cls){
     if (NULL == *con_cls){
       struct connection_info_struct *con_info;
       con_info = malloc (sizeof (struct connection_info_struct));
       if (NULL == con_info)
         return MHD_NO;
       con_info->answerstring = NULL;
       if (0 == strcmp (method, "POST")){
         con_info->postprocessor =
           MHD_create_post_processor (connection, POSTBUFFERSIZE,
                                     iterate_post, (void *) con_info);
         
         if (NULL == con_info->postprocessor){
           free (con_info);
           return MHD_NO;
         }
         con_info->connectiontype = POST;
       }
       else
         con_info->connectiontype = GET;
   
       *con_cls = (void *) con_info;
   
       return MHD_YES;
     }
   
     if (0 == strcmp(method, "GET") && 0 == strcmp(url, "/listar_dispositivos")){
       printf("Obteniendo lista de dispositivos conectados...\n");
       /*
         comunicarse con el daemon para traer la lista de dispositivos conectados...
         armar la respuesta y responer como JSON
       */
          int cliente;
          //Direccion del cliente
          struct sockaddr_in direccion_cliente;
          //ponemos en 0 la estructura direccion_cliente
          memset(&direccion_cliente, 0, sizeof(direccion_cliente));

          //llenamos los campos
          //IPv4
          direccion_cliente.sin_family = AF_INET;		
          //Convertimos el numero de puerto al endianness de la red
          direccion_cliente.sin_port = htons(4545);	
          //Nos tratamos de conectar a esta direccion
          direccion_cliente.sin_addr.s_addr = inet_addr("127.0.0.1");

          //Abrimos un socket
          cliente = socket(((struct sockaddr *)&direccion_cliente)->sa_family, SOCK_STREAM, 0);
          if (cliente == -1)
          {
            printf("Error al abrir el socket\n");
            return -1;
          }
          printf("Abierto el socket para el cliente...\n");

          //Conectamos
          int conectar = connect(cliente, (struct sockaddr *)&direccion_cliente, sizeof(direccion_cliente));
          if (conectar != 0)
          {
            printf("Error: No es posible conectar\n");
            return 1;
          }
          printf("conectado...\n");
          
          //Enviamos la ruta del archivo para que el servidor lo busque
          send(cliente, "listar_dispositivos", 512, 0);
          char* respuesta = (char *)malloc(1024*sizeof(char));
          //Leemos la respuesta del servidor
          recv(cliente, respuesta, 512, 0);
          close(cliente);

       return send_page (connection, respuesta);
     }
   
     else if (0 == strcmp(method, "POST")){
       struct connection_info_struct *con_info = *con_cls;
         
       if (*upload_data_size != 0){
         //printf("Uploaded data: %s\n",upload_data);
         MHD_post_process (con_info->postprocessor, upload_data,
                           *upload_data_size);
         *upload_data_size = 0;
   
         return MHD_YES;
       }
       else if (NULL != con_info->answerstring)
         return send_page (connection, "Json de respuesta a POST");
     }
     printf("Solicitud no se puede procesar...\n");
     return send_page (connection, "Metodo no existente");
   }
    
   int main (){
     struct MHD_Daemon *daemon;
   
     daemon = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                               &answer_to_connection, NULL,
                               MHD_OPTION_NOTIFY_COMPLETED, request_completed,
                               NULL, MHD_OPTION_END);
     if (NULL == daemon)
       return 1;
   
     (void) getchar ();
   
     MHD_stop_daemon (daemon);
   
     return 0;
   }