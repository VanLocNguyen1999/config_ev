/*
 * ascii_msg_builder.h
 *
 *  Created on: Sep 21, 2025
 *      Author: Admin
 */

#ifndef MSG_BUILDER_ASCII_MSG_BUILDER_H_
#define MSG_BUILDER_ASCII_MSG_BUILDER_H_
#include <stdint.h>
#include <stddef.h>

#define MAX_ASCII_MSG_SIZE		100 // 64Byte
typedef void (*ascii_field_cb_t)(const char* type, const char* data, void* _arg);

void ascii_msg_builder_start(char *buffer, uint8_t buf_size);
void ascii_msg_builder_end(char *buffer, uint8_t buf_size);
/**
// Add a data field (e.g., I:101010, O:1111)
// type    : 'I', 'O', ...
// state   : state value (bitwise)
// n_bits  : number of bits to encode
// is_first: =1 if this is the first field, =0 if there are previous fields
*/
int32_t ascii_msg_builder_frame(char *buffer, uint8_t buf_size,
                                const char *type, const char* data);
int32_t ascii_msg_parse_frame(const char *msg, char *data, size_t data_size);
int32_t ascii_msg_parse_type(const char *payload, char *data_type,
        const char *type, size_t data_size);
int32_t ascii_msg_iterate_fields(char* frame, ascii_field_cb_t cb, void* _arg);
#endif /* MSG_BUILDER_ASCII_MSG_BUILDER_H_ */
