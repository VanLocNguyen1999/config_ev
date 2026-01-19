/*
 * sm_hal_flash.h
 *
 *  Created on: Jul 5, 2023
 *      Author: Admin
 */

#ifndef SM_HAL_FLASH_H_
#define SM_HAL_FLASH_H_

#include <stdint.h>

typedef void sm_hal_flash_t;

/**
 * @fn sm_hal_flash_t sm_hal_flash_init*(sm_hal_flash_proc_t*, void*)
 * @brief
 *
 * @param m_proc
 * @param handle
 * @return
 */
sm_hal_flash_t* sm_hal_flash_init(const void* _instance);

/**
 * @fn void sm_hal_flash_deinit(sm_hal_flash_t*)
 * @brief
 *
 * @param _this
 */
void sm_hal_flash_deinit(sm_hal_flash_t *_this);

int32_t sm_hal_flash_get_block_size(sm_hal_flash_t *_this);

/**
 * @fn int32_t sm_hal_flash_write(sm_hal_flash_t*, uint32_t, void*, size_t)
 * @brief
 *
 * @param _this
 * @param addr
 * @param data
 * @param size
 * @return
 */
int32_t sm_hal_flash_write_block(sm_hal_flash_t *_this, uint32_t _addr, void *_data, int32_t _size);

/**
 * @fn int32_t sm_hal_flash_write
 * @brief
 *
 * @param _this
 * @param addr
 * @param data
 * @param size
 * @return
 */
int32_t sm_hal_flash_write_page(sm_hal_flash_t *_this, uint32_t _addr, void *_data, int32_t _size);

/**
 * @fn int32_t sm_hal_flash_read(sm_hal_flash_t*, uint32_t, void*, size_t)
 * @brief
 *
 * @param _this
 * @param addr
 * @param data
 * @param size
 * @return
 */
int32_t sm_hal_flash_read(sm_hal_flash_t *_this, uint32_t _addr, void *_data, int32_t _size);

/**
 * @fn int32_t sm_hal_flash_erase(sm_hal_flash_t*, uint32_t, size_t)
 * @brief
 * @param _this
 * @param addr
 * @param size
 * @return
 */
int32_t sm_hal_flash_erase_block(sm_hal_flash_t *_this, uint32_t _addr, int32_t _size);

/**
 * @fn int32_t sm_hal_flash_erase(sm_hal_flash_t*, uint32_t, size_t)
 * @brief
 * @param _this
 * @param addr
 * @param size
 * @return
 */
int32_t sm_hal_flash_erase_page(sm_hal_flash_t *_this, uint32_t _addr, int32_t _size);


#endif /* SM_HAL_FLASH_H_ */
