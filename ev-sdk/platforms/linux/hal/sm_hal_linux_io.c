//
// Created by vnbk on 13/09/2024.
//
#include "sm_hal_io.h"

#include <stdlib.h>

typedef struct {
  int32_t m_pin;
  int32_t m_inst;
  int32_t m_oust;
}sm_hal_io_linux_t;


/**
 * @fn sm_hal_io_t sm_hal_io_init*(sm_hal_io_proc_t*, void*, uint16_t)
 * @brief
 *
 * @param m_proc
 * @param handle
 * @param pin
 * @return
 */
sm_hal_io_t* sm_hal_io_init(uint16_t _pin){
  sm_hal_io_linux_t *_this = (sm_hal_io_linux_t *)malloc(sizeof(sm_hal_io_linux_t));
  _this->m_pin = _pin;
 _this->m_inst = 0;
 _this->m_oust = 0;
  return (sm_hal_io_t *)_this;
}
/**
 * @fn void sm_hal_io_deinit(sm_hal_io_t*)
 * @brief
 *
 * @param _this
 */
void sm_hal_io_deinit(sm_hal_io_t *_this){

}
/**
 * @fn int32_t sm_hal_io_set_value(sm_hal_io_t*, uint8_t)
 * @brief
 *
 * @param _this
 * @param level
 * @return
 */
int32_t sm_hal_io_set_value(sm_hal_io_t *_this, uint8_t _value){
 if(!_this) return -1;
 sm_hal_io_linux_t *this = (sm_hal_io_linux_t *)_this;
 this->m_inst = _value;
 this->m_oust = _value;
}
/**
 * @fn uint8_t sm_hal_io_get_value(sm_hal_io_t*)
 * @brief
 *
 * @param _this
 * @return
 */
uint8_t sm_hal_io_get_value(sm_hal_io_t *_this){
 sm_hal_io_linux_t *this = (sm_hal_io_linux_t *)_this;
 return this->m_oust;
}
/**
 * @fn int32_t sm_hal_io_open(sm_hal_io_t*)
 * @brief
 *
 * @param _this
 * @return
 */
int32_t sm_hal_io_open(sm_hal_io_t *_this, sm_hal_io_mode_t _mode){

}
/**
 * @fn int32_t sm_hal_io_close(sm_hal_io_t*)
 * @brief
 *
 * @param _this
 * @return
 */
int32_t sm_hal_io_close(sm_hal_io_t *_this){

}


