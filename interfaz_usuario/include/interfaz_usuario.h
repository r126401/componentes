/*
 * interfaz_usuario.h
 *
 *  Created on: 21 sept. 2020
 *      Author: t126401
 */

#ifndef COMPONENTS_INTERFAZ_USUARIO_INCLUDE_INTERFAZ_USUARIO_H_
#define COMPONENTS_INTERFAZ_USUARIO_INCLUDE_INTERFAZ_USUARIO_H_

#include "configuracion_usuario.h"
#include "programmer.h"




/**
 * @fn esp_err_t appUser_analizarComandoAplicacion(cJSON*, int, DATOS_APLICACION*, cJSON*)
 * @brief Funcion llamada desde analizar_comando cuando el idComando > 50. Son comandos especificos del dispositivo concreto.
 *
 *
 * @pre
 * @post
 * @param peticion Es el texto de la peticion recibida
 * @param nComando Es la identidad el comando
 * @param datosApp Es la estructura de la aplicacion
 * @param respuesta Es parametro de vuelta con la informacion del comando despues de procesar por el dispositivo
 * @return ESP_OK si no ha habido ningun error sintactico o semantico
 */
esp_err_t appUser_analizarComandoAplicacion(cJSON *peticion, int nComando, DATOS_APLICACION *datosApp, cJSON *respuesta);
/**
 * @fn esp_err_t appuser_temporizador_cumplido(DATOS_APLICACION*)
 * @brief Esta funcion es invocada cuando se llega al temporizador activo. En esta funcion el usuario puede definir que acciones
 * específicas se realizan sobre el dispositivo cuando cumple el temporizador.
 *
 * @pre
 * @post
 * @param datosApp Es la estructura de la aplicacion
 * @return ESP_OK cuando el resultado es ok
 */
esp_err_t appuser_temporizador_cumplido(DATOS_APLICACION *datosApp);
/**
 * @fn esp_err_t appuser_inicializar_aplicacion(DATOS_APLICACION*)
 * @brief Esta funcion es llamada desde la aplicacion de usuario y determina las acciones de inicializacion del despositivo especifico.
 *
 * @pre
 * @post
 * @param datosApp es la estructura de aplicacion
 * @return ESP_OK cuando la inicializacion del dispositivo especifico es correcta
 */
esp_err_t appuser_inicializar_aplicacion(DATOS_APLICACION *datosApp);
/**
 * @fn esp_err_t appuser_wifi_conectando()
 * @brief El usuario puede indicar acciones en el dispositivo para notificar el evento.
 *
 * @pre
 * @post
 * @return
 */
esp_err_t appuser_wifi_conectando();
/**
 * @fn esp_err_t appuser_wifi_conectado()
 * @brief El usuario puede indicar acciones en el dispositivo para notificar el evento.
 *
 * @pre
 * @post
 * @return
 */
esp_err_t appuser_wifi_conectado();
/**
 * @fn esp_err_t appuser_broker_conectando()
 * @brief El usuario puede indicar acciones en el dispositivo para notificar el evento.
 *
 * @pre
 * @post
 * @return
 */
esp_err_t appuser_broker_conectando();
/**
 * @fn esp_err_t appuser_broker_conectado()
 * @brief El usuario puede indicar acciones en el dispositivo para notificar el evento.
 *
 * @pre
 * @post
 * @return
 */
esp_err_t appuser_broker_conectado();
/**
 * @fn esp_err_t appuser_broker_desconectado()
 * @brief El usuario puede indicar acciones en el dispositivo para notificar el evento.
 *
 * @pre
 * @post
 * @return
 */
esp_err_t appuser_broker_desconectado();
/**
 * @fn esp_err_t appuser_obteniendo_sntp()
 * @brief El usuario puede indicar acciones en el dispositivo para notificar el evento.
 *
 * @pre
 * @post
 * @return
 */
esp_err_t appuser_obteniendo_sntp();
/**
 * @fn esp_err_t app_user_error_obteniendo_sntp()
 * @brief El usuario puede indicar acciones en el dispositivo para notificar el evento.
 *
 * @pre
 * @post
 * @return
 */
