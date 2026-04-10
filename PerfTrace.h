/* **************************************************************************** */
/* File Name	: PerfTrace.h												*/
/* Description	: Tracing tool to trace tasks 									*/
/* Author		: Aswin Prasad <aswin.prasad@electrifex.com>					*/
/* Copyright (C) 2026 Electrifex Technologies India Pvt. Ltd.					*/
/* Model (Derivation)	: The Original											*/
/* **************************************************************************** */
#ifndef PERF_TRACE_H_
#define PERF_TRACE_H_

#include <stdint.h>
#include <stddef.h>

#define DEF_TRACE_ENABLE

#define DEF_STACK_SIZE              50
#define DEF_BACKGROUND_PROCESS_ID   0x0B
#define DEF_PERF_TRACE_FUNC_START	3
#define DEF_PERF_TRACE_FUNC_END		4
#define DEF_TRACE_STOP				(1)
#define DEF_TRACE_RUNNING			(0)
#define PERF_TRACE_ENABLE           (1)
#define PERF_TRACE_DISABLE          (0)

#ifdef DEF_TRACE_ENABLE
#define	PERF_TRACE_FN_START(func_id)		PerfTrace_GetLog(func_id, DEF_PERF_TRACE_FUNC_START)
#define	PERF_TRACE_FN_END(func_id)			PerfTrace_GetLog(func_id, DEF_PERF_TRACE_FUNC_END)
#define PERF_PUSH_CONTEXT(context_id)		PerfTrace_PushContext(context_id)
#define PERF_POP_CONTEXT()			        PerfTrace_PopContext()
#else
#define PERF_TRACE_FN_START(func_id)		((void)0)
#define PERF_TRACE_FN_END(func_id)		    ((void)0)
#define PERF_PUSH_CONTEXT(context_id)		((void)0)
#define PERF_POP_CONTEXT()			        ((void)0)
#endif

/* ************************************************************************** */
/* Structure Type Definition Section                                          */
/* ************************************************************************** */
typedef struct
{
    uint32_t  func_id;
    uint32_t  timestamp;
    uint32_t  context_id;
    uint16_t  core_id;
    uint16_t  event_id;
} perftrace_record_t;

typedef struct
{
    uint32_t    stack[DEF_STACK_SIZE];
    int32_t     top;
} perftrace_context_stack_t;

typedef struct {
    uint32_t (*get_time_cb)(void);       /* Callback to fetch high-res timestamp */
    uint16_t (*get_core_id_cb)(void);    /* Callback to get current Core ID */
    void (*lock_cb)(void);               /* Callback to disable interrupts/spin_lock */
    void (*unlock_cb)(void);             /* Callback to enable interrupts/spin_unlock */
    perftrace_record_t *trace_buffer;    /* Pointer to trace buffer memory */
    uint32_t max_records;                /* Maximum number of trace records the buffer can hold */
} perftrace_config_t;

/* ************************************************************************** */
/* Extern Declaration Section                                                 */
/* ************************************************************************** */
extern volatile uint32_t g_trace_index;

/* ************************************************************************** */
/* Functions                                                                  */
/* ************************************************************************** */
void PerfTrace_Init(perftrace_config_t *config);
void PerfTrace_GetLog(uint32_t func_id, uint16_t event_id);
void PerfTrace_PushContext(uint32_t context_id);
void PerfTrace_PopContext(void);

#endif /* PERF_TRACE_H_ */
