idf_component_register(SRCS "mqtt.c" "conexiones.c" INCLUDE_DIRS "include" 
REQUIRES alarmas 
api_json
comunes
nvslib
interfaz_usuario
configuracion
espota
programador
mqtt 
EMBED_TXTFILES certs/mqtt_cert.crt
)
