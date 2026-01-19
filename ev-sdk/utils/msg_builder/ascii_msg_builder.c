/*
 * ascii_msg_builder.c
 *
 *  Created on: Sep 21, 2025
 *      Author: Admin
 */
#include "ascii_msg_builder.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define CHECK_SUM_SIZE      1  // 1 byte
#define SOF_BYTE _SIZE      1  // 1 byte
#define EOF_BYTE _SIZE      1  // 1 byte
#define MIN_MSG_SIZE        5
/*checksum XOR from payload*/
static uint8_t msg_calc_checksum(const char *data) {
    uint8_t cs = 0;
    for (int i = 0; data[i] != '\0'; i++) {
        cs ^= (uint8_t)data[i];
    }
    return cs;
}

void ascii_msg_builder_start(char *buffer, uint8_t buf_size) {
    if (buf_size > 0) {
        buffer[0] = '<';
        buffer[1] = '\0';
    }
}
void ascii_msg_builder_end(char *buffer, uint8_t buf_size) {
    char cs_str[12] = "";
    uint8_t cs = msg_calc_checksum(buffer + CHECK_SUM_SIZE);

    snprintf(cs_str, sizeof(cs_str), ",CS:%02X>", cs);
    size_t cur_len = strlen(buffer);
    size_t cs_len  = strlen(cs_str);

    if (cur_len + cs_len < buf_size) {
        strcat(buffer, cs_str);
        buffer[cur_len + cs_len] = '\0';  // đảm bảo '\0' ở cuối
    }
}
int32_t ascii_msg_builder_frame(char *buffer, uint8_t buf_size,
                                const char *type, const char* data){
    char frame[MAX_ASCII_MSG_SIZE] = {0};
    uint8_t len = 0;

    if (strlen(buffer) == 0) {
        ascii_msg_builder_start(buffer, buf_size);
    }else {
        char *cs_pos = strstr(buffer, ",CS:");
        if (cs_pos) {
            *cs_pos = '\0';
        }
        frame[len++] = ',';
    }

    strncat(frame + len, type, sizeof(frame) - len - 1);
    len += (uint8_t)strlen(type);

    frame[len++] = ':';
    frame[len] = '\0';

    strncat(frame, data, sizeof(frame) - len - 1);
    len += (uint8_t)strlen(data);
    frame[len] = '\0';

    if ((uint8_t)strlen(buffer) + len >= buf_size - 1) {
        return -1;
    }

    strcat(buffer, frame);
    ascii_msg_builder_end(buffer, buf_size);

    return 0;
}

int32_t ascii_msg_parse_frame(const char *msg, char *data, size_t data_size) {
    if (!msg || !data || msg[0] != '<')
        return -1;

    size_t len = strlen(msg);
    if (len < MIN_MSG_SIZE || msg[len - 1] != '>')
        return -2;

    char payload[MAX_ASCII_MSG_SIZE] = "";
    if (len - 2 >= sizeof(payload))
        return -3; //

    // copy phần giữa < >
    strncpy(payload, msg + 1, len - 2);
    payload[len - 2] = '\0';

    // tìm checksum
    char *cs_pos = strstr(payload, ",CS:");
    if (!cs_pos)
        return -4;

    *cs_pos = '\0'; // cắt bỏ phần checksum
    uint8_t cs_recv = (uint8_t) strtol(cs_pos + 4, NULL, 16);

    // tính checksum
    uint8_t cs_calc = msg_calc_checksum(payload);
    if (cs_calc != cs_recv)
        return -5; // checksum sai

    // copy payload ra ngoài
    strncpy(data, payload, data_size - 1);
    data[data_size - 1] = '\0';

    return 0;
}


int32_t ascii_msg_parse_type(const char *payload, char *data_type,
                             const char *type, size_t data_size) {
    char tmp[MAX_ASCII_MSG_SIZE] = "";
    strncpy(tmp, payload, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';

    char *field = strtok(tmp, ",");
    while (field) {
        size_t type_len = strlen(type);
        if (strncmp(field, type, type_len) == 0 && field[type_len] == ':') {
            size_t len = strlen(field + type_len + 1);
            if (len >= data_size)
                return -3;
            strcpy(data_type, field + type_len + 1);
            return 0;
        }
        field = strtok(NULL, ",");
    }

    return -2; // không tìm thấy field
}

int32_t ascii_msg_iterate_fields(char* frame, ascii_field_cb_t cb, void* _arg) {
    if (!frame || !cb) return -1;

    uint8_t found = 0;
    char* token = strtok(frame, ",");
    while (token) {
        char* colon = strchr(token, ':');
        if (colon) {
            *colon = '\0';
            const char* type = token;
            const char* data = colon + 1;
            cb(type, data, _arg);
            found = 1;
        }
        token = strtok(NULL, ",");
    }

    return found ? 0 : -1;
}



