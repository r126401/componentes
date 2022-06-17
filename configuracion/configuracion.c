/*
 * configuracion.c
 *
 *  Created on: 10 sept. 2020
 *      Author: t126401
 */


#include "dialogos_json.h"
#include "interfaz_usuario.h"
#include "esp_log.h"
#include "esp_err.h"
#include "configuracion_usuario.h"
#include "conexiones.h"
#include "cJSON.h"
#include "nvs_flash.h"
#include "nvslib.h"
#include "esp_wifi.h"
#include "configuracion.h"
#include "programmer.h"
#include "errores_proyecto.h"
#include "api_json.h"
#include "alarmas.h"
#include "esp_app_format.h"
#include "esp_ota_ops.h"

static const char *TAG = "CONFIGURACION";



esp_err_t crear_programas_defecto(DATOS_APLICACION *datosApp) {

	cJSON *array;
	char* texto = NULL;
	int longitud;
	ESP_LOGI(TAG, ""TRAZAR"CREAMOS EL ARRAY DE EJEMPLO", INFOTRAZA);
	array = cJSON_CreateArray();
	//cJSON_AddItemToArray(array, item = cJSON_CreateObject());
	//cJSON_AddStringToObject(item, PROGRAM_ID, "002359007f10");
	appuser_cargar_programas_defecto(datosApp, array);

	texto = cJSON_Print(array);
	if (texto != NULL) {
		longitud = strlen(texto);
		ESP_LOGI(TAG, ""TRAZAR"%s\n longitud %d",INFOTRAZA, texto, longitud);
		escribir_dato_string_nvs(&datosApp->handle, CONFIG_CLAVE_PROGRAMACION, texto);
		cJSON_Delete(array);
		free(texto);

	} else {
		ESP_LOGW(TAG, ""TRAZAR" NO HAY PROGRAMAS", INFOTRAZA);
	}

	return ESP_OK;
}


esp_err_t configuracion_a_json(DATOS_APLICACION *datosApp, cJSON *conf) {




	ESP_LOGI(TAG, ""TRAZAR"(COMENZAMOS)", INFOTRAZA);
	cJSON_AddNumberToObject(conf, DEVICE , datosApp->datosGenerales->tipoDispositivo);
	cJSON_AddStringToObject(conf, MQTT_BROKER, datosApp->datosGenerales->parametrosMqtt.broker);
	cJSON_AddNumberToObject(conf, MQTT_PORT, datosApp->datosGenerales->parametrosMqtt.port);
	cJSON_AddStringToObject(conf, MQTT_USER, datosApp->datosGenerales->parametrosMqtt.user);
	cJSON_AddStringToObject(conf, MQTT_PASS, datosApp->datosGenerales->parametrosMqtt.password);
	cJSON_AddStringToObject(conf, MQTT_SUBSCRIBE, datosApp->datosGenerales->parametrosMqtt.subscribe);
	cJSON_AddStringToObject(conf, MQTT_PUBLISH, datosApp->datosGenerales->parametrosMqtt.publish);
	cJSON_AddNumberToObject(conf, MQTT_QOS , datosApp->datosGenerales->parametrosMqtt.qos);
	cJSON_AddNumberToObject(conf, PROGRAM_STATE, datosApp->datosGenerales->estadoProgramacion);
	cJSON_AddStringToObject(conf, OTA_SW_VERSION, datosApp->datosGenerales->ota.swVersion->version);
	cJSON_AddBoolToObject(conf, MQTT_TLS, datosApp->datosGenerales->parametrosMqtt.tls);
	//cJSON_AddStringToObject(conf, MQTT_CERT_TLS, datosApp->datosGenerales->parametrosMqtt.cert);
    appuser_configuracion_a_json(datosApp, conf);

    	ESP_LOGI(TAG, ""TRAZAR"JSON creado:", INFOTRAZA);
    	return JSON_OK;


}

