/***************************************************************************************************
**    Copyright (C) 1999-2026 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: benchmark_zephyr.c
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

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

#include "benchmark_common.h"

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

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);

#define LED_RED_TOGGLE() gpio_pin_toggle_dt(&led0)
#define LED_GREEN_TOGGLE() gpio_pin_toggle_dt(&led1)
#define LED_BLUE_TOGGLE() gpio_pin_toggle_dt(&led2)

/***************************************************************************************************
**    code
***************************************************************************************************/

uint32_t BENCHMARK_MS_to_Ticks(uint32_t ms)
{
    return ms * (BENCHMARK_TIMER_FREQUENCY / 1000U);
}

void BENCHMARK_hardware_init(void)
{
    /*done by board early init hook just check*/

    BENCHMARK_ASSERT(gpio_is_ready_dt(&led0));
    BENCHMARK_ASSERT(gpio_is_ready_dt(&led1));
    BENCHMARK_ASSERT(gpio_is_ready_dt(&led2));
    int ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);
    BENCHMARK_ASSERT(ret == 0);
    ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
    BENCHMARK_ASSERT(ret == 0);
    ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);
    BENCHMARK_ASSERT(ret == 0);

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

void Benchmark_disable_sys_tick(void)
{
    SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
}

void Benchmark_enable_sys_tick(void)
{
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
}
