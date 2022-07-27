/*
 * alarmas.c
 *
 *  Created on: 10 oct. 2020
 *      Author: t126401
 */


#include "alarmas.h"
#include "ntp.h"
#include "stdint.h"
#include "datos_comunes.h"
#include "configuracion_usuario.h"
#include "api_json.h"
#include "interfaz_usuario.h"



static const char *TAG = "ALARMAS";




esp_err_t registrar_alarma(DATOS_APLICACION *datosApp, char* mnemonico_alarma, uint8_t tipo_alarma, enum ESTADO_ALARMA estado_alarma, bool flag_envio) {

	//Registramos la alarma.
	datosApp->alarmas[tipo_alarma].estado_alarma = estado_alarma;
	datosApp->alarmas[tipo_alarma].fecha_alarma = datosApp->datosGenerales->clock.time;
	if (flag_envio == true) {
		notificar_evento_alarma(datosApp, tipo_alarma, mnemonico_alarma);
	}
	appuser_notificar_alarma_localmente(datosApp, tipo_alarma);
	ESP_LOGW(TAG, ""TRAZAR" ALARMA %d ESTADO %d REGISTRADA", INFOTRAZA, tipo_alarma, estado_alarma);

	return ESP_OK;
}





esp_err_t inicializacion_registros_alarmas(DATOS_APLICACION *datosApp) {

	int i;
	NTP_CLOCK clock;
	for (i=0;i<NUM_TIPOS_ALARMAS;i++) {

		datosApp->alarmas[i].tipo_alarma = i;
		datosApp->alarmas[i].estado_alarma = ALARMA_OFF;
		actualizar_hora(&clock);
		datosApp->alarmas[i].fecha_alarma = clock.time;
		ESP_LOGI(TAG, ""TRAZAR" INICIALIZADAS ALARMAS, TIPO_ALARMA: %d, ESTADO: %d, FECHA: %ld", INFOTRAZA,
				datosApp->alarmas[i].tipo_alarma,
				datosApp->alarmas[i].estado_alarma,
				datosApp->alarmas[i].fecha_alarma);
		nemonicos_alarmas(datosApp, i);
	}

	ESP_LOGI(TAG, ""TRAZAR"REGISTROS DE ALARMAS INICIALIZADOS", INFOTRAZA);

	return ESP_OK;

}