esp_err_t app_user_error_obteniendo_sntp();
/**
 * @fn esp_err_t appuser_sntp_ok()
 * @brief El usuario puede indicar acciones en el dispositivo para notificar el evento.
 *
 * @pre
 * @post
 * @return
 */
esp_err_t appuser_sntp_ok();
/**
 * @fn esp_err_t appuser_arranque_aplicacion(DATOS_APLICACION*)
 * @brief Esta funcion es llamada despues de la inicializacion del dispositivo y una vez suscrito al topic para que se pueda notificar
 * a la aplicacion remota que el dispositivo esta arrancado y preparado. El usuario puede definir que acciones quiere realizar
 * para notificarlo.
 *
 * @pre
 * @post
 * @param datosApp
 * @return
 */
esp_err_t appuser_arranque_aplicacion(DATOS_APLICACION *datosApp);
/**
 * @fn void appuser_ejecucion_accion_temporizada(void*)
 * @brief Esta funcion es invocada una vez expira el temporizacion de duracion de un evento de programacion. En esta funcion,
 * el usuario puede definir las acciones a realizar en el dispitivo.
 *
 * @pre
 * @post
 * @param datosApp
 */
void appuser_ejecucion_accion_temporizada(void *datosApp);
/**
 * @fn esp_err_t appuser_acciones_ota(DATOS_APLICACION*)
 * @brief Esta funcion es invocada para que el usuario pueda definir las acciones especificas en el dispositivo según el
 * dispositivo.
 *
 * @pre
 * @post
 * @param datosApp
 * @return
 */
esp_err_t appuser_acciones_ota(DATOS_APLICACION *datosApp);

esp_err_t appuser_notificar_smartconfig();

esp_err_t appuser_notificar_alarma_localmente(DATOS_APLICACION *datosApp, uint8_t indice);

esp_err_t appuser_configuracion_defecto(DATOS_APLICACION *datosApp);
esp_err_t appuser_configuracion_a_json(DATOS_APLICACION *datosApp, cJSON *conf);
esp_err_t appuser_json_a_datos(DATOS_APLICACION *datosApp, cJSON *datos);
esp_err_t appUser_ejecutar_accion_programa_defecto(DATOS_APLICACION *datosApp);
esp_err_t appuser_cargar_programas_defecto(DATOS_APLICACION *datosApp, cJSON *array);
esp_err_t appuser_extraer_datos_programa(TIME_PROGRAM *programa_actual, cJSON *nodo);
esp_err_t appuser_modificar_dato_programa(TIME_PROGRAM *programa_actual, cJSON *nodo);
esp_err_t appuser_visualizar_dato_programa(TIME_PROGRAM *programa_actual, cJSON *nodo);
esp_err_t appuser_modificarConfApp(cJSON *root, DATOS_APLICACION *datosApp, cJSON *respuesta);
esp_err_t app_user_mensaje_recibido_especifico(DATOS_APLICACION *datosApp);
//int appUser_ejecutarAccionProgramaDefecto(DATOS_APLICACION *datosApp);
void appUser_notificarWifiConectando();
void appUser_notificarWifiConectado();
void appUser_notificarWifiDesconectado();
void appUser_notificarBrokerDesconectado();
void appUser_notificarBrokerConectado();
void appUser_notificarSmartconfig();
void appUser_notificarSmartconfigStop();

//void appUser_accionesOta(DATOS_APLICACION *datosApp);
void appUser_notificarUpgradeOta();
void appuser_ConfiguracionDefecto(DATOS_APLICACION *datosApp);



//cJSON* generarReporte(DATOS_APLICACION *datosApp, enum TIPO_INFORME tipoInforme, cJSON *comandoOriginal);
cJSON* appuser_generar_informe_espontaneo(DATOS_APLICACION *datosApp, enum TIPO_INFORME tipoInforme, cJSON *comandoOriginal);
esp_err_t appuser_cargar_programa_especifico(DATOS_APLICACION *datosApp, TIME_PROGRAM *programa_actual, cJSON *nodo);
#endif /* COMPONENTS_INTERFAZ_USUARIO_INCLUDE_INTERFAZ_USUARIO_H_ */
