/*
 * alarmas.h
 *
 *  Created on: 10 oct. 2020
 *      Author: t126401
 */

#include "stdint.h"
#include "sys/types.h"
#include "esp_err.h"
#include "esp_log.h"
#include "configuracion_usuario.h"




#ifndef COMPONENTS_ALARMAS_INCLUDE_ALARMAS_H_
#define COMPONENTS_ALARMAS_INCLUDE_ALARMAS_H_


esp_err_t registrar_alarma(DATOS_APLICACION *datosApp, char* nemonico_alarma, uint8_t tipo_alarma, enum ESTADO_ALARMA estado_alarma, bool flag_envio);
esp_err_t inicializacion_registros_alarmas(DATOS_APLICACION *datosApp);


#endif /* COMPONENTS_ALARMAS_INCLUDE_ALARMAS_H_ */
