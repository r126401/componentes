/*
 * interfaz_usuario.c
 *
 *  Created on: 21 sept. 2020
 *      Author: t126401
 */

#include "dialogos_json.h"
#include "interfaz_usuario.h"
#include "configuracion_usuario.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "api_json.h"
#include "driver/gpio.h"
#include "conexiones_mqtt.h"
#include "programmer.h"
#include "conexiones.h"
#include "nvslib.h"
#include "alarmas.h"
#include "funciones_usuario.h"
#include "esp_timer.h"

#define CADENCIA_WIFI 250
#define CADENCIA_BROKER 300
#define CADENCIA_SMARTCONFIG 80
#define CADENCIA_SNTP 1000
#define CADENCIA_ALARMA 1500



static const char *TAG = "INTERFAZ_USUARIO";


//static os_timer_t temporizador_general;
static esp_timer_handle_t temporizador_nuevo = NULL;




void parapadeo_led() {

	static bool luz = false;

	if (luz == false) {
		gpio_set_level(CONFIG_GPIO_PIN_LED, ON);
		luz = true;
	} else {
		gpio_set_level(CONFIG_GPIO_PIN_LED, OFF);
		luz = false;
	}


}

void eliminar_temporizacion(char* nombre) {

	if (temporizador_nuevo != NULL) {
		ESP_LOGW(TAG, ""TRAZAR" elminando temporizador %s ", INFOTRAZA, nombre);
		esp_timer_stop(temporizador_nuevo);
		esp_timer_delete(temporizador_nuevo);
		temporizador_nuevo = NULL;
	}

}

void aplicar_temporizacion(int cadencia, esp_timer_cb_t funcion, char* nombre) {

	esp_timer_create_args_t temporizador;

	ESP_LOGW(TAG, ""TRAZAR" aplicando temporizador %s ", INFOTRAZA, nombre);
	eliminar_temporizacion(nombre);
	temporizador.arg = NULL;
	temporizador.callback = funcion;
	temporizador.name = nombre;

	esp_timer_create(&temporizador, &temporizador_nuevo);
	esp_timer_start_periodic(temporizador_nuevo, cadencia * 1000);


}

esp_err_t appuser_configuracion_defecto(DATOS_APLICACION *datosApp) {

	ESP_LOGI(TAG, ""TRAZAR"Ejecutando configuraciones adicionales de la aplicacion por defecto...", INFOTRAZA);
	datosApp->datosGenerales->tipoDispositivo = INTERRUPTOR;
    //Escribe aqui el codigo de inicializacion por defecto de la aplicacion.
	// Esta funcion es llamada desde el componente configuracion defaultConfig.
	// Aqui puedes establecer los valores por defecto para tu aplicacion.

	return ESP_OK;
}




esp_err_t appUser_ejecutar_accion_programa_defecto(DATOS_APLICACION *datosApp) {

	ESP_LOGI(TAG, ""TRAZAR"Ejecutando acciones de programa por defecto", INFOTRAZA);
	//Escribe aquí lo que quieres que tu aplicacion haga tenga que ejecutar una accion por defecto
    //GPIO_OUTPUT_SET(GPIO_ID_PIN(GPIO_RELE), OFF);

    return ESP_OK;
}


esp_err_t appuser_notificar_smartconfig() {


	aplicar_temporizacion(CADENCIA_SMARTCONFIG, parapadeo_led, "smartconfig");

	return ESP_OK;
}

esp_err_t appuser_arranque_aplicacion(DATOS_APLICACION *datosApp) {

	cJSON *informe;

	informe = appuser_generar_informe_espontaneo(datosApp, ARRANQUE_APLICACION, NULL);

	ESP_LOGI(TAG, ""TRAZAR" vamos a publicar el arranque del dispositivo", INFOTRAZA);
	if (informe != NULL) {
		publicar_mensaje_json(datosApp, informe, NULL);
		ESP_LOGI(TAG, ""TRAZAR" PUBLICADO", INFOTRAZA);
	}
	datosApp->datosGenerales->estadoApp = NORMAL_AUTO;

	return ESP_OK;
}

