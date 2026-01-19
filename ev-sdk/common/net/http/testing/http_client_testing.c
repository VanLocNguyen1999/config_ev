#include <sm_string_util.h>
#include <unistd.h>

#include "sm_ec200.h"
#include "sm_logger.h"
#include "pthread.h"
#include "linux/utils/linux_serial.h"
#include "sm_elapsed_timer.h"
#include "sm_hal_delay.h"
#include "string.h"
#include "stdio.h"
#include "sm_http_client.h"
int32_t g_fd;

static const char* TAG = "HTTP_CLIENT_TEST";
sm_hal_uart_t *g_lte_uart;
sm_hal_io_t *g_lte_reset;
sm_modem_t *g_modem;
sm_ec200_t g_ec200;
sm_http_client_t *http_client;

FILE *file_log;
FILE* file_real;
FILE* file_fake;

volatile bool is_passed = true;

int64_t get_tick_count() {
    struct timespec ts;
    unsigned int tick = 0U;
    clock_gettime( CLOCK_MONOTONIC, &ts );
    tick  = ts.tv_nsec / 1000000;
    tick += ts.tv_sec * 1000;
    return (int64_t )tick;
}
void log_puts(char* _str) {
    if (strstr(_str, "MODEM")) return;
    printf("%s\n", _str);
    file_log = fopen("./log.txt", "a");
    fprintf(file_log, "%s\n", _str);
    fclose(file_log);
}

void init() {
    g_lte_uart = sm_hal_uart_init(115200, 0 ,0);
    g_lte_reset = sm_hal_io_init(0);
    sm_hal_io_open(g_lte_reset, SM_HAL_IO_OUTPUT);
    sm_ec200_init(&g_ec200, g_lte_reset, g_lte_uart);
    sm_ec200_get_network_reg(&g_ec200);
    sm_ec200_get_ip(&g_ec200);
    CLIENT: http_client = sm_http_client_create(&g_ec200);
    while (http_client == NULL) {
        sm_hal_delay_ms(1000);
        goto CLIENT;
    }

    g_modem = &g_ec200.base;

    LOG_DBG(TAG, "READY");
}

