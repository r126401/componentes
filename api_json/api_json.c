/*
 * api_json.c
 *
 *  Created on: 21 sept. 2020
 *      Author: t126401
 */



#include "dialogos_json.h"
#include "api_json.h"
#include "cJSON.h"
#include "esp_err.h"
#include "configuracion.h"
#include "conexiones.h"
#include "datos_comunes.h"
#include "esp_wifi.h"
#include "programmer.h"
#include "conexiones_mqtt.h"
//#include "ota.h"
#include "esp_system.h"
#include "interfaz_usuario.h"
#include "alarmas.h"
#include "esp_timer.h"
#include "espota.h"

#define DELAY_TIME_RESET 3000 //ms o 3s

static const char *TAG = "API_JSON";
extern xQueueHandle cola_mqtt;
esp_err_t   comandoDesconocido(DATOS_APLICACION *datosApp, cJSON *respuesta) {


    codigoRespuesta(respuesta, RESP_NOK);


    return ESP_OK;
}



void   codigoRespuesta(cJSON *respuesta, enum CODIGO_RESPUESTA codigo) {

    cJSON_AddNumberToObject(respuesta, DLG_COD_RESPUESTA, codigo);
    return;
}



cJSON*  analizar_comando(DATOS_APLICACION *datosApp, char* info) {


	ESP_LOGI(TAG, ""TRAZAR"Entramos en analizar_comando", INFOTRAZA);
    cJSON *idComando = NULL;
    cJSON *peticion = NULL;
    cJSON *respuesta = NULL;
    enum COMANDOS com;
    //time_t t_siguiente_intervalo;
    //1.- Analizar la peticion y vemos que comando nos piden.


    peticion = cJSON_Parse(info);
    if (peticion == NULL) {
        com = NO_JSON;
        free(peticion);
    } else {
        idComando = cJSON_GetObjectItem(peticion, COMANDO);
        if (idComando == NULL) {
            com = COMANDO_DESCONOCIDO;
        } else {
            idComando = cJSON_GetObjectItem(idComando, DLG_COMANDO);
            if (idComando == NULL) {
                com = NO_IMPLEMENTADO;
            } else {
                com = idComando->valueint;
            }
        }

    }
    ESP_LOGW(TAG, ""TRAZAR"Memoria libre(3) Se ha creado el objeto peticion: %d\n", INFOTRAZA, esp_get_free_heap_size());
    respuesta = cabeceraRespuestaComando(datosApp, peticion);
    ESP_LOGW(TAG, ""TRAZAR"Memoria libre(4) Se ha creado la respuesta: %d\n", INFOTRAZA, esp_get_free_heap_size());
    if (com >= 50) {
        appUser_analizarComandoAplicacion(peticion, idComando->valueint, datosApp, respuesta);
        ESP_LOGW(TAG, ""TRAZAR"Memoria libre(5). Se aumentan los campos de la cabecera: %d\n", INFOTRAZA, esp_get_free_heap_size());
        printf("analizarComando-->comando de aplicacion ejecutado!!\n");
    } else {

        //4.- En funcion del comando ejecutamos una cosa u otra
        switch (com) {
            case CONSULTAR_CONF_APP: //ok--- con cabecera
            	visualizar_datos_aplicacion(datosApp, respuesta);
                //AppJsonComun(datosApp, respuesta);
                break;
            case CONSULTAR_CONF_MQTT: //probada ok--- con cabecera
                 //Visualizar la configuracion mqtt de aplicacion
            	visualizar_configuracion_mqtt(datosApp, respuesta);
            	break;

            case CONSULTAR_CONF_WIFI: // probado ok--- con cabecera
                //Visualizar la configuracion wifi
                visualizar_datos_wifi(datosApp, respuesta);
                break;
            case CONSULTAR_CONF_PROGRAMACION: // probado ok--- con cabecera
                visualizar_programas(datosApp, respuesta);
                break;
            case INSERTAR_PROGRAMACION: // probado ok-- con construirCabecera
            	insertar_nuevo_programa(peticion, datosApp, respuesta);
            	//calcular_programa_activo(datosApp, &t_siguiente_intervalo);
                break;
            case BORRAR_PROGRAMACION: // probado ok-- con construirCabecera
                 borrar_programa(peticion, datosApp, respuesta);
                 //calcular_programa_activo(datosApp, &t_siguiente_intervalo);
                 break;
            case MODIFICAR_PROGRAMACION: // probado ok-- con construirCabecera
            	modificar_programa(peticion, datosApp, respuesta);
            	//calcular_programa_activo(datosApp, &t_siguiente_intervalo);
                break;
            case EJECUTAR_RESET: // probado ok-- con cabecera
                ejecutar_reset(datosApp, respuesta);
                break;
            case EJECUTAR_FACTORY_RESET: // probado ok-- con cabecera
                ejecutar_factory_reset(datosApp, respuesta);
                break;
            case UPGRADE_FIRMWARE_OTA:
                upgrade_ota(peticion, datosApp, respuesta);
                break;
            case ALARMAS_ACTIVAS:
            	visualizar_alarmas_activas(datosApp, respuesta);
            	break;
            case MODIFICAR_CONF_APP:
            	modificar_configuracion_app(peticion, datosApp, respuesta);
                break;

            default:
            	visualizar_comando_desconocido(datosApp, respuesta);
            	break;
        }
    }




    cJSON_Delete(peticion);
    ESP_LOGW(TAG, ""TRAZAR"Memoria libre(6) Se libera el json de la peticion: %d\n", INFOTRAZA, esp_get_free_heap_size());
	return respuesta;
}


 void  mensaje_recibido(DATOS_APLICACION *datosApp) {


	 char* peticion = NULL;
	 cJSON *root = NULL;
	 char *respuesta = NULL;
	 COLA_MQTT cola;

	 ESP_LOGE(TAG, ""TRAZAR"Memoria libre(1): %d\n", INFOTRAZA, esp_get_free_heap_size());

	 if(datosApp->handle_mqtt->data_len == 0) {
		 ESP_LOGW(TAG, ""TRAZAR"MENSAJE VACIO", INFOTRAZA);
		 return;
	 }
	 peticion = (char*) calloc((datosApp->handle_mqtt->data_len + 1), sizeof(char));
	 strncpy(peticion,datosApp->handle_mqtt->data, datosApp->handle_mqtt->data_len);
	 ESP_LOGI(TAG, ""TRAZAR"Recibido %s, longitud %d", INFOTRAZA, peticion, datosApp->handle_mqtt->data_len);

	 root = analizar_comando(datosApp, peticion);
     if (root != NULL) {
         // 3.- Una vez ejecutado enviamos la respuesta y liberamos recursos.

    	 //publicar_mensaje_json(datosApp, root, NULL);
         // obtenemos el json de la respuesta para escribir
         respuesta = cJSON_Print(root);
         if (respuesta != NULL) {
        	 strcpy(cola.topic, datosApp->datosGenerales->parametrosMqtt.publish);
        	 strcpy(cola.buffer, respuesta);
             // preparamos los datos para el envio
              //publicar_mensaje(datosApp, respuesta);
              xQueueSend(cola_mqtt, &cola,0);

         }



     }
     free(respuesta);
	 free(peticion);
	 cJSON_Delete(root);
	 ESP_LOGE(TAG, ""TRAZAR"Memoria despues: %d\n", INFOTRAZA, esp_get_free_heap_size());
 }


 cJSON   *cabeceraRespuestaComando(DATOS_APLICACION *datosApp, cJSON *peticion) {

     cJSON* clave = NULL;
     cJSON *parteComando = NULL;
     cJSON *respuesta = NULL;
     cJSON *com = NULL;

     respuesta = cabeceraGeneral(datosApp);
     if (peticion != NULL) {
         parteComando = cJSON_GetObjectItem(peticion, COMANDO);
         if (parteComando != NULL) {
             clave = cJSON_GetObjectItem(parteComando, DLG_KEY);
             //cJSON_AddItemToObject(respuesta, DLG_KEY, clave);
             if (clave != NULL) cJSON_AddStringToObject(respuesta, DLG_KEY, clave->valuestring);
         } else {
             return respuesta;
         }
     }else {
         ESP_LOGE(TAG, ""TRAZAR"json invalido\n", INFOTRAZA);
         //printf("cabeceraRespuestaComando--> cabecera defecto_ %s\n", cJSON_Print(respuesta));
         return respuesta;
     }
     com = cJSON_GetObjectItem(parteComando, DLG_COMANDO);
     if (com != NULL) {
         cJSON_AddNumberToObject(respuesta, DLG_COMANDO, com->valueint);

     }
     return respuesta;
 }

 cJSON*  cabeceraGeneral(DATOS_APLICACION *datosApp) {

     char fecha[100];
     cJSON *respuesta = NULL;

     sprintf(fecha, "%02d/%02d/%d %02d:%02d:%02d",
        datosApp->datosGenerales->clock.date.tm_mday,datosApp->datosGenerales->clock.date.tm_mon+1, datosApp->datosGenerales->clock.date.tm_year+1900,
        datosApp->datosGenerales->clock.date.tm_hour, datosApp->datosGenerales->clock.date.tm_min, datosApp->datosGenerales->clock.date.tm_sec);

     respuesta = cJSON_CreateObject();
     cJSON_AddStringToObject(respuesta, ID_DEVICE, get_my_id());
     cJSON_AddNumberToObject(respuesta, DEVICE, datosApp->datosGenerales->tipoDispositivo);
     cJSON_AddStringToObject(respuesta, OTA_SW_VERSION, datosApp->datosGenerales->ota.swVersion->version);
     cJSON_AddStringToObject(respuesta, DATE, fecha);
     return respuesta;


 }


 int   extraer_dato_tm(char *id_programa, int desplazamiento, uint8_t tamano) {

 	char dato_temporal[5] = {0};
 	int dato;
 	memcpy(dato_temporal, id_programa+desplazamiento, tamano);
 	dato = atoi(dato_temporal);
 	ESP_LOGI(TAG, ""TRAZAR"El dato con desplazamiento %d es %d", INFOTRAZA, desplazamiento, dato);
 	return dato;
 }


 esp_err_t   extraer_dato_string(cJSON *nodo, char *nombre_campo, char* dato) {

     cJSON *campo = NULL;
     campo =cJSON_GetObjectItem(nodo, nombre_campo);
     if((campo != NULL) && (campo->type == cJSON_String)) {
     	strcpy(dato, campo->valuestring);
         ESP_LOGI(TAG, ""TRAZAR"extraerDatoString--> campo %s = %s\n", INFOTRAZA, nombre_campo, dato);
         return ESP_OK;
     } else {
     	ESP_LOGW(TAG, ""TRAZAR"No se ha podido extraer el dato %s que aparece de tipo %d", INFOTRAZA, nombre_campo, campo->type);
     	strcpy(dato, "NULL");
     	return ESP_FAIL;
     }
 }

 esp_err_t   extraer_dato_int(cJSON *nodo, char* nombre_campo, int *dato) {


     cJSON *campo = NULL;
     campo = cJSON_GetObjectItem(nodo, nombre_campo);
     if((campo != NULL) && (campo->type == cJSON_Number)) {
        *dato = campo->valueint;
        ESP_LOGI(TAG, ""TRAZAR"extraer_dato_int--> campo %s = %d\n", INFOTRAZA, nombre_campo, *dato);
        return ESP_OK;
     } else {
         ESP_LOGW(TAG, ""TRAZAR"extraer_dato_int-->campo %s no aparece\n", INFOTRAZA, nombre_campo);
         return ESP_FAIL;
     }

 }

 esp_err_t   extraer_dato_uint8(cJSON *nodo, char* nombre_campo, uint8_t *dato) {


     cJSON *campo = NULL;
     campo = cJSON_GetObjectItem(nodo, nombre_campo);
     if((campo != NULL) && ((campo->type == cJSON_Number) || (campo->type == cJSON_False) || (campo->type == cJSON_True))) {
        *dato = (uint8_t) campo->valueint;
        ESP_LOGI(TAG, ""TRAZAR"extraer_dato_int--> campo %s = %d\n", INFOTRAZA, nombre_campo, *dato);
        return ESP_OK;
     } else {
         ESP_LOGW(TAG, ""TRAZAR"extraer_dato_int-->campo %s no aparece\n", INFOTRAZA, nombre_campo);
         return ESP_FAIL;
     }

 }
 esp_err_t   extraer_dato_uint32(cJSON *nodo, char* nombre_campo, uint32_t *dato) {


     cJSON *campo = NULL;
     campo = cJSON_GetObjectItem(nodo, nombre_campo);
     if((campo != NULL) && (campo->type == cJSON_Number)) {
        *dato = (uint32_t) campo->valueint;
        ESP_LOGI(TAG, ""TRAZAR"extraer_dato_int--> campo %s = %d\n", INFOTRAZA, nombre_campo, *dato);
        return ESP_OK;
     } else {
         ESP_LOGW(TAG, ""TRAZAR"extraer_dato_int-->campo %s no aparece\n", INFOTRAZA, nombre_campo);
         return ESP_FAIL;
     }

 }


 esp_err_t 	extraer_dato_float(cJSON *nodo, char *nombre_campo, float *dato) {

	 double dato_double;

	 char cadena[10] = {0};

	 if ((extraer_dato_double(nodo, nombre_campo, &dato_double) == ESP_OK)) {
		 sprintf(cadena, "%02lf", dato_double);
		 *dato = atoff(cadena);
		 ESP_LOGI(TAG, ""TRAZAR"extraer_dato_float--> campo %s = %.02f\n", INFOTRAZA, nombre_campo, *dato);
		 return ESP_OK;

	 } else {
		 ESP_LOGW(TAG, ""TRAZAR"extraer_dato_float-->campo %s no aparece\n", INFOTRAZA, nombre_campo);
		 return ESP_FAIL;
	 }
 }

 esp_err_t   extraer_dato_double(cJSON *nodo, char *nombre_campo, double *dato) {


     cJSON *campo = NULL;
     campo =cJSON_GetObjectItem(nodo, nombre_campo);
     if((campo != NULL) && (campo->type == cJSON_Number)) {
         *dato = campo->valuedouble;
         ESP_LOGI(TAG, ""TRAZAR"extraer_dato_double--> campo %s = %f\n", INFOTRAZA, nombre_campo, *dato);
         return ESP_OK;
     } else {
     	ESP_LOGW(TAG, ""TRAZAR"extraer_dato_double-->campo %s no aparece\n", INFOTRAZA, nombre_campo);
     	return ESP_FAIL;
     }
 }




 uint32_t   hex2int(char *hex) {
     uint32_t val = 0;
     while (*hex) {
         // get current character then increment
         uint8_t byte = *hex++;
         // transform hex character to the 4bit equivalent number, using the ascii table indexes
         if (byte >= '0' && byte <= '9') byte = byte - '0';
         else if (byte >= 'a' && byte <='f') byte = byte - 'a' + 10;
         else if (byte >= 'A' && byte <='F') byte = byte - 'A' + 10;
         // shift 4 to make space for new digit, and add the 4 bits of the new digit
         val = (val << 4) | (byte & 0xF);
     }
     return val;
 }

 int   extraer_dato_hex(char* texto, int desplazamiento, uint8_t tamano) {

 	char dato_temporal[5] = {0};
 	int dato = 0;
 	ESP_LOGI(TAG, ""TRAZAR"dato temporal es %s", INFOTRAZA, dato_temporal);
 	memcpy(dato_temporal, texto+desplazamiento, tamano);
 	dato = hex2int(dato_temporal);
 	ESP_LOGI(TAG, ""TRAZAR"El dato con desplazamiento %d es %d", INFOTRAZA, desplazamiento, dato);

 	return dato;
 }
 esp_err_t   visualizar_datos_aplicacion(struct DATOS_APLICACION *datosApp, cJSON *respuesta) {



     cJSON_AddNumberToObject(respuesta, PROGRAMMER_STATE, datosApp->datosGenerales->estadoProgramacion);
     cJSON_AddNumberToObject(respuesta, NUMBER_PROGRAMS, datosApp->datosGenerales->nProgramacion);
     cJSON_AddStringToObject(respuesta, MQTT_BROKER, datosApp->datosGenerales->parametrosMqtt.broker);
     cJSON_AddNumberToObject(respuesta, MQTT_PORT, datosApp->datosGenerales->parametrosMqtt.port);
     cJSON_AddStringToObject(respuesta, MQTT_USER, datosApp->datosGenerales->parametrosMqtt.user);
     cJSON_AddStringToObject(respuesta, MQTT_PASS, datosApp->datosGenerales->parametrosMqtt.password);
     cJSON_AddStringToObject(respuesta, MQTT_PUBLISH, datosApp->datosGenerales->parametrosMqtt.publish);
     cJSON_AddStringToObject(respuesta, MQTT_SUBSCRIBE, datosApp->datosGenerales->parametrosMqtt.subscribe);
     cJSON_AddNumberToObject(respuesta, FREE_MEMORY, esp_get_free_heap_size());
     cJSON_AddNumberToObject(respuesta, "uptime", esp_timer_get_time());
     //cJSON_AddNumberToObject(respuesta, UPTIME, uptime);
     codigoRespuesta(respuesta, RESP_OK);
     printf("AppJsonComun--> vamos a imprimir\n");

     return ESP_OK;
 }

