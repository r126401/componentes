/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "programmer.h"
#include "configuracion_usuario.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
//#include "FreeRTOS.h"
#include "esp_err.h"
#include "errores_proyecto.h"
#include "interfaz_usuario.h"

//static ETSTimer timerDuracion;
static const char *TAG = "PROGRAMADOR";
static ETSTimer temporizador_duracion;

#define TEMPORIZADOR_MAXIMO_EN_SEGUNDOS 428
//#define TEMPORIZADOR_MAXIMO_EN_SEGUNDOS 30



//#define uint8_t uint8_t_t
static ETSTimer temporizador;

struct TIME_PROGRAM*  insertarElemento(struct TIME_PROGRAM *listProgram, uint8_t *elements) {
    
    TIME_PROGRAM *list;

    //Si es el primer elemento, se crea la lista
    if ((listProgram == NULL) || (*elements == 0)) {
        list = (TIME_PROGRAM*) calloc(1,sizeof(*listProgram));
        (*elements) ++;
       //printf("reservados : %d\n", sizeof(*list));
    } else {
        //Se añade un elemento mas
        (*elements) ++;
        list = (TIME_PROGRAM*) realloc(listProgram, sizeof(*list) * (*elements) );
        //printf("reservados : %d\n", sizeof(*list) * (*elements));
    }
    
    if (list == NULL) {
        printf("insertarElemento--> Error al crear el elemento\n");
        return NULL;
    } else {
        printf("insertarElemento--> Elemento creado o anadido\n");

        return list;
                
    }
    
    
}

/**
 * Esta funcion ordena los tiempos time_t en orden decreciente. Siendo el mas reciente el primero de la lista.
 */
int compararDatos(struct TIME_PROGRAM *dat1, struct TIME_PROGRAM *dat2) {
    
    if (dat1->programa < dat2->programa)
        return -1;
    else if(dat1->programa > dat2->programa)
        return 1;
    else return 0;
    
}


TIME_PROGRAM* crearPrograma(TIME_PROGRAM *peticion, TIME_PROGRAM *listaProgramas, uint8_t *nElementos) {
    
    TIME_PROGRAM *list;
    uint8_t elem;
    elem = *nElementos;
    generarIdPrograma(peticion);
    //Si se encuentra duplicado, no se crea el programa
    if (buscarProgramaDuplicado(peticion->idPrograma, listaProgramas, elem) == true) {
        printf("crearPrograma-->Programa no creado\n");
        return listaProgramas;
    }
    
    //Primero insertamos el elemento en la lista
    list = insertarElemento(listaProgramas, (uint8_t*)nElementos);
    if (list == NULL) return NULL;
    
    
    //Lo actualizamos con la informacion de la peticion.
    memcpy(&list[elem], peticion, sizeof(*list));
    return list;
        
        
    }
    
uint8_t InicializarListaProgramas(TIME_PROGRAM *listaProgramas, int nElementos, struct NTP_CLOCK *clock) {

    int i;
    for(i=0;i<nElementos;i++) {
        if (listaProgramas[i].tipo == SEMANAL) {
            listaProgramas[i].programacion.tm_mday = listaProgramas[i].programacion.tm_mday +7;
        }
    } 
    return 0;
    
    
}
    
bool calcularMascara(int diaActual, int mascara) {
    
    int i;
    int j=1;
    int k;
    printf("calcularMascara--> dia actual: %d, mascara : %d\n", diaActual, mascara);
    for (i=0;i<diaActual;i++) {
        
        j = j << 1;      
    }
    
    
    k= j & mascara;
    
    if (k == j) {
        printf("calcularMascara-->programacion activa %d\n", k);
        return true;
    } else {
        printf("calcularMascara-->programacion inactiva %d\n", k);
        return false;
    }

}




