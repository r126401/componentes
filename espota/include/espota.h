/*
 * espota.h
 *
 *  Created on: 16 feb. 2021
 *      Author: t126401
 */

#ifndef COMPONENTS_ESPOTA_INCLUDE_ESPOTA_H_
#define COMPONENTS_ESPOTA_INCLUDE_ESPOTA_H_
#include "configuracion_usuario.h"

typedef enum ESTADO_OTA {

    OTA_ERROR,
            OTA_DESCARGA_FIRMWARE,
            OTA_BORRANDO_SECTORES,
            OTA_COPIANDO_SECTORES,
            OTA_UPGRADE_FINALIZADO,
            OTA_FALLO_CONEXION,
            OTA_DATOS_CORRUPTOS,
            OTA_PAQUETES_ERRONEOS,
            OTA_CRC_ERRONEO,
            OTA_ERROR_PARTICION,
            OTA_CABECERA_MAL_FORMADA,
            OTA_ERROR_MEMORIA_DISPONIBLE,
            OTA_ERROR_OTA_BEGIN,
            OTA_ERROR_AL_ESCRIBIR_EN_MEMORIA
} ESTADO_OTA;


void tarea_upgrade_firmware(DATOS_APLICACION *datosApp);


#endif /* COMPONENTS_ESPOTA_INCLUDE_ESPOTA_H_ */
