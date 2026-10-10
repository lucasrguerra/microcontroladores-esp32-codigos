# O modelo (2,5 KB) vem pronto no exemplo hello_world do próprio componente.
set(EXEMPLO ${CMAKE_SOURCE_DIR}/managed_components/espressif__esp-tflite-micro/examples)
idf_component_register(SRCS "main.cc" "${EXEMPLO}/hello_world/main/model.cc"
                       INCLUDE_DIRS "." "${EXEMPLO}/hello_world/main")