esp_err_t ordenarListaProgramas(TIME_PROGRAM *listaProgramas, int nElementos, struct tm clock) {
    
    int i;
    
    // Añado los atributos necesarios en funcion del tipo de programacion para convertirlo en time_t valido.
    
    for(i=0;i<nElementos;i++) {
        //listaProgramas[i].programacion.tm_year = clock.tm_isdst;
        printf("ordenarListaProgramas--> El ano es: %d\n", clock.tm_year);
        switch (listaProgramas[i].tipo) {
            case DIARIA:
                listaProgramas[i].programacion.tm_year = clock.tm_year;
                listaProgramas[i].programacion.tm_mon = clock.tm_mon;
                listaProgramas[i].programacion.tm_mday = clock.tm_mday;
                listaProgramas[i].programacion.tm_wday = clock.tm_wday;
                if (calcularMascara(clock.tm_wday, listaProgramas[i].mascara) == true) {
                    listaProgramas[i].activo = 1;
                    ESP_LOGW(TAG, ""TRAZAR" PROGRAMA CON MASCARA ACTIVA", INFOTRAZA);
                } else {
                    listaProgramas[i].activo = 0;
                    ESP_LOGW(TAG, ""TRAZAR" PROGRAMA CON MASCARA INACTIVA", INFOTRAZA);
                }
                
                listaProgramas[i].programacion.tm_yday = clock.tm_yday;
                break;
                
            case SEMANAL:
                listaProgramas[i].programacion.tm_year = clock.tm_year;
                
                if (clock.tm_wday > listaProgramas[i].programacion.tm_wday) {
                   // printf("dia de la semana actual > programa, %d, %d \n", (clock.tm_wday - listaProgramas[i].programacion.tm_wday),
                     //       (clock.tm_wday - listaProgramas[i].programacion.tm_wday));
                    listaProgramas[i].programacion.tm_yday = clock.tm_yday - (clock.tm_wday - listaProgramas[i].programacion.tm_wday);
                    listaProgramas[i].programacion.tm_mday = clock.tm_mday - (clock.tm_wday - listaProgramas[i].programacion.tm_wday);
                    listaProgramas[i].programacion.tm_mon = clock.tm_mon;
                    //Si ya ha pasado la programacion, le sumamos 7 dias
                    //listaProgramas[i].programacion.tm_mday = listaProgramas[i].programacion.tm_mday;
                } else if (clock.tm_wday < listaProgramas[i].programacion.tm_wday) {
                  //  printf("dia de la semana menor que el programado. Sumamos %d dias\n", (listaProgramas[i].programacion.tm_wday- clock.tm_wday));
                    listaProgramas[i].programacion.tm_yday = clock.tm_yday + (listaProgramas[i].programacion.tm_wday - clock.tm_wday);
                    listaProgramas[i].programacion.tm_mday = clock.tm_mday + (listaProgramas[i].programacion.tm_wday - clock.tm_wday);
                    listaProgramas[i].programacion.tm_mon = clock.tm_mon;
                    
                } else {
                    
                    listaProgramas[i].programacion.tm_yday = clock.tm_yday;
                    listaProgramas[i].programacion.tm_mday = clock.tm_mday;
                    listaProgramas[i].programacion.tm_mon = clock.tm_mon;

                    
                }
                break;
                
            case FECHADA:
                printf("ordenarListaProgramas--> Pinto la fechada\n");
                //listaProgramas[i].programacion.tm_yday = 0;
                break;
                
        }
        // Convierto la fecha calculada en un time_t para ordenarlo despues.
        //Adapto la hora a la zona horaria
        //listaProgramas[i].programa = localtime_r(&listaProgramas[i].programacion);
        listaProgramas[i].programacion.tm_isdst = -1;
        listaProgramas[i].programa = mktime(&listaProgramas[i].programacion);

        //ESP_LOGI(TAG, ""TRAZAR"ordenarListaProgramas-->tPrograma : %ld, tActual: %ld, dif: %ld\n", listaProgramas[i].programa, clock->time,(listaProgramas[i].programa - clock->time) );
        
    }
    // Una vez rellena toda la lista de programacion, se ordena de menor a mayor.
    qsort(listaProgramas, nElementos, sizeof(TIME_PROGRAM),(compfn) compararDatos);
    //despues de ordenar
    //printf("ordenarListaProgramas-->Despues de ordenar...\n");
    
    for(i=0;i<nElementos;i++) {
        ESP_LOGI(TAG, ""TRAZAR"ordenarListaProgramas-->%d/%02d/%02d %d %02d %02d:%02d:%02d %d time_t = %ld", INFOTRAZA,
                listaProgramas[i].programacion.tm_year + 1900,
                listaProgramas[i].programacion.tm_mon +1,
                listaProgramas[i].programacion.tm_mday,
                listaProgramas[i].programacion.tm_yday,
                listaProgramas[i].programacion.tm_wday,
                listaProgramas[i].programacion.tm_hour,
                listaProgramas[i].programacion.tm_min,
                listaProgramas[i].programacion.tm_sec,
                listaProgramas[i].programacion.tm_isdst,
				listaProgramas[i].programa);
        
    }
    
    
    
    return ESP_OK;
}

int calcularProgramaActivoActual(TIME_PROGRAM *listaProgramas, int nElementos, int actual) {
    
    int i;

    
    for (i=actual; i > -1; i--) {
        printf("calcularProgramaActivoActual-->actual: %d\n", i);
        if ((listaProgramas[i].activo == 0) || (listaProgramas[i].estadoPrograma == INACTIVO)) {
            visualizartiempo(listaProgramas[i].programacion);
             actual --;
           
        } else {
            printf("calcularProgramaActivoActual-->encontrado actual: %d\n", i);
            return i;
        }
    }
    printf("calcularProgramaActivoActual--> actual no encontrado. Retornado: %d\n", i);
    return i;
    

    
}

bool esActivoElPrograma(TIME_PROGRAM *listaProgramas, int nElementos, int indice) {
    
    if ((listaProgramas->activo == 1) && (listaProgramas->estadoPrograma == ACTIVO)) {
        printf("esActivoElPrograma--> activo, idPrograma: %s, indice:%d\n", listaProgramas->idPrograma, indice);
        
        return true;
    } else {
        printf("esActivoElPrograma--> inactivo, idPrograma: %s, indice: %d\n", listaProgramas->idPrograma, indice);
        return false;
    }
    
    
}






void visualizartiempo(struct tm tiempo) {

    ESP_LOGI(TAG, ""TRAZAR"%02d/%02d/%d %02d:%02d:%02d diaSem: %d, dias del ano: %d, ", INFOTRAZA,
    		tiempo.tm_mday, tiempo.tm_mon + 1, (tiempo.tm_year + 1900),
			tiempo.tm_hour, tiempo.tm_min, tiempo.tm_sec, tiempo.tm_wday,
			tiempo.tm_yday);

               
               
}

struct TIME_PROGRAM * borrarPrograma(struct TIME_PROGRAM *listProgram, uint8_t *elements, uint8_t element) {
    
    uint8_t i;
    struct TIME_PROGRAM *list = NULL;
    
