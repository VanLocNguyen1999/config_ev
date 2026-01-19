/*
 * sm_syscalls.c
 *
 *  Created on: Aug 26, 2025
 *      Author: Admin
 */

#include <sys/stat.h>
#include <unistd.h>
#include "sm_syscalls.h"
int _close(int file) {

    (void) file;
    return -1;
}

int _fstat(int file, struct stat *st) {
    (void) file;
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int file) {
    (void) file;
    return 1;
}

int _lseek(int file, int ptr, int dir) {
    (void) file;
    (void) ptr;
    (void) dir;
    return 0;
}

int _read(int file, char *ptr, int len) {
    (void) file;
    (void) ptr;
    (void) len;
    return 0;  // hoặc return len nếu muốn giả lập có dữ liệu
}

int _write(int file, char *ptr, int len) {

    (void)file;
    (void)ptr;
    // Không gửi gì, chỉ giả vờ là đã gửi hết dữ liệu
    return len;
}
