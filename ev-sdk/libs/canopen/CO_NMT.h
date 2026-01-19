#ifndef CO_NMT_H_
#define CO_NMT_H_
typedef enum CO_NMT_CS_t CO_NMT_CS;

enum CO_NMT_CS_t{
        CO_NMT_CS_START_NODE=                      1,
        CO_NMT_CS_STOP_NODE=                       2,
        CO_NMT_CS_ENTER_PRE_OP=                    128,
        CO_NMT_CS_RESET_NODE=                      129,
        CO_NMT_CS_RESET_COMM=                      130
};
#endif

