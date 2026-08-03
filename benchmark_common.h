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

#ifdef __CORTEX_M
#include "core_cm33.h"
#else
#error "Unsupported core architecture. This benchmark is intended for specific architectures."
#endif

/***************************************************************************************************
**    definitions
***************************************************************************************************/

#define TEST_SIGNALING_EVNT_DURATION (100U) /* in ms */

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

extern void BENCHMARK_assertQuietSystem(void);

extern void BENCHMARK_assertNeededComponents(void);

extern uint32_t BENCHMARK_MS_to_Ticks(uint32_t ms);

#endif /* BENCHMARK_COMMON_H */