void uart_process(sm_hal_uart_t *_this, void* _arg) {
    if (!_this) return;
    if (serial_is_open(g_fd)) {
        static uint8_t data[512] = {0,};
        memset(data, 0, 512);
        int32_t len = serial_recv_bytes(g_fd, data, 512);
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
char *p_buff ;
char c_buff ;

void get_cb(const sm_http_header_t* hd, const uint8_t* pl, uint32_t len, void* _arg);
void script_test(uint32_t times);
void file_compare();

int main(void){
    file_fake = fopen("./hex_fake.hex", "w");
    fclose(file_fake);

    sm_logger_init(log_puts, LOG_LEVEL_DEBUG);

    pthread_t mcu_thread_id;
    pthread_create(&mcu_thread_id, NULL, mcu, NULL);

    init();
    printf("START AUTO TEST ? (y/n) ");
    scanf("%c", &c_buff);
    if (c_buff == 'y') {
        script_test(250);
        return 0;
    }
    char ch;
    while (1)
    {
        printf("q: send command \n");
        printf("w: add header default\n");
        printf("e: add header  \n");
        printf("r: add headers \n");
        printf("t: get partition\n");
        printf("y: get method\n");
        printf("u: post method\n");
        printf("i: \n");
        printf("o: \n");
        scanf("%c", &ch);
        while(ch == '\n')
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
                sm_http_client_add_header_default(http_client);
                break;
            case 'e':
                sm_http_client_add_header(http_client, "Content-Type", "application/json");
                break;
            case 'r': {
                sm_http_header_t header[] = {
                  {.m_type = "Date", .m_content = "2020-02-20 12:00:00"},
                };
                sm_http_client_add_headers(http_client, header, 1);
            }
                break;
            case 't': {
                uint32_t start =0;
                uint32_t len = 0;
                printf("Start: ");
                scanf("%d", &start);
                printf("Lenght: ");
                scanf("%d", &len);
                sm_http_client_set_range_header(http_client, start, len);
            }

                break;
            case 'y': {
                char* gurl = "http://s3.ap-southeast-1.amazonaws.com/selex-bss-v2-log/ota_bss%2FBMS_V1.0.17.rc05%20.hex";
                int32_t st;
                st = sm_http_client_get_method(http_client, gurl, get_cb, NULL);
                LOG_DBG(TAG, "Get Method Status Code: %d", st);
            }
                break;
            case 'u': {
                char* purl = "https://echo.hoppscotch.io";
                int32_t st;
                st = sm_http_client_post_method(http_client, purl, "{\n\"test\":\"data\"\n}",get_cb, NULL);
                LOG_DBG(TAG, "Post Method Status Code: %d", st);
            }
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
void get_cb(const sm_http_header_t* hd, const uint8_t* pl, uint32_t len, void* _arg) {
    LOG_DBG(TAG, "Payload: Lenght: %d Lenght_CF: %d", len, strlen((char*)pl));
    file_fake = fopen("./hex_fake.hex", "a");
    fwrite(pl, 1, len, file_fake);
    fclose(file_fake);
}
void script_test(uint32_t times) {
    uint32_t succ = 0;
    uint32_t sizeoffile = 270163;
    int64_t time_start = 0;
    for (int i = 0; i < times; i++) {
        time_start = get_tick_count();
        file_fake = fopen("./hex_fake.hex", "w");
        fclose(file_fake);
        uint32_t downloaded = 0;
        uint32_t lenght = 0;
        time_start = get_tick_count();
        while (downloaded < sizeoffile) {
            if (sizeoffile-downloaded >= 1024)
                lenght = 1024;
            else
                lenght = (sizeoffile-downloaded)%1025;
            LOG_INF(TAG, "Start: %d, lenght: %d", downloaded, lenght);
            sm_http_client_set_range_header(http_client, downloaded, lenght);
            char* url = "http://s3.ap-southeast-1.amazonaws.com/selex-bss-v2-log/ota_bss%2FBMS_V1.0.17.rc05%20.hex";
            is_passed = false;
            uint8_t retry = 0, fail = 0;
            uint32_t st_code = sm_http_client_get_method(http_client, url, get_cb, NULL);
            LOG_DBG(TAG, "STATUS CODE: %d", st_code);
            while (st_code > 206) {
                LOG_WRN(TAG, "Retry Download: %d", retry);
                st_code = sm_http_client_get_method(http_client, url, get_cb, NULL);
                LOG_DBG(TAG, "STATUS CODE: %d", st_code);
                retry++;
                if (retry > 10) {
                    fail = 1;
                    break;
                }
            }
            if (fail) break;
            downloaded+=lenght;
        }
        file_fake = fopen("./hex_fake.hex", "r");
        file_compare();
        fclose(file_fake);
        if (is_passed) {
            succ++;
            LOG_DBG(TAG, "Download success %d / %d", succ, times);
        }else {
            LOG_WRN(TAG, "Download failed");
        }
        LOG_INF(TAG, "DOWNLOAD TIME: %d", get_tick_count()-time_start);
        sm_hal_delay_ms(1000*60);
    }
    if (succ - times)
        LOG_DBG(TAG, "FAIL");
    else
        LOG_DBG(TAG, "SUCCESS");
}
void file_compare() {
    FILE* _real = fopen("./hex_real.hex", "r");
    FILE* _fake = fopen("./hex_fake.hex", "r");
    if (_fake == NULL || _real == NULL) {
        printf("Error: One or both files could not be opened.\n");
        return;
    }

    int position = 0;
    int diff_found = 0;
    while (!feof(_fake) && !feof(_real)) {
        // Read one byte from each file
        unsigned char fake_byte = 0, real_byte = 0;
        fake_byte = fgetc(_fake);
        real_byte = fgetc(_real);
        if ((fake_byte) && (!real_byte)) {
            if (fake_byte != real_byte) {
                LOG_WRN(TAG,"Difference at byte %d: 0x%02X (fake) vs 0x%02X (real)\n", position, fake_byte, real_byte);
                diff_found = 1;
                is_passed = false;
            }
        }
        position++;
    }
    // Check if one file has more data than the other
    fseek(_fake, 0L, SEEK_END);
    fseek(_real, 0L, SEEK_END);
    uint32_t fake_size = ftell(_fake);
    uint32_t real_size = ftell(_real);
    if ( fake_size!= real_size ) {
        LOG_WRN(TAG,"Files have different sizes. fake: %d, real: %d ",fake_size,real_size);
        diff_found = 1;
        is_passed = false;
    }
    if (!diff_found) {
        LOG_INF(TAG,"Files are identical.");
        is_passed = true;
    }
    fclose(_fake);
    fclose(_real);
}
