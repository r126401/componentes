idf_component_register(
    SRCS "ds18x20.c" "lectura.c"
    INCLUDE_DIRS include
    REQUIRES onewire esp_idf_lib_helpers
)