esp_err_t visualizar_datos_wifi(DATOS_APLICACION *datosApp, cJSON *respuesta) {

     wifi_config_t config;

     esp_wifi_get_config(WIFI_IF_STA, &config);
     cJSON_AddStringToObject(respuesta, CONF_WIFI_ESSID, (char*) config.sta.ssid);
     cJSON_AddStringToObject(respuesta, CONF_WIFI_PASS, (char*) config.sta.password);
     codigoRespuesta(respuesta, RESP_OK);
     return ESP_OK;


 }

esp_err_t   visualizar_programas(struct DATOS_APLICACION *datosApp, cJSON *respuesta) {

	cJSON *array = NULL;

    escribir_programa_actual(datosApp, respuesta);
    cJSON_AddNumberToObject(respuesta, DEVICE_STATE, datosApp->datosGenerales->estadoApp);
    cJSON_AddNumberToObject(respuesta, PROGRAMMER_STATE, datosApp->datosGenerales->estadoProgramacion);
	array = cJSON_CreateArray();

    crear_programas_json(datosApp, array);
    cJSON_AddItemToObject(respuesta, PROGRAM, array);
    codigoRespuesta(respuesta, RESP_OK);

    return ESP_OK;



}

esp_err_t   escribir_programa_actual(DATOS_APLICACION *datosApp, cJSON *respuesta) {

	NTP_CLOCK hora;
	actualizar_hora(&hora);
	time_t hora_actual = hora.time;
	TIME_PROGRAM *schedule;
	int indice;


	schedule = datosApp->datosGenerales->programacion;
	indice = datosApp->datosGenerales->nProgramaCandidato;
	if (datosApp->datosGenerales->nProgramacion > 0){
		if (datosApp->datosGenerales->nProgramaCandidato >= 0) {
			if ((schedule[indice].programa + schedule[indice].duracion) <= hora_actual) {
				ESP_LOGE(TAG, ""TRAZAR" no se pinta el programa actual ya que excedio la duracion del programa", INFOTRAZA);
			} else {
				cJSON_AddStringToObject(respuesta, CURRENT_PROGRAM_ID, datosApp->datosGenerales->programacion[datosApp->datosGenerales->nProgramaCandidato].idPrograma);
				ESP_LOGW(TAG, ""TRAZAR" programa candidato vale ok %d", INFOTRAZA, datosApp->datosGenerales->nProgramaCandidato);
			}
		} else {
			ESP_LOGW(TAG, ""TRAZAR" programa candidato vale %d", INFOTRAZA, datosApp->datosGenerales->nProgramaCandidato);
		}


	} else {
		ESP_LOGW(TAG, ""TRAZAR"NO SE PUEDE ESCRIBIR EL PROGRAMA ACTUAL PORQUE NO HAY PROGRAMAS REGISTRADOS O EL PROGRAMA VALE %d", INFOTRAZA, datosApp->datosGenerales->nProgramaCandidato );
	}

    return ESP_OK;
}

