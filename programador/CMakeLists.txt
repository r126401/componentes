idf_component_register(SRCS "programmer.c" "ntp.c" INCLUDE_DIRS "include" "." 
REQUIRES alarmas 
api_json
comunes
conexiones
nvslib
interfaz_usuario
configuracion
espota)
