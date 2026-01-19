//
// Created by vnbk on 10/06/2023.
//

#ifndef SM_CORE_FS_IF_H
#define SM_CORE_FS_IF_H

#ifdef __cplusplus
extern "C"{
#endif

#include "sm_memory.h"

typedef struct sm_core_fs           sm_core_fs_t;
typedef struct sm_core_file         sm_core_fs_file_t;
typedef struct sm_fs_dir_if         sm_core_fs_dir_t;

typedef enum {
    FS_SEEK_BEGIN,
    FS_SEEK_CUR,
    FS_SEEK_END
}SM_FS_POS;

int32_t sm_fs_mount(sm_core_fs_t*, void*);
int32_t sm_fs_umount(sm_core_fs_t*);
int32_t sm_fs_format(sm_core_fs_t*);

int32_t sm_fs_open(sm_core_fs_t*, sm_core_fs_file_t*, const char*, uint8_t);
int32_t sm_fs_close(sm_core_fs_t*, sm_core_fs_file_t*);
int32_t sm_fs_read(sm_core_fs_t*, sm_core_fs_file_t*, char*, int32_t);
int32_t sm_fs_write(sm_core_fs_t*, sm_core_fs_file_t*, const char*, int32_t);
int32_t sm_fs_size(sm_core_fs_t*, sm_core_fs_file_t*);
int32_t sm_fs_seek(sm_core_fs_t*, sm_core_fs_file_t*, int32_t, int32_t);
int32_t sm_fs_tell(sm_core_fs_t*, sm_core_fs_file_t* _file);

#ifdef __cplusplus
};
#endif

#endif //SM_CORE_FS_IF_H
