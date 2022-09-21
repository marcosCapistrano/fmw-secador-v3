#include "server_controller.h"

#include <string.h>

#include "common.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_vfs.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

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

typedef struct {
    int m1_sock_fd;
    int m2_sock_fd;
    int m3_sock_fd;
    int m4_sock_fd;
} RemoteSensors_t;

static RemoteSensors_t sensors = {-1, -1, -1, -1};

typedef enum {
    INACTIVE,
    ACTIVE
} ServerControllerState_t;

ServerControllerState_t curr_state = INACTIVE;

extern QueueHandle_t server_update_q;
extern QueueHandle_t state_manager_q;

static TimerHandle_t m1_timer;
static TimerHandle_t m2_timer;
static TimerHandle_t m3_timer;
static TimerHandle_t m4_timer;

#define CHECK_FILE_EXTENSION(filename, ext) (strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0)

static void connect_sensor(int sensor_id, int temperature, int sock_fd) {
    if (sensor_id == 1) {
        if (sensors.m1_sock_fd == -1) {
            common_send_state_msg(STA_MSG_CHANGE_CONNECT, (void*)1, portMAX_DELAY);
        }

        sensors.m1_sock_fd = sock_fd;
        xTimerReset(m1_timer, portMAX_DELAY);
        common_send_state_msg(STA_MSG_CHANGE_SENSOR_M1, (void*)temperature, portMAX_DELAY);
    } else if (sensor_id == 2) {
        if (sensors.m2_sock_fd == -1) {
            common_send_state_msg(STA_MSG_CHANGE_CONNECT, (void*)2, portMAX_DELAY);
        }
        sensors.m2_sock_fd = sock_fd;
        xTimerReset(m2_timer, portMAX_DELAY);
        common_send_state_msg(STA_MSG_CHANGE_SENSOR_M2, (void*)temperature, portMAX_DELAY);
    } else if (sensor_id == 3) {
        if (sensors.m3_sock_fd == -1) {
            common_send_state_msg(STA_MSG_CHANGE_CONNECT, (void*)3, portMAX_DELAY);
        }
        sensors.m3_sock_fd = sock_fd;
        xTimerReset(m3_timer, portMAX_DELAY);
        common_send_state_msg(STA_MSG_CHANGE_SENSOR_M3, (void*)temperature, portMAX_DELAY);
    } else if (sensor_id == 4) {
        if (sensors.m4_sock_fd == -1) {
            common_send_state_msg(STA_MSG_CHANGE_CONNECT, (void*)4, portMAX_DELAY);
        }
        sensors.m4_sock_fd = sock_fd;
        xTimerReset(m4_timer, portMAX_DELAY);
        common_send_state_msg(STA_MSG_CHANGE_SENSOR_M4, (void*)temperature, portMAX_DELAY);
    }
}

static void disconnect_sensor(TimerHandle_t timer) {
    int sensor_id = (int)pvTimerGetTimerID(timer);

    if (sensor_id == 1) {
        if (sensors.m1_sock_fd != -1) {
            sensors.m1_sock_fd = -1;

            common_send_state_msg(STA_MSG_CHANGE_DISCONNECT, (void*)sensor_id, portMAX_DELAY);
        }
    } else if (sensor_id == 2) {
        if (sensors.m2_sock_fd != -1) {
            sensors.m2_sock_fd = -1;

            common_send_state_msg(STA_MSG_CHANGE_DISCONNECT, (void*)sensor_id, portMAX_DELAY);
        }
    } else if (sensor_id == 3) {
        if (sensors.m3_sock_fd != -1) {
            sensors.m3_sock_fd = -1;

            common_send_state_msg(STA_MSG_CHANGE_DISCONNECT, (void*)sensor_id, portMAX_DELAY);
        }
    } else if (sensor_id == 4) {
        if (sensors.m4_sock_fd != -1) {
            sensors.m4_sock_fd = -1;

            common_send_state_msg(STA_MSG_CHANGE_DISCONNECT, (void*)sensor_id, portMAX_DELAY);
        }
    }
}

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
    char* buf = NULL;
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
        ws_pkt.payload = (uint8_t*)buf;
        /* Set max_len = ws_pkt.len to get the frame payload */
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
            free(buf);
            return ret;
        }

        ESP_LOGI(TAG, "Got packet with message: %s", buf);

        char* temp_buf = NULL;
        temp_buf = calloc(1, ws_pkt.len - 2);
        if (temp_buf == NULL) {
            ESP_LOGE(TAG, "Failed to calloc memory for temp_buf");
            return ESP_ERR_NO_MEM;
        }

        memcpy(temp_buf, &buf[2], (ws_pkt.len - 2) * (sizeof buf[0]));

        int sensor_id = atoi(&buf[0]);
        int temperature = atoi(temp_buf);

        if (temperature < 0)
            temperature = 0;

        int sock_fd = httpd_req_to_sockfd(req);

        ESP_LOGI(TAG, "Sensor ID: %d - Temperature: %d", sensor_id, temperature);
        connect_sensor(sensor_id, temperature, sock_fd);

        free(temp_buf);
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