    /*
    uint8_t nProgramas=0;
    //printf("tamaño antes: %d\n", sizeof(*listProgram));
    printf("borrarPrograma-->posicion encontrada:%d, totalElementos:%d\n", element, *elements);
    for (i=0;i<*elements;i++) {
        if (i != element) {
            list = crearPrograma(listProgram[i], list, &nProgramas);
        } else {
            printf("borrarPrograma-->elemento %d borrado\n", i);
            
        }
    }
    *elements = nProgramas;
    free(listProgram);
    return list;
    
    
   */
    
    for (i=element; i< *elements - 1; i++) {
        memcpy(listProgram+i, listProgram+i+1, sizeof(*listProgram));
        printf("pongo el elemento %d en el %d", i, i+1);
    }
    *elements = *elements - 1;
    
    list = (TIME_PROGRAM*) realloc(listProgram, sizeof(*listProgram) * (*elements) );
    
    listProgram = list;
    return list;
 
}


uint8_t modificarElemento(struct TIME_PROGRAM *listProgram, uint8_t elements, struct TIME_PROGRAM *idPrograma) {
    
    uint8_t i;
    
    for(i=0;i<elements;i++) {
        if (idPrograma->programa == listProgram[i].programa) {
            memcpy(&(listProgram[i]), idPrograma, sizeof(struct TIME_PROGRAM));
            printf("modificarElemento--> Programacion modificada\n");
            return 0;
        }
    }
    
    
    return -1;
}

bool buscarProgramaDuplicado(char* idPrograma, struct TIME_PROGRAM *program, uint8_t elements) {
    
    if(elements == 0) {
        printf("buscarProgramaDuplicado--> primer elemento, se devuelve false\n");
        return false;
    }
    
    if (localizarProgramaPorId(idPrograma, program, elements) < 0 ) {
        return false;
    } else {
        printf("buscarProgramaDuplicado--> Registro %s duplicado. No se inserta\n", idPrograma);
        return true;
    }
/*
    uint8_t i;
    
    for(i=0;i<elements;i++) {
        printf("idPrograma: %s, idProgramaInsertado:%s\n", idPrograma, program[i].idPrograma);
        if (strcmp(idPrograma, program[i].idPrograma) == 0) {
            printf("buscarProgramaDuplicado:Registro %s duplicado. No se inserta\n", idPrograma);
            return false;
        }
    }
*/    
    return true;
}

TIME_PROGRAM* generarIdPrograma(struct TIME_PROGRAM *program) {
    
    char mascara[3];
    uint8_t i;
    
    for (i=0;i<19;i++) {
        program->idPrograma[i] = '\0';
    }
    
    
    switch (program->tipo) {
        case DIARIA:
            sprintf(mascara, "%x", program->mascara);
            printf("generarIdPrograma-->Generando diaria\n");
            sprintf(program->idPrograma, "%02d%02d%02d%02d%s",
                program->tipo, program->programacion.tm_hour,
                program->programacion.tm_min,program->programacion.tm_sec, mascara );
            break;
            
        case SEMANAL:
            printf("generarIdPrograma-->Generando semanal\n");
            sprintf(program->idPrograma, "%02d%02d%02d%02d%02d", 
                program->tipo, program->programacion.tm_hour,
                program->programacion.tm_min,program->programacion.tm_sec,
                program->programacion.tm_wday);
            break;
            
        case FECHADA:
            printf("generarIdPrograma-->Generando fechada\n");
            sprintf(program->idPrograma, "%02d%02d%02d%02d%04d%02d%02d", 
                program->tipo, program->programacion.tm_hour,
                program->programacion.tm_min,program->programacion.tm_sec,
                program->programacion.tm_year, program->programacion.tm_mon,
                program->programacion.tm_mday);

            break;
            
        default:
            break;
    }
    printf("generarIdPrograma--> Generado id:%s\n", program->idPrograma);
    visualizartiempo(program->programacion);
    
    return program;
    

    
}

int localizarProgramaPorId(char *idPrograma, struct TIME_PROGRAM *program, uint8_t elements) {

    uint8_t i;
    char *cadena1;
    char *cadena2;
    char *tipo;
    cadena1 = (char*) calloc(19, sizeof(char));
    cadena2 = (char*) calloc(19, sizeof(char));
    tipo = (char*) calloc(3, sizeof(char));
    
    strncpy(tipo, idPrograma, 2);
    printf("localizarProgramaPorId--> tipo programa es %s\n", tipo);
    if ((strcmp(tipo, "00") == 0) || (strcmp(tipo, "01") == 0)) {
        //Diaria o semanal
        printf("localizarProgramaPorId--> Programacion diaria o semanal\n");
        strncpy(cadena1, idPrograma, 8);
        for(i=0;i<elements;i++) {
            strncpy(cadena2, program[i].idPrograma, 8);
             printf("localizarProgramaPorId--> comparamos %s con %s\n", cadena1, cadena2);
            if (strcmp(cadena1, cadena2) == 0) {
                free(cadena1);
                free(cadena2);
                free(tipo);
                printf("localizarProgramaPorId--> registro duplicado\n");
                return i;
            }
    }
        
        
    } else {
        //Fechada
        printf("localizarProgramaPorId--> Programacion fechada\n");
        strcpy(cadena1, idPrograma);
        for(i=0;i<elements;i++) {
            strcpy(cadena2, program[i].idPrograma);
             printf("localizarProgramaPorId--> comparamos %s con %s\n", cadena1, cadena2);
            if (strcmp(cadena1, cadena2) == 0) {
                free(cadena1);
                free(cadena2);
                free(tipo);
                printf("localizarProgramaPorId--> registro duplicado\n");
                return i;
            }
    }
        
        
    }

    printf("localizarProgramaPorId--> registro no localizado\n");
    free(cadena1);
    free(cadena2);
    free(tipo);
    return -1;
    
    
    
/*

    for(i=0;i<elements;i++) {
        strcpy(cadena2, program[i].idPrograma);
         printf("localizarProgramaPorId--> comparamos %s con %s\n", cadena1, cadena2);
        if (strcmp(cadena1, cadena2) == 0) {
            free(cadena1);
            free(cadena2);
            printf("localizarProgramaPorId--> registro duplicado\n");
            return i;
        }
    }
    printf("localizarProgramaPorId--> registro no localizado\n");
    free(cadena1);
    free(cadena2);
    return -1;
*/
    
}


