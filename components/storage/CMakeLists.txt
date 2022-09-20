idf_component_register(SRCS "storage.c"
                    INCLUDE_DIRS "include"
                    REQUIRES nvs_flash spiffs esp-ds3231)

set(WEB_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../website")
if(EXISTS ${WEB_SRC_DIR}/dist)
    spiffs_create_partition_image(website ${WEB_SRC_DIR}/dist FLASH_IN_PROJECT)
else()
    message(FATAL_ERROR "${WEB_SRC_DIR}/dist doesn't exit. Please run 'yarn build' in ${WEB_SRC_DIR}")
endif()