esp_err_t   visualizar_configuracion_mqtt(DATOS_APLICACION *datosApp, cJSON *respuesta) {

    cJSON_AddStringToObject(respuesta, MQTT_BROKER, datosApp->datosGenerales->parametrosMqtt.broker);
    cJSON_AddNumberToObject(respuesta, MQTT_PORT, datosApp->datosGenerales->parametrosMqtt.port);
    cJSON_AddStringToObject(respuesta, MQTT_USER, datosApp->datosGenerales->parametrosMqtt.user);
    cJSON_AddStringToObject(respuesta, MQTT_PASS, datosApp->datosGenerales->parametrosMqtt.password);
    cJSON_AddStringToObject(respuesta, MQTT_PUBLISH, datosApp->datosGenerales->parametrosMqtt.publish);
    cJSON_AddStringToObject(respuesta, MQTT_SUBSCRIBE, datosApp->datosGenerales->parametrosMqtt.subscribe);
    codigoRespuesta(respuesta, RESP_OK);

    return ESP_OK;


}
esp_err_t   visualizar_comando_desconocido(DATOS_APLICACION *datosApp, cJSON *respuesta) {


    codigoRespuesta(respuesta, RESP_NOK);


    return true;
}

esp_err_t   crear_programas_json(DATOS_APLICACION *datosApp, cJSON *respuesta) {

	cJSON *array;
	cJSON *item;
	int i;
    char dato[30];

    array = respuesta;


    for (i=0;i<datosApp->datosGenerales->nProgramacion;i++) {
        cJSON_AddItemToArray(array, item = cJSON_CreateObject());
        sprintf(dato, "%s%d%d",datosApp->datosGenerales->programacion[i].idPrograma,
                datosApp->datosGenerales->programacion[i].estadoPrograma,
                datosApp->datosGenerales->programacion[i].accion);

        cJSON_AddStringToObject(item, PROGRAM_ID, dato);
        appuser_visualizar_dato_programa(&datosApp->datosGenerales->programacion[i], item);
        /*
        if (datosApp->datosGenerales->programacion[i].duracion > 0) {
            cJSON_AddNumberToObject(item,DURATION_PROGRAM, datosApp->datosGenerales->programacion[i].duracion );
        }
        switch(datosApp->datosGenerales->tipoDispositivo) {

        case INTERRUPTOR:
        	break;
        case CRONOTERMOSTATO:
            cJSON_AddNumberToObject(item,UMBRAL_TEMPERATURA, datosApp->datosGenerales->programacion[i].temperatura );
        	break;
        case TERMOMETRO:
        	break;
        default:
        	break;
        }
        */

    }


        return ESP_OK;

}