void chequear_ejecucion_programa(NTP_CLOCK *clock, TIME_PROGRAM *programs) {

	ESP_LOGI(TAG, ""TRAZAR"Chequeando si cumple algun temporizador", INFOTRAZA);

}

/*
void accionTemporizada(DATOS_APLICACION *datosApp) {

    int tiempo;
    uint32_t temporizacion;
    int programaActual= -1000;
    time_t now;
    struct tm clock;

    time(&now);
    localtime_r(&now, &clock);

    ordenarListaProgramas(datosApp->datosGenerales->programacion, datosApp->datosGenerales->nProgramacion, clock);
    //buscarPrograma(datosApp, &programaActual);
    //buscarprogramaActivo(datosApp->datosGenerales->nProgramacion, &datosApp->datosGenerales->nProgramaCandidato, &datosApp->datosGenerales->clock, datosApp->datosGenerales->programacion, &programaActual);
    // Es el tiempo que se ha de poner al intervalo total.
    tiempo = (datosApp->datosGenerales->programacion[programaActual].programa +
                datosApp->datosGenerales->programacion[programaActual].duracion ) -
                datosApp->datosGenerales->clock.time;

    ESP_LOGI(TAG, ""TRAZAR"accionTemporizada--> La duracion propuesta es de %d\n", INFOTRAZA, datosApp->datosGenerales->programacion[programaActual].duracion);

    if (datosApp->datosGenerales->programacion[programaActual].duracion > 0)  {


        if (tiempo > 0 ) {

            if (tiempo >= INTERVALO) {
                temporizacion = INTERVALO;
                ESP_LOGI(TAG, ""TRAZAR"accionTemporizada--> La duracion es de %d\n", INFOTRAZA, temporizacion);
                ets_timer_disarm(&timerDuracion);
                ets_timer_setfn(&timerDuracion, (ETSTimerFunc*)accionTemporizada, datosApp);
                ets_timer_arm(&timerDuracion, temporizacion*1000, false);

            } else {
                temporizacion = tiempo;
                //CreamosTemporizador para reposar el rele tras ese tiempo.
                ESP_LOGI(TAG,"accionTemporizada--> La duracion es de %d\n", temporizacion);
                ets_timer_disarm(&timerDuracion);
                ets_timer_setfn(&timerDuracion, (ETSTimerFunc*)appuser_ejecucion_accion_temporizada, datosApp);
                ets_timer_arm(&timerDuracion, temporizacion*1000, false);

            }
        } else {
            ESP_LOGI(TAG,"accionTemporizada--> Ya paso el intervalo de duracion\n");
            appuser_ejecucion_accion_temporizada(datosApp);
        }
    }
}
*/
/*
int ajustarProgramacion(TIME_PROGRAM *programacion, uint8_t *nProgramacion, NTP_CLOCK *clock) {

    bool res = false;
    int actual= -1;

    ESP_LOGI(TAG,"ajustarProgramacion--> inicio\n");
    ordenarListaProgramas(programacion, nProgramacion, clock);
    ESP_LOGI(TAG,"ajustarProgramacion-->voy a buscar programa\n");
    buscarprogramaActivo(nProgramacion, nProgramaCandidato, clock, programacion, &actual);
    ESP_LOGI(TAG,"ajustarProgramacion-->he buscado el programa\n");
    ESP_LOGI(TAG,"ajustarProgramacion: actual %d, futuro %u\n", actual, ProgramaCandidato);

    if (res == true) {
        appUser_ejecutarAccionPrograma(datosApp, actual);
        accionTemporizada(datosApp);

    } else {
        appUser_ejecutarAccionProgramaDefecto(datosApp);
    }

    return datosApp->datosGenerales->nProgramaCandidato;

}

*/