/**
 * se guardaran las configuraciones en las claves que se indiquen
 */
esp_err_t guardar_configuracion(DATOS_APLICACION *datosApp, char * clave, char* configuracion) {

	esp_err_t error;

	error = escribir_dato_string_nvs(&datosApp->handle, clave, configuracion);
	if (error != ESP_OK) {
		ESP_LOGE(TAG, ""TRAZAR"Error %d al escribir el dato con clave %s", INFOTRAZA, error, clave);
	} else {
		ESP_LOGI(TAG, ""TRAZAR"Configuracion guardada", INFOTRAZA);
	}

	return error;
}

/**
 * Esta funcion cargara valores de fabrica en la aplicacion.
 */
esp_err_t cargar_configuracion_defecto(DATOS_APLICACION *datosApp) {


	esp_err_t error;
	ESP_LOGI(TAG, ""TRAZAR"Se cargan parametros comunes de defecto", INFOTRAZA);
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    strcpy(datosApp->datosGenerales->parametrosMqtt.broker, CONFIG_BROKER_DEFECTO);
    datosApp->datosGenerales->tipoDispositivo = CONFIG_TIPO_DISPOSITIVO;
    datosApp->datosGenerales->parametrosMqtt.port = CONFIG_PUERTO_DEFECTO;
    strcpy(datosApp->datosGenerales->parametrosMqtt.user, CONFIG_USUARIO_BROKER_DEFECTO);
    strcpy(datosApp->datosGenerales->parametrosMqtt.password, CONFIG_PASSWORD_BROKER_DEFECTO);
    strcpy(datosApp->datosGenerales->parametrosMqtt.publish, "/pub_");
    strcat(datosApp->datosGenerales->parametrosMqtt.publish, get_my_id());
    strcpy(datosApp->datosGenerales->parametrosMqtt.subscribe, "/sub_");
    strcat(datosApp->datosGenerales->parametrosMqtt.subscribe, get_my_id());
    datosApp->datosGenerales->parametrosMqtt.qos = 0;
    datosApp->datosGenerales->parametrosMqtt.tls = CONFIG_MQTT_TLS;
    //strcpy(datosApp->datosGenerales->parametrosMqtt.cert, (const char *) mqtt_jajica_pem_start);
    //datosApp->datosGenerales->parametrosMqtt.cert = (const char *) mqtt_jajica_pem_start;
    ESP_LOGI(TAG, ""TRAZAR"PARAMETROS CARGADOS EN DATOSAPP", INFOTRAZA);
    datosApp->datosGenerales->estadoApp = NORMAL_ARRANCANDO;
    datosApp->datosGenerales->estadoProgramacion = INVALID_PROG;
    datosApp->datosGenerales->nProgramacion=0;
    datosApp->datosGenerales->nProgramaCandidato = -1;
    datosApp->datosGenerales->programacion = NULL;
    appuser_configuracion_defecto(datosApp);
	ESP_LOGI(TAG, ""TRAZAR" SALVAMOS LA CONFIGURACION GENERAL A NVS...", INFOTRAZA);
	error = salvar_configuracion_general(datosApp);
	if (error != ESP_OK) {
		ESP_LOGE(TAG, ""TRAZAR"ERROR AL SALVAR LA CONFIGURACION", INFOTRAZA);
		return ERROR;
	}
/*
    ESP_LOGI(TAG, ""TRAZAR"fin de Carga de configuracion de defecto...", INFOTRAZA);
    if ((error = configuracion_a_json(datosApp, configuracion) != JSON_OK)) {
    	ESP_LOGE(TAG, ""TRAZAR"Error al crear el json de configuracion", INFOTRAZA);
    	return error;
    }
    */
    crear_programas_defecto(datosApp);
	return ESP_OK;

}

