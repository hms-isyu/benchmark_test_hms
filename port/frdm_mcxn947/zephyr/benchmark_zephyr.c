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

#define LED_BLUE_TOGGLE() gpio_pin_toggle_dt(&led2)
#define LED_RED_TOGGLE() gpio_pin_toggle_dt(&led0)
#define LED_GREEN_TOGGLE() gpio_pin_toggle_dt(&led1)

/***************************************************************************************************
**    code
***************************************************************************************************/

uint32_t BENCHMARK_MS_to_Ticks(uint32_t ms)
{
    return (ms * BENCHMARK_TIMER_FREQUENCY) / 1000U;
}

void BENCHMARK_hardware_init(void)
{
    /*done by board early init hook just check*/

    assert(gpio_is_ready_dt(&led0));
    assert(gpio_is_ready_dt(&led1));
    assert(gpio_is_ready_dt(&led2));
    int ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);
    assert(ret == 0);
    ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
    assert(ret == 0);
    ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);
    assert(ret == 0);

    HW_initialized = true;
}

void BENCHMARK_signal_measurement_start(void)
{
    assert(HW_initialized == true);
    const uint32_t signalize_event_duration_ticks = BENCHMARK_MS_to_Ticks(TEST_SIGNALING_EVNT_DURATION);
    LED_BLUE_TOGGLE();
    for (uint32_t i = 0; i < signalize_event_duration_ticks; i++)
    {
        __NOP();
    }
    LED_BLUE_TOGGLE();
}

void BENCHMARK_signal_measurement_stop(void)
{
    assert(HW_initialized == true);
    const uint32_t signalize_event_duration_ticks = BENCHMARK_MS_to_Ticks(TEST_SIGNALING_EVNT_DURATION);
    LED_BLUE_TOGGLE();
    for (uint32_t i = 0; i < signalize_event_duration_ticks; i++)
    {
        __NOP();
    }
    LED_BLUE_TOGGLE();
}

void BENCHMARK_signal_jitter_detected(uint32_t *array, size_t size)
{
    (void) array;
    (void) size;
    assert(HW_initialized == true);
    const uint32_t signalize_event_duration_ticks = BENCHMARK_MS_to_Ticks(TEST_SIGNALING_EVNT_DURATION);
    LED_RED_TOGGLE();
    for (uint32_t i = 0; i < signalize_event_duration_ticks; i++)
    {
        __NOP();
    }
    LED_RED_TOGGLE();
}