esp_err_t buscar_programa(TIME_PROGRAM *programas, int elementos, int *programa_actual, time_t *t_tiempo_siguiente) {


	NTP_CLOCK hora;
	actualizar_hora(&hora);
	time_t hora_actual = hora.time;
	int numero_programas;
	struct tm tm_dia_siguiente;
	tm_dia_siguiente = hora.date;
	numero_programas = 0;
	int i;




	if (elementos <= 0) {
		ESP_LOGW(TAG, ""TRAZAR"No hay programas registrados", INFOTRAZA);
		*programa_actual = -2;
		*t_tiempo_siguiente = -1;
		return PROGRAMACION_NO_EXISTE;
	}

	ESP_LOGW(TAG, ""TRAZAR"PASAMOS POR EL ELEMENTO" , INFOTRAZA);
	for (i=(elementos - 1);i>-1;i--) {
		numero_programas++;
		ESP_LOGW(TAG, ""TRAZAR"PASAMOS POR EL ELEMENTO %d", INFOTRAZA, i);
		if((hora_actual >= programas[i].programa) &&
				(programas[i].estadoPrograma == ACTIVO) && (programas[i].activo == ACTIVO)){
			ESP_LOGE(TAG, ""TRAZAR"Puntero asignado al indice %d. Hora actual: %ld >=  %ld", INFOTRAZA, i, hora.time, programas[i].programa);
			*programa_actual = i;
			break;

		} else {
			ESP_LOGW(TAG, ""TRAZAR"elemento %d no es seleccionado, se pasa al siguiente", INFOTRAZA, i);
		}
	}
/*
	do {
		elementos--;
		ESP_LOGI(TAG, ""TRAZAR"PASAMOS POR EL ELEMENTO %d", INFOTRAZA, elementos);
		if((hora_actual >= programas[elementos].programa) &&
				(programas[elementos].estadoPrograma == ACTIVO)){
			ESP_LOGI(TAG, ""TRAZAR"ELEMENTO ENCONTRADO. VEMOS SI SUPERA LA DURACION", INFOTRAZA);
			if((programas[elementos].programa + programas[elementos].duracion ) > hora_actual) {
				ESP_LOGI(TAG, ""TRAZAR"Puntero asignado al indice %d. Hora actual: %ld >=  %ld", INFOTRAZA, elementos, hora.time, programas[elementos].programa);
				*programa_actual = elementos;
				break;
			}

		}

	} while (elementos >= 0);
	*/

	// Si todos los elementos son ha pasado, calculamos el temporizador hasta las 00 horas del dia siguiente
	// para ordenar la lista y buscar el nuevo temporizador restante.

	ESP_LOGW(TAG, ""TRAZAR"PROGRAMA ACTUAL VALE %d y el numero de programas es %d", INFOTRAZA, *programa_actual, numero_programas);


	if(i == numero_programas) {
		tm_dia_siguiente.tm_mday++;
		tm_dia_siguiente.tm_hour = 0;
		tm_dia_siguiente.tm_min = 0;
		tm_dia_siguiente.tm_sec = 0;
		*t_tiempo_siguiente = mktime(&tm_dia_siguiente);
		ESP_LOGW(TAG, ""TRAZAR" EL PROXIMO INTERVALO ES %ld y se corresponde con la media noche", INFOTRAZA, *t_tiempo_siguiente);


	} else {

		*t_tiempo_siguiente = programas[i+1].programa;




	}



	ESP_LOGI(TAG, ""TRAZAR"programa activo: %d, tiempo del siguiente programa: %ld", INFOTRAZA, i, *t_tiempo_siguiente);
	if (i < 0) {
		return -1;
	} else {
		return ESP_OK;
	}



}

esp_err_t calcular_programa_activo(DATOS_APLICACION *datosApp, time_t *t_siguiente_intervalo) {

	esp_err_t error;

	ESP_LOGI(TAG, ""TRAZAR"CALCULAR_PROGRAMA_ACTIVO...", INFOTRAZA);
	datosApp->datosGenerales->nProgramaCandidato = -1;
	ordenarListaProgramas(datosApp->datosGenerales->programacion, datosApp->datosGenerales->nProgramacion, datosApp->datosGenerales->clock.date);
	error = buscar_programa(datosApp->datosGenerales->programacion,
			datosApp->datosGenerales->nProgramacion,
			&datosApp->datosGenerales->nProgramaCandidato,
			t_siguiente_intervalo);


	if (error == ESP_OK) {
		ESP_LOGI(TAG, ""TRAZAR"El elemento seleccionado es %d", INFOTRAZA, datosApp->datosGenerales->nProgramaCandidato);
		visualizartiempo(datosApp->datosGenerales->programacion[datosApp->datosGenerales->nProgramaCandidato].programacion);
		//actualizar_programa_real(datosApp);
		activacion_programa(datosApp);

	} else {
		error = PROGRAMACION_NO_EXISTE;
		ESP_LOGW(TAG, ""TRAZAR"No hay programacion o programa activo en este momento", INFOTRAZA);
		appUser_ejecutar_accion_programa_defecto(datosApp);
	}

	return error;

}