esp_err_t json_a_datos_aplicacion(DATOS_APLICACION *datosApp, char *datos) {

	cJSON *nodo;

	nodo = cJSON_Parse(datos);
	if (nodo != NULL) {

		ESP_LOGI(TAG, ""TRAZAR"La configuracion es :%s", INFOTRAZA, datos);
		extraer_dato_string(nodo, MQTT_BROKER, datosApp->datosGenerales->parametrosMqtt.broker);
		extraer_dato_int(nodo, MQTT_PORT, &datosApp->datosGenerales->parametrosMqtt.port);
		extraer_dato_string(nodo, MQTT_USER, datosApp->datosGenerales->parametrosMqtt.user);
		extraer_dato_string(nodo, MQTT_PASS, datosApp->datosGenerales->parametrosMqtt.password);
		extraer_dato_string(nodo, MQTT_PUBLISH, datosApp->datosGenerales->parametrosMqtt.publish);
		extraer_dato_string(nodo, MQTT_SUBSCRIBE, datosApp->datosGenerales->parametrosMqtt.subscribe);
		extraer_dato_int(nodo, MQTT_QOS, &datosApp->datosGenerales->parametrosMqtt.qos);
		extraer_dato_int(nodo, PROGRAM_STATE, (int*) &datosApp->datosGenerales->estadoProgramacion);
		extraer_dato_int(nodo, DEVICE, &datosApp->datosGenerales->tipoDispositivo );
		extraer_dato_uint8(nodo, MQTT_TLS, (uint8_t*) &datosApp->datosGenerales->parametrosMqtt.tls);
		//extraer_dato_string(nodo, MQTT_CERT_TLS, datosApp->datosGenerales->parametrosMqtt.cert);
		/*
		extraer_dato_int(nodo, OTA_SW_VERSION, &version);
		if (version <= datosApp->datosGenerales->ota.swVersion) {
			datosApp->datosGenerales->ota.swVersion = datosApp->datosGenerales->ota.swVersion;
			ESP_LOGI(TAG, ""TRAZAR"Se indica version de compilacion %d", INFOTRAZA, datosApp->datosGenerales->ota.swVersion);
		} else {
			datosApp->datosGenerales->ota.swVersion = version;
			ESP_LOGI(TAG, ""TRAZAR"Se indica version OTA %d", INFOTRAZA, datosApp->datosGenerales->ota.swVersion);

		}*/
		appuser_json_a_datos(datosApp, nodo);
	} else {
		ESP_LOGE(TAG, ""TRAZAR"Error al extraer los datos del json", INFOTRAZA);
		return ESP_FAIL;
	}


	cJSON_Delete(nodo);
	ESP_LOGI(TAG, ""TRAZAR"Datos cargados en la estructura datosApp", INFOTRAZA);
	return ESP_OK;


}



/**
 * Se cargaran las configuraciones que se indiquen
 */
esp_err_t leer_configuracion(DATOS_APLICACION *datosApp, char* clave, char* valor) {

	size_t longitud = CONFIG_TAMANO_BUFFER_LECTURA;
	esp_err_t error;
	ESP_LOGI(TAG, ""TRAZAR"Se lee la configuracion con clave %s", INFOTRAZA, clave);
	if ((error = leer_dato_string_nvs(&datosApp->handle, clave, valor, longitud)) != ESP_OK) {
		ESP_LOGE(TAG, ""TRAZAR"Error al leer la configuracion con clave %s", INFOTRAZA, clave);
	}

	return error;
}




/**
 * @brief Rutina principal que trata la configuracion de arranque del dispositivo.
 *
 */
