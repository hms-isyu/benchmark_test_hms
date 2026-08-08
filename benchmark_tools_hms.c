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
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void     BMTH_system_warmup(void);
static uint32_t BMTH_measure_empty_loop(uint32_t loop_count);

__attribute__((noinline, aligned(16))) static uint32_t BMTH_measure_empty_loop(
  uint32_t loop_count)
{
  volatile uint32_t t0, t1;
  BMTH_GET_START_TIME(t0);
  for (uint32_t i = 0; i < loop_count; i++)
  {
    __ASM volatile("" ::: "memory");
  }
  BMTH_GET_STOP_TIME(t1);
  return t1 - t0;
}

/*******************************************************************************
 * Code
 ******************************************************************************/

/*
    The system can cause on bring up a lot of jitter in the the cycle
   measurement. This happens because of Debbugger
*/
static void BMTH_system_warmup(void)
{
  g_WarmUp = true;
  for (uint32_t i = 0; i < LOOP_GUARD_COUNT; i++)
  {
    __NOP();
  }
}

#if (defined(BMTH_GLOBAL_TIME_STORAGE) && (BMTH_GLOBAL_TIME_STORAGE == 1))
void BMTH_global_start_time(BMTH_time_marker_t t0)
{
  global_start_time += t0;
  global_start_count++;
}

void BMTH_global_stop_time(BMTH_time_marker_t t1)
{
  global_stop_time += t1;
  global_stop_count++;
}

bool BMTH_global_get_time_difference(float *result)
{
  if (global_stop_time != global_start_time)
  {
    return false;
  }

  *result = (global_stop_time - global_start_time) / global_start_count;

  return true;
}
#endif

bool BMTH_calc_overhead(uint32_t loop_count, uint32_t *loop_overhead,
                        uint32_t *cyccnt_assignment_overhead)
{
  if (g_WarmUp == false)
  {
    BMTH_assert_quiet_system();
    BMTH_assert_needed_components();
    BMTH_system_warmup();
  }
  volatile uint32_t loop_oh_1  = 0U;
  volatile uint32_t loop_oh_2  = 0U;
  volatile uint32_t t0         = 0;
  volatile uint32_t t1         = 0;
  uint32_t          dwt_oh     = 0;
  uint32_t          dwt_oh_min = UINT32_MAX;
  uint32_t          dwt_oh_max = 0U;
  bool              success    = false;

  BMTH_GET_START_TIME(t0);
  BMTH_GET_STOP_TIME(t1);
  *cyccnt_assignment_overhead = t1 - t0;

  __DSB();
  __ISB();
#pragma GCC unroll 1
  for (uint32_t i = 0; i < loop_count; i++)
  {
    BMTH_GET_START_TIME(t0);
    BMTH_GET_STOP_TIME(t1);
    dwt_oh = t1 - t0;
    if (dwt_oh < dwt_oh_min)
    {
      dwt_oh_min = dwt_oh;
    }
    if (dwt_oh > dwt_oh_max)
    {
      dwt_oh_max = dwt_oh;
    }
  }
  if (dwt_oh_min != dwt_oh_max)
  {
    BMTH_signal_jitter_detected(NULL, 0U);
    assert(true);
  }

  BMTH_reset_counter();

  loop_oh_1 = BMTH_measure_empty_loop(loop_count);
  loop_oh_2 = BMTH_measure_empty_loop(loop_count);

  if (loop_oh_1 != loop_oh_2)
  {
    BMTH_signal_jitter_detected(NULL, 0U);
  }

  if (loop_overhead != NULL)
  {
    *loop_overhead = loop_oh_1 % loop_count;
    success        = true;
  }
  return success;
}

void BMTH_check_hw_influence(uint32_t loop_count)
{
  static uint32_t raw[64U];
  uint32_t        min           = 0;
  uint32_t        max           = 0;
  uint32_t        outlier_count = 0;
  uint32_t        result[2] = {0U, 0U}; /* result[0] = min, result[1] = max */
  uint32_t       *data      = result;

  BMTH_assert_needed_components();

  for (uint32_t r = 0; r < 64U; r++)
  {
    raw[r] = BMTH_measure_empty_loop(loop_count);
    if (r == 0)
    {
      min = raw[r];
      max = raw[r];
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
      BMTH_signal_jitter_detected(data, 2U);
    }
  }
  __NOP(); // Allow setting BP in the IDE.
}

uint32_t BMTH_MS_to_Ticks(uint32_t ms)
{
  return ms * (BMTH_TIMER_FREQUENCY / 1000U);
}

void BMTH_signal_measurement_start(void)
{
  const uint32_t signalize_event_duration_ticks =
    BMTH_MS_to_Ticks(TEST_SIGNALING_EVNT_DURATION);
  BMTH_reset_counter();
  volatile uint32_t counter = BMTH_get_counter_value();
  BMTH_SIGNAL_EVENT();
  while (BMTH_get_counter_value() - counter < signalize_event_duration_ticks)
  {
    __NOP();
  }

  counter = BMTH_get_counter_value();
  BMTH_SIGNAL_EVENT();
  while (BMTH_get_counter_value() - counter < signalize_event_duration_ticks)
  {
    __NOP();
  }
}

void BMTH_signal_measurement_stop(bool success)
{
  const uint32_t signalize_event_duration_ticks =
    BMTH_MS_to_Ticks(TEST_SIGNALING_EVNT_DURATION);
  BMTH_reset_counter();

  if (success)
  {
    BMTH_SIGNAL_SUCCESS();
  }
  else
  {
    BMTH_SIGNAL_FAILURE();
  }
  volatile uint32_t counter = BMTH_get_counter_value();
  while (BMTH_get_counter_value() - counter < signalize_event_duration_ticks)
  {
    __NOP();
  }
  counter = BMTH_get_counter_value();
  if (success)
  {
    BMTH_SIGNAL_SUCCESS();
  }
  else
  {
    BMTH_SIGNAL_FAILURE();
  }
  while (BMTH_get_counter_value() - counter < signalize_event_duration_ticks)
  {
    __NOP();
  }
}

void BMTH_signal_jitter_detected(uint32_t *array, size_t size)
{
  (void) array;
  (void) size;
  const uint32_t signalize_event_duration_ticks =
    BMTH_MS_to_Ticks(TEST_SIGNALING_EVNT_DURATION);
  BMTH_reset_counter();
  volatile uint32_t counter = BMTH_get_counter_value();

  BMTH_SIGNAL_FAILURE();
  while (BMTH_get_counter_value() - counter < signalize_event_duration_ticks)
  {
    __NOP();
  }
  BMTH_SIGNAL_FAILURE();
}

/*!@}*/
