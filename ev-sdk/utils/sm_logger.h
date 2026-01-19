//
// Created by vnbk on 24/03/2023.
//

#ifndef LOGGER_H
#define LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>

#ifdef _SEGGER
#include "common_utils.h"
#endif

#define LOG_BUFFER_SIZE 4096

typedef void (*logger_output_fn_t)(const char*);

extern void sm_logger_put(const char*);

typedef enum {
    LOG_LEVEL_ERR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_TRACE,
    LOG_LEVEL_NUMBER
}LOG_LEVEL;

int32_t sm_logger_init(logger_output_fn_t _fn, LOG_LEVEL _level);
int32_t sm_logger_set_level(LOG_LEVEL _level);
LOG_LEVEL sm_logger_get_level();

//extern int64_t get_tick_count();

/**
 * init logger
 * @param _level : level debug
 * @param _tag   : name
 * @param _log   : log data
 */
void sm_logger_show(LOG_LEVEL _level, const char* _tag, const char* _log, ...);

#if 1

#define LOG_ERR_METHOD(...)   ((void)0)
#define LOG_WRN_METHOD(...)   ((void)0)
#define LOG_INF_METHOD(...)   ((void)0)
#define LOG_DBG_METHOD(...)   ((void)0)

//#define LOG_METHOD APP_PRINT
  
#define LOG_ERR(LOG_TAG,fn_, ...)       LOG_ERR_METHOD("\x1B[31m %d: ", (uint32_t)get_tick_count()); LOG_ERR_METHOD((fn_), ##__VA_ARGS__); LOG_ERR_METHOD("\n");
#define LOG_WRN(LOG_TAG,fn_, ...)       LOG_WRN_METHOD("\x1B[33m %d: ", (uint32_t)get_tick_count()); LOG_WRN_METHOD((fn_), ##__VA_ARGS__); LOG_WRN_METHOD("\n");
#define LOG_INF(LOG_TAG,fn_, ...)       LOG_INF_METHOD("\x1B[32m %d: ", (uint32_t)get_tick_count()); LOG_INF_METHOD((fn_), ##__VA_ARGS__); LOG_INF_METHOD("\n");
#define LOG_DBG(LOG_TAG,fn_, ...)       LOG_DBG_METHOD("\x1B[37m %d: ", (uint32_t)get_tick_count()); LOG_DBG_METHOD((fn_), ##__VA_ARGS__); LOG_DBG_METHOD("\n");

#else 

#define LOG_ERR(LOG_TAG, ...)       sm_logger_show(LOG_LEVEL_ERR,     LOG_TAG,    __VA_ARGS__)
#define LOG_WRN(LOG_TAG, ...)       sm_logger_show(LOG_LEVEL_WARN,    LOG_TAG,    __VA_ARGS__)
#define LOG_INF(LOG_TAG, ...)       sm_logger_show(LOG_LEVEL_INFO,    LOG_TAG,    __VA_ARGS__)
#define LOG_DBG(LOG_TAG, ...)       sm_logger_show(LOG_LEVEL_DEBUG,   LOG_TAG,    __VA_ARGS__)
#define LOG_TRACE(LOG_TAG, ...)     sm_logger_show(LOG_LEVEL_TRACE,   LOG_TAG,    __VA_ARGS__)

#endif


#ifdef __cplusplus
};
#endif

#endif //LOGGER_H
