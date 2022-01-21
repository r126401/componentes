/**
Proyecto realizado por Emilio Jimenez Prieto
*/

/*
 * configuracion.h
 *
 *  Created on: 10 sept. 2020
 *      Author: t126401
 */

#include "configuracion_usuario.h"
#include "cJSON.h"
#include "programmer.h"

#ifndef COMPONENTS_CONFIGURACION_INCLUDE_CONFIGURACION_H_
#define COMPONENTS_CONFIGURACION_INCLUDE_CONFIGURACION_H_

/**
 * @fn esp_err_t leer_configuracion(DATOS_APLICACION*, char*, char*)
 * @brief Lee una etiqueta nvs donde se guarda la connfiguracion correspondiente del dispositivo
 *
 * @pre Es necesaria haber llamado antes a la funion inicializar_nvs
 * @post
 * @param datosApp
 * @param clave Es la etiqueta nvs correspondiente a la configuracion buscada
 * @param valor Es el contenido de la etiqueta.
 * @return Retorna ESP_OK, o el codigo de error en caso de no poder leerla.
 */
esp_err_t leer_configuracion(DATOS_APLICACION *datosApp, char* clave, char* valor);
/**
 * @fn esp_err_t inicializacion(DATOS_APLICACION*, bool)
 * @brief funcion que realiza todas las labores de inicializacion previas y comunes a cualquier aplicacion. Realiza las siguiente funciones.
 * - coloca el estado en NORMAL_ARRANCANDO.
 * - Llama a inicializar_parametros_ntp para inicializar los parametros ntp
 * - Lama a appuser_obteniendo_sntp para que la aplicacion pueda notificar que está obtiendo la hora via sntp
 * - Llama a obtener_fecha_hora para obtener la hora desde los servidores ntp.
 * - Llama a inicializar_nvs para inicializar la memoria nvs y poder acceder a ella.
 * - Intenta cargar la configuracion del dispositivo desde la memoria nvs. Si esto no fuera posible, lo haría desde
 * la configuracion por defecto de fabrica.
 * - Se leen los programas almacenados si esto fuera necesario.
 * @pre
 * @post
 * @param datosApp
 * @param forzado
 * @return
 */
esp_err_t inicializacion(DATOS_APLICACION *datosApp, bool forzado);
/**
 * @fn esp_err_t cargar_configuracion_defecto(DATOS_APLICACION*)
 * @brief Realiza la carga de configuracion minima que necesita el dispositivo para funcionar.
 *
 * @pre Para que esta funcion se ejecute es necesario que no haya ninguna configuracion en el dispositivo,
 * o que la variable de compilacion CARGA_CONFIGURACION este puesto a 1 en menuconfig
 * @post
 * @param datosApp
 * @return ESP_OK cuando la operacion se realice correctamente.
 */
esp_err_t cargar_configuracion_defecto(DATOS_APLICACION *datosApp);
/**
 * @fn esp_err_t guardar_configuracion(DATOS_APLICACION*, char*, char*)
 * @brief Esta funcion recibe la configuracion json en formato texto y la guarda en la etiqueta nvs.
 *
 * @pre
 * @post
 * @param datosApp es la estructura de la aplicacion
 * @param clave es la clave nvs para guardar la configuracion
 * @param configuracion Es el texto json guardado con la configuracion.
 * @return ESP_OK si la operacion se realiza correctamente.
 */
esp_err_t guardar_configuracion(DATOS_APLICACION *datosApp, char * clave, char* configuracion);
/**
 * @fn esp_err_t guardar_programas(DATOS_APLICACION*, char*)
 * @brief Esta funcion guarda la configuracion de programas en la clave nvs asociada a la programacion
 *
 * @pre
 * @post
 * @param datosApp es la estructura de la aplicacion
 * @param clave es la clave nvs para la programacion.
 * @return ESP_OK si la operacion se realiza correctamente.
 */
esp_err_t guardar_programas(DATOS_APLICACION *datosApp, char* clave);
// Esta deberia ir al componente dialogo_json

//esp_err_t insertar_programa(DATOS_APLICACION *datosApp, char* programas) ;
/**
 * @fn esp_err_t cargar_programas(DATOS_APLICACION*, char*)
 * @brief Esta funcion carga en la estructura TIME_PROGRAM la configuracion leida desde nvs
 *
 * @pre
 * @post
 * @param datosAppes la estructura de la aplicacion
 * @param programas es la configuracion json leida desde nvs
 * @return
 */
esp_err_t cargar_programas(DATOS_APLICACION *datosApp, char* programas);
/**
 * @fn esp_err_t crear_json_configuracion(DATOS_APLICACION*, cJSON*)
 * @brief  Esta funcion lee la configuracion json leida desde nvs y la carga en la estructura DATOS_APLICACION.
 *
 * @pre
 * @post
 * @param datosApp es la estructura de la aplicacion
 * @param conf es la configuracion json leida desde nvs
 * @return
 */
esp_err_t configuracion_a_json(DATOS_APLICACION *datosApp, cJSON *conf);



esp_err_t salvar_configuracion_general(DATOS_APLICACION *datosApp);

esp_err_t crear_programas_defecto(DATOS_APLICACION *datosApp);
/**
 * @brief Esta funcion extrae la configuracion ota y la pasa a json
 *
 * @param datosApp es la estructura de la aplicacion
 * @param ota
 *
 */
esp_err_t ota_a_json(DATOS_APLICACION *datosApp);
esp_err_t json_a_ota(DATOS_APLICACION *datosApp);
/**
 * Chequea si la configuracion del dispositivo es de fabrica.
 */
esp_err_t configurado_de_fabrica();


#endif /* COMPONENTS_CONFIGURACION_INCLUDE_CONFIGURACION_H_ */