esp_err_t   insertar_nuevo_programa(cJSON *peticion,DATOS_APLICACION *datosApp, cJSON *respuesta) {

   cJSON *nodo;
   TIME_PROGRAM *programaActual;
   uint8_t elements;



   elements= datosApp->datosGenerales->nProgramacion;

   printf("funcion insertarProgramacion\n");
   nodo = cJSON_GetObjectItem(peticion, PROGRAM);
   if(nodo == NULL) {
       printf("program no encontrado\n");
       codigoRespuesta(respuesta, RESP_NOK);
       return false;

   }

   //Revision de campos para crear la nueva programacion
   programaActual = (TIME_PROGRAM*) calloc(1, sizeof(TIME_PROGRAM));
   extraer_dato_int(nodo, PROGRAM_TYPE, (int*) &programaActual->tipo);
   extraer_dato_int(nodo, HOUR, &programaActual->programacion.tm_hour);
   extraer_dato_int(nodo, MINUTE, &programaActual->programacion.tm_min);
   extraer_dato_int(nodo, SECOND, &programaActual->programacion.tm_sec);
   extraer_dato_int(nodo, YEAR, &programaActual->programacion.tm_yday);
   extraer_dato_int(nodo, MONTH, &programaActual->programacion.tm_mon);
   extraer_dato_int(nodo, DAY, &programaActual->programacion.tm_mday);
   if(extraer_dato_int(nodo, WEEKDAY, &programaActual->programacion.tm_wday) != ESP_OK) {
	   programaActual->programacion.tm_wday = 0;
   }
   /*
   if (programaActual->programacion.tm_wday = extraer_dato_int(nodo, WEEKDAY, &) == -1000) {
       programaActual->programacion.tm_wday = 0;
   }*/
   extraer_dato_int(nodo, PROGRAM_STATE, (int*) &programaActual->estadoPrograma);
   extraer_dato_int(nodo, PROGRAM_STATE, (int*) &programaActual->activo);
   //datosApp->datosGenerales->estadoApp = NORMAL_AUTO;
   extraer_dato_int(nodo, PROGRAM_ACTION, &programaActual->accion);
   extraer_dato_int(nodo, PROGRAM_MASK, (int*)&programaActual->mascara);
   appuser_extraer_datos_programa(programaActual, nodo);
   generarIdPrograma(programaActual);
   datosApp->datosGenerales->programacion = crearPrograma(programaActual, datosApp->datosGenerales->programacion, &datosApp->datosGenerales->nProgramacion);


   printf("borrarProgramacion--> (1)\n");
   if (datosApp->datosGenerales->nProgramacion == elements) {
       codigoRespuesta(respuesta, RESP_NOK);
         printf("borrarProgramacion--> (1')\n");
       return false;


   } else {
       cJSON_AddStringToObject(respuesta, PROGRAM_ID, programaActual->idPrograma);
       cJSON_AddNumberToObject(respuesta, DEVICE_STATE, datosApp->datosGenerales->estadoApp);
       cJSON_AddNumberToObject(respuesta, PROGRAMMER_STATE, datosApp->datosGenerales->estadoProgramacion);
       appuser_notify_app_status(datosApp, NORMAL_SINCRONIZANDO);
       datosApp->datosGenerales->estadoApp = NORMAL_SINCRONIZANDO;
       gestion_programas(datosApp);
       escribir_programa_actual(datosApp, respuesta);
       codigoRespuesta(respuesta, RESP_OK);

   }
   guardar_programas(datosApp, CONFIG_CLAVE_PROGRAMACION);

   return ESP_OK;

}

