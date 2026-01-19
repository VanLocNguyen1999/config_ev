#include "sm_string_util.h"
#include "sm_ec200.h"
#include "mqtt/testing/porting/sm_porting.h"
#include "sm_logger.h"
#include "pthread.h"
#include "linux/utils/linux_serial.h"
#include "sm_ec200_mqtt.h"
#include "sm_elapsed_timer.h"
#include "sm_mqtt_client.h"

#define HOST_NAME   "hub.dev.selex.vn"
#define TOPIC_NAME  "test"
int32_t g_fd;

static const char* TAG = "MQTT_CLIENT_TESTING";
sm_hal_uart_t *g_lte_uart;
sm_hal_io_t *g_lte_reset;
sm_modem_t *g_modem;
sm_ec200_t g_ec200;
sm_mqtt_client_t * mqtt_client;

char topicb[128];
char datab[2048];

int64_t get_tick_count() {
    struct timespec ts;
    unsigned int tick = 0U;
    clock_gettime( CLOCK_REALTIME, &ts );
    tick  = ts.tv_nsec / 1000000;
    tick += ts.tv_sec * 1000;
    return (int64_t )tick;
}
void log_puts(char* _str) {
    printf("%s\n", _str);
}

void on_connected(int _success, void* _arg);
void on_disconnected(int _success, void* _arg);
void on_subscribed(int _success, void* _arg);
void on_unsubscribed(int _success, void* _arg);
void on_published(int _success, void* _arg);
void on_message(sm_mqtt_client_t* _mqtt_client, sm_mqtt_msg_t* _msg, void* _arg);
void on_message1(sm_mqtt_client_t* _mqtt_client, sm_mqtt_msg_t* _msg, void* _arg);
void on_message2(sm_mqtt_client_t* _mqtt_client, sm_mqtt_msg_t* _msg, void* _arg);

void init() {
    g_lte_uart = sm_hal_uart_init(&uart_func, NULL);


    g_lte_reset = sm_hal_io_init(&io_func, NULL, 0);
    sm_hal_io_open(g_lte_reset, SM_HAL_IO_OUTPUT);

    sm_ec200_init(&g_ec200, g_lte_reset, g_lte_uart);
    sm_ec200_config_mqtt(&g_ec200);
    g_modem = &g_ec200.base;

    CLIENT: mqtt_client = sm_mqtt_init(&g_ec200);
    if (!mqtt_client) {
        LOG_ERR(TAG, "MQTT_CLIENT INIT FAIL");
        sm_hal_delay_ms(500);
        goto CLIENT;
    }
    LOG_DBG(TAG, "READY");
}

void uart_process(sm_hal_uart_t *_this, void* _arg) {
    if (!_this) return;
    if (serial_is_open(g_fd)) {
        static uint8_t data[129] = {0,};
        memset(data, '\0',129);
        int32_t len = serial_recv_bytes(g_fd, data, 128);
        if (len >0) {
            sm_hal_uart_rx_callback(_this, data, (uint16_t)len);
            printf("%s", data);
        }
    }
}
uint8_t mqtt_connected = 0;
void mqtt_process(sm_ec200_t *_this) {
    if (!_this) return;
    sm_mqtt_client_process(mqtt_client);
}
void *mcu(void *_arg) {
    while (1) {
        uart_process(g_lte_uart, _arg);
        sm_hal_delay_ms(1);
    }
}

void *app(void* _arg) {
    while(1) {
        if (mqtt_connected) {
            mqtt_process(&g_ec200);
            sm_hal_delay_ms(1);
        }
    }
}

