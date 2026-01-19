/*
 * segger_shell.c
 *
 *  Created on: Jan 22, 2025
 *      Author: admin
 */
#include "sm_shell.h"
#include "common_utils.h"
#include "sm_hal.h"
#include <string.h>

typedef struct {
	char		m_name[SHELL_CMD_NAME_SIZE];
	char		m_description[SHELL_CMD_DESC_SIZE];
	void 		(*handle_fn_t)(uint8_t _argc, char* _args[]);
	uint8_t 	m_arg_num;
} shell_cmd_t;

typedef struct {
	shell_cmd_t		m_cmds[SHELL_CMD_NUM_MAX];
	uint8_t			m_cmd_num;
} segger_shell_t;



static segger_shell_t g_segger_shell_impl;
static segger_shell_t* this = &g_segger_shell_impl;

static void shell_cmd_clear(shell_cmd_t* cmd){
	if (!cmd) {
		return;
	}

	memset(cmd->m_name, 0, SHELL_CMD_NAME_SIZE);
	memset(cmd->m_description, 0, SHELL_CMD_DESC_SIZE);
	cmd->handle_fn_t = NULL;
	cmd->m_arg_num = 0;
}

static void shell_help_func(uint8_t _argc, char* _args[]){
	char		name[SHELL_CMD_NAME_SIZE];
	char		desc[SHELL_CMD_DESC_SIZE];
	__disable_irq();
	for (int i =0; i< this->m_cmd_num; i++){
		memset(name, 32, SHELL_CMD_NAME_SIZE);
		memset(desc, 32, SHELL_CMD_DESC_SIZE);
		memcpy(name, this->m_cmds[i].m_name, strlen(this->m_cmds[i].m_name));
		memcpy(desc, this->m_cmds[i].m_description, strlen(this->m_cmds[i].m_description));
		name[SHELL_CMD_NAME_SIZE-1] = 0;
		desc[SHELL_CMD_DESC_SIZE-1] = 0;
		APP_PRINT(" 	%s : %s \n", name, desc);
		sm_hal_delay_us(300);
	}
	__enable_irq();
}
void sm_shell_init(){
	for (int i=0; i<SHELL_CMD_NUM_MAX; i++){
		shell_cmd_clear(&this->m_cmds[i]);
	}
	this->m_cmd_num = 1;

	memcpy(this->m_cmds[0].m_name, "-help", 5);
	memcpy(this->m_cmds[0].m_description, "Show available commands", sizeof("Show available commands"));
	this->m_cmds[0].handle_fn_t = shell_help_func;
	this->m_cmds[0].m_arg_num = 0;

}

void sm_shell_add_cmd(const char* _cmd_name,
							const char* _cmd_desc,
							void (*handle_fn_t)(uint8_t _argc, char* _args[]),
							uint8_t _arg_num){
	if (!_cmd_name || !handle_fn_t || _cmd_name[0] != '-'){
		return;
	}

	if (strlen(_cmd_name) > SHELL_CMD_NAME_SIZE){
		APP_PRINT("Name of command is too long !");
		return;
	}

	if (strlen(_cmd_desc) > SHELL_CMD_DESC_SIZE){
		APP_PRINT("Description of command is too long !");
		return;
	}

	if (_arg_num > SHELL_CMD_ARG_NUM_MAX){
		APP_PRINT("The number of argument is too much !");
		return;
	}

	if (++this->m_cmd_num > SHELL_CMD_NUM_MAX){
		this->m_cmd_num = SHELL_CMD_NUM_MAX;
		APP_PRINT("Could't add any command more");
		return;
	}
	uint8_t id = 1;
	id = this->m_cmd_num - 1;
	memcpy(this->m_cmds[id].m_name, _cmd_name, strlen(_cmd_name));
	memcpy(this->m_cmds[id].m_description, _cmd_desc, strlen(_cmd_desc));
	this->m_cmds[id].handle_fn_t = handle_fn_t;
	this->m_cmds[id].m_arg_num = _arg_num;
}

void sm_shell_cmd_process(){
	// CHECK INPUT
	if (APP_CHECK_DATA){
		char buff[SHELL_CMD_NAME_SIZE+2];
		memset(buff, 0, SHELL_CMD_NAME_SIZE+2);
		unsigned len = APP_READ(buff);

		if (len < 2) {
			return;
		}

		if (buff[0] == '?'){
			this->m_cmds[0].handle_fn_t(0, NULL);
			return;
		}

		if (buff[0] == '-' && buff[len-1] == '\n'){
			char cmd[SHELL_CMD_NAME_SIZE+2];
			for (int i = 0; i< this->m_cmd_num; i++){
				memcpy(cmd, buff, len);
				char* token = NULL;
				if (this->m_cmds[i].m_arg_num != 0){
					token = strtok(cmd, " ");
				}else{
					token = strtok(cmd, "\n");
				}

				if (token != NULL && !strcmp(this->m_cmds[i].m_name, token) && this->m_cmds[i].handle_fn_t){
					char* args[SHELL_CMD_ARG_NUM_MAX];
					for (int j=0; j < this->m_cmds[i].m_arg_num; j++){
						if (j == this->m_cmds[i].m_arg_num - 1){// the last argument
							token = strtok(NULL, "\n");
						}else{
							token = strtok(NULL, " ");
						}

						if (token == NULL){
							APP_PRINT("!!! Too few arguments to this command !!!\n");
							APP_PRINT(" %s Usage: %s\n", this->m_cmds[i].m_name, this->m_cmds[i].m_description);
							return;
						}
						args[j] = token;
					}
					this->m_cmds[i].handle_fn_t(this->m_cmds[i].m_arg_num, &args[0]);
					return;
				}
			}
			APP_PRINT(" Command is not supported !\n"
						" Use \"-help\" or \"?\" to show all available commands\n");
		}else{
			APP_PRINT(" Command is not supported !\n"
						" Use \"-help\" or \"?\" to show all available commands\n");
		}
	}
}