esp_err_t   borrar_programa(cJSON *peticion,struct DATOS_APLICACION *datosApp, cJSON *respuesta) {

   cJSON *nodo, *campo;
   char idPrograma[20] = {0};
   int nPrograma;




   //Buscamos el nodo programa
   printf("borrarProgramacion-->borrar programa\n");
   nodo = cJSON_GetObjectItem(peticion, PROGRAM);
   if(nodo == NULL) {
       codigoRespuesta(respuesta, RESP_NOK);
       return false;
   }


   campo = cJSON_GetObjectItem(nodo, PROGRAM_ID);
   if((campo != NULL) && (campo->type == cJSON_String)) {
       printf("idPrograma vale %s\n", campo->valuestring);
       strcpy(idPrograma, campo->valuestring);
   }
   //Localizamos el programa
   nPrograma = localizarProgramaPorId(idPrograma, datosApp->datosGenerales->programacion, datosApp->datosGenerales->nProgramacion);


   if (nPrograma == -1) {

       codigoRespuesta(respuesta, RESP_NOK);
       return false;


   } else {

       //borramos el programa
       datosApp->datosGenerales->programacion = borrarPrograma(datosApp->datosGenerales->programacion, &datosApp->datosGenerales->nProgramacion, nPrograma);
       if (datosApp->datosGenerales->nProgramacion == 0) {
           //datosApp->datosGenerales->estadoApp = NORMAL_SIN_PROGRAMACION;
           appuser_notify_app_status(datosApp, NORMAL_SIN_PROGRAMACION);
       }
       cJSON_AddNumberToObject(respuesta, DEVICE_STATE, datosApp->datosGenerales->estadoApp);
       cJSON_AddNumberToObject(respuesta, PROGRAMMER_STATE, datosApp->datosGenerales->estadoProgramacion);
       printf("idprograma borrado:%s\n", idPrograma);
       cJSON_AddStringToObject(respuesta, PROGRAM_ID, idPrograma);
       datosApp->datosGenerales->estadoApp = NORMAL_SINCRONIZANDO;
       appuser_notify_app_status(datosApp, NORMAL_SINCRONIZANDO);
       gestion_programas(datosApp);
       escribir_programa_actual(datosApp, respuesta);
       codigoRespuesta(respuesta, RESP_OK);

   }

   guardar_programas(datosApp, CONFIG_CLAVE_PROGRAMACION);
   return ESP_OK;
}

