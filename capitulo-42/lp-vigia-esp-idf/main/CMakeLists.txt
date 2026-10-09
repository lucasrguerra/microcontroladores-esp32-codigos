idf_component_register(SRCS "main.c"
                       REQUIRES ulp esp_driver_gpio
                       WHOLE_ARCHIVE)
set(ulp_app_name ulp_${COMPONENT_NAME})
ulp_embed_binary(${ulp_app_name} "ulp/vigia.c" "main.c")
