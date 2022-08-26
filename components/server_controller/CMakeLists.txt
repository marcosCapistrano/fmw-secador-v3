idf_component_register(SRCS "server_controller.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_wifi esp_netif esp_http_server)