esp_err_t inicializacion(DATOS_APLICACION *datosApp, bool forzado) {


	esp_err_t error;
	char datos[CONFIG_TAMANO_BUFFER_LECTURA];
    datosApp->datosGenerales->ota.swVersion = esp_ota_get_app_description();
    //free(aplicacion);
    ESP_LOGE(TAG, ""TRAZAR" VERSION DE LA APLICACION (1)", INFOTRAZA);
    ESP_LOGE(TAG, ""TRAZAR" VERSION DE LA APLICACION %s", INFOTRAZA, datosApp->datosGenerales->ota.swVersion->version);

	inicializacion_registros_alarmas(datosApp);
	datosApp->datosGenerales->estadoApp = NORMAL_ARRANCANDO;
	inicializar_parametros_ntp(&datosApp->datosGenerales->clock);
	appuser_obteniendo_sntp(datosApp);
	ESP_LOGW(TAG, ""TRAZAR"(1)", INFOTRAZA);
	error = obtener_fecha_hora(&datosApp->datosGenerales->clock);
	ESP_LOGW(TAG, ""TRAZAR"(2)", INFOTRAZA);

	if (error != ESP_OK) {
		ESP_LOGW(TAG, ""TRAZAR"NO SE HA PODIDO OBTENER LA HORA DEL SERVIDOR NTP. NO HABRA PROGRAMACION. error: %d", INFOTRAZA, error);
		datosApp->datosGenerales->estadoProgramacion = INVALID_PROG;
		app_user_error_obteniendo_sntp(datosApp);
		registrar_alarma(datosApp, NOTIFICACION_ALARMA_NTP, ALARMA_NTP, ALARMA_OFF, false);

	} else {
		ESP_LOGI(TAG, ""TRAZAR" VAMOS A REGISTRAR ALARMA", INFOTRAZA);
		registrar_alarma(datosApp, NOTIFICACION_ALARMA_NTP, ALARMA_NTP, ALARMA_OFF, false);
		appuser_sntp_ok(datosApp);
		actualizar_hora(&datosApp->datosGenerales->clock);
		//ESP_LOGI(TAG, ""TRAZAR"Hora inicializada:%s", INFOTRAZA, pintar_fecha(datosApp->datosGenerales->clock.date);
		ESP_LOGI(TAG, ""TRAZAR"Hora inicializada:%s", INFOTRAZA,pintar_fecha(datosApp->datosGenerales->clock.date));
	    datosApp->datosGenerales->estadoProgramacion = VALID_PROG;


	}

	if ((error = inicializar_nvs(CONFIG_NAMESPACE, &datosApp->handle))!= ESP_OK) {
		ESP_LOGE(TAG, ""TRAZAR"ERROR POR FALLO NVS %d", INFOTRAZA, error);
		registrar_alarma(datosApp, NOTIFICACION_ALARMA_NVS, ALARMA_NVS, ALARMA_ON, true);
		return error;

	}
	if (forzado) {
		error = cargar_configuracion_defecto(datosApp);
		ESP_LOGI(TAG, ""TRAZAR" Cargada configuracion de defecto", INFOTRAZA);
		salvar_configuracion_general(datosApp);
	} else {
		// leemos la configuracion general desde nvs
		if ((error = leer_configuracion(datosApp, CONFIG_CLAVE_CONFIGURACION_GENERAL, datos)) == ESP_OK) {
			ESP_LOGI(TAG, ""TRAZAR" Configuracion general leida correctamente",INFOTRAZA);
			// cargamos en la estructura dinamica la configuracion leida
			error = json_a_datos_aplicacion(datosApp, datos);
		} else {
			ESP_LOGW(TAG, ""TRAZAR"La configuracion no se ha cargado. Se carga la de defecto.", INFOTRAZA);
			cargar_configuracion_defecto(datosApp);
			return error;
		}

		//ESP_LOGI(TAG, ""TRAZAR" SALVAMOS LA CONFIGURACION GENERAL A NVS...", INFOTRAZA);
		//salvar_configuracion_general(datosApp);

		// leemos la configuracion de programas desde nvs
		if ((error = leer_configuracion(datosApp, CONFIG_CLAVE_PROGRAMACION, datos)) == ESP_OK) {
			ESP_LOGI(TAG, ""TRAZAR"Programas leidos desde nvs", INFOTRAZA);
			// cargamos en la estructura de programacion dinamica la configuracion leida
			cargar_programas(datosApp, datos);
		} else {
			ESP_LOGW(TAG, ""TRAZAR"No se ha encontrado programacion en el dispositivo", INFOTRAZA);
			error = ESP_OK;
		}

	}
	ESP_LOGE(TAG, ""TRAZAR" ESTADO DE LA APLICACION %d", INFOTRAZA, datosApp->datosGenerales->estadoApp);


	return error;
}

