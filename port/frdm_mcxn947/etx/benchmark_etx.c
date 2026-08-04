/***************************************************************************************************
**    Copyright (C) 1999-2026 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: benchmark_etx.c
**     Summary: ETX project specific configuration
**              NXP MCXN947, ARM Cortex-M33 (FRDM-MCXN947, ETX benchmark)
**
**      Author: Isaac L. L. Yuki
**
****************************************************************************************************
**    Template Version 4
***************************************************************************************************/

/***************************************************************************************************
**    include-files
***************************************************************************************************/

#include "benchmark_common.h"
#include "app.h"

/***************************************************************************************************
**    definitions
***************************************************************************************************/

/***************************************************************************************************
**    prototypes
***************************************************************************************************/

/***************************************************************************************************
**    variables
***************************************************************************************************/

static bool HW_initialized = false;

/***************************************************************************************************
**    code
***************************************************************************************************/

uint32_t BENCHMARK_MS_to_Ticks(uint32_t ms)
{
    return ms * (BENCHMARK_TIMER_FREQUENCY / 1000U);
}

void BENCHMARK_hardware_init(void)
{
    BOARD_InitHardware();
    HW_initialized = true;
}

void BENCHMARK_signal_measurement_start(void)
{
    BENCHMARK_ASSERT(HW_initialized == true);
    const uint32_t signalize_event_duration_ticks = BENCHMARK_MS_to_Ticks(TEST_SIGNALING_EVNT_DURATION);
    BENCHMARK_reset_counter();
    volatile uint32_t counter = BENCHMARK_get_counter_value();
    LED_BLUE_TOGGLE();
    while (BENCHMARK_get_counter_value() - counter < signalize_event_duration_ticks)
    {
        __NOP();
    }

    counter = BENCHMARK_get_counter_value();
    LED_BLUE_TOGGLE();
    while (BENCHMARK_get_counter_value() - counter < signalize_event_duration_ticks)
    {
        __NOP();
    }
}

void BENCHMARK_signal_measurement_stop(void)
{
    BENCHMARK_ASSERT(HW_initialized == true);
    const uint32_t signalize_event_duration_ticks = BENCHMARK_MS_to_Ticks(TEST_SIGNALING_EVNT_DURATION);
    BENCHMARK_reset_counter();
    volatile uint32_t counter = BENCHMARK_get_counter_value();
    LED_GREEN_TOGGLE();
    while (BENCHMARK_get_counter_value() - counter < signalize_event_duration_ticks)
    {
        __NOP();
    }

    counter = BENCHMARK_get_counter_value();
    LED_GREEN_TOGGLE();
    while (BENCHMARK_get_counter_value() - counter < signalize_event_duration_ticks)
    {
        __NOP();
    }
}

void BENCHMARK_signal_jitter_detected(uint32_t *array, size_t size)
{
    (void) array;
    (void) size;
    BENCHMARK_ASSERT(HW_initialized == true);
    const uint32_t signalize_event_duration_ticks = BENCHMARK_MS_to_Ticks(TEST_SIGNALING_EVNT_DURATION);
    BENCHMARK_reset_counter();
    volatile uint32_t counter = BENCHMARK_get_counter_value();

    LED_RED_TOGGLE();
    while (BENCHMARK_get_counter_value() - counter < signalize_event_duration_ticks)
    {
        __NOP();
    }
    LED_RED_TOGGLE();
}