esp_err_t   modificar_programa(cJSON *peticion,struct DATOS_APLICACION *datosApp, cJSON *respuesta) {


	cJSON *campo;
    cJSON *nodo;
    //TIME_PROGRAM programaActual;
    char idPrograma[19];
    int nPrograma;




    //Buscamos el nodo programa
    nodo = cJSON_GetObjectItem(peticion, PROGRAM);
    if(nodo == NULL) {
        printf("modificarProgramacion-->program no encontrado\n");
        codigoRespuesta(respuesta, RESP_NOK);
        return ESP_FAIL;
    }


    //extraer_dato_string(nodo, PROGRAM_ID, idPrograma);

    campo = cJSON_GetObjectItem(nodo, PROGRAM_ID);
    if((campo != NULL) && (campo->type == cJSON_String)) {
        printf("modificarProgramacion-->idPrograma vale %s\n", campo->valuestring);
        strcpy(idPrograma, campo->valuestring);
    }

    //Localizamos el programa
    nPrograma = localizarProgramaPorId(idPrograma, datosApp->datosGenerales->programacion, datosApp->datosGenerales->nProgramacion);


    if (nPrograma == -1) {
        codigoRespuesta(respuesta, RESP_NOK);
        return false;

    } else {
    	extraer_dato_int(nodo, YEAR, &datosApp->datosGenerales->programacion[nPrograma].programacion.tm_year);
    	extraer_dato_int(nodo, MONTH, &datosApp->datosGenerales->programacion[nPrograma].programacion.tm_mon);
    	extraer_dato_int(nodo, DAY, &datosApp->datosGenerales->programacion[nPrograma].programacion.tm_mday);
    	extraer_dato_int(nodo, HOUR, &datosApp->datosGenerales->programacion[nPrograma].programacion.tm_hour);
    	extraer_dato_int(nodo, MINUTE, &datosApp->datosGenerales->programacion[nPrograma].programacion.tm_min);
    	extraer_dato_int(nodo, SECOND, &datosApp->datosGenerales->programacion[nPrograma].programacion.tm_sec);
    	extraer_dato_int(nodo, WEEKDAY, &datosApp->datosGenerales->programacion[nPrograma].programacion.tm_wday);
    	extraer_dato_int(nodo, PROGRAM_TYPE, (int*) &datosApp->datosGenerales->programacion[nPrograma].tipo);
    	extraer_dato_int(nodo, PROGRAM_STATE, (int*) &datosApp->datosGenerales->programacion[nPrograma].estadoPrograma);
    	extraer_dato_int(nodo, PROGRAM_ACTION, &datosApp->datosGenerales->programacion[nPrograma].accion);
    	extraer_dato_uint8(nodo, PROGRAM_MASK, &datosApp->datosGenerales->programacion[nPrograma].mascara);
        appuser_modificar_dato_programa(&datosApp->datosGenerales->programacion[nPrograma], nodo);

        generarIdPrograma(&datosApp->datosGenerales->programacion[nPrograma]);
        cJSON_AddStringToObject(respuesta, PROGRAM_ID, idPrograma);
        cJSON_AddStringToObject(respuesta, NEW_PROGRAM_ID, datosApp->datosGenerales->programacion[nPrograma].idPrograma);
        cJSON_AddNumberToObject(respuesta, DEVICE_STATE, datosApp->datosGenerales->estadoApp);
        cJSON_AddNumberToObject(respuesta, PROGRAM_STATE, datosApp->datosGenerales->programacion[nPrograma].estadoPrograma);
        cJSON_AddNumberToObject(respuesta, PROGRAM_ACTION, datosApp->datosGenerales->programacion[nPrograma].accion);
        appuser_visualizar_dato_programa(&datosApp->datosGenerales->programacion[nPrograma], respuesta);

        datosApp->datosGenerales->estadoApp = NORMAL_SINCRONIZANDO;
        appuser_notify_app_status(datosApp, NORMAL_SINCRONIZANDO);
        gestion_programas(datosApp);
        escribir_programa_actual(datosApp, respuesta);
        codigoRespuesta(respuesta, RESP_OK);
    }

    guardar_programas(datosApp, CONFIG_CLAVE_PROGRAMACION);


    return ESP_OK;
}