int scmp(const void* p1, const void* p2) {
    char *v1, *v2;

    v1 = *(char**)p1;
    v2 = *(char**)p2;

    return strcmp(v1, v2);
}

typedef struct Lote_t Lote_t;
struct Lote_t {
    char* name;
    Lote_t* next;
};

Lote_t* new_lote(char* name) {
    Lote_t* newl;
    newl = (Lote_t*)malloc(sizeof(Lote_t));

    if (newl == NULL)
        return NULL;

    newl->name = name;
    newl->next = NULL;

    return newl;
}

Lote_t* add_lote_end(Lote_t* listl, Lote_t* newl) {
    Lote_t* p;

    if (listl == NULL)
        return newl;

    for (p = listl; p->next != NULL; p = p->next)
        ;

    p->next = newl;
    return listl;
}

static esp_err_t on_get_lotes_handler(httpd_req_t* req) {
    struct dirent* entry;
    DIR* dir = opendir("/storage");

    Lote_t* lote_list = NULL;

    while ((entry = readdir(dir)) != NULL) {
    }

    closedir(dir);

    char response[250] = {0};
    Lote_t* curr_lote;
    for (curr_lote = lote_list; curr_lote != NULL; curr_lote = curr_lote->next) {
        ESP_LOGE(TAG, "%s", curr_lote->name);
        char append[25] = {0};
        if (curr_lote->next != NULL) {
            sprintf(append, "%s,", curr_lote->name);
        } else {
            sprintf(append, "%s", curr_lote->name);
        }

        strcat(response, append);
    }

    httpd_resp_sendstr(req, response);

    return ESP_OK;
}
static httpd_uri_t uri_get_lotes = {

    .uri = "/lotes",
    .method = HTTP_GET,
    .handler = on_get_lotes_handler};

static esp_err_t on_get_lote_id_handler(httpd_req_t* req) {
    char path[600] = {0};

    const char s[2] = "/";
    char* token = strtok(req->uri, s);
    token = strtok(NULL, s);

    sprintf(path, "/storage/%s", token);

    FILE* file = fopen(path, "r");
    if (file == NULL) {
        httpd_resp_send_404(req);
        return ESP_OK;
    }

    char lineRead[256];
    while (fgets(lineRead, sizeof(lineRead), file)) {
        httpd_resp_sendstr_chunk(req, lineRead);
    }

    httpd_resp_sendstr_chunk(req, NULL);
    fclose(file);
    return ESP_OK;
}
static httpd_uri_t uri_get_lote_id = {
    .uri = "/lote/*",
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

static httpd_close_func_t on_close_session(httpd_handle_t* handle, int sock_fd) {
    int sensor_id = -1;

    if (sock_fd == sensors.m1_sock_fd) {
        sensor_id = 1;

        common_send_state_msg(STA_MSG_CHANGE_DISCONNECT, (void*)sensor_id, portMAX_DELAY);
        sensors.m1_sock_fd = -1;
    } else if (sock_fd == sensors.m2_sock_fd) {
        sensor_id = 2;

        common_send_state_msg(STA_MSG_CHANGE_DISCONNECT, (void*)sensor_id, portMAX_DELAY);
        sensors.m1_sock_fd = -1;
    } else if (sock_fd == sensors.m3_sock_fd) {
        sensor_id = 3;

        common_send_state_msg(STA_MSG_CHANGE_DISCONNECT, (void*)sensor_id, portMAX_DELAY);
        sensors.m4_sock_fd = -1;
    } else if (sock_fd == sensors.m4_sock_fd) {
        sensor_id = 4;
        
        common_send_state_msg(STA_MSG_CHANGE_DISCONNECT, (void*)sensor_id, portMAX_DELAY);
        sensors.m4_sock_fd = -1;
    }

    close(sock_fd);

    return ESP_OK;
}

static httpd_handle_t start_webserver(void) {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;
    config.close_fn = on_close_session;

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

    m1_timer = xTimerCreate("TIMER M1", pdMS_TO_TICKS(15000), pdTRUE, (void*)1, disconnect_sensor);
    m2_timer = xTimerCreate("TIMER M2", pdMS_TO_TICKS(15000), pdTRUE, (void*)2, disconnect_sensor);
    m3_timer = xTimerCreate("TIMER M3", pdMS_TO_TICKS(15000), pdTRUE, (void*)3, disconnect_sensor);
    m4_timer = xTimerCreate("TIMER M4", pdMS_TO_TICKS(15000), pdTRUE, (void*)4, disconnect_sensor);

    xTimerStart(m1_timer, portMAX_DELAY);
    xTimerStart(m2_timer, portMAX_DELAY);
    xTimerStart(m3_timer, portMAX_DELAY);
    xTimerStart(m4_timer, portMAX_DELAY);

    ESP_LOGI(TAG, "Iniciando Servidor WEB...");
    server_handle = start_webserver();
    ESP_LOGI(TAG, "Servidor WEB OK!");
}
