#include "PerfTrace.h"
#include <stdint.h>

// ---------------------------------------------------------
// 1. Define hardware-specific callbacks for your Baremetal target
// ---------------------------------------------------------

static uint32_t my_get_time(void) {
    // Example: Return 32-bit hardware timer tick
    // In ADI DSP, this might be your adi_tmr_GetCount or EMUCLK read.
    return *pREG_TIMER0_TMR0_CNT;
}

static uint16_t my_get_core_id(void) {
    // Example: Return local core ID
    return adi_core_id(); 
}

static void my_lock(void) {
    // Example: Disable interrupts before writing to trace buffer
    disable_interrupts();
}

static void my_unlock(void) {
    // Example: Restore/Enable interrupts after writing
    enable_interrupts();
}

// Global buffer storage block
#define MAX_RECORDS 10000
TraceDataStruct_ts trace_memory_pool[MAX_RECORDS];

// ---------------------------------------------------------
// 2. Initialization Routine
// ---------------------------------------------------------

void App_InitTracing(void) {
    // Wire up your port-specific functions to the tracing config
    PerfTraceConfig_t cfg;
    cfg.get_time_cb    = my_get_time;
    cfg.get_core_id_cb = my_get_core_id;
    cfg.lock_cb        = my_lock;
    cfg.unlock_cb      = my_unlock;
    
    // Wire your memory pointers
    cfg.trace_buffer   = trace_memory_pool;
    cfg.max_records    = MAX_RECORDS;
    
    PerfTrace_Init(&cfg);
}

// ---------------------------------------------------------
// 3. Usage Example: Tracing a normal application Function
// ---------------------------------------------------------
// It is recommended to define some enumerations for your context and function
void AudioFilterTask(void) {
    // 1. Signal context switch into this task (context push)
    PERF_PUSH_CONTEXT((uint32_t)(AudioFilterTask));
    
    // 2. Start evaluating your function
    PERF_TRACE_FN_START((uint32_t)(AudioFilterTask));
    
    // ... [DO HARD DSP WORK HERE] ...
    
    // 3. Stop evaluating your function
    PERF_TRACE_FN_END((uint32_t)(AudioFilterTask));
    
    // 4. Pop back to background context
    PERF_POP_CONTEXT();
}

// ---------------------------------------------------------
// 4. Usage Example: Tracing inside a Hardware Interrupt
// ---------------------------------------------------------

void UART_DMA_Interrupt_Handler(void) {
    // As soon as the ISR fires, we push the context. This registers that we preempted 
    // whatever was currently running.
    PERF_PUSH_CONTEXT((uint32_t)(UART_DMA_Interrupt_Handler));
    
    // Start timing the ISR function body
    PERF_TRACE_FN_START((uint32_t)(UART_DMA_Interrupt_Handler));
    
    // ... [CLEAR INTERRUPT FLAGS & DO DMA WORK] ...
    
    // Stop timing the ISR function body
    PERF_TRACE_FN_END((uint32_t)(UART_DMA_Interrupt_Handler));

    // Pop the ISR context to hand context time back to the task we interrupted
    PERF_POP_CONTEXT();
}