cJSON*   cabecera_espontaneo(DATOS_APLICACION *datosApp, enum TIPO_INFORME tipo_report) {

    cJSON *respuesta;
    respuesta = cabeceraGeneral(datosApp);
    cJSON_AddNumberToObject(respuesta, TIPO_REPORT, tipo_report);
    return respuesta;

}

void restart_normal(void* parametros) {
    esp_restart();
}

esp_err_t   ejecutar_reset(DATOS_APLICACION *datosApp, cJSON *respuesta) {

    //static os_timer_t restart;
    esp_timer_handle_t tiempo;
    esp_timer_create_args_t temporizador;
    temporizador.callback = &restart_normal;
    temporizador.arg = NULL;
    temporizador.name = "restart";
    esp_timer_create(&temporizador, &tiempo);
    esp_timer_start_once(tiempo, 3000000);
    if (respuesta != NULL) {
    	codigoRespuesta(respuesta, RESP_OK);
    }


    return ESP_OK;
}

esp_err_t   ejecutar_factory_reset(DATOS_APLICACION *datosApp, cJSON *respuesta) {

    //Habria que eliminar la configuracion wifi y las etiquetas nvs
	reinicio_fabrica(datosApp);
    ejecutar_reset(datosApp, respuesta);
    return ESP_OK;
}

