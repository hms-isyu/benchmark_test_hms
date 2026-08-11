/******************************************************************************/
/*!
 * \copyright
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026 HMS Industrial Networks GmbH & Co. KG
 * \author Isaac L. L. Yuki
 */
/******************************************************************************/

/*!
 * \ingroup BMT_HMS
 * \brief Benchmarking tools for HMS systems.
 * @{
 * \file
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "benchmark_tools_hms.h"
#include "stdbool.h"
#include "math.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define LOOP_GUARD_COUNT                                                       \
  (10000000U) /* number of iterations to run before measuring the overhead     \
                 of the loop itself */

/*******************************************************************************
 * Variables
 ******************************************************************************/

static bool g_WarmUp =
  false; /* flag to indicate if the system has been warmed up */

#if (defined(BMTH_GLOBAL_TIME_STORAGE) && (BMTH_GLOBAL_TIME_STORAGE == 1))
static BMTH_time_marker_t global_start_time  = 0U;
static BMTH_time_marker_t global_stop_time   = 0U;
static uint32_t           global_start_count = 0U;
static uint32_t           global_stop_count  = 0U;
static bool               global_activated =
  false; /* flag to indicate if the global time storage is activated */
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void  BMTH_system_warmup(void);
static void  BMTH_empty_loop(uint32_t loop_count);
static float BMTH_mseries_calc_average_cyc(BMTH_measurement_series_t *mseries);

/*******************************************************************************
 * Code
 ******************************************************************************/

/*
    The system can cause on bring up a lot of jitter in the cycle
   measurement. This happens because of the Debugger
*/
static void BMTH_system_warmup(void)
{
  g_WarmUp = true;
  for (uint32_t i = 0; i < LOOP_GUARD_COUNT; i++)
  {
    __NOP();
  }
}

__attribute__((always_inline)) static inline void BMTH_empty_loop(
  uint32_t loop_count)
{
  for (uint32_t i = 0; i < loop_count; i++)
  {
    __ASM volatile("" ::: "memory");
  }
}

#if (defined(BMTH_GLOBAL_TIME_STORAGE) && (BMTH_GLOBAL_TIME_STORAGE == 1))

void BMTH_global_activate(void)
{
  global_activated = true;
}

void BMTH_global_deactivate(void)
{
  global_activated = false;
}

__attribute__((noinline)) void BMTH_set_global_start_cnt(BMTH_time_marker_t t0)
{
  if (!global_activated)
  {
    return;
  }
  global_start_cnt_marker = t0;
  global_start_count++;
}

uint32_t BMTH_get_global_start_cnt_function_overhead(void)
{
  BMTH_time_marker_t t1        = 0U;
  BMTH_time_marker_t t0        = 0U;
  BMTH_time_marker_t calibrate = 0U;
  BMTH_global_activate();

  BMTH_RESET_COUNTER();

  BMTH_GET_START_CNT(t0);
  BMTH_set_global_start_cnt(calibrate);
  BMTH_GET_STOP_CNT(t1);

  global_start_count--;

  BMTH_global_deactivate();
  return (t1 - t0);
}

__attribute__((noinline)) void BMTH_set_global_stop_cnt(BMTH_time_marker_t t1)
{
  if (!global_activated)
  {
    return;
  }
  global_stop_cnt_marker = t1;
  global_stop_count++;
}

uint32_t BMTH_get_global_start_cnt(void)
{
  return global_start_cnt_marker;
}

uint32_t BMTH_get_global_stop_cnt(void)
{
  return global_stop_cnt_marker;
}

#endif

bool BMTH_get_counter_overhead(uint32_t *cyccnt_assignment_overhead,
                               uint32_t  iterations)
{
  if (g_WarmUp == false)
  {
    BMTH_assert_quiet_system();
    BMTH_assert_needed_components();
    BMTH_system_warmup();
  }
  static BMTH_time_marker_t t0 = 0;
  static BMTH_time_marker_t t1 = 0;

  BMTH_GET_START_CNT(t0);
  BMTH_GET_STOP_CNT(t1);
  *cyccnt_assignment_overhead = t1 - t0;

  uint32_t reference = 0U;
#pragma GCC unroll 1
  for (uint32_t i = 0U; i < iterations; i++)
  {
    BMTH_GET_START_CNT(t0);
    BMTH_GET_STOP_CNT(t1);
    uint32_t oh = t1 - t0;
    if (i == 0U)
    {
      reference = oh;
    }
    else if (oh != reference)
    {
      return false;
    }
  }
  return true;
}

void BMTH_mseries_set_static_overhead(BMTH_measurement_series_t *mseries,
                                      uint32_t                   oh)
{
  mseries->values_static_overhead = oh;
}

