/*
 * Copyright (C) 2021 Ant Group.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#ifndef _DEBUG_ENGINE_H
#define _DEBUG_ENGINE_H

#include "bh_list.h"
#include "gdbserver.h"
#include "thread_manager.h"

typedef enum WASMDebugControlThreadStatus {
    RUNNING,
    STOPPED,
} WASMDebugControlThreadStatus;

struct WASMDebugEngine;
struct WASMDebugInstance;

typedef struct WASMDebugControlThread {
    WASMGDBServer *server;
    korp_tid tid;
    korp_mutex wait_lock;
    char ip_addr[128];
    int port;
    WASMDebugControlThreadStatus status;
    struct WASMDebugEngine *debug_engine;
    struct WASMDebugInstance *debug_instance;
} WASMDebugControlThread;

typedef struct WASMDebugBreakPoint {
    struct WASMDebugBreakPoint *next;
    uint64 addr;
    uint64 orignal_data;
} WASMDebugBreakPoint;

typedef struct WASMDebugInstance {
    struct WASMDebugInstance *next;
    WASMDebugControlThread *control_thread;
    bh_list break_point_list;
    WASMCluster *cluster;
    uint32 id;
    korp_tid current_tid;
    korp_mutex wait_lock;
    korp_cond wait_cond;
} WASMDebugInstance;

typedef enum WASMDebugEventKind {
    BREAK_POINT_ADD,
    BREAK_POINT_REMOVE
} WASMDebugEventKind;

typedef struct WASMDebugEvent {
    WASMDebugEventKind kind;
    unsigned char metadata[0];
} WASMDebugEvent;

typedef struct WASMDebugMemoryInfo {
    uint64 start;
    uint64 size;
    char name[128];
    char permisson[4];
} WASMDebugMemoryInfo;

typedef enum WasmAddressType {
    WasmMemory = 0x00,
    WasmObj = 0x01,
    WasmInvalid = 0x03
} WasmAddressType;

#define WASM_ADDR(type, id, offset) \
    (((uint64)type << 62) | ((uint64)0 << 32) | ((uint64)offset << 0))

#define WASM_ADDR_TYPE(addr) (((addr)&0xC000000000000000) >> 62)
#define WASM_ADDR_OFFSET(addr) (((addr)&0x00000000FFFFFFFF))

#define INVALIED_ADDR (0xFFFFFFFFFFFFFFFF)

WASMDebugInstance *
wasm_debug_instance_create(WASMCluster *cluster);

void
wasm_debug_instance_destroy(WASMCluster *cluster);

WASMDebugInstance *
wasm_exec_env_get_instance(WASMExecEnv *exec_env);

bool
wasm_debug_engine_init(char *ip_addr, int platform_port, int process_port);

void
wasm_debug_engine_destroy();

void
wasm_debug_set_engine_active(bool active);

bool
wasm_debug_get_engine_active(void);

uint64
wasm_debug_instance_get_pid(WASMDebugInstance *instance);

uint64
wasm_debug_instance_get_tid(WASMDebugInstance *instance);

int
wasm_debug_instance_get_tids(WASMDebugInstance *instance, uint64 tids[],
                             int len);

void
wasm_debug_instance_set_cur_thread(WASMDebugInstance *instance, uint64 tid);

uint64
wasm_debug_instance_get_pc(WASMDebugInstance *instance);

uint64
wasm_debug_instance_get_load_addr(WASMDebugInstance *instance);

WASMDebugMemoryInfo *
wasm_debug_instance_get_memregion(WASMDebugInstance *instance, uint64 addr);

void
wasm_debug_instance_destroy_memregion(WASMDebugInstance *instance,
                                      WASMDebugMemoryInfo *mem_info);

bool
wasm_debug_instance_get_obj_mem(WASMDebugInstance *instance, uint64 addr,
                                char *buf, uint64 *size);

bool
wasm_debug_instance_get_linear_mem(WASMDebugInstance *instance, uint64 addr,
                                   char *buf, uint64 *size);

bool
wasm_debug_instance_get_mem(WASMDebugInstance *instance, uint64 addr, char *buf,
                            uint64 *size);

bool
wasm_debug_instance_set_mem(WASMDebugInstance *instance, uint64 addr, char *buf,
                            uint64 *size);

int
wasm_debug_instance_get_call_stack_pcs(WASMDebugInstance *instance, uint64 tid,
                                       uint64 buf[], uint64 size);

bool
wasm_debug_instance_add_breakpoint(WASMDebugInstance *instance, uint64 addr,
                                   uint64 length);

bool
wasm_debug_instance_remove_breakpoint(WASMDebugInstance *instance, uint64 addr,
                                      uint64 length);

bool
wasm_debug_instance_continue(WASMDebugInstance *instance);

bool
wasm_debug_instance_kill(WASMDebugInstance *instance);

uint64
wasm_debug_instance_wait_thread(WASMDebugInstance *instance, uint64 tid,
                                uint32 *status);

uint32
wasm_debug_instance_get_thread_status(WASMDebugInstance *instance, uint64 tid);

bool
wasm_debug_instance_singlestep(WASMDebugInstance *instance, uint64 tid);

bool
wasm_debug_instance_get_local(WASMDebugInstance *instance, int frame_index,
                              int local_index, char buf[], int *size);

bool
wasm_debug_instance_get_global(WASMDebugInstance *instance, int frame_index,
                               int global_index, char buf[], int *size);

#if WASM_ENABLE_LIBC_WASI != 0
bool
wasm_debug_instance_get_current_object_name(WASMDebugInstance *instance,
                                            char name_buffer[], int len);
#endif

uint64
wasm_debug_instance_mmap(WASMDebugInstance *instance, uint32 size,
                         int map_port);

bool
wasm_debug_instance_ummap(WASMDebugInstance *instance, uint64 addr);
#endif