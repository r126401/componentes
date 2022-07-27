/*
 * mqtt.c
 *
 *  Created on: 22 sept. 2020
 *      Author: t126401
 */


#include "esp_err.h"
#include "esp_log.h"
#include "conexiones_mqtt.h"
#include "mqtt_client.h"
#include "datos_comunes.h"
#include "api_json.h"
#include "alarmas.h"
#include "interfaz_usuario.h"
#include "dialogos_json.h"
#include "esp_tls.h"
#include "conexiones_mqtt.h"


static const char *TAG = "MQTT";


//extern const uint8_t mqtt_jajica_pem_start[]   asm("_binary_mqtta_eclipse_org_pem_start");
//extern const uint8_t mqtt_jajica_pem_end[]   asm("_binary_mqtt_eclipse_org_pem_end");
extern const uint8_t mqtt_jajica_pem_start[]   asm("_binary_mqtt_cert_crt_start");
extern const uint8_t mqtt_jajica_pem_end[]   asm("_binary_mqtt_cert_crt_end");
extern DATOS_APLICACION datosApp;

xQueueHandle cola_mqtt = NULL;
esp_mqtt_client_handle_t client;
TaskHandle_t handle;

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id = -1;
    // your_context_t *context = event->context;
    datosApp.handle_mqtt = event;
    static bool arranque = false;

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, ""TRAZAR"MQTT_EVENT_CONNECTED: CONECTADO AL BROKER", INFOTRAZA);
            appuser_broker_conectado(&datosApp);
            msg_id = esp_mqtt_client_subscribe(client, datosApp.datosGenerales->parametrosMqtt.subscribe,datosApp.datosGenerales->parametrosMqtt.qos);
            ESP_LOGI(TAG, ""TRAZAR"ACCION PARA SUBSCRIBIR AL TOPIC :%s msg_id=%d", INFOTRAZA, datosApp.datosGenerales->parametrosMqtt.subscribe, msg_id);
            if (datosApp.alarmas[ALARMA_MQTT].estado_alarma == ALARMA_ON) {
            	registrar_alarma(&datosApp, NOTIFICACION_ALARMA_MQTT, ALARMA_MQTT, ALARMA_OFF, true);
            }

            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, ""TRAZAR"MQTT_EVENT_DISCONNECTED: Desconectado del broker :%s msg_id=%d", INFOTRAZA, datosApp.datosGenerales->parametrosMqtt.broker, msg_id);
            registrar_alarma(&datosApp, NOTIFICACION_ALARMA_MQTT, ALARMA_MQTT, ALARMA_ON, false);
            appuser_broker_desconectado(&datosApp);
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, ""TRAZAR"MQTT_EVENT_SUBSCRIBED: SUBSCRITOS CON EXITO AL TOPIC :%s msg_id=%d", INFOTRAZA, datosApp.datosGenerales->parametrosMqtt.subscribe, msg_id);
            if (arranque == false ){
            	appuser_arranque_aplicacion(&datosApp);
            	arranque = true;
            }
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGW(TAG, ""TRAZAR"MQTT_EVENT_UNSUBSCRIBED: YA NO ESTAS SUBSCRITO AL TOPIC :%s msg_id=%d", INFOTRAZA, datosApp.datosGenerales->parametrosMqtt.subscribe, msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, ""TRAZAR"MQTT_EVENT_PUBLISHED: CONFIRMACION DE EVENTO PUBLICADO. TOPIC :%s msg_id=%d", INFOTRAZA, datosApp.datosGenerales->parametrosMqtt.subscribe, msg_id);

            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, ""TRAZAR"MQTT_EVENT_DATA: RECIBIDO MENSAJE", INFOTRAZA);

            char topic[55] = {0};
            strncpy(topic, event->topic, event->topic_len);

            if (strcmp(datosApp.datosGenerales->parametrosMqtt.subscribe, topic) == 0) {
            	mensaje_recibido(&datosApp);
            } else {
            	app_user_mensaje_recibido_especifico(&datosApp);
            }

            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, ""TRAZAR"MQTT_EVENT_ERROR", INFOTRAZA);
            registrar_alarma(&datosApp, NOTIFICACION_ALARMA_MQTT, ALARMA_MQTT, ALARMA_ON, false);
            break;
        case MQTT_EVENT_BEFORE_CONNECT:
        	ESP_LOGE(TAG, ""TRAZAR"MQTT_EVENT_BEFORE_CONNECT. EVENTO NUEVO DESPUES DE CONECTAR", INFOTRAZA);
        	break;
        default:
        	break;
    }
    return ESP_OK;
}

void mqtt_task(void *arg) {

	//DATOS_APLICACION *datosApp = (DATOS_APLICACION*) arg;
	COLA_MQTT cola;
	cola_mqtt = xQueueCreate(10, sizeof(COLA_MQTT));

	ESP_LOGI(TAG, ""TRAZAR"COMIENZO MQTT_TASK", INFOTRAZA);
	establecer_conexion_mqtt(&datosApp);


	for(;;) {
		 ESP_LOGI(TAG, ""TRAZAR"ESPERANDO MENSAJE...Memoria libre: %d\n", INFOTRAZA, esp_get_free_heap_size());
		if (xQueueReceive(cola_mqtt, &cola, portMAX_DELAY) == pdTRUE) {
			publicar_mensaje(&datosApp, &cola);

		} else {
			ESP_LOGE(TAG, ""TRAZAR"NO SE HA PODIDO PROCESAR LA PETICION", INFOTRAZA);
		}

	}
	vTaskDelete(NULL);


}





