/* Common functions for protocol examples, to establish Wi-Fi or Ethernet connection.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
 */

#include "conexiones.h"
#include "esp_wifi.h"
#include "freertos/event_groups.h"
#include "configuracion.h"
#include "alarmas.h"
#include "interfaz_usuario.h"
#include "smartconfig_ack.h"
#include "esp_smartconfig.h"



#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
//#include "tcpip_adapter.h"
//#include "esp_netif.h"
//#include "esp_smartconfig.h"
#include "smartconfig_ack.h"
#include "esp_err.h"
#include "esp_smartconfig.h"
#include "esp_netif.h"
#include "alarmas.h"



static const int CONNECTED_BIT = BIT0;
static const int ESPTOUCH_DONE_BIT = BIT1;


static EventGroupHandle_t grupo_eventos;
 ip4_addr_t s_ip_addr;
static const char *TAG = "CONEXIONES";
extern DATOS_APLICACION datosApp;

/*
void extraer_datos_mqtt(void * event_data, wifi_config_t *wifi_config) {

	char* ptr;
	char broker[100] = "mqtt://";
	char pass[65] = {0};
	char *texto;

	texto = (char*) calloc(256, sizeof(char));
	memcpy(texto, event_data+32, 256);
	texto[253] = 33;
	texto[255]=0;
	ESP_LOGI(TAG, ""TRAZAR" datos:%s, longitud: %d", INFOTRAZA, texto, strlen(texto));

	smartconfig_event_got_ssid_pswd_t* evt = (smartconfig_event_got_ssid_pswd_t*)event_data;


	memcpy(pass, (char*) evt->password, sizeof(wifi_config->sta.password) );
    ptr = strtok((char*) evt->password, ":");
    strcpy(pass, ptr);
    if ((ptr = strtok(NULL, ":")) != NULL) {
        strcpy(datosApp.datosGenerales->parametrosMqtt.broker, ptr);
        strcat(broker, datosApp.datosGenerales->parametrosMqtt.broker);
        strcpy(datosApp.datosGenerales->parametrosMqtt.broker, broker);
        printf ("broker:%s\n", datosApp.datosGenerales->parametrosMqtt.broker);
    }

    if ((ptr = strtok(NULL, ":")) != NULL) {
        datosApp.datosGenerales->parametrosMqtt.port = atoi(ptr);
        ESP_LOGI(TAG, ""TRAZAR" PUERTO: %d\n",INFOTRAZA, datosApp.datosGenerales->parametrosMqtt.port);
    }

    if ((ptr = strtok(NULL, ":")) != NULL) {
        strcpy(datosApp.datosGenerales->parametrosMqtt.user, ptr);
        ESP_LOGI(TAG, ""TRAZAR"user: %s\n", INFOTRAZA, datosApp.datosGenerales->parametrosMqtt.user);
    }

    if ((ptr = strtok(NULL, ":")) != NULL) {
        strcpy(datosApp.datosGenerales->parametrosMqtt.password, ptr);
        ESP_LOGI(TAG, ""TRAZAR"password: %s\n",INFOTRAZA, datosApp.datosGenerales->parametrosMqtt.password);
    }

    memset(wifi_config->sta.password, 0, 64);
    memcpy((char*) wifi_config->sta.password, pass,sizeof(wifi_config->sta.password) );



}
*/
void extraer_datos_smartconfig(void * event_data, wifi_config_t *wifi_config) {



	 smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
	 bzero(wifi_config, sizeof(wifi_config_t));
	 memcpy(wifi_config->sta.password, evt->password, sizeof(wifi_config->sta.password));
	 memcpy(wifi_config->sta.ssid, evt->ssid, sizeof(wifi_config->sta.ssid));


}




void volcar_datos_conexion(void * event_data){

    smartconfig_event_got_ssid_pswd_t* evt = (smartconfig_event_got_ssid_pswd_t*)event_data;
    wifi_config_t wifi_config;


    bzero(&wifi_config, sizeof(wifi_config_t));
    memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));

    ESP_LOGI(TAG, ""TRAZAR" vamos a extraer los datos de conexion", INFOTRAZA);
    //extraer_datos_mqtt(event_data, &wifi_config);
    extraer_datos_smartconfig(event_data, &wifi_config);

    ESP_LOGI(TAG, ""TRAZAR" SSID: %s, PASS:%s", INFOTRAZA,
    		wifi_config.sta.ssid, wifi_config.sta.password);


    wifi_config.sta.bssid_set = evt->bssid_set;

    if (wifi_config.sta.bssid_set == true) {
        memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));
    }
    ESP_ERROR_CHECK(esp_wifi_disconnect());
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_connect());



}





