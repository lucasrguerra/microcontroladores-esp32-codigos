idf_component_register(SRCS "main.c"
                       REQUIRES ulp)
set(ulp_app_name ulp_${COMPONENT_NAME})
ulp_embed_binary(${ulp_app_name} "ulp/conta.S" "main.c")
