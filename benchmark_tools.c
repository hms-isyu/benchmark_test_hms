/*******************************************************************************
 **    Copyright (C) 1999-2026 HMS Technology Center Ravensburg GmbH
 **    All rights reserved
 **
 **        File: benchmark_tools.c
 **     Summary: Benchmarking tools for Cycle-Count benchmarking methods.
 **      Author: Isaac L. L. Yuki
 ** Responsible: T. Drexel
 **
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "benchmark_common.h"
#include "benchmark_tools.h"
#include "stdbool.h"
#include "math.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define LOOP_GUARD_COUNT (10000000U) /* number of iterations to run before measuring the overhead of the loop itself */

/*******************************************************************************
 * Variables
 ******************************************************************************/

static bool g_WarmUp = false; /* flag to indicate if the system has been warmed up */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void     BENCHMARK_systemWarmup(void);
static uint32_t BENCHMARK_measureEmptyLoop(uint32_t loop_count);

__attribute__((noinline, aligned(16))) static uint32_t BENCHMARK_measureEmptyLoop(uint32_t loop_count)
{
    volatile uint32_t t0, t1;
    __DSB();
    __ISB();
    t0 = BENCHMARK_get_counter_value(); /* no reset */
    for (uint32_t i = 0; i < loop_count; i++)
    {
    }
    t1 = BENCHMARK_get_counter_value();
    return t1 - t0;
}

/*******************************************************************************
 * Code
 ******************************************************************************/

/*
    The system can cause on bring up a lot of jitter in the the cycle measurement.
    This happens because of Debbugger
*/
static void BENCHMARK_systemWarmup(void)
{
    g_WarmUp = true;
    for (uint32_t i = 0; i < LOOP_GUARD_COUNT; i++)
    {
        __NOP();
    }
}

bool BENCHMARK_calc_overhead(uint32_t loop_count, uint32_t *loop_overhead, uint32_t *cyccnt_assignment_overhead)
{
    if (g_WarmUp == false)
    {
        BENCHMARK_assertQuietSystem();
        BENCHMARK_systemWarmup();
        BENCHMARK_assertNeededComponents();
    }
    volatile uint32_t loop_oh_1 = 0U;
    volatile uint32_t loop_oh_2 = 0U;
    volatile uint32_t t0        = 0;
    volatile uint32_t t1        = 0;

    __DSB();
    __ISB();
    t0                          = BENCHMARK_get_counter_value();
    t1                          = BENCHMARK_get_counter_value();
    *cyccnt_assignment_overhead = t1 - t0;

    __DSB();
    __ISB();
    for (uint32_t i = 0; i < loop_count; i++)
    {
        t0 = BENCHMARK_get_counter_value();
        t1 = BENCHMARK_get_counter_value();
        if ((t1 - t0) != *cyccnt_assignment_overhead)
        {
            BENCHMARK_signal_jitter_detected(NULL, 0U);
        }
    }

    BENCHMARK_reset_counter();

    for (uint32_t i = 0; i < 10; i++)
    {
        __NOP();
    }

    loop_oh_1 = BENCHMARK_measureEmptyLoop(loop_count);
    loop_oh_2 = BENCHMARK_measureEmptyLoop(loop_count);

    if (loop_oh_1 != loop_oh_2)
    {
        BENCHMARK_signal_jitter_detected(NULL, 0U);
    }

    if (loop_overhead != NULL)
    {
        *loop_overhead = loop_oh_1 % loop_count;
    }
    return true;
}

void BENCHMARK_check_hw_influence(uint32_t loop_count)
{
    static uint32_t raw[64U];
    uint32_t        min           = 0;
    uint32_t        max           = 0;
    uint32_t        outlier_count = 0;
    uint32_t        result[2]     = {0U, 0U}; /* result[0] = min, result[1] = max */
    uint32_t       *data          = result;
    BENCHMARK_assertQuietSystem();
    for (uint32_t r = 0; r < 64U; r++)
    {
        raw[r] = BENCHMARK_measureEmptyLoop(loop_count);
        if (r == 0)
        {
            min = raw[r];
            continue;
        }
        else
        {
            if (raw[r] < min)
            {
                min = raw[r];
            }
        }
        if (raw[r] > max)
        {
            max = raw[r];
        }
        if (raw[r] != min)
        {
            result[0] = ++outlier_count;
            result[1] = raw[r];
            BENCHMARK_signal_jitter_detected(data, 2U);
        }
    }
    __NOP(); // Allow setting BP in the IDE.
}
