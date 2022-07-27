/**
Proyecto realizado por Emilio Jimenez Prieto
*/

/*
 * datos_comunes.h
 *
 *  Created on: 10 sept. 2020
 *      Author: t126401
 */





#ifndef COMPONENTS_COMUNES_INCLUDE_DATOS_COMUNES_H_
#define COMPONENTS_COMUNES_INCLUDE_DATOS_COMUNES_H_



#include "ntp.h"
#include "stdint.h"
#include "nvs_flash.h"
#include "esp_app_format.h"

char* pintar_fecha();




#define TRAZAR "%s:%s:%d-->"
#define INFOTRAZA pintar_fecha(), __func__, __LINE__

enum ESTADO_RELE {
    INDETERMINADO = -1,
    OFF = 0,
    ON = 1
};

enum TIPO_ACTUACION_RELE {
    MANUAL,
    REMOTA,
    TEMPORIZADA,
    ARRANQUE_RELE,
    EXPIRADA
};

enum ESTADO_ALARMA {
	ALARMA_INDETERMINADA = -1,
	ALARMA_OFF,
	ALARMA_WARNING,
	ALARMA_ON,


}ESTADO_ALARMA;

enum TIPO_NOTIFICACION {
	NOTIFICACION_LOCAL,
	NOTIFICACION_REMOTA
};

typedef struct ALARMA {

	uint8_t tipo_alarma;
	enum ESTADO_ALARMA estado_alarma;
	time_t fecha_alarma;
	char nemonico[50];

} ALARMA;


enum TIPO_MENSAJE_PROGRAMA {
	CREAR_TEMPORIZADOR,
	CANCELAR_TEMPORIZADOR,
	CONSULTAR_TEMPORIZADOR,
	RENOVAR_TEMPORIZADOR,
	EJECUTAR_ACCION
}TIPO_MENSAJE_PROGRAMA;

enum TIPO_INFORME {
    ARRANQUE_APLICACION,
    ACTUACION_RELE_LOCAL,
    ACTUACION_RELE_REMOTO,
    UPGRADE_FIRMWARE_FOTA,
    CAMBIO_DE_PROGRAMA,
    COMANDO_APLICACION,
    CAMBIO_TEMPERATURA,
    ESTADO,
    RELE_TEMPORIZADO,
    INFORME_ALARMA,
    CAMBIO_UMBRAL_TEMPERATURA,
	CAMBIO_ESTADO_APLICACION,
    ERROR
}TIPO_INFORME;


typedef struct OTADATA {
    char server[100];
    char url[150];
    int puerto;
    char file[50];
    const esp_app_desc_t* swVersion;
    char newVersion[32];


}OTADATA;

typedef struct MQTT_PARAM {
    char broker[100];
    int port;
    char user[25];
    char password[25];
    char publish[50];
    char subscribe[50];
    int qos;
    bool tls;
    char* cert;

} MQTT_PARAM;

enum ESTADO_APP {
    NORMAL_AUTO,
    NORMAL_AUTOMAN,
    NORMAL_MANUAL,
    NORMAL_ARRANCANDO,
    NORMAL_SIN_PROGRAMACION,
    UPGRADE_EN_PROGRESO,
	NORMAL_SINCRONIZANDO,
	ESPERA_FIN_ARRANQUE,
	ARRANQUE_FABRICA,
	NORMAL_FIN_PROGRAMA_ACTIVO



} ESTADO_APP;

enum TIPO_DISPOSITIVO {
	DESCONOCIDO = -1,
	INTERRUPTOR = 0,
	CRONOTERMOSTATO = 1,
	TERMOMETRO = 2
}TIPO_DISPOSITIVO;



typedef struct DATOS_GENERALES {
	enum TIPO_DISPOSITIVO tipoDispositivo;
    MQTT_PARAM parametrosMqtt;
    struct NTP_CLOCK clock;
    struct TIME_PROGRAM *programacion;
    uint8_t nProgramacion;
    uint8_t programa_estatico_real;
    int nProgramaCandidato;
    PROG_STATE estadoProgramacion;
    OTADATA ota;
    bool botonPulsado;
    //Estado de la aplicacion
    enum ESTADO_APP estadoApp;


}DATOS_GENERALES;







#endif /* COMPONENTS_COMUNES_INCLUDE_DATOS_COMUNES_H_ */
