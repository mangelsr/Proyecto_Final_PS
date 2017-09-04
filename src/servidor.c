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
       return send_page (connection, "json de respuesta a GET");
     }
   
     else if (0 == strcmp(method, "POST")){
       struct connection_info_struct *con_info = *con_cls;
         
       if (*upload_data_size != 0){
         printf("Uploaded data: %s\n",upload_data);
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