esp_err_t guardar_programas(DATOS_APLICACION *datosApp, char* clave) {

	char* valor;
	esp_err_t error = ESP_OK;;
	cJSON *respuesta = NULL;

	respuesta = cJSON_CreateArray();


	crear_programas_json(datosApp, respuesta);


	valor = cJSON_Print(respuesta);
	error = escribir_dato_string_nvs(&datosApp->handle, clave, valor);
	if (error == ESP_OK) {
		ESP_LOGI(TAG, ""TRAZAR"Programas guardados correctamente\n %s", INFOTRAZA, valor);
	} else {
		ESP_LOGE(TAG, ""TRAZAR"No se han podido guardar los programas en nvs", INFOTRAZA);
	}
	free(valor);
	cJSON_Delete(respuesta);


	return error;

}
/*
esp_err_t insertar_programa(DATOS_APLICACION *datosApp, char* programas) {



	uint8_t elementos;
	cJSON *array;
	int i;
	cJSON *nodo;
	cJSON *item;
	char *dato;

	array = cJSON_Parse(programas);

	   elementos = cJSON_GetArraySize(array);
	   ESP_LOGI(TAG, ""TRAZAR"El array tiene %d elementos", INFOTRAZA, elementos);
	   for (i=0;i<elementos;i++) {
		   nodo = cJSON_GetArrayItem(array, i);
		   item = cJSON_GetObjectItem(nodo, PROGRAM_ID);
		   dato = cJSON_GetStringValue(item);

		   ESP_LOGI(TAG, ""TRAZAR"valor: %s", INFOTRAZA, dato);

	   }

	   cJSON_Delete(array);

	return ESP_OK;
}
*/


esp_err_t cargar_programas_cronotermostato(TIME_PROGRAM *programa_actual, cJSON *nodo) {

	error_t error;


	error = extraer_dato_double(nodo, UMBRAL_TEMPERATURA, &programa_actual->temperatura);
	if (error != ESP_OK) {
		ESP_LOGE(TAG, ""TRAZAR"ERROR AL EXTRAER LA TEMPERATURA", INFOTRAZA);
	} else {
		ESP_LOGI(TAG, ""TRAZAR"TEMPERATURA: %lf", INFOTRAZA, programa_actual->temperatura);
	}

	programa_actual->accion = ON;


	return ESP_OK;
}


