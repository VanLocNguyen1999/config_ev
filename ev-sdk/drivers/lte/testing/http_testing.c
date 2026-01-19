#include <sm_string_util.h>

#include "sm_ec200.h"
#include "sm_logger.h"
#include "pthread.h"
#include "linux/utils/linux_serial.h"
#include "sm_ec200_mqtt.h"
#include "sm_ec200_http.h"
#include "sm_elapsed_timer.h"
#include "string.h"
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
    g_lte_uart = sm_hal_uart_init(115200, 0 ,0);
    g_lte_reset = sm_hal_io_init(0);
    sm_hal_io_open(g_lte_reset, SM_HAL_IO_OUTPUT);

    sm_ec200_init(&g_ec200, g_lte_reset, g_lte_uart);
    g_modem = &g_ec200.base;

    LOG_DBG(TAG, "READY");
}

void uart_process(sm_hal_uart_t *_this, void* _arg) {
    if (!_this) return;
    if (serial_is_open(g_fd)) {
        static uint8_t data[129] = {0,};
        memset(data, '\0',129);
        int32_t len = serial_recv_bytes(g_fd, data, 128);
        if (len >0) {
            for(int i=0; i<len; i++) {
                sm_hal_uart_rx_irq(_this, data[i]);
            }
        }
    }
}
void *mcu(void *_arg) {
    while (1) {
        uart_process(g_lte_uart, _arg);
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
        // printf("q: sm_ec200_config_mqtt\n ");
        scanf("%c", &ch);
        switch (ch) {
            case 'q':
            {
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
                sm_modem_send_cmd(g_modem, cmd, "zzzz","zzzz", 10000);
            }
                break;
            case 'w':
                break;
            case 'e':
                break;
            case 'r':
                break;
            case 't':
                break;
            case 'y':
                break;
            case 'u':
                break;
            case 'i':
                break;
            case 'o':
                break;
            case 'p':
                break;
            case 'a':
                break;
            case 's':
                break;
            case 'd':
                break;
            case 'f':
                break;
            case 'g':
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