esp_err_t   upgrade_ota(cJSON *peticion, struct DATOS_APLICACION *datosApp, cJSON *respuesta) {


    cJSON *nodo;


   ESP_LOGI(TAG, ""TRAZAR"UPGRADE OTA...", INFOTRAZA);
   nodo = cJSON_GetObjectItem(peticion, UPGRADE_FIRMWARE);
   if(nodo == NULL) {
       codigoRespuesta(respuesta, RESP_NOK);
       return ESP_FAIL;
   }

   extraer_dato_string(nodo, OTA_SERVER, datosApp->datosGenerales->ota.server);
   extraer_dato_string(nodo, OTA_URL, datosApp->datosGenerales->ota.url);
   extraer_dato_string(nodo, OTA_FILE, datosApp->datosGenerales->ota.file);
   extraer_dato_string(nodo, OTA_SW_VERSION, datosApp->datosGenerales->ota.newVersion);
   extraer_dato_int(nodo, OTA_PORT, &datosApp->datosGenerales->ota.puerto);
   ESP_LOGI(TAG, ""TRAZAR"PUERTO: %d", INFOTRAZA, datosApp->datosGenerales->ota.puerto);



   if (datosApp->datosGenerales->estadoApp != UPGRADE_EN_PROGRESO) {
	   codigoRespuesta(respuesta, RESP_OK);
	   if (appuser_acciones_ota(datosApp) == RESP_RESTART) {
		   ejecutar_reset(datosApp, NULL);

	   }else {
		   //parar_gestion_programacion(datosApp);
		   tarea_upgrade_firmware(datosApp);
	   }


   } else {
	   codigoRespuesta(respuesta, RESP_NOK);
   }

   return ESP_OK;

}


esp_err_t   reinicio_fabrica(DATOS_APLICACION *datosApp) {

	wifi_config_t conf_wifi;
	memset(&conf_wifi, 0, sizeof(wifi_config_t));
	restaurar_wifi_fabrica();
	// Restear claves wifi
	esp_wifi_set_config(WIFI_IF_STA, &conf_wifi);
	//resetar nvs
	nvs_erase_all(datosApp->handle);
	//nvs_flash_erase();


	return ESP_OK;
}

esp_err_t notificar_evento_ota(DATOS_APLICACION *datosApp, ESTADO_OTA codeOta) {

    cJSON *respuesta = NULL;

    respuesta = cabecera_espontaneo(datosApp, UPGRADE_FIRMWARE_FOTA);
    cJSON_AddNumberToObject(respuesta, CODIGO_OTA, codeOta);
    publicar_mensaje_json(datosApp, respuesta, NULL);
    //notificarAccion(respuesta, datosApp);
    printf("notificarOta-->enviado\n");
    return ESP_OK;

}

esp_err_t notificar_evento_alarma(DATOS_APLICACION *datosApp, int tipo_alarma, char* mnemonico_alarma) {

	cJSON *respuesta;

	respuesta = cabecera_espontaneo(datosApp, INFORME_ALARMA);
	cJSON_AddNumberToObject(respuesta, mnemonico_alarma, datosApp->alarmas[tipo_alarma].estado_alarma);
	cJSON_AddNumberToObject(respuesta, FECHA_ALARMA, datosApp->alarmas[tipo_alarma].fecha_alarma);
	publicar_mensaje_json(datosApp, respuesta, NULL);
	return ESP_OK;

}

esp_err_t    visualizar_alarmas_activas(DATOS_APLICACION *datosApp, cJSON *respuesta) {


	cJSON_AddNumberToObject(respuesta, NOTIFICACION_ALARMA_WIFI, datosApp->alarmas[0].estado_alarma);
	cJSON_AddNumberToObject(respuesta, NOTIFICACION_ALARMA_MQTT, datosApp->alarmas[1].estado_alarma);
	cJSON_AddNumberToObject(respuesta, NOTIFICACION_ALARMA_NTP, datosApp->alarmas[2].estado_alarma);
	cJSON_AddNumberToObject(respuesta, NOTIFICACION_ALARMA_NVS, datosApp->alarmas[3].estado_alarma);
	codigoRespuesta(respuesta, RESP_OK);

	return ESP_OK;
}

esp_err_t   modificar_configuracion_app(cJSON *peticion,struct DATOS_APLICACION *datosApp, cJSON *respuesta) {

	return appuser_modificarConfApp(peticion, datosApp, respuesta);
}
