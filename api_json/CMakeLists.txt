idf_component_register(SRCS "comandos_json.c" "api_json.c" INCLUDE_DIRS "include" 
REQUIRES alarmas
comunes
conexiones
nvslib
interfaz_usuario
configuracion
espota
programador
json)