esp_err_t cargar_programas_interruptor(TIME_PROGRAM *programa_actual, cJSON *nodo) {

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

esp_err_t cargar_programas(DATOS_APLICACION *datosApp, char *programas) {

	cJSON *array = NULL;
	cJSON *nodo;
	cJSON *item;
	uint8_t elementos = 0;
	int i;
	char *dato;
	char tipo[2];
	TIME_PROGRAM programa_actual;

	array = cJSON_Parse(programas);

	if (array == NULL) {
		ESP_LOGE(TAG, ""TRAZAR"El fichero de programacion nvs no es valido", INFOTRAZA);
		return PROGRAMACION_NO_EXISTE;
	}


	elementos = cJSON_GetArraySize(array);

	ESP_LOGI(TAG, ""TRAZAR"El array tiene %d elementos", INFOTRAZA, elementos);
	for(i=0;i<elementos;i++) {
		ESP_LOGI(TAG, ""TRAZAR"PASAMOS POR EL BUCLE %d", INFOTRAZA, datosApp->datosGenerales->nProgramacion);
		nodo = cJSON_GetArrayItem(array, i);
		item = cJSON_GetObjectItem(nodo, PROGRAM_ID);
		dato = cJSON_GetStringValue(item);
		strncpy(tipo, dato, 2);
		programa_actual.tipo = atoi(tipo);
		//cargar_programas_general(datosApp, programas, &programa_actual);
		programa_actual.programacion.tm_hour = extraer_dato_tm(dato, 2, 2);
		programa_actual.programacion.tm_min = extraer_dato_tm(dato,4, 2);
		programa_actual.programacion.tm_sec = extraer_dato_tm(dato, 6, 2);

		switch(programa_actual.tipo) {
		case DIARIA:
			ESP_LOGI(TAG, ""TRAZAR"%s, Programa diario", INFOTRAZA, dato);
			programa_actual.mascara = extraer_dato_hex(dato, 8,2);
			programa_actual.estadoPrograma = extraer_dato_tm(dato, 10, 1);
			//programa_actual.accion = extraer_dato_tm(dato, 11, 1);
			break;
		case SEMANAL:
			ESP_LOGI(TAG, ""TRAZAR"%s, Programa semanal", INFOTRAZA, dato);

			break;
		case FECHADA:
			ESP_LOGI(TAG, ""TRAZAR"%s, Programa fechado", INFOTRAZA, dato);
			programa_actual.programacion.tm_year = extraer_dato_tm(dato, 8, 4);
			programa_actual.programacion.tm_mon = extraer_dato_tm(dato, 12, 2);
			programa_actual.programacion.tm_mday = extraer_dato_tm(dato, 14, 2);
			programa_actual.estadoPrograma = extraer_dato_tm(dato, 16, 1);
			//programa_actual.accion = extraer_dato_tm(dato, 17, 1);

			break;
		default:
			ESP_LOGE(TAG,"Error al capturar el tipo de programa %s", tipo);
			break;

		}
		appuser_cargar_programa_especifico(datosApp, &programa_actual, nodo);

		/*
		switch(datosApp->datosGenerales->tipoDispositivo) {

		case INTERRUPTOR:
			cargar_programas_interruptor(&programa_actual, nodo);
			break;
		case CRONOTERMOSTATO:
			cargar_programas_cronotermostato(&programa_actual, nodo);
			break;
		case TERMOMETRO:
			break;
			default:
			break;
		}
*/
		visualizartiempo(programa_actual.programacion);
		datosApp->datosGenerales->programacion = crearPrograma(&programa_actual, datosApp->datosGenerales->programacion, &datosApp->datosGenerales->nProgramacion);

	}
	datosApp->datosGenerales->nProgramacion = elementos;
	ESP_LOGI(TAG, ""TRAZAR"CONFIGURACION DE PROGRAMAS CARGADA CORRECTAMENTE. PROGRAMACION VALIDA", INFOTRAZA);
	if (datosApp->datosGenerales->estadoProgramacion == INVALID_PROG) {
		datosApp->datosGenerales->estadoProgramacion = VALID_PROG;
	}

	return ESP_OK;
}

esp_err_t salvar_configuracion_general(DATOS_APLICACION *datosApp) {

	esp_err_t error;
	cJSON *configuracion = NULL;
	char* texto_configuracion = NULL;
	ESP_LOGI(TAG, ""TRAZAR"SALVAR CONFIGURACION", INFOTRAZA);
	configuracion = cJSON_CreateObject();
	error = configuracion_a_json(datosApp, configuracion);

	if (error != JSON_OK) {
		ESP_LOGI(TAG, ""TRAZAR" ERROR AL SALVAR LA CONFIGURACION", INFOTRAZA);
	} else {
		ESP_LOGI(TAG, ""TRAZAR" VAMOS A CREAR EL JSON ANTES DE SALVAR", INFOTRAZA);
		if (configuracion == NULL) {
			ESP_LOGE(TAG, ""TRAZAR" la configuracion es nula", INFOTRAZA);
			return ESP_FAIL;
		}
		texto_configuracion = cJSON_Print(configuracion);
		if(texto_configuracion != NULL) {
			error = guardar_configuracion(datosApp, CONFIG_CLAVE_CONFIGURACION_GENERAL, texto_configuracion);
			free(texto_configuracion);
		} else {
			ESP_LOGE(TAG, ""TRAZAR" NO SE HA PODIDO GUARDAR LA CONFIGURACION", INFOTRAZA);
		}
		cJSON_Delete(configuracion);

	}


	return error;
}

esp_err_t ota_a_json(DATOS_APLICACION *datosApp) {

	esp_err_t error;
	char *texto;
	cJSON *ota;
	ota = cJSON_CreateObject();
	ESP_LOGI(TAG, ""TRAZAR"(OTA A JSON)", INFOTRAZA);
	cJSON_AddStringToObject(ota, OTA_SERVER, datosApp->datosGenerales->ota.server);
	cJSON_AddStringToObject(ota, OTA_URL, datosApp->datosGenerales->ota.url);
	cJSON_AddStringToObject(ota, OTA_FILE, datosApp->datosGenerales->ota.file);
	cJSON_AddStringToObject(ota, OTA_SW_VERSION , datosApp->datosGenerales->ota.newVersion);
	cJSON_AddNumberToObject(ota, OTA_PORT, datosApp->datosGenerales->ota.puerto);
	texto = cJSON_Print(ota);
	if (texto != NULL) {
		ESP_LOGI(TAG, ""TRAZAR" UPGRADE FIRMWARE PREPARADO %s", INFOTRAZA, texto);
		//error = escribir_dato_string_nvs(&datosApp->handle, "UPGRADE_FIRMWARE", texto);
		error = guardar_configuracion(datosApp, "UPGRADE", texto);
		if (error != ESP_OK) {
			ESP_LOGE(TAG, ""TRAZAR" UPGRADE FIRMWARE ERROR %d", INFOTRAZA, error);
			return error;
		}

	} else {
		error = ESP_FAIL;
	}
	free(texto);
	cJSON_Delete(ota);



	return error;
}

esp_err_t json_a_ota(DATOS_APLICACION *datosApp) {

	esp_err_t error;
	cJSON *nodo;
	char datos[CONFIG_TAMANO_BUFFER_LECTURA];
	error = leer_configuracion(datosApp, "UPGRADE", datos);
	if (error == ESP_OK) {
		nodo = cJSON_Parse(datos);
		extraer_dato_string(nodo, OTA_SERVER, datosApp->datosGenerales->ota.server);
		extraer_dato_string(nodo, OTA_URL, datosApp->datosGenerales->ota.url);
		extraer_dato_string(nodo, OTA_FILE, datosApp->datosGenerales->ota.file);
		extraer_dato_string(nodo, OTA_SW_VERSION, datosApp->datosGenerales->ota.newVersion);
		extraer_dato_int(nodo, OTA_PORT, &datosApp->datosGenerales->ota.puerto);
	}

	return error;
}

esp_err_t configurado_de_fabrica() {




	wifi_config_t conf_wifi = {0};
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	esp_wifi_init(&cfg);
	esp_wifi_get_config(WIFI_IF_STA, &conf_wifi);
	int i;
	for (i=0;i<32;i++) {
		if (conf_wifi.sta.ssid[i] != 0) {
			ESP_LOGW(TAG, ""TRAZAR" WIFI CONFIGURADA %s, %s", INFOTRAZA, (char*) conf_wifi.sta.ssid, (char*) conf_wifi.sta.password);
			return ESP_FAIL;
		}
	}

	ESP_LOGW(TAG, ""TRAZAR" WIFI NO CONFIGURADA", INFOTRAZA);

	return ESP_OK;


}


