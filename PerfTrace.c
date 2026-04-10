/* **************************************************************************** */
/* File Name	: PerfTrace.c													*/
/* Description	: Tracing tool to trace tasks 									*/
/* Author		: Aswin Prasad <aswin.prasad@electrifex.com>					*/
/* Copyright (C) 2026 Electrifex Technologies India Pvt. Ltd.					*/
/* Model (Derivation)	: The Original											*/
/* **************************************************************************** */

/*------------------------------------------------------------------------------*/
/*  Includes                                                                    */
/*------------------------------------------------------------------------------*/
#include "PerfTrace.h"
#include <string.h>

/* ************************************************************************** */
/* Static Variable Definition Section                                         */
/* ************************************************************************** */

#ifdef DEF_TRACE_ENABLE
static perftrace_config_t g_config;
static perftrace_context_stack_t g_ctx_stack;

/* ************************************************************************** */
/* Global Variable Definition Section                                         */
/* ************************************************************************** */
volatile uint32_t g_trace_index;
uint32_t g_current_context_id = DEF_BACKGROUND_PROCESS_ID;
uint32_t g_trace_stop = DEF_TRACE_RUNNING;
uint8_t g_trace_enable = PERF_TRACE_ENABLE;

/**
 * @brief Initialise the perf trace
 * @attention none
 * @param[in] config - PerfTrace configuration structure
 * @retval    none
*/
void PerfTrace_Init(perftrace_config_t *config)
{
    if (config != NULL)
    {
        g_config = *config;

        g_ctx_stack.top = 0;
        g_ctx_stack.stack[0] = DEF_BACKGROUND_PROCESS_ID;
        g_current_context_id = DEF_BACKGROUND_PROCESS_ID;

        if (g_config.trace_buffer != NULL && g_config.max_records > 0)
        {
            memset(g_config.trace_buffer, 0, g_config.max_records * sizeof(perftrace_record_t));
        }

        g_trace_index = 0;
        g_trace_stop = DEF_TRACE_RUNNING;
    }
}

/**
 * @brief Get the trace log data
 * @attention none
 * @param[in] func_id Func ID
 * @param[in] event_id Event ID
 * @retval    none
*/
void PerfTrace_GetLog(uint32_t func_id, uint16_t event_id)
{
    if (g_trace_stop == DEF_TRACE_RUNNING && g_trace_enable == PERF_TRACE_ENABLE)
    {
        if (g_config.lock_cb) //disable interrupts
        {
            g_config.lock_cb();
        }

        if (g_trace_index >= g_config.max_records)
        {
            /* g_trace_index = 0;   - avoid buffer overflow */
            /* no process */
        }
        else if (g_config.trace_buffer != NULL)
        {
            uint32_t ts = 0;
            uint16_t core_id = 0;

            if (g_config.get_time_cb)		// get time from hardware timer
            {
                ts = g_config.get_time_cb();
            }
            if (g_config.get_core_id_cb)	// get core id from hardware
            {
                core_id = g_config.get_core_id_cb();
            }

            g_config.trace_buffer[g_trace_index].timestamp = ts;
            g_config.trace_buffer[g_trace_index].func_id = func_id;
            g_config.trace_buffer[g_trace_index].context_id = g_current_context_id;
            g_config.trace_buffer[g_trace_index].core_id = core_id;
            g_config.trace_buffer[g_trace_index].event_id = event_id;
            ++g_trace_index;
        }

        if (g_config.unlock_cb)    //enable interrupts
        {
            g_config.unlock_cb();
        }
    }
}

/**
 * @brief set the context ID of entry function or interrupt
 * @attention none
 * @param[in] context_id - context ID
 * @retval    none
*/
void PerfTrace_PushContext(uint32_t context_id)
{
    if(g_trace_stop == DEF_TRACE_RUNNING && g_trace_enable == PERF_TRACE_ENABLE)
    {
        if(g_ctx_stack.top < DEF_STACK_SIZE - 1)
        {
            g_ctx_stack.stack[++g_ctx_stack.top] = context_id;
            g_current_context_id = context_id;
        }
    }
}

/**
 * @brief Get the context ID of previous exit function or interrupt
 * @attention none
 * @param[in] none
 * @retval    none
*/
void PerfTrace_PopContext(void)
{
    if (g_trace_stop == DEF_TRACE_RUNNING && g_trace_enable == PERF_TRACE_ENABLE)
    {
        g_ctx_stack.top--;
        if(g_ctx_stack.top >= 0)
        {
            g_current_context_id = g_ctx_stack.stack[g_ctx_stack.top];
        }
        else
        {
            g_ctx_stack.top = 0;
            g_current_context_id = DEF_BACKGROUND_PROCESS_ID;
        }
    }
}

#endif /* DEF_TRACE_ENABLE */
