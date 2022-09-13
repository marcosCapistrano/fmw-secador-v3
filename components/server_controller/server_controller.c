#include "server_controller.h"

#include <string.h>

#include "esp_err.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_vfs.h"
#include "esp_wifi.h"

#define WIFI_AP_SSID "AusyxSolucoes"
#define WIFI_AP_PASS "12345678"

#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + 128)
#define SCRATCH_BUFSIZE (10240)

static const char* TAG = "SERVER_CONTROLLER";

typedef struct rest_server_context {
    char base_path[ESP_VFS_PATH_MAX + 1];
    char scratch[SCRATCH_BUFSIZE];
} rest_server_context_t;

static httpd_handle_t server_handle = NULL;
static rest_server_context_t rest_context = {
    .base_path = "/website",
    .scratch = {0}};

extern QueueHandle_t server_update_q;
extern QueueHandle_t state_manager_q;

#define CHECK_FILE_EXTENSION(filename, ext) (strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0)

static esp_err_t set_content_type_from_file(httpd_req_t* req, const char* filepath) {
    const char* type = "text/plain";
    if (CHECK_FILE_EXTENSION(filepath, ".html")) {
        type = "text/html";
    } else if (CHECK_FILE_EXTENSION(filepath, ".js")) {
        type = "application/javascript";
    } else if (CHECK_FILE_EXTENSION(filepath, ".css")) {
        type = "text/css";
    } else if (CHECK_FILE_EXTENSION(filepath, ".png")) {
        type = "image/png";
    } else if (CHECK_FILE_EXTENSION(filepath, ".ico")) {
        type = "image/x-icon";
    } else if (CHECK_FILE_EXTENSION(filepath, ".svg")) {
        type = "text/xml";
    }
    return httpd_resp_set_type(req, type);
}

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

static esp_err_t on_get_lotes_handler(httpd_req_t* req) {
    ESP_LOGI(TAG, "Got on lotes");
    return ESP_OK;
}
static httpd_uri_t uri_get_lotes = {

    .uri = "/lotes",
    .method = HTTP_GET,
    .handler = on_get_lotes_handler};

static esp_err_t on_get_lote_id_handler(httpd_req_t* req) {
    return ESP_OK;
}
static httpd_uri_t uri_get_lote_id = {
    .uri = "/lote/:id",
    .method = HTTP_GET,
    .handler = on_get_lote_id_handler};

static esp_err_t on_root_handler(httpd_req_t* req) {
    char filepath[FILE_PATH_MAX];

    strlcpy(filepath, rest_context.base_path, sizeof(filepath));
    if (req->uri[strlen(req->uri) - 1] == '/') {
        strlcat(filepath, "/index.html", sizeof(filepath));
    } else {
        strlcat(filepath, req->uri, sizeof(filepath));
    }

    int fd = open(filepath, O_RDONLY, 0);
    if (fd == -1) {
        ESP_LOGE(TAG, "Failed to open file : %s", filepath);
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
        return ESP_FAIL;
    }

    set_content_type_from_file(req, filepath);

    char* chunk = rest_context.scratch;
    ssize_t read_bytes;
    do {
        /* Read file in chunks into the scratch buffer */
        read_bytes = read(fd, chunk, SCRATCH_BUFSIZE);
        if (read_bytes == -1) {
            ESP_LOGE(TAG, "Failed to read file : %s", filepath);
        } else if (read_bytes > 0) {
            /* Send the buffer contents as HTTP response chunk */
            if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK) {
                close(fd);
                ESP_LOGE(TAG, "File sending failed!");
                /* Abort sending file */
                httpd_resp_sendstr_chunk(req, NULL);
                /* Respond with 500 Internal Server Error */
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
                return ESP_FAIL;
            }
        }
    } while (read_bytes > 0);
    /* Close file after sending complete */
    close(fd);
    ESP_LOGI(TAG, "File sending complete");
    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}
static httpd_uri_t uri_root = {
    .uri = "/*",
    .method = HTTP_GET,
    .handler = on_root_handler};

static httpd_handle_t start_webserver(void) {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &uri_ws);  // WEBSOCKET dos ESP32
        httpd_register_uri_handler(server, &uri_get_lotes);
        httpd_register_uri_handler(server, &uri_get_lote_id);
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
