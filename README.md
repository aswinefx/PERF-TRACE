# PerfTrace

PerfTrace is a highly-portable, hardware-agnostic tracing library for embedded baremetal C systems. It allows developers to record timeline data of task executions, system interrupts, and function calls, and then convert that binary trace data into a rich visual JSON format compatible with [Google Perfetto](https://ui.perfetto.dev/).

---

## 🏗️ Architecture

The PerfTrace ecosystem is built entirely decoupled from any specific toolchain or hardware. It consists of two components:
1. **The C Tracing Library (`PerfTrace.c` / `PerfTrace.h`)**: Pure C library to buffer microsecond/nanosecond executions natively on your target embedded hardware.
2. **The Python Converter (`tool_perf_trace.py`)**: A command-line utility used on your host machine to convert binary footprint data collected from the device into the Perfetto `.json` timeline format.

---

## 1. The C Tracing Library

**Features:**
- Nested context awareness (knows when a background thread is preempted by a hardware interrupt ISR, and restores the sequence afterward).
- Extremely lightweight struct sizes for fast memory dumping.

### Integration
To use PerfTrace, populate a `perftrace_config_t` with function pointers wrapping your target hardware's timer and lock operations, and initialize the system.

```c
#include "PerfTrace.h"

// 1. Declare the trace buffer footprint in memory
#define MAX_TRACE_RECORDS 10000
perftrace_record_t g_trace_buffer[MAX_TRACE_RECORDS];

// 2. Wrap your target OS/hardware APIs
uint32_t my_get_time()    { return *MY_HW_TIMER_REGISTER; }
uint16_t my_get_core_id() { return ARM_READ_CORE_ID(); }
void     my_lock()        { DISABLE_INTERRUPTS(); }
void     my_unlock()      { ENABLE_INTERRUPTS(); }

// 3. Initialize tracer
void Init_Tracing() {
    perftrace_config_t cfg;
    cfg.get_time_cb    = my_get_time;
    cfg.get_core_id_cb = my_get_core_id;
    cfg.lock_cb        = my_lock;
    cfg.unlock_cb      = my_unlock;
    cfg.trace_buffer   = g_trace_buffer;
    cfg.max_records    = MAX_TRACE_RECORDS;
    
    PerfTrace_Init(&cfg);
}
```

### Basic Usage

PerfTrace creates rich track lanes by grouping timing into **Contexts** and **Functions**.

```c
// Track some Heavy Computation Thread taking place over time:
void HeavyComputationTask(void) {
    // Tell the tool to enter the "Computation" timeline lane
    PERF_PUSH_CONTEXT((uint32_t)HeavyComputationTask);
    
    // Start the stopwatch for a specific event
    PERF_TRACE_FN_START(0x101);
    
    // ... Hard CPU work ...
    
    // Stop the stopwatch
    PERF_TRACE_FN_END(0x101);
    
    // Pop back out so other things can be tracked correctly
    PERF_POP_CONTEXT();
}
```

When your buffer is filled up, your application is responsible for pushing `g_trace_buffer` across whatever peripheral you have (UART, Ethernet, Flash storage) as a binary `.bin` or `.dat` dump!

---

## 2. Python Converter (`tool_perf_trace.py`)

Once you've dumped the raw binary data out of your chip, use the Python utility to inject it into the industry-standard visual format.

### Prerequisites
Make sure your python environment has pandas and numpy installed:
```bash
pip install pandas numpy
```

### Usage
Run the script passing your input binary dump and your desired output JSON file perfectly ready for `ui.perfetto.dev`.

**Standard Mode:**
If your Hardware Timer directly outputs the unit you want to analyze (or if you don't need scaling):
```bash
python tool_perf_trace.py trace_dump.bin output.json
```

**Global Synchronization / Single-Core Multiplier:**
If your CPU timer is running at clock speeds where the output tick corresponds to a unit that needs to be scaled identically across all tracking:
```bash
python tool_perf_trace.py trace_dump.bin output.json --multipliers 10
```
*(This multiplies all timestamp ticks found in the binary block by 10 uniformly)*

**Complex Mutli-Core Systems:**
If you have a dual-M7 or a massive multicore DSP cluster where different core zones run at wildly different clock speeds, you can apply scalers granularly to core IDs directly:
```bash
python tool_perf_trace.py trace_dump.bin output.json --multipliers "1:8,2:8,3:15"
```
*(Multiplies ticks on Core 1 by 8, Core 2 by 8, Core 3 by 15).*

### Visualize
Once the script successfully completes, navigate to [Google Perfetto UI](https://ui.perfetto.dev/) in Google Chrome or Edge, click "Open Trace file", and load the generated `output.json`. You will instantly see fully charted, zoomed timeline blocks representing your baremetal behavior.