esp_err_t appuser_acciones_ota(DATOS_APLICACION *datosApp) {


	//datosApp->datosGenerales->ota.puerto = 80;
	ESP_LOGI(TAG, ""TRAZAR"PUERTO: %d", INFOTRAZA, datosApp->datosGenerales->ota.puerto);
	//ESP_LOGI(TAG, ""TRAZAR"PUERTO OTRA VEZ: %d", datosApp->datosGenerales->ota.puerto);
	ESP_LOGI(TAG, ""TRAZAR"servidor ota: %s\n, puerto: %d\n, url: %s, version %d", INFOTRAZA,
			datosApp->datosGenerales->ota.server, datosApp->datosGenerales->ota.puerto, datosApp->datosGenerales->ota.url, datosApp->datosGenerales->ota.swVersion);


	return ESP_OK;
}

esp_err_t appuser_obteniendo_sntp() {

	aplicar_temporizacion(CADENCIA_SNTP, parapadeo_led, "sntp");
	return ESP_OK;
}
esp_err_t app_user_error_obteniendo_sntp() {

	aplicar_temporizacion(CADENCIA_SNTP, parapadeo_led, "sntp_error");
	return ESP_OK;
}

esp_err_t appuser_sntp_ok() {

	eliminar_temporizacion("sntp");
	gpio_set_level(CONFIG_GPIO_PIN_LED, ON);
	return ESP_OK;
}



esp_err_t appuser_wifi_conectando() {


	aplicar_temporizacion(CADENCIA_WIFI, parapadeo_led, "wifi");


	return ESP_OK;
}

esp_err_t appuser_wifi_conectado() {


	eliminar_temporizacion("wifi");
	//os_timer_disarm(&temporizador_general);
	gpio_set_level(CONFIG_GPIO_PIN_LED, ON);


	return ESP_OK;
}

esp_err_t appuser_broker_conectando() {

	ESP_LOGI(TAG, ""TRAZAR"BROKER CONECTANDO", INFOTRAZA);
	aplicar_temporizacion(CADENCIA_BROKER, parapadeo_led, "mqtt");
	return ESP_OK;
}
esp_err_t appuser_broker_conectado() {

	eliminar_temporizacion("mqtt");
	gpio_set_level(CONFIG_GPIO_PIN_LED, ON);

	return ESP_OK;
}
esp_err_t appuser_broker_desconectado() {

	aplicar_temporizacion(CADENCIA_BROKER, parapadeo_led, "mqtt_error");
	return ESP_OK;
}





void appuser_ejecucion_accion_temporizada(void *datosApp) {

    cJSON * respuesta = NULL;
    ESP_LOGI(TAG, ""TRAZAR"FIN DE LA TEMPORIZACION. SE APAGA EL RELE", INFOTRAZA);
    operacion_rele(datosApp, TEMPORIZADA, OFF);
    respuesta = appuser_generar_informe_espontaneo(datosApp, RELE_TEMPORIZADO, NULL);
    if (respuesta != NULL) {
    	publicar_mensaje_json(datosApp, respuesta, NULL);
    }
    ESP_LOGI(TAG, ""TRAZAR"FIN DE LA TEMPORIZACION. RELE APAGADO", INFOTRAZA);
}

esp_err_t appuser_temporizador_cumplido(DATOS_APLICACION *datosApp) {

	cJSON *respuesta;
	ESP_LOGI(TAG, ""TRAZAR"ejecutamos la accion del programa de aplicacion", INFOTRAZA);
	operacion_rele(datosApp, TEMPORIZADA, datosApp->datosGenerales->programacion[datosApp->datosGenerales->nProgramaCandidato].accion);
	respuesta = appuser_generar_informe_espontaneo(datosApp, CAMBIO_DE_PROGRAMA, NULL);
	if (respuesta != NULL) {
		publicar_mensaje_json(datosApp, respuesta, NULL);
	}

	return ESP_OK;
}