char buff[128] = {0,};
char *p_buff = buff;
int main(void){
    sm_logger_init(log_puts, LOG_LEVEL_DEBUG);

    pthread_t mcu_thread_id;
    pthread_t app_process_id;
    pthread_create(&mcu_thread_id, NULL, mcu, NULL);
    pthread_create(&app_process_id, NULL, app, NULL);

    init();
    char ch;
    while (1)
    {
        printf("\
    q: sm_mqtt_connect\n\
    w: sm_mqtt_disconnect\n\
    e: sm_mqtt_subscribe\n\
    r: sm_mqtt_unsubscribes\n\
    t: sm_mqtt_publish\n\
    y: \n\
    u: sm_ec200_data_iscomming_mqtt\n\
    i: \n\
    o: AT+QMTRECV?\n\
    p: AT+QMTRECV=1,0\n\
    a: AT+QMTRECV=1,1\n\
    s: AT+QMTRECV=1,2\n\
    d: AT+QMTRECV=1,3\n\
    f: AT+QMTRECV=1,4\n\
");
        scanf("%c", &ch);
        while(ch == '\n')
            scanf("%c", &ch);
        switch (ch) {
            case 'q':
                if (!sm_mqtt_connect(mqtt_client, HOST_NAME, 1883, "haizzz", "", "", on_connected, NULL)) {
                    LOG_DBG(TAG, "sm_mqtt_connect SUCC");
                }else {
                    LOG_ERR(TAG, "sm_mqtt_connect FAIL");
                }
                break;
            case 'w':
                if (!sm_mqtt_disconnect(mqtt_client, on_disconnected, NULL)) {
                    LOG_DBG(TAG, "sm_mqtt_disconnect SUCC");
                }else {
                    LOG_ERR(TAG, "sm_mqtt_disconnect FAIL");
                }
                break;
            case 'e':
                if(!sm_mqtt_subscribe(mqtt_client, TOPIC_NAME, 1, on_subscribed, NULL)) {
                    LOG_DBG(TAG, "sm_mqtt_subscribe SUCC");
                }else {
                    LOG_ERR(TAG, "sm_mqtt_subscribe FAIL");
                }
                break;
            case 'r':
               if (!sm_mqtt_unsubscribes(mqtt_client, TOPIC_NAME, on_unsubscribed, NULL)) {
                   LOG_DBG(TAG, "sm_mqtt_unsubscribes SUCC");
               }else {
                   LOG_ERR(TAG, "sm_mqtt_unsubscribes FAIL");
               }
                break;
            case 't':
                if (!sm_mqtt_publish(mqtt_client, TOPIC_NAME, "data test 123456789 {} [] : '' \" \" ", 1, 0, on_published, NULL)) {
                    LOG_DBG(TAG, "sm_mqtt_publish SUCC");
                }else {
                    LOG_ERR(TAG, "sm_mqtt_publish FAIL");
                }
                break;
            case 'y':

                break;
            case 'u':
                memset(topicb, '\0', 128);
                memset(datab, '\0', 2048);
                if (!sm_ec200_data_incoming_mqtt(&g_ec200, topicb, datab)) {
                    LOG_DBG(TAG, "READ INCOMING DATA SUCC");
                }else {
                    LOG_DBG(TAG, "READ INCOMING DATA FAIL");
                }
                break;
            case 'i':
                sm_ec200_subscribes_mqtt(&g_ec200, TOPIC_NAME, 1);
                break;
            case 'o':
                sm_modem_send_cmd(g_modem, "AT+QMTRECV?\r\n", "OK\r\n","OK\r\n", 2000);
                break;
            case 'p':
                if (!sm_modem_send_cmd(g_modem, "AT+QMTRECV=1,0\r\n", "OK\r\n","OK\r\n", 2000)) {
                    LOG_DBG(TAG, "AT+QMTRECV=1,0 SUCC");
                }else {
                    LOG_DBG(TAG, "AT+QMTRECV=1,0 FAIL");
                }
                break;
            case 'a':
                sm_modem_send_cmd(g_modem, "AT+QMTRECV=1,1\r\n", "OK\r\n","OK\r\n", 2000);
                break;
            case 's':
                sm_modem_send_cmd(g_modem, "AT+QMTRECV=1,2\r\n", "OK\r\n","OK\r\n", 2000);
                break;
            case 'd':
                sm_modem_send_cmd(g_modem, "AT+QMTRECV=1,3\r\n", "OK\r\n","OK\r\n", 2000);
                break;
            case 'f':
                sm_modem_send_cmd(g_modem, "AT+QMTRECV=1,4\r\n", "OK\r\n","OK\r\n", 2000);
                break;
            case 'g':
                sm_modem_send_cmd(g_modem, "AT+QMTOPEN?\r\n", "OK\r\n","OK\r\n", 2000);
                break;
            case 'h':
                sm_mqtt_set_incoming_data_cb(mqtt_client, on_message, NULL);
                break;
            case 'j':
                sm_mqtt_set_incoming_data_cb(mqtt_client, on_message1, NULL);
                break;
            case 'k':
                sm_mqtt_set_incoming_data_cb(mqtt_client, on_message2, NULL);
                break;
            case 'l':
                sm_modem_send_cmd(g_modem, "AT+QMTCFG=?\r\n", "OK\r\n","OK\r\n", 2000);
                break;
            case 'z': {
                char cmd[128] = {'\0',};
                memset(cmd, '\0', 128);
                scanf("%s",cmd);
                if (cmd[strlen(cmd)-1] == '\n') {
                    cmd[strlen(cmd)-1] = '\0';
                }
                uint8_t len = strlen(cmd);
                cmd[len] = '\r';
                cmd[len+1] = '\n';
                cmd[len+2] = '\0';
                printf("%s",cmd);
                sm_modem_send_cmd(g_modem, cmd, "OK\r\n","OK\r\n", 2000);
            }
                break;
            case 'x':
                break;
            case 'c':
                break;
            case 'v':
                break;
            case 'b':
                break;
            case 'n':
                break;
            case 'm':
                break;
            case '0':
                if (mqtt_connected)
                    mqtt_connected = 0;
                else
                    mqtt_connected = 1;
            default:
                break;
        }
    }
    return 0;
}
void on_connected(int _success, void* _arg){
    LOG_DBG(TAG, "MQTT Client on connected");
    // mqtt_connected = 1;
}

void on_disconnected(int _success, void* _arg){
    LOG_DBG(TAG, "MQTT Client on disconnected, Reconnecting to broker");
    mqtt_connected = 0;
}

void on_subscribed(int _success, void* _arg){
    LOG_DBG(TAG, "MQTT Client on subscribed %s ", !_success ? "SUCCESS":"FAILURE" );
}

void on_unsubscribed(int _success, void* _arg){
    LOG_DBG(TAG, "MQTT Client on unsubscribed");
}

void on_published(int _success, void* _arg){
    LOG_DBG(TAG, "MQTT Client on published");
}

void on_message(sm_mqtt_client_t* _mqtt_client, sm_mqtt_msg_t* _msg, void* _arg){
    LOG_DBG(TAG, "MQTT Client 0 received message| \n topic: %s\n payload: %s", _msg->m_topic, _msg->m_payload);
}

void on_message1(sm_mqtt_client_t* _mqtt_client, sm_mqtt_msg_t* _msg, void* _arg){
    LOG_DBG(TAG, "MQTT Client 1 received message| \n topic: %s\n payload: %s", _msg->m_topic, _msg->m_payload);
}

void on_message2(sm_mqtt_client_t* _mqtt_client, sm_mqtt_msg_t* _msg, void* _arg){
    LOG_DBG(TAG, "MQTT Client 2 received message| \n topic: %s\n payload: %s", _msg->m_topic, _msg->m_payload);
}