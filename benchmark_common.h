/***************************************************************************************************
**    Copyright (C) 1999-2026 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: BENCHMARK_common.h
**     Summary: ETX project specific configuration
**              NXP MCXN947, ARM Cortex-M33 (FRDM-MCXN947, ETX benchmark)
**
**
**      Author: Isaac L. L. Yuki
**
****************************************************************************************************
**    Template Version 4
***************************************************************************************************/

#ifndef BENCHMARK_COMMON_H
#define BENCHMARK_COMMON_H

/***************************************************************************************************
**    include-files
***************************************************************************************************/

#include "stdint.h"
#include "stddef.h"

#include "benchmark_config.h"

/***************************************************************************************************
**    definitions
***************************************************************************************************/

#define TEST_SIGNALING_EVNT_DURATION (1000U) /* in ms */

/*
    Precondition check for the benchmark package.

    Deliberately independent of <assert.h>. The MCUX HAL glue defines NDEBUG build-wide whenever
    CONFIG_ASSERT is off (zephyr/modules/hal_nxp/mcux/CMakeLists.txt, to squelch a warning in
    fsl_flexcan.c), which compiles every assert() in this package to nothing without a diagnostic.
    These checks only run outside a measured window, so they cost nothing where it matters and must
    therefore hold in every build - including the one the published numbers come from.

    The halt itself is a port primitive, see BENCHMARK_haltOnFailedPrecondition.
*/
#define BENCHMARK_ASSERT(cond)                                                                                         \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(cond))                                                                                                   \
        {                                                                                                              \
            BENCHMARK_haltOnFailedPrecondition();                                                                      \
        }                                                                                                              \
    } while (0)

__attribute__((always_inline)) static inline uint32_t BENCHMARK_get_counter_value(void)
{
#ifdef __CORTEX_M
    return DWT->CYCCNT;
#else
    return 0U;
#endif
}

__attribute__((always_inline)) static inline void BENCHMARK_reset_counter(void)
{
#ifdef __CORTEX_M
    DWT->CYCCNT = 0U;
#else
    return;
#endif
}

/***************************************************************************************************
**    prototypes
***************************************************************************************************/

extern void BENCHMARK_hardware_init(void);

extern void BENCHMARK_signal_measurement_start(void);

extern void BENCHMARK_signal_measurement_stop(void);

extern void BENCHMARK_signal_jitter_detected(uint32_t *array, size_t size);

/*
    Stop the core on a violated precondition: mask interrupts, then spin forever. The failing frame
    is left on the stack so a debugger attached afterwards can identify which check tripped.

    Masks configurable-priority exceptions only - NMI and HardFault still take, and on ARMv8-M the
    mask is banked per security state. That is sufficient to stop everything schedulable, which is
    what a violated precondition requires; it is not a hermetic halt.
*/
extern __attribute__((noreturn)) void BENCHMARK_haltOnFailedPrecondition(void);

extern void BENCHMARK_assertQuietSystem(void);

extern void BENCHMARK_assertNeededComponents(void);

extern uint32_t BENCHMARK_MS_to_Ticks(uint32_t ms);

extern void Benchmark_disable_sys_tick(void);

extern void Benchmark_enable_sys_tick(void);

#endif /* BENCHMARK_COMMON_H */