void gestion_programas(DATOS_APLICACION *datosApp) {

	struct tm hora;
	actualizar_hora(&datosApp->datosGenerales->clock);
	hora = datosApp->datosGenerales->clock.date;
	static time_t t_siguiente_intervalo;


	switch(datosApp->datosGenerales->estadoApp) {

	case NORMAL_ARRANCANDO:
		ESP_LOGW(TAG, ""TRAZAR"ESTAMOS EN NORMAL ARRANCANDO", INFOTRAZA);
		if (calcular_programa_activo(datosApp, &t_siguiente_intervalo) == ESP_OK) {
			//appuser_cambiar_modo_aplicacion(datosApp, NORMAL_AUTO);
			datosApp->datosGenerales->estadoApp = NORMAL_AUTO;
			//appUser_temporizador_cumplido(datosApp);
		} else {
			ESP_LOGW(TAG, ""TRAZAR"NO HAY PROGRAMAS REGISTRADOS", INFOTRAZA);
			appuser_cambiar_modo_aplicacion(datosApp, NORMAL_SIN_PROGRAMACION);
			//datosApp->datosGenerales->estadoApp = NORMAL_MANUAL;
		}
		appuser_cambiar_modo_aplicacion(datosApp, ESPERA_FIN_ARRANQUE);
		//datosApp->datosGenerales->estadoApp = ESPERA_FIN_ARRANQUE;
		break;

	case NORMAL_AUTO:
	case NORMAL_AUTOMAN:
		if((hora.tm_hour == 0) && (hora.tm_min == 0) && (hora.tm_sec == 0)) {
			calcular_programa_activo(datosApp, &t_siguiente_intervalo);
		}
		if (datosApp->datosGenerales->clock.time == t_siguiente_intervalo){
			ESP_LOGI(TAG, ""TRAZAR"AQUI HABRIA CUMPLIDO EL TEMPORIZADOR", INFOTRAZA);
			//appUser_temporizador_cumplido(datosApp);
			if (calcular_programa_activo(datosApp, &t_siguiente_intervalo) != ESP_OK) {
				ESP_LOGW(TAG, ""TRAZAR"NO HAY PROGRAMAS ALMACENADOS", INFOTRAZA);
			}

		} else {
			//ESP_LOGI(TAG, ""TRAZAR"HORA: %ld. siguiente intervalo: %ld, diff: %ld", INFOTRAZA, datosApp->datosGenerales->clock.time, t_siguiente_intervalo, (t_siguiente_intervalo - datosApp->datosGenerales->clock.time ));

		}

		break;
	case NORMAL_SIN_PROGRAMACION:

		break;
	case NORMAL_SINCRONIZANDO:
		if (calcular_programa_activo(datosApp, &t_siguiente_intervalo) == ESP_OK) {
			ESP_LOGI(TAG, ""TRAZAR"AJUSTE DE PROGRAMA REALIZADO", INFOTRAZA);
			datosApp->datosGenerales->estadoApp = NORMAL_AUTO;
			/*
			if (datosApp->datosGenerales->nProgramacion == 0) {
				ESP_LOGE(TAG, ""TRAZAR"CAMBIANDO A NORMAL_SIN_PROGRAMACION PORQUE HAY PROBLEMAS", INFOTRAZA);
				appuser_cambiar_modo_aplicacion(datosApp, NORMAL_SIN_PROGRAMACION);
			} else {
				ESP_LOGE(TAG, ""TRAZAR"CAMBIANDO A AUTO PORQUE HAY PROBLEMAS", INFOTRAZA);
				appuser_cambiar_modo_aplicacion(datosApp, NORMAL_AUTO);
			}*/

		} else {
			ESP_LOGE(TAG, ""TRAZAR"ERROR AL AJUSTAR LOS PROGRAMAS", INFOTRAZA);
			appuser_cambiar_modo_aplicacion(datosApp, NORMAL_SIN_PROGRAMACION);

		}
		//appuser_cambiar_modo_aplicacion(datosApp, NORMAL_AUTO);
		break;

	case ESPERA_FIN_ARRANQUE:
		ESP_LOGI(TAG, ""TRAZAR" EN ESPERA DE FIN DE ARRANQUE", INFOTRAZA);
		break;

	case NORMAL_FIN_PROGRAMA_ACTIVO:
		if (calcular_programa_activo(datosApp, &t_siguiente_intervalo) == ESP_OK) {
					ESP_LOGI(TAG, ""TRAZAR"FIN DE PROGRAMA REALIZADO", INFOTRAZA);
					//datosApp->datosGenerales->estadoApp = NORMAL_AUTO;
					appuser_cambiar_modo_aplicacion(datosApp, NORMAL_AUTO);
				} else {
					ESP_LOGE(TAG, ""TRAZAR"ERROR AL AJUSTAR LOS PROGRAMAS", INFOTRAZA);

				}

		break;

	default:
		break;
	}

	appuser_actualizar_gestion_programas(datosApp);


}

esp_err_t actualizar_programa_real(DATOS_APLICACION *datosApp) {


	ESP_LOGI(TAG, ""TRAZAR"ENTRAMOS EN ACTUALIZAR PROGRAMA REAL, handle:%d", INFOTRAZA, datosApp->handle);


	if (datosApp->datosGenerales->nProgramaCandidato >= 0) {
		datosApp->datosGenerales->programa_estatico_real = datosApp->datosGenerales->nProgramaCandidato;
		ESP_LOGI(TAG, ""TRAZAR"VAMOS A ACTUALIZAR EL PROGRAMA ESTATICO", INFOTRAZA);
		nvs_set_u8(datosApp->handle, CONFIG_CLAVE_PROGRAMA_ESTATICO, datosApp->datosGenerales->programa_estatico_real);
		ESP_LOGI(TAG, ""TRAZAR"Actualizado dinamico --> estatico", INFOTRAZA);
	} else {
		nvs_get_u8(datosApp->handle, CONFIG_CLAVE_PROGRAMA_ESTATICO, &datosApp->datosGenerales->programa_estatico_real);
		datosApp->datosGenerales->nProgramaCandidato = datosApp->datosGenerales->programa_estatico_real;
		ESP_LOGI(TAG, ""TRAZAR"Actualizado estatico-->dinamico", INFOTRAZA);
	}

	ESP_LOGI(TAG, ""TRAZAR"Programa activo dinamico: %d, Programa_activo_estatico: %d", INFOTRAZA, datosApp->datosGenerales->nProgramaCandidato, datosApp->datosGenerales->programa_estatico_real);

	return ESP_OK;
}

void temporizacion_intermedia(DATOS_APLICACION *datosApp) {

	ESP_LOGW(TAG, ""TRAZAR"EJECUTANDO TEMPORIZADOR DE TEMPORIZACION INTERMEDIA", INFOTRAZA);
	logica_temporizacion(datosApp);
	//activacion_programa(datosApp);
}



