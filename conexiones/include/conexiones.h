/* Common functions for protocol examples, to establish Wi-Fi or Ethernet connection.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"
//#include "tcpip_adapter.h"
#include "esp_smartconfig.h"
#include "smartconfig_ack.h"
#include "configuracion_usuario.h"


/**
 * Counterpart to example_connect, de-initializes Wi-Fi or Ethernet
 */
//esp_err_t example_disconnect(void);

/**
 * @brief Configure stdin and stdout to use blocking I/O
 *
 * This helper function is used in ASIO examples. It wraps installing the
 * UART driver and configuring VFS layer to use UART driver for console I/O.
 */
//esp_err_t example_configure_stdin_stdout(void);

/**
 * @brief Configure SSID and password
 */
//esp_err_t example_set_connection_info(const char *ssid, const char *passwd);

/**
 * @fn const char get_my_id*(void)
 * @brief Funcion para extraer la MAC del dispositivo
 *
 * @pre
 * @post
 * @return
 */
const char * get_my_id(void);
//void sc_callback(smartconfig_status_t status, void *pdata);
void tarea_smartconfig(void * parm);
esp_err_t establecer_conexion_wifi(void);
esp_err_t restaurar_wifi_fabrica();
esp_err_t conectar_dispositivo_wifi();



#ifdef __cplusplus
}
#endif