esp_err_t establecer_conexion_mqtt(DATOS_APLICACION *datosApp) {
	esp_err_t error;

    esp_mqtt_client_config_t mqtt_cfg = {
		.uri = datosApp->datosGenerales->parametrosMqtt.broker,
		.port = datosApp->datosGenerales->parametrosMqtt.port,
        .event_handle = mqtt_event_handler,
		.username = datosApp->datosGenerales->parametrosMqtt.user,
		.password = datosApp->datosGenerales->parametrosMqtt.password,
		//.cert_pem = (const char *) mqtt_jajica_pem_start,
    };

    if (datosApp->datosGenerales->parametrosMqtt.tls == true) {
    	ESP_LOGI(TAG, ""TRAZAR"Añadimos el certificado %s", INFOTRAZA, mqtt_jajica_pem_start);

    	esp_tls_init();
    	error = esp_tls_init_global_ca_store ();
    	if (error != ESP_OK) {
    		ESP_LOGE(TAG, ""TRAZAR"ERROR al inicializar el almacen CA %d", INFOTRAZA, sizeof ((const char*)mqtt_jajica_pem_start));
    	}


    	ESP_LOGE(TAG, ""TRAZAR"CA inicializada %d", INFOTRAZA, strlen ((const char*)mqtt_jajica_pem_start));

    	error = esp_tls_set_global_ca_store (( const  unsigned  char *) mqtt_jajica_pem_start, mqtt_jajica_pem_end - mqtt_jajica_pem_start );
    	if (error != ESP_OK) {
    		ESP_LOGE(TAG, ""TRAZAR"ERROR al crear el almacen CA", INFOTRAZA);
    	}
    	//mqtt_cfg.cert_pem = (const char *) mqtt_jajica_pem_start;
    	mqtt_cfg.use_global_ca_store = true;
    	mqtt_cfg.skip_cert_common_name_check = true;

    } else {

    	ESP_LOGE(TAG, ""TRAZAR"No se ha añadido el certificado para la conexion mqtt", INFOTRAZA);
    	datosApp->datosGenerales->parametrosMqtt.cert = (char*) mqtt_jajica_pem_start;
    	//strcpy(datosApp->datosGenerales->parametrosMqtt.cert, (const char*) mqtt_jajica_pem_start);
    }




    ESP_LOGI(TAG, ""TRAZAR"Nos conectamos al broker %s", INFOTRAZA, mqtt_cfg.uri);
    appuser_broker_conectando(datosApp);
    client = esp_mqtt_client_init(&mqtt_cfg);
    //esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    //esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    error = esp_mqtt_client_start(client);


    return error;
}
esp_err_t publicar_mensaje(DATOS_APLICACION *datosApp, COLA_MQTT *cola) {

	int msg_id;

	//esp_mqtt_client_handle_t client = cliente->client;
	if (datosApp->handle_mqtt == NULL) {
		ESP_LOGE(TAG, ""TRAZAR"NO HAY CONEXION CON EL BROKER Y NO SE PUEDE ENVIAR EL MENSAJE", INFOTRAZA);
		return ESP_FAIL;
	}

	if (datosApp->datosGenerales->estadoApp == NORMAL_ARRANCANDO) {
		ESP_LOGW(TAG, "NO SE ENVIA NADA PORQUE EL DISPOSITIVO ESTA AUN EN FASE DE ARRANQUE");
		return ESP_OK;
	}

	msg_id = esp_mqtt_client_publish(datosApp->handle_mqtt->client,
			cola->topic,
			cola->buffer,
			0,
			datosApp->datosGenerales->parametrosMqtt.qos,
			0);
	ESP_LOGI(TAG, ""TRAZAR" MENSAJE %d PUBLICADO.%s \n %s", INFOTRAZA, msg_id, cola->topic, cola->buffer);
	return ESP_OK;
}

esp_err_t publicar_mensaje_json(DATOS_APLICACION *datosApp, cJSON *mensaje, char *topic) {

	char* texto;
	COLA_MQTT cola;
	memset(&cola, 0, sizeof(COLA_MQTT));
	if (datosApp->handle_mqtt == NULL) {
		ESP_LOGE(TAG, ""TRAZAR"NO HAY CONEXION CON EL BROKER Y NO SE PUEDE ENVIAR EL MENSAJE", INFOTRAZA);
		return ESP_FAIL;
	}
	texto = cJSON_Print(mensaje);
	if (topic != NULL) {
		strcpy(cola.topic, topic);
	} else {
		strcpy(cola.topic, datosApp->datosGenerales->parametrosMqtt.publish);
	}
	strcpy(cola.buffer, texto);
	ESP_LOGE(TAG, ""TRAZAR" tamano de cola %d", INFOTRAZA, strlen(cola.topic) + strlen(cola.buffer));
	if (texto != NULL) {
		xQueueSend(cola_mqtt, &cola,0);
		free(texto);
	}
	cJSON_Delete(mensaje);
	return ESP_OK;
}


void crear_tarea_mqtt(DATOS_APLICACION *datosApp) {



    xTaskCreate(mqtt_task, "mqtt_task", 8192, (void*) datosApp, 10, &handle);
    configASSERT(handle);

    if (handle == NULL) {
    	ESP_LOGE(TAG, ""TRAZAR"handle es nulo", INFOTRAZA);

    } else {
    	ESP_LOGE(TAG, ""TRAZAR"handle no es nulo", INFOTRAZA);
    }


}


void eliminar_tarea_mqtt() {

	esp_mqtt_client_destroy(client);
	vTaskDelete(handle);



}


esp_err_t obtener_certificado(DATOS_APLICACION *datosApp) {

	datosApp->datosGenerales->parametrosMqtt.cert = (char*) mqtt_jajica_pem_start;




	return ESP_OK;
}



