idf_component_register(SRCS "espota.c" INCLUDE_DIRS "include" 
#EMBED_TXTFILES certs/ca_cert.pem
REQUIRES alarmas
api_json
comunes
conexiones
nvslib
configuracion
programador
main)

