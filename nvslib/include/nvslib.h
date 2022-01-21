/**
Proyecto realizado por Emilio Jimenez Prieto
*/

/*
 * nvs.h
 *
 *  Created on: 10 sept. 2020
 *      Author: t126401
 */

#include "nvs_flash.h"





#ifndef COMPONENTS_NVS_INCLUDE_NVS_H_
#define COMPONENTS_NVS_INCLUDE_NVS_H_

/**
 * @fn esp_err_t inicializar_nvs(char* namespace, nvs_handle *handle)
 * @brief Esta funcion inicializa la memoria nvs para su uso
 *
 * @pre
 * @post
 * @param namespace Nombre del espacio que se va a utilizar para la configuracion.
 * @param handle usado para la gestion de las acciones sobre nvs
 * @return
 */
esp_err_t inicializar_nvs(char* namespace, nvs_handle *handle);
/**
 * @fn esp_err_t leer_dato_string_nvs(nvs_handle*, char*, char*, size_t)
 * @brief
 *
 * @pre
 * @post
 * @param handle Referencia para el uso de nvs
 * @param clave Valor de la clave a leer sobre la memoria nvs
 * @param valor Valor de vuelta con el valor leido.
 * @param longitud Es la longitud de los datos.
 * @return ESP_OK en caso de que la operacion sea correcta.
 */
esp_err_t leer_dato_string_nvs(nvs_handle *handle, char* clave, char* valor, size_t longitud);
/**
 * @fn esp_err_t escribir_dato_string_nvs(nvs_handle*, char*, char*)
 * @brief Funcion para escribir una configuracion sobre la memoria nvs
 *
 * @pre
 * @post
 * @param handle Referencia para el uso de nvs
 * @param clave Valor de la clave a leer sobre la memoria nvs
 * @param valor valor a escribir en la memoria nvs
 * @return
 */
esp_err_t escribir_dato_string_nvs(nvs_handle *handle, char* clave, char* valor);
/**
 * @fn esp_err_t cerrar_nvs(nvs_handle)
 * @brief Funcion para cerrar la gestion sobre la memoria nvs.
 *
 * @pre
 * @post
 * @param handle Referencia para el uso de nvs
 * @return
 */
esp_err_t cerrar_nvs(nvs_handle handle);
esp_err_t borrar_clave(nvs_handle *handle, char* clave);



#endif /* COMPONENTS_NVS_INCLUDE_NVS_H_ */