esp_err_t logica_temporizacion(DATOS_APLICACION *datosApp) {

	uint8_t indice;
	TIME_PROGRAM programa_actual;
	int duracion;
	int tiempo_restante;
	time_t hora;

	indice = datosApp->datosGenerales->nProgramaCandidato;
	programa_actual = datosApp->datosGenerales->programacion[indice];
	duracion = programa_actual.duracion;
	hora = datosApp->datosGenerales->clock.time;
	tiempo_restante = (programa_actual.programa + duracion) - hora;
	if (duracion > 0) {



		if (tiempo_restante > TEMPORIZADOR_MAXIMO_EN_SEGUNDOS ) {
			ESP_LOGW(TAG, ""TRAZAR"ACTIVADO TEMPORIZADOR DE TEMPORIZACION INTERMEDIA.  QUEDAN %d repeticiones", INFOTRAZA, tiempo_restante/TEMPORIZADOR_MAXIMO_EN_SEGUNDOS);
			ets_timer_disarm(&temporizador_duracion);
			ets_timer_setfn(&temporizador_duracion, (ETSTimerFunc*) temporizacion_intermedia, datosApp);
			ets_timer_arm(&temporizador_duracion, (TEMPORIZADOR_MAXIMO_EN_SEGUNDOS * 1000), false);
		} else {
			ets_timer_disarm(&temporizador_duracion);
						ets_timer_setfn(&temporizador_duracion, (ETSTimerFunc*) appuser_ejecucion_accion_temporizada, datosApp);
						ets_timer_arm(&temporizador_duracion, (tiempo_restante * 1000), false);
						ESP_LOGI(TAG, ""TRAZAR"ACTIVADO TEMPORIZADOR DE %d SEGUNDOS", INFOTRAZA, tiempo_restante);
		}


		if (tiempo_restante <= 0 ) {
			ESP_LOGW(TAG, ""TRAZAR"LA DURACION YA HA EXCEDIDO DE LA HORA Y NO SE ACTIVA", INFOTRAZA);
			appuser_ejecucion_accion_temporizada(datosApp);
			return PROGRAMACION_DURACION_EXCEDIDA;
		}
	}
	return ESP_OK;
}





esp_err_t activacion_programa(DATOS_APLICACION *datosApp) {




	logica_temporizacion(datosApp);
	appuser_temporizador_cumplido(datosApp);



	return ESP_OK;

}

esp_err_t iniciar_gestion_programacion(DATOS_APLICACION *datosApp) {


	gestion_programas(datosApp);
    ets_timer_disarm(&temporizador);
    ets_timer_setfn(&temporizador, (ETSTimerFunc*) gestion_programas, datosApp);
    ets_timer_arm(&temporizador, 1000, true);

	return ESP_OK;
}

esp_err_t parar_gestion_programacion(DATOS_APLICACION *datosApp) {

	ets_timer_disarm(&temporizador);
	return ESP_OK;
}