esp_err_t appuser_notificar_alarma_localmente(DATOS_APLICACION *datosApp, uint8_t indice) {

	if (datosApp->alarmas[indice].estado_alarma == ALARMA_OFF) {
		//eliminar_temporizacion();
		//os_timer_disarm(&temporizador_general);
		ESP_LOGI(TAG, ""TRAZAR" ALARMA OFF NOTIFICADA", INFOTRAZA);
	} else {
		//aplicar_temporizacion(CADENCIA_ALARMA, parapadeo_led);
		/*
		os_timer_disarm(&temporizador_general);
		os_timer_setfn(&temporizador_general, (os_timer_func_t*) parapadeo_led, NULL);
		os_timer_arm(&temporizador_general, CADENCIA_ALARMA, true);
		*/
		ESP_LOGI(TAG, ""TRAZAR" ALARMA ON NOTIFICADA", INFOTRAZA);

	}


	return ESP_OK;
}

cJSON* appuser_generar_informe_espontaneo(DATOS_APLICACION *datosApp, enum TIPO_INFORME tipoInforme, cJSON *comandoOriginal) {


    cJSON *respuesta = NULL;


    respuesta = cabecera_espontaneo(datosApp, tipoInforme);
    switch(tipoInforme) {
        case ARRANQUE_APLICACION:
            printf("generarReporte--> enviando arranqueAplicacion");
            cJSON_AddNumberToObject(respuesta, APP_COMAND_ESTADO_RELE, gpio_get_level(CONFIG_GPIO_PIN_RELE));
            cJSON_AddNumberToObject(respuesta, PROGRAMMER_STATE, datosApp->datosGenerales->estadoProgramacion);
            cJSON_AddNumberToObject(respuesta, DEVICE_STATE, datosApp->datosGenerales->estadoApp);
            escribir_programa_actual(datosApp, respuesta);
            codigoRespuesta(respuesta,RESP_OK);
            break;
        case ACTUACION_RELE_LOCAL:
            cJSON_AddNumberToObject(respuesta, APP_COMAND_ESTADO_RELE, gpio_get_level(CONFIG_GPIO_PIN_RELE));
            cJSON_AddNumberToObject(respuesta, PROGRAMMER_STATE, datosApp->datosGenerales->estadoProgramacion);
            cJSON_AddNumberToObject(respuesta, DEVICE_STATE, datosApp->datosGenerales->estadoApp);
            escribir_programa_actual(datosApp, respuesta);
            codigoRespuesta(respuesta,RESP_OK);

            break;
        case CAMBIO_DE_PROGRAMA:
            cJSON_AddNumberToObject(respuesta, PROGRAMMER_STATE, datosApp->datosGenerales->estadoProgramacion);
            cJSON_AddNumberToObject(respuesta, APP_COMAND_ESTADO_RELE, gpio_get_level(CONFIG_GPIO_PIN_RELE));
            cJSON_AddNumberToObject(respuesta, DEVICE_STATE, datosApp->datosGenerales->estadoApp);
            escribir_programa_actual(datosApp, respuesta);
            codigoRespuesta(respuesta,RESP_OK);
            break;
        case RELE_TEMPORIZADO:
            cJSON_AddNumberToObject(respuesta, PROGRAMMER_STATE, datosApp->datosGenerales->estadoProgramacion);
            cJSON_AddNumberToObject(respuesta, APP_COMAND_ESTADO_RELE, gpio_get_level(CONFIG_GPIO_PIN_RELE));
            cJSON_AddNumberToObject(respuesta, DEVICE_STATE, datosApp->datosGenerales->estadoApp);
            escribir_programa_actual(datosApp, respuesta);
            codigoRespuesta(respuesta,RESP_OK);
            break;
        default:
            codigoRespuesta(respuesta, RESP_NOK);
            printf("enviarReporte--> Salida no prevista\n");
            break;
    }

    return respuesta;

}
esp_err_t appuser_cargar_programa_especifico(DATOS_APLICACION *datosApp, TIME_PROGRAM *programa_actual, cJSON *nodo) {

	cJSON *item;
	char* dato;


	item = cJSON_GetObjectItem(nodo, PROGRAM_ID);
	dato = cJSON_GetStringValue(item);
	if (item != NULL) {
		switch(programa_actual->tipo) {
		case DIARIA:
			programa_actual->accion = extraer_dato_tm(dato, 11, 1);
			break;
		case SEMANAL:
			break;
		case FECHADA:
			programa_actual->accion = extraer_dato_tm(dato, 17, 1);
			break;
		}


	}

	item = cJSON_GetObjectItem(nodo, DURATION_PROGRAM);
	if (item != NULL) {
		programa_actual->duracion = item->valueint;
		ESP_LOGI(TAG, ""TRAZAR"DURACION = %d", INFOTRAZA, programa_actual->duracion);
	} else {
		programa_actual->duracion = 0;
		ESP_LOGI(TAG, ""TRAZAR"NO SE GUARDA DURACION: %d", INFOTRAZA, programa_actual->duracion);
	}




	return ESP_OK;

}

