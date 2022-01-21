/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ntp.h
 * Author: emilio
 *
 * Created on 20 de julio de 2017, 22:38
 */
//////////////////////////////////////////////////
// Simple NTP client for ESP8266.
// Copyright 2015 Richard A Burton
// richardaburton@gmail.com
// See license.txt for license terms.
//////////////////////////////////////////////////


/**
 * @brief reloj en tiempo real para apliacion general. Se utiliza junto con programmer.h
 * proporciona la funcionalidad de reloj en tiempo real para proposito general.
 * 
 * Para poder utilizar la libreria se necesitan los siguientes pasos.
 * 1.- Incluir la libreria ntp.h. Opcionalmente programmer.h
 * 2.- Declarar la variable extern NTP_CLOCK reloj en la aplicacion principal.
 * 3.- Llamar a initParNtp(&reloj) para configurar los parametros del reloj.
 * 4.- realClock(&reloj). Implementa un temporizador infinito de 1 sg para mantener en 
 * cualquier sistema un reloj de tiempo real.
 */

#ifndef __NTP_H__
#define __NTP_H__

#include "lwip/apps/sntp.h"



//#include "esp_common.h"
//#include "espconn.h"

// Maxima temporizacion a la espera de la respuesta ntp
//#define NTP_TIMEOUT_MS 5000
//Maximo numero de reintentos
#define MAX_RETRIES_QUERYS_NTP 5
// Tiempo entre reintentos
#define TIME_RETRY 3600

#define INTERVALO 300


/**
  * @brief     Temporizador que se activa cuando falla la consulta ntp.
  * 
  * 
  */

//static os_timer_t ntp_timeout;


/**
  * @brief     Temporizador de un segundo para generar el reloj de tiempo real.
  * 
  */



/**
  * @brief     ESTATDO_RELOJ muestra el estado actual del reloj en tiempo real.
  * SIN_HORA estado inicial del reloj cuando arranca.
  * EN_HORA: Estado despues de sincronizar hasta que expire el timeExpired para la nueva consulta
  * EXPIRADO: Estado transitorio en el cual se genera la siguiente consulta ntp para resincronizar el reloj
  * SINCRONIZANDO: Estado transitorio cuando el reloj esta tratando de sincronizar o resincronizar
 * ERROR_NTP: Estado del reloj cuando la consulta has servidor ntp ha fallado
  */



enum ESTADO_RELOJ {
	SIN_HORA, // El reloj no esta sincronizado y no esta operativo.
	SINCRONIZANDO, // El reloj esta en fase de sincronizacion
	SINCRONIZADO, // El reloj esta sincronizado pero hay operaciones a nivel de aplicacion pendientes. Es un estado transitorio.
	EN_HORA, // El reloj esta sincronizado y operativo
	DESAJUSTADO,  // el reloj ha presentado desviacion y necesita ser ajustado
	ERROR_NTP // El reloj no ha podido sincronizarse y por lo tanto la hora no es valida.
}ESTADO_RELOJ;

typedef enum PROG_STATE {
    INVALID_PROG, // La programacion no es valida. Es un estado transitorio que solo debe pasar en el arranque
    VALID_PROG, // Se han ordenado las programaciones y se pueden usar dentro de la aplicacion.
    INH_PROG // Se han inhibido todos los programas y la aplicacion no ejecutara ninguna accion programada.

} PROG_STATE;


typedef struct {
	uint8_t options;
	uint8_t stratum;
	uint8_t poll;
	uint8_t precision;
	uint32_t root_delay;
	uint32_t root_disp;
	uint32_t ref_id;
	uint8_t ref_time[8];
	uint8_t orig_time[8];
	uint8_t recv_time[8];
	uint8_t trans_time[8];
} ntp_t;

/**
  * @brief     Estructura necesaria para el funcionamiento del reloj
  * 
  * @attention 
  *
  * @param     date Contiene la hora actual en formato struct tm
  * @param     time contiene la hora actual en formato time_t
  * @param     ntpTimeZone contiene la correccion local de la hora a aplicar. De momento no se usa
  * @param     ntpTimeExp indica cada cuantos segundos se resincroniza el reloj via ntp
  * @param     ntpTimeOut indica el valor de timeout para las consultas ntp
  * @param     ntpServerActive indica el sevidor ntp actualmente configurado
  * @param     ntpServer es un Array de 5 servidores ntp alternativos
  * @param     ESTADO_RELOJ indica el estado actual del reloj
  * @param     timeValid indica si la hora es valida y se puede seguir usando en el sistema.
  */



typedef struct NTP_CLOCK {
    struct tm date; // Fecha en formato tm
    time_t time; // fecha en formato time_t
    char* ntpTimeZone; // Zona horaria a aplicar segun el pais.
    enum ESTADO_RELOJ estado;
    bool timeValid;
    
} NTP_CLOCK;                                       

/**
  * @brief     funcion llamada cuando se reciben los datos del servidor ntp actualizando la estructura
  * 
  * @attention Esta funcion deja el servidor en estado EN_HORA
  */




/**
  * @brief     Consulta para obtener la hora del servidor ntp
  * 
  * @attention 
  *
  * @param     clock estructura para guardar la hora y el estado del reloj
  * @param     Se apoya en la funcion ntp_udp_timeout para controlar las consultas erroneas al servidor ntp.
  */

void ntp_get_time(NTP_CLOCK *clock);

/**
  * @brief     Se le llama dentro de la funcion ntp_udp_recv para aplicar la zona horaria.
  * 
  * @attention 
  */

 void  applyTZ(struct NTP_CLOCK *clock);
/**
  * @brief     Logica principal del reloj en tiempo real. Se ejecuta cada segundo.
  * Implementa la logica de reintentos para sincronizar el reloj
  * 
  * @attention 
  */

void relojTiempoReal(NTP_CLOCK *clock); 
/**
  * @brief Implementa la funcion que actualiza el servidor actual ntpServerActive
  *                                                    .
  * @attention 
  *
  * @param     *clock contiene la estructura y datos del reloj
  * @param     nretries necesario para elegir un ntpServer alternativo
  * 
  *
  * @return    TRUE mientras aun quedan servidores alternativos que consultar
  * @return    FALSE cuando se han consultado todos sin exito.
  * 
  */


bool retryQueryNtp(NTP_CLOCK *clock, uint8_t *nretries);

/**
  * @brief Pone los parametros principales de configuracion del reloj.
  * Es imprescindible llamarla una vez antes de utilizar el reloj. Si no es asi no funcionara!!.
  * 
  * @param clock es la estructura del reloj.
  * @attention 
  */


void inicializar_parametros_ntp(NTP_CLOCK *clock);
/**
  * @brief Se llama dentro de initParNtp para poner el servidor activo por defecto
  * 
  * 
  * @param clock es la estructura del reloj.
  * @attention 
  */


void ntpServerDefault(NTP_CLOCK *clock);

void realClock(NTP_CLOCK *clock);
void activar_reloj(NTP_CLOCK *clock);
void desactivar_reloj(NTP_CLOCK *clock);
esp_err_t obtener_fecha_hora(NTP_CLOCK *clock);
void actualizar_hora(NTP_CLOCK *clock);



#endif
