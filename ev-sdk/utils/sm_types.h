//
// Created by vnbk on 12/04/2024.
//

#ifndef EV_SDK_SM_TYPES_H
#define EV_SDK_SM_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifndef uint8_t
typedef uint8_t uint8_t;
#endif

#ifndef int8_t
typedef int8_t int8_t;
#endif

#ifndef uint16_t
typedef uint16_t uint16_t;
#endif

#ifndef int16_t
typedef int16_t int16_t;
#endif

#ifndef uint32_t
typedef uint32_t uint32_t;
#endif

#ifndef int32_t
typedef int32_t int32_t;
#endif

#ifndef bool
#typedef bool uint8_t
#endif

#define PI_NUMBER  (3.14159265359f)

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#ifdef linuxddd
#include <pthread.h>
typedef pthread_mutex_t sm_mutex;

#define MUTEX_INIT(x)           (pthread_mutex_init((x), NULL))

#define ENTER_CRITICAL(x)       (pthread_mutex_lock(x))
#define EXIT_CRITICAL(x)        (pthread_mutex_unlock(x))


#elif defined(__RTOS)

#include "FreeRTOS.h"
#include "semphr.h"
typedef SemaphoreHandle_t sm_mutex;

#define MUTEX_INIT(x)           (x=xSemaphoreCreateMutex())

#define ENTER_CRITICAL(x)       (xSemaphoreTake(x,portMAX_DELAY))
#define EXIT_CRITICAL(x)        (xSemaphoreGive(x))

#else

typedef void* sm_mutex;

#define MUTEX_INIT(x)           (void)(x)

#define ENTER_CRITICAL(x)       (void)(x)
#define EXIT_CRITICAL(x)        (void)(x)

#endif

#endif //EV_SDK_SM_TYPES_H
