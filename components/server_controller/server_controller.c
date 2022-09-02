#include "server_controller.h"

#include <string.h>

#include "esp_err.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_wifi.h"

#define WIFI_AP_SSID "AusyxSolucoes"
#define WIFI_AP_PASS "12345678"

static const char* TAG = "server_controller";

static httpd_handle_t server_handle = NULL;
extern QueueHandle_t server_update_q;
extern QueueHandle_t state_manager_q;

static esp_err_t on_ws_handler(httpd_req_t* req) {
    if (req->method == HTTP_GET) {
        ESP_LOGI(TAG, "Handshake done, the new connection was opened");
        return ESP_OK;
    }

    httpd_ws_frame_t ws_pkt;
    uint8_t* buf = NULL;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    /* Set max_len = 0 to get the frame len */
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
        return ret;
    }
    ESP_LOGI(TAG, "frame len is %d", ws_pkt.len);
    if (ws_pkt.len) {
        /* ws_pkt.len + 1 is for NULL termination as we are expecting a string */
        buf = calloc(1, ws_pkt.len + 1);
        if (buf == NULL) {
            ESP_LOGE(TAG, "Failed to calloc memory for buf");
            return ESP_ERR_NO_MEM;
        }
        ws_pkt.payload = buf;
        /* Set max_len = ws_pkt.len to get the frame payload */
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
            free(buf);
            return ret;
        }
        ESP_LOGI(TAG, "Got packet with message: %s", ws_pkt.payload);
    }

    // TODO: Use ws payload with ws_pkt.payload

    free(buf);
    return ret;
}
static httpd_uri_t uri_ws = {
    .uri = "/ws",
    .method = HTTP_GET,
    .handler = on_ws_handler,
    .is_websocket = true};

static esp_err_t on_get_events_handler(httpd_req_t* req) {
    int length = 0;
    int* all_lotes = storage_get_all_lotes(&length);

    for (int i = 0; i < length; i++) {
        ESP_LOGI(TAG, "Found lote: %d", all_lotes[i]);
    }
    return ESP_OK;
}
static httpd_uri_t uri_get_events = {
    .uri = "/events",
    .method = HTTP_GET,
    .handler = on_lotes_handler};

static esp_err_t on_root_handler(httpd_req_t* req) {
    char path[600];
    if (strcmp(req->uri, "/") == 0) {
        strcpy(path, "/spiffs/index.html");
    } else {
        sprintf(path, "/spiffs%s", req->uri);
    }

    char* ext = strrchr(path, '.');
    if (strcmp(ext, ".css") == 0) {
        httpd_resp_set_type(req, "text/css");
    }
    if (strcmp(ext, ".js") == 0) {
        httpd_resp_set_type(req, "text/javascript");
    }
    if (strcmp(ext, ".png") == 0) {
        httpd_resp_set_type(req, "image/png");
    }

    // handle other files

    // FILE* file = storage_open_file_r(path);
    // if (file == NULL) {
    //     httpd_resp_send_404(req);
    //     return ESP_OK;
    // }

    // char lineRead[256];
    // while (fgets(lineRead, sizeof(lineRead), file)) {
    //     httpd_resp_sendstr_chunk(req, lineRead);
    // }

    // httpd_resp_sendstr_chunk(req, NULL);
    // storage_close_file(file);
    return ESP_OK;
}
static httpd_uri_t uri_root = {
    .uri = "/*",
    .method = HTTP_GET,
    .handler = on_root_handler};

static httpd_handle_t start_webserver(void) {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &uri_ws);  // WEBSOCKET dos ESP32
        httpd_register_uri_handler(server, &uri_get_events);
        httpd_register_uri_handler(server, &uri_root);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static void stop_webserver(httpd_handle_t server) {
    // Stop the httpd server
    httpd_stop(server);
}
static void disconnect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    httpd_handle_t* server = (httpd_handle_t*)arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        stop_webserver(*server);
        *server = NULL;
    }
}
static void connect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    httpd_handle_t* server = (httpd_handle_t*)arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}

void server_controller_init(void) {
    ESP_LOGI(TAG, "Iniciando Wi-Fi...");
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t wifi_init_conf = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_conf));

    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server_handle));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server_handle));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_AP_SSID,
            .ssid_len = strlen(WIFI_AP_SSID),
            .password = WIFI_AP_PASS,
            .max_connection = 10,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK}};

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "Wi-Fi OK!");

    ESP_LOGI(TAG, "Iniciando Servidor WEB...");
    server_handle = start_webserver();
    ESP_LOGI(TAG, "Servidor WEB OK!");
}
