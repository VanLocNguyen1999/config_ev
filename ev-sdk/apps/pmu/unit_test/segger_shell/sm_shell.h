/*
 * segger_shell.h
 *
 *  Created on: Jan 22, 2025
 *      Author: admin
 */

#ifndef APPS_PMU_UNIT_TEST_SEGGER_SHELL_SEGGER_SHELL_H_
#define APPS_PMU_UNIT_TEST_SEGGER_SHELL_SEGGER_SHELL_H_

#include <stdint.h>

#define SHELL_CMD_NAME_SIZE		20
#define SHELL_CMD_DESC_SIZE		128
#define	SHELL_CMD_NUM_MAX		32
#define SHELL_CMD_ARG_NUM_MAX	10

void sm_shell_init();

void sm_shell_add_cmd(const char* _cmd_name,
							const char* _cmd_desc,
							void (*handle_fn_t)(uint8_t _argc, char* _args[]),
							uint8_t _arg_num);

void sm_shell_cmd_process();
#endif /* APPS_PMU_UNIT_TEST_SEGGER_SHELL_SEGGER_SHELL_H_ */
