/*
 * mqtt.h
 *
 *  Created on: 22 sept. 2020
 *      Author: t126401
 */

#include "configuracion_usuario.h"

#ifndef COMPONENTS_CONEXIONES_INCLUDE_MQTT_H_
#define COMPONENTS_CONEXIONES_INCLUDE_MQTT_H_

typedef struct COLA_MQTT {
	char topic[50];
	char buffer[1024];
}COLA_MQTT;


esp_err_t establecer_conexion_mqtt(DATOS_APLICACION *datosApp);
esp_err_t publicar_mensaje(DATOS_APLICACION *datosApp, COLA_MQTT *cola);
esp_err_t publicar_mensaje_json(DATOS_APLICACION *datosApp, cJSON *mensaje, char *topic);
esp_err_t publicar_mensaje_json_generico(DATOS_APLICACION *datosApp, cJSON *mensaje, char* topic);
void mqtt_task(void *datosApp);
void crear_tarea_mqtt(DATOS_APLICACION *datosApp);
void eliminar_tarea_mqtt();
esp_err_t obtener_certificado(DATOS_APLICACION *datosApp);

#endif /* COMPONENTS_CONEXIONES_INCLUDE_MQTT_H_ */
