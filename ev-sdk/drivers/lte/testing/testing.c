#include <sm_string_util.h>

#include "sm_ec200.h"
#include "porting/sm_porting.h"
#include "sm_logger.h"
#include "pthread.h"
#include "linux/utils/linux_serial.h"
#include "sm_ec200_mqtt.h"
#include "sm_ec200_http.h"
#include "sm_elapsed_timer.h"
int32_t g_fd;

static const char* TAG = "LTE_TESTING";
sm_hal_uart_t *g_lte_uart;
sm_hal_io_t *g_lte_reset;
sm_modem_t *g_modem;
sm_ec200_t g_ec200;

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

void init() {
    g_lte_uart = sm_hal_uart_init(&uart_func, NULL);
    g_lte_uart->proc->open(g_lte_uart);

    g_lte_reset = sm_hal_io_init(&io_func, NULL, 0);
    sm_hal_io_open(g_lte_reset, SM_HAL_IO_OUTPUT);

    sm_ec200_init(&g_ec200, g_lte_reset, g_lte_uart);
    g_modem = &g_ec200.base;


    LOG_DBG(TAG, "READY");
}

void uart_process(sm_hal_uart_t *_this, void* _arg) {
    if (!_this) return;
    if (serial_is_open(g_fd)) {
        uint8_t data = 0;
        int32_t len = serial_recv_byte(g_fd, &data);
        if (len >0) {
            sm_hal_uart_rx_irq(_this, data, (uint16_t)len);
        }
    }
}
uint8_t mqtt_connected = 0;
void mqtt_process(sm_ec200_t *_this) {
    if (!_this) return;
    char topic[512] = {0};
    char payload[512] = {0};
    if (!sm_ec200_data_iscomming_mqtt(_this,topic,payload)) {
        LOG_DBG(TAG, "MQTT RECV: topic: %s, payload: %s",topic, payload);
    }
}
void *mcu(void *_arg) {
    while (1) {
        uart_process(g_lte_uart, _arg);
        if (mqtt_connected)
            mqtt_process(&g_ec200);
        sm_hal_delay_ms(10);
    }
}

char buff[128] = {0,};
char *p_buff = buff;
int main(void){
    sm_logger_init(log_puts, LOG_LEVEL_DEBUG);

    pthread_t mcu_thread_id;
    pthread_create(&mcu_thread_id, NULL, mcu, NULL);

    init();
    char ch;
    while (1)
    {
        printf("q: sm_ec200_config_mqtt\n \
w: sm_ec200_open_mqtt\n \
e: sm_ec200_connect_mqtt\n \
r: sm_ec200_disconnect_mqtt\n \
t: sm_ec200_publish_mqtt\n \
y: sm_ec200_subscribes_mqtt\n\
u: sm_ec200_data_iscomming_mqtt\n \
i: sm_ec200_unsubscribes_mqtt\n \
o: AT+QMTRECV?\n \
p: AT+QMTRECV=1,0\n \
a: AT+QMTRECV=1,1\n \
s: AT+QMTRECV=1,2\n \
d: AT+QMTRECV=1,3\n \
f: AT+QMTRECV=1,4\n \
");
        scanf("%c", &ch);
        switch (ch) {
            case 'q':
                sm_ec200_config_mqtt(&g_ec200);
                break;
            case 'w':
                sm_ec200_open_mqtt(&g_ec200, "hub.dev.selex.vn", 1883);
                break;
            case 'e':
                if (!sm_ec200_connect_mqtt(&g_ec200, "lte_testing","", ""))
                    mqtt_connected = 1;
                break;
            case 'r':
                if (!sm_ec200_disconnect_mqtt(&g_ec200))
                    mqtt_connected = 0;
                break;
            case 't':
                sm_ec200_publish_mqtt(&g_ec200, "selex/test", "123456789", 9, 1, 0);
                break;
            case 'y':
                sm_ec200_subscribes_mqtt(&g_ec200,"selex/test",1);
                break;
            case 'u': {
                char *buffer;
                sm_ec200_data_iscomming_mqtt(&g_ec200, "selex/test", buffer);
                printf("%s\n", buffer);
            }
                break;
            case 'i':
                sm_ec200_unsubscribes_mqtt(&g_ec200, "selex/test");
                break;
            case 'o':
                sm_modem_send_cmd(g_modem, "AT+QMTRECV?\r\n", "OK\r\n","OK\r\n", 2000);
                break;
            case 'p':
                sm_modem_send_cmd(g_modem, "AT+QMTRECV=1,0\r\n", "OK\r\n","OK\r\n", 2000);
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
                break;
            case 'j':
                break;
            case 'k':
                break;
            case 'l':
                break;
            case 'z':
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
            default:
                break;
        }
    }
    return 0;
}