/*
bool buscarprogramaActivo(uint8_t nProgramacion, uint8_t *nProgramaCandidato, NTP_CLOCK *clock, TIME_PROGRAM *programacion, int *actual) {


	int i;
    int programaActivo = -1;
    ESP_LOGI(TAG, ""TRAZAR"buscarPrograma--> nProgramas: %d\n", INFOTRAZA, nProgramacion);

    if(nProgramacion == 0) {
        *actual = -1;
        *nProgramaCandidato = -1;
        ESP_LOGI(TAG, ""TRAZAR"buscarPrograma--> (1) nProgramas: %d\n", INFOTRAZA, nProgramacion);
        return false;
    }


    if (nProgramacion == 1) {
        if((clock->time < programacion->programa)) {
            if (esActivoElPrograma(programacion, nProgramacion, 0)) {
                programaActivo = 0;
                *actual = -1;
                nProgramaCandidato = 0;
                ESP_LOGI(TAG, ""TRAZAR"buscarPrograma--> a futuro (2), actual: %d, futuro: %d\n ", INFOTRAZA, *actual, *nProgramaCandidato);
                return false;
            } else {
                *actual = -1;
                *nProgramaCandidato = -1;
                ESP_LOGI(TAG, ""TRAZAR"buscarPrograma--> (3), actual: %d, futuro: %d\n ", INFOTRAZA, *actual, *nProgramaCandidato);

                return false;
            }
        } else {
            if (esActivoElPrograma(programacion, nProgramacion, 0)) {
                programaActivo = 0;
                *actual = 0;
                *nProgramaCandidato = 0;
                ESP_LOGI(TAG, ""TRAZAR"buscarPrograma--> a pasado (4), actual: %d, futuro: %d\n ", INFOTRAZA, *actual, *nProgramaCandidato);

                return true;

            } else {
                *actual = -1;
                *nProgramaCandidato = -1;
                ESP_LOGI(TAG, ""TRAZAR"buscarPrograma--> (5), actual: %d, futuro: %d\n ", INFOTRAZA, *actual, *nProgramaCandidato);

                return false;
            }
        }
    }


    for (i=0;nProgramacion;i++) {
        ESP_LOGI(TAG,"buscarProgramas-> bucle: %d\n", i);
        if (esActivoElPrograma(&programacion[i], nProgramacion, i) == true) {
            programaActivo = i;
            ESP_LOGI(TAG, ""TRAZAR"buscarPrograma--> programaActivo vale: %d\n", INFOTRAZA, programaActivo);
            if (clock->time < programacion[i].programa) {

                *nProgramaCandidato = programaActivo;

                if (programaActivo == 0) {
                    *actual = nProgramacion - 1;
                } else {
                    *actual = calcularProgramaActivoActual(programacion, nProgramacion, i-1);

                }
                ESP_LOGI(TAG, ""TRAZAR"buscarPrograma--> (6), actual: %d, futuro: %d\n ", INFOTRAZA, *actual, *nProgramaCandidato);

                return true;
            }
        }
    }
    i--;

    ESP_LOGI(TAG, ""TRAZAR"buscarPrograma--> no se ha encontrado ningun programa a futuro: progActivo vale:%d, i vale %d\n", INFOTRAZA, programaActivo, i);
    if (esActivoElPrograma(&programacion[i], nProgramacion, i) == true) {
        if (programaActivo == i) {
            *actual = programaActivo;
            *nProgramaCandidato = 0;
            ESP_LOGI(TAG, ""TRAZAR"buscarPrograma--> (7), actual: %d, futuro: %d\n ", INFOTRAZA, *actual, *nProgramaCandidato);
            return true;
        } else {
            *actual = programaActivo;
            *nProgramaCandidato = *actual + 1;
            ESP_LOGI(TAG, ""TRAZAR"buscarPrograma--> (8), actual: %d, futuro: %d\n ", INFOTRAZA, *actual, *nProgramaCandidato);

            return true;
        }
    } else {
        *actual = programaActivo;
        *nProgramaCandidato = *actual + 1;
        ESP_LOGI(TAG, ""TRAZAR"buscarPrograma--> (8), actual: %d, futuro: %d\n ", INFOTRAZA, *actual, *nProgramaCandidato);
        return true;
    }

}
*/
/*
bool buscarPrograma(DATOS_APLICACION *datosApp, int *actual) {


    int i=0;
    int programaActivo = -1;
    printf("buscarPrograma-->inicio\n");
    printf("buscarPrograma--> nProgramas: %d\n", datosApp->datosGenerales->nProgramacion);

    if(datosApp->datosGenerales->nProgramacion == 0) {
        *actual = -1;
        datosApp->datosGenerales->nProgramaCandidato = -1;
        printf("buscarPrograma--> (1) nProgramas: %d\n", datosApp->datosGenerales->nProgramacion);
        return false;
    }


    if (datosApp->datosGenerales->nProgramacion == 1) {

        if((datosApp->datosGenerales->clock.time < datosApp->datosGenerales->programacion[i].programa)) {
            if (esActivoElPrograma(&datosApp->datosGenerales->programacion[i], datosApp->datosGenerales->nProgramacion, 0)) {
                programaActivo = 0;
                *actual = -1;
                datosApp->datosGenerales->nProgramaCandidato = 0;
                printf("buscarPrograma--> a futuro (2), actual: %d, futuro: %d\n ", *actual, datosApp->datosGenerales->nProgramaCandidato);
                return false;
            } else {
                *actual = -1;
                datosApp->datosGenerales->nProgramaCandidato = -1;
                printf("buscarPrograma--> (3), actual: %d, futuro: %d\n ", *actual, datosApp->datosGenerales->nProgramaCandidato);

                return false;
            }

        } else {
            if (esActivoElPrograma(&(datosApp->datosGenerales->programacion[i]), datosApp->datosGenerales->nProgramacion, 0)) {
                programaActivo = 0;
                *actual = 0;
                datosApp->datosGenerales->nProgramaCandidato = 0;
                printf("buscarPrograma--> a pasado (4), actual: %d, futuro: %d\n ", *actual, datosApp->datosGenerales->nProgramaCandidato);

                return true;

            } else {
                *actual = -1;
                datosApp->datosGenerales->nProgramaCandidato = -1;
                printf("buscarPrograma--> (5), actual: %d, futuro: %d\n ", *actual, datosApp->datosGenerales->nProgramaCandidato);

                return false;
            }
        }
    }


    for (i=0;i<datosApp->datosGenerales->nProgramacion;i++) {
        printf("buscarProgramas-> bucle: %d\n", i);
        if (esActivoElPrograma(&(datosApp->datosGenerales->programacion[i]), datosApp->datosGenerales->nProgramacion, i) == true) {
            programaActivo = i;
            printf("buscarPrograma--> programaActivo vale: %d\n", programaActivo);
            if (datosApp->datosGenerales->clock.time < datosApp->datosGenerales->programacion[i].programa) {
                datosApp->datosGenerales->nProgramaCandidato = programaActivo;
                if (programaActivo == 0) {
                    *actual = datosApp->datosGenerales->nProgramacion - 1;
                } else {
                    *actual = calcularProgramaActivoActual(datosApp->datosGenerales->programacion, datosApp->datosGenerales->nProgramacion, i-1);

                }
                printf("buscarPrograma--> (6), actual: %d, futuro: %d\n ", *actual, datosApp->datosGenerales->nProgramaCandidato);

                return true;
            }
        }
    }
    i--;

    printf("buscarPrograma--> no se ha encontrado ningun programa a futuro: progActivo vale:%d, i vale %d\n", programaActivo, i);
    if (esActivoElPrograma(&(datosApp->datosGenerales->programacion[i]), datosApp->datosGenerales->nProgramacion, i) == true) {

        if (programaActivo == i) {
            *actual = programaActivo;
            datosApp->datosGenerales->nProgramaCandidato = 0;
            printf("buscarPrograma--> (7), actual: %d, futuro: %d\n ", *actual, datosApp->datosGenerales->nProgramaCandidato);
            return true;
        } else {
            *actual = programaActivo;
            datosApp->datosGenerales->nProgramaCandidato = *actual + 1;
            printf("buscarPrograma--> (8), actual: %d, futuro: %d\n ", *actual, datosApp->datosGenerales->nProgramaCandidato);

            return true;
        }
    } else {
        *actual = programaActivo;
        datosApp->datosGenerales->nProgramaCandidato = *actual + 1;
        printf("buscarPrograma--> (8), actual: %d, futuro: %d\n ", *actual, datosApp->datosGenerales->nProgramaCandidato);
        return true;
    }


}
*/

