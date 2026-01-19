#include <stdint.h>
#include <float.h>

#define HEX_ASCII_2_U8(chr)             (uint8_t)strtol(chr, NULL, 16)
#define HEX_ASCII_2_U16(chr)            (uint16_t)strtol(chr, NULL, 16)

void string_swap(const char* p_buffer,char* p_msg,const uint8_t len);
int long_to_string(const uint32_t data,char* s);
int slong_to_string(const int32_t data,char* s);
void byte_to_hex_ascii(const uint8_t s,char* hex);
uint16_t slen(const char *str);
void string_join(char* dst,const char* str);
uint8_t str_contain(const char* dst,const char* str);
uint16_t string_copy(char* dst,const char*src);
uint16_t string_package(char* dst, const char* src);
uint16_t string_count_character(char* str, char chr);
char* string_split_token(char* buff, char* token);
// void parse_mqtt(char * source, char *topic, char *paylad);
int32_t shortest_substring(const char *str, const char *start_sub, const char *end_sub,
                            char *result, int32_t max_len);

char *sm_strtok(char *str, const char *delim);

/// String to number
double sm_atof(const char *str);
int sm_atoi(const char *s);