static float BMTH_mseries_calc_average_cyc(BMTH_measurement_series_t *mseries)
{
  return (mseries->values_accumulated / mseries->iteration_count);
}

bool BMTH_mseries_iterate(BMTH_measurement_series_t *mseries, uint32_t t0,
                          uint32_t t1)
{
  bool no_jitter = true;
  t1             = t1 - t0;
  BMTH_ASSERT(t1 > mseries->values_static_overhead);
  t1 = t1 - mseries->values_static_overhead;

  if (mseries->values_buffer_size == 0)
  {
    BMTH_ASSERT(mseries->values_buffer == NULL);
  }
  else
  {
    BMTH_ASSERT(mseries->values_buffer != NULL);
    BMTH_ASSERT(mseries->iteration_count < mseries->values_buffer_size);
  }

  if (mseries->iteration_count == 0)
  {
    mseries->values_accumulated = 0;
    mseries->values_min         = t1;
    mseries->values_max         = t1;
    mseries->last_value         = t1;
    mseries->values_average     = (float) t1;
  }
  else
  {
    if (t1 < mseries->values_min)
    {
      mseries->values_min = t1;
    }
    if (t1 > mseries->values_max)
    {
      mseries->values_max = t1;
    }
    if (t1 != (uint32_t) (mseries->values_average))
    {
      mseries->values_outlier_count++;
      no_jitter = false;
    }
  }

  if (mseries->values_buffer != NULL && mseries->values_buffer_size > 0)
  {
    mseries->values_buffer[mseries->iteration_count] = t1;
  }

  mseries->values_accumulated += t1;
  mseries->last_value = t1;
  mseries->iteration_count++;

  BMTH_mseries_calc_average_cyc(mseries);

  return no_jitter;
}

void BMTH_check_hw_influence(uint32_t                   loop_count,
                             BMTH_measurement_series_t *mseries)
{
  BMTH_time_marker_t t0, t1;

  BMTH_assert_needed_components();

  for (uint32_t i = 0; i < mseries->values_buffer_size; i++)
  {
    BMTH_GET_START_CNT(t0);
    BMTH_empty_loop(loop_count);
    BMTH_GET_STOP_CNT(t1);
    if (!BMTH_mseries_iterate(mseries, t0, t1))
    {
      BMTH_signalize_jitter_detected();
    }
  }
  __NOP(); // Allow setting BP in the IDE.
}

uint32_t BMTH_MS_to_Ticks(uint32_t ms)
{
  return ms * (BMTH_COUNTER_FREQUENCY / 1000U);
}

void BMTH_signalize_mseries_start(void)
{
  const uint32_t signalize_event_duration_ticks =
    BMTH_MS_to_Ticks(BMTH_SIGNALING_EVNT_DURATION);
  BMTH_reset_timer();
  volatile uint32_t counter = BMTH_get_timer_value();
  BMTH_TOGGLE_SIGNAL_EVENT();
  while (BMTH_get_timer_value() - counter < signalize_event_duration_ticks)
  {
    __NOP();
  }

  counter = BMTH_get_timer_value();
  BMTH_TOGGLE_SIGNAL_EVENT();
  while (BMTH_get_timer_value() - counter < signalize_event_duration_ticks)
  {
    __NOP();
  }
}

void BMTH_signalize_mseries_stop(bool success)
{
  const uint32_t signalize_event_duration_ticks =
    BMTH_MS_to_Ticks(BMTH_SIGNALING_EVNT_DURATION);
  BMTH_reset_timer();

  if (success)
  {
    BMTH_TOGGLE_SIGNAL_SUCCESS();
  }
  else
  {
    BMTH_TOGGLE_SIGNAL_FAILURE();
  }
  volatile uint32_t counter = BMTH_get_timer_value();
  while (BMTH_get_timer_value() - counter < signalize_event_duration_ticks)
  {
    __NOP();
  }
  counter = BMTH_get_timer_value();
  if (success)
  {
    BMTH_TOGGLE_SIGNAL_SUCCESS();
  }
  else
  {
    BMTH_TOGGLE_SIGNAL_FAILURE();
  }
  while (BMTH_get_timer_value() - counter < signalize_event_duration_ticks)
  {
    __NOP();
  }
}

void BMTH_signalize_jitter_detected(void)
{
  const uint32_t signalize_event_duration_ticks =
    BMTH_MS_to_Ticks(BMTH_SIGNALING_EVNT_DURATION);
  BMTH_reset_timer();
  volatile uint32_t counter = BMTH_get_timer_value();

  BMTH_TOGGLE_SIGNAL_FAILURE();
  while (BMTH_get_timer_value() - counter < signalize_event_duration_ticks)
  {
    __NOP();
  }
  BMTH_TOGGLE_SIGNAL_FAILURE();
}

/*!@}*/
