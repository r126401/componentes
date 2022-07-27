/*
 * nvs.c
 *
 *  Created on: 10 sept. 2020
 *      Author: t126401
 */


#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvslib.h"
#include "esp_partition.h"
#include "datos_comunes.h"

static const char *TAG = "NVS";

esp_err_t inicializar_nvs(char* namespace, nvs_handle *handle) {

	esp_err_t error;
	error = nvs_flash_init();

    if (error == ESP_ERR_NVS_NO_FREE_PAGES) {
        //ESP_LOGW(TAG, ""TRAZAR"nvs_flash_init failed (0x%x), erasing partition and retrying", err);
        ESP_LOGW(TAG, ""TRAZAR"fallo en la inicializacion de nvs((0x%x). Se borra la particion y se reintenta", INFOTRAZA, error);
        const esp_partition_t* nvs_partition = esp_partition_find_first(
                ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS, NULL);
        assert(nvs_partition && "partition table must have an NVS partition");
        ESP_ERROR_CHECK( esp_partition_erase_range(nvs_partition, 0, nvs_partition->size) );
        error = nvs_flash_init();
    }
    // Se abre el espacio de nombres
    error = nvs_open(namespace, NVS_READWRITE, handle);
    if (error == ESP_OK) {
    	ESP_LOGI(TAG, ""TRAZAR"NVS abierto correctamente. handle = %d", INFOTRAZA, *handle);
    } else {
    	ESP_LOGE(TAG, ""TRAZAR"Error al abrir nvs", INFOTRAZA);
    }

	return error;
}

esp_err_t leer_dato_string_nvs(nvs_handle *handle, char* clave, char* valor, size_t longitud) {

	esp_err_t error;

	error = nvs_get_str(*handle, clave, valor, &longitud);

	if (error == ESP_OK) {
		ESP_LOGI(TAG, ""TRAZAR"clave: %s, dato leido: %s", INFOTRAZA, clave, valor);
	} else {
		ESP_LOGW(TAG, ""TRAZAR"clave no encontrada en memoria", INFOTRAZA);
	}

	return error;

}

esp_err_t escribir_dato_string_nvs(nvs_handle *handle, char* clave, char* valor) {

	esp_err_t error;

	ESP_LOGI(TAG, ""TRAZAR"vamos a escribir el dato con handle %d", INFOTRAZA, *handle);
	error = nvs_set_str(*handle, clave, valor);
	if (error == ESP_OK) {
		ESP_LOGI(TAG, ""TRAZAR"clave %s, dato escrito: %s", INFOTRAZA, clave, valor);
	} else {
		ESP_LOGW(TAG, ""TRAZAR"error al escribir en nvs", INFOTRAZA);
	}

	return error;

}

esp_err_t cerrar_nvs(nvs_handle handle) {

	esp_err_t error = ESP_OK;

	nvs_close(handle);

	error = nvs_flash_deinit();
	if (error == ESP_OK) {
		ESP_LOGI(TAG, ""TRAZAR"nvs cerrado", INFOTRAZA);
	} else {
		ESP_LOGW(TAG, ""TRAZAR"error al cerrar nvs", INFOTRAZA);
	}
	return error;
}

esp_err_t borrar_clave(nvs_handle *handle, char* clave) {

	esp_err_t error;
	error = nvs_erase_key(*handle, clave);
	if (error == ESP_OK) {
		ESP_LOGI(TAG, ""TRAZAR"clave %s borrada", INFOTRAZA, clave);
	} else {
		ESP_LOGW(TAG, ""TRAZAR"error al borrar la clave en nvs", INFOTRAZA);
	}

	return error;

}




