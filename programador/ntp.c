/*
 * ntp.c
 *
 *  Created on: 4 sept. 2020
 *      Author: t126401
 */



#include "esp_log.h"
#include "esp_err.h"
#include "ntp.h"
#include "configuracion_usuario.h"
#include "programmer.h"
#include "errores_proyecto.h"
#include "time.h"
#include "esp_sntp.h"


#define TIMEOUT_NTP CONFIG_TIMEOUT_NTP
#define ZONA_HORARIA "CET-1CEST,M3.5.0,M10.5.0/3"


static const char *TAG = "NTP";


void inicializar_parametros_ntp(NTP_CLOCK *clock) {

	ESP_LOGI(TAG, ""TRAZAR"cargando parametros ntp...", INFOTRAZA);
	clock->ntpTimeZone = (char*) calloc (150, sizeof(char));
	strcpy(clock->ntpTimeZone, ZONA_HORARIA);
    clock->estado = SIN_HORA;
    clock->timeValid = false;

 }







 void notificar_sincronizacion_ntp(struct timeval *tv)
 {
     ESP_LOGI(TAG, ""TRAZAR"Evento de notificacion de sincronizacion ntp", INFOTRAZA);
 }



esp_err_t obtener_fecha_hora(NTP_CLOCK *clock) {


    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 10;

	ESP_LOGI(TAG, ""TRAZAR"Inicializando ntp", INFOTRAZA);
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, CONFIG_SERVIDOR_NTP);
    sntp_set_time_sync_notification_cb(notificar_sincronizacion_ntp);
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
    sntp_init();


    while (timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
        ESP_LOGW(TAG, ""TRAZAR"Waiting for system time to be set... (%d/%d)", INFOTRAZA, retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);

    }

    if (retry == retry_count) {
    	ESP_LOGE(TAG, ""TRAZAR"Error al obtener la hora del sistema", INFOTRAZA);
    	clock->estado = ERROR_NTP;
    	clock->timeValid = false;
    	return RELOJ_SIN_HORA;

    } else {
    	ESP_LOGI(TAG, ""TRAZAR"VAMOS A APLICAR LA ZONA %s", INFOTRAZA, clock->ntpTimeZone);
        setenv("TZ", clock->ntpTimeZone, 1);
        //setenv("TZd", "EST+5",10);
        tzset();
        
        actualizar_hora(clock);
        ESP_LOGI(TAG, ""TRAZAR"APLICADA ZONA HORARIA %s", INFOTRAZA, clock->ntpTimeZone);
    }



//    clock->time = now;
//    clock->date = timeinfo;
    ESP_LOGI(TAG, ""TRAZAR" VAMOS A PINTAR LA FECHA", INFOTRAZA);
    ESP_LOGI(TAG,"%02d/%02d/%04d %02d:%02d:%02d", clock->date.tm_mday, clock->date.tm_mon + 1, clock->date.tm_year + 1900, clock->date.tm_hour, clock->date.tm_min, clock->date.tm_sec);

    clock->estado = EN_HORA;
    clock->timeValid = true;

    return ESP_OK;

}

void actualizar_hora(NTP_CLOCK *clock) {

	time_t now;
	struct tm timeinfo;
	//ESP_LOGI(TAG, ""TRAZAR"ACTUALIZAR_HORA", INFOTRAZA);
    time(&now);
    localtime_r(&now, &timeinfo);
    clock->time = now;
    clock->date = timeinfo;

    //ESP_LOGI(TAG, ""TRAZAR"Hora actualizada: %02d/%02d/%04d %02d:%02d:%02d",INFOTRAZA, clock->date.tm_mday, clock->date.tm_mon + 1, clock->date.tm_year + 1900, clock->date.tm_hour, clock->date.tm_min, clock->date.tm_sec);

}

void acciones_reloj(NTP_CLOCK *clock, TIME_PROGRAM *programs) {

	switch (clock->estado) {

	case SIN_HORA:
		ESP_LOGW(TAG, ""TRAZAR"Esperando sincronizacion", INFOTRAZA);
		break;
	case EN_HORA:
		chequear_ejecucion_programa(clock, programs);

		break;
	case ERROR_NTP:
		break;
	default:
		break;
	}

}