static void manejador_eventos_smart(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data) {


    switch (event_id) {
        case SC_EVENT_SCAN_DONE:
        	ESP_LOGI(TAG, ""TRAZAR"Escaneo realizado", INFOTRAZA);
            break;
        case SC_EVENT_FOUND_CHANNEL:
        	ESP_LOGI(TAG, ""TRAZAR"Encontrado canal...", INFOTRAZA);
            break;
        case SC_EVENT_GOT_SSID_PSWD:
        	ESP_LOGI(TAG, ""TRAZAR"Obteniendo datos de conexion...", INFOTRAZA);
        	volcar_datos_conexion(event_data);
            break;
        case SC_EVENT_SEND_ACK_DONE:
            ESP_LOGW(TAG, "SC_EVENT_SEND_ACK_DONE");
            xEventGroupSetBits(grupo_eventos, ESPTOUCH_DONE_BIT);
            break;
        default:
            break;
    }

}







const char * get_my_id(void)
{
   // Use MAC address for Station as unique ID
   static char my_id[13];
   static bool my_id_done = false;
   esp_err_t error;
   int8_t i;
   uint8_t x;
   if (my_id_done)
       return my_id;
   //if (!wifi_get_macaddr(STATION_IF, my_id))

   if ((error = esp_wifi_get_mac(WIFI_IF_STA, (uint8_t*) my_id)) == ESP_OK) {
   	ESP_LOGI(TAG, ""TRAZAR"Mac extraida correctamente", INFOTRAZA);
   } else {
   	ESP_LOGE(TAG, ""TRAZAR"ERROR AL OBTENER LA MAC. ERROR : %d", INFOTRAZA, error);
   	return NULL;
   }

   //if ((esp_wifi_get_mac(WIFI_IF_STA, (uint8_t*) my_id)) != ESP_OK)
   //    return NULL;
   for (i = 5; i >= 0; --i)
   {
       x = my_id[i] & 0x0F;
       if (x > 9) x += 7;
       my_id[i * 2 + 1] = x + '0';
       x = my_id[i] >> 4;
       if (x > 9) x += 7;
       my_id[i * 2] = x + '0';
   }
   my_id[12] = '\0';
   my_id_done = true;
   return my_id;
}





static void on_wifi_disconnect(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{

    //system_event_sta_disconnected_t *event = (system_event_sta_disconnected_t *)event_data;

    ESP_LOGW(TAG, ""TRAZAR"Wi-Fi desconectado, se intenta la reconexion...", INFOTRAZA);
    /*
    if (event->reason == WIFI_REASON_BASIC_RATE_NOT_SUPPORT) {

        esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N);
    }
*/
    if (datosApp.datosGenerales->estadoApp != UPGRADE_EN_PROGRESO) {
    	registrar_alarma(&datosApp, NOTIFICACION_ALARMA_WIFI, ALARMA_WIFI, ALARMA_ON, false);
    	ESP_ERROR_CHECK(esp_wifi_connect());
    	xEventGroupClearBits(grupo_eventos, CONNECTED_BIT);
    }


}

static void on_got_ip(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data)
{
	ESP_LOGW(TAG, ""TRAZAR"Wi-Fi conectado", INFOTRAZA);
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    memcpy(&s_ip_addr, &event->ip_info.ip, sizeof(s_ip_addr));
    xEventGroupSetBits(grupo_eventos, CONNECTED_BIT);
    registrar_alarma(&datosApp, NOTIFICACION_ALARMA_WIFI, ALARMA_WIFI, ALARMA_ON, false);
    appuser_wifi_conectado(&datosApp);
}


inline static void conectar_wifi() {

	ESP_LOGW(TAG, ""TRAZAR"Wi-Fi vamos a conectar", INFOTRAZA);
	esp_wifi_connect();
	ESP_LOGW(TAG, ""TRAZAR"Wi-Fi vamos 2 a conectar", INFOTRAZA);
	xEventGroupWaitBits(grupo_eventos, CONNECTED_BIT, true, true, portMAX_DELAY);
	ESP_LOGW(TAG, ""TRAZAR"Wi-Fi fin de vamos a conectar", INFOTRAZA);

}



inline static void inicializar_wifi() {

#ifdef CONFIG_IDF_TARGET_ESP32
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
#endif

	ESP_LOGI(TAG, ""TRAZAR" INICIALIZAR_WIFI", INFOTRAZA);
    //tcpip_adapter_init();
	appuser_wifi_conectando(&datosApp);
    grupo_eventos = xEventGroupCreate();
    esp_netif_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
#ifdef CONFIG_IDF_TARGET_ESP32
    esp_netif_create_default_wifi_sta ();
#endif


    //ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
#ifdef CONFIG_IDF_TARGET_ESP32
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &on_wifi_disconnect, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ip, &instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &manejador_eventos_smart, NULL));
#else
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &on_wifi_disconnect, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ip, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &manejador_eventos_smart, NULL));
#endif
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));


    //ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    //ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, ""TRAZAR" WIFI INICIALIZADO", INFOTRAZA);
}