esp_err_t appuser_configuracion_a_json(DATOS_APLICACION *datosApp, cJSON *conf) {

	//cJSON_AddNumberToObject(conf, DEVICE , INTERRUPTOR);
	ESP_LOGI(TAG, ""TRAZAR" CONFIGURACION A JSON DEL DISPOSITIVO...", INFOTRAZA);
	return ESP_OK;
}

esp_err_t appuser_json_a_datos(DATOS_APLICACION *datosApp, cJSON *datos) {

	return ESP_OK;
}

esp_err_t appuser_cargar_programas_defecto(DATOS_APLICACION *datosApp, cJSON *array) {

	cJSON *item = NULL;

	cJSON_AddItemToArray(array, item = cJSON_CreateObject());
	cJSON_AddStringToObject(item, PROGRAM_ID, "000000006700");
	cJSON_AddNumberToObject(item, DURATION_PROGRAM, 420);
	cJSON_AddItemToArray(array, item = cJSON_CreateObject());
	cJSON_AddStringToObject(item, PROGRAM_ID, "001200007f10");
	cJSON_AddItemToArray(array, item = cJSON_CreateObject());
	cJSON_AddStringToObject(item, PROGRAM_ID, "001555007f11");
	cJSON_AddItemToArray(array, item = cJSON_CreateObject());
	cJSON_AddStringToObject(item, PROGRAM_ID, "001038007f11");
	cJSON_AddItemToArray(array, item = cJSON_CreateObject());
	cJSON_AddStringToObject(item, PROGRAM_ID, "021715000120081611");

	return ESP_OK;
}

esp_err_t appuser_extraer_datos_programa(TIME_PROGRAM *programa_actual, cJSON *nodo) {


    if(extraer_dato_int(nodo, DURATION_PROGRAM, (int*) &programa_actual->duracion) != ESP_OK) {
    	programa_actual->duracion = 0;
    }


	return ESP_OK;
}

esp_err_t appuser_modificar_dato_programa(TIME_PROGRAM *programa_actual,cJSON *nodo) {

	extraer_dato_uint32(nodo, DURATION_PROGRAM, &programa_actual->duracion);


	return ESP_OK;
}

esp_err_t appuser_visualizar_dato_programa(TIME_PROGRAM *programa_actual, cJSON *nodo) {

    if (programa_actual->duracion > 0) {
        cJSON_AddNumberToObject(nodo, DURATION_PROGRAM, programa_actual->duracion);

    }

	return ESP_OK;
}

esp_err_t appuser_modificarConfApp(cJSON *root, DATOS_APLICACION *datosApp, cJSON *respuesta) {

	return ESP_OK;

}

esp_err_t app_user_mensaje_recibido_especifico(DATOS_APLICACION *datosApp) {

	ESP_LOGI(TAG, ""TRAZAR" mensaje del topic: %s", INFOTRAZA, datosApp->handle_mqtt->topic);
	return ESP_OK;
}