esp_err_t restaurar_wifi_fabrica() {

	wifi_config_t wifi_config;
	strcpy((char*) &wifi_config.sta.ssid, "none");
	strcpy((char*) &wifi_config.sta.password, "none");
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

    return ESP_OK;

}

void tarea_smartconfig(void* parm) {
    EventBits_t uxBits;
    DATOS_APLICACION *datosApp;
    datosApp = (DATOS_APLICACION*) parm;
    appuser_notificar_smartconfig(datosApp);
    ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH));
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_smartconfig_start(&cfg));

    while (1) {
        uxBits = xEventGroupWaitBits(grupo_eventos, (CONNECTED_BIT | ESPTOUCH_DONE_BIT), true, false, portMAX_DELAY);

        if (uxBits & CONNECTED_BIT) {
            ESP_LOGI(TAG, "CONECTADO A LA ESTACION WIFI DESPUES DE SMARTCONFIG");
        }

        if (uxBits & ESPTOUCH_DONE_BIT) {
            ESP_LOGI(TAG, "SMARTCONFIG TERMINADO");
            esp_smartconfig_stop();
            vTaskDelete(NULL);
        }
    }
}


esp_err_t conectar_dispositivo_wifi() {

	wifi_config_t conf_wifi;
	int i;
	inicializar_wifi();
    esp_wifi_get_config(WIFI_IF_STA, &conf_wifi);
    conf_wifi.sta.pmf_cfg.capable = true;
    conf_wifi.sta.pmf_cfg.required = false;
    //ESP_LOGW(TAG, ""TRAZAR"(1)", INFOTRAZA);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    //ESP_LOGW(TAG, ""TRAZAR"(2)", INFOTRAZA);
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &conf_wifi) );
    //ESP_LOGW(TAG, ""TRAZAR"(3)", INFOTRAZA);
    ESP_ERROR_CHECK(esp_wifi_start());
    //ESP_LOGW(TAG, ""TRAZAR"(4)", INFOTRAZA);

	for (i=0;i<32;i++) {
		if (conf_wifi.sta.ssid[i] != 0) {
			ESP_LOGW(TAG, ""TRAZAR" WIFI CONFIGURADA %s, %s", INFOTRAZA, (char*) conf_wifi.sta.ssid, (char*) conf_wifi.sta.password);
			conectar_wifi();
			return ESP_OK;
		}
	}

	ESP_LOGW(TAG, ""TRAZAR" WIFI NO CONFIGURADA", INFOTRAZA);
	xTaskCreate(tarea_smartconfig, "tarea_smart", 4096, (void*)&datosApp, tskIDLE_PRIORITY + 0, NULL);
	//xEventGroupWaitBits(grupo_eventos, CONNECTED_BIT, true, true, portMAX_DELAY);
	conectar_wifi();
	return ESP_FAIL;




	return ESP_OK;
}





