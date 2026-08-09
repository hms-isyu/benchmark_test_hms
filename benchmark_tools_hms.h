/******************************************************************************/
/*!
 * \copyright
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026 HMS Industrial Networks GmbH & Co. KG
 * \author Isaac L. L. Yuki
 */
/******************************************************************************/

/*!
 * \defgroup BMT_HMS
 * \brief Benchmarking tools by HMS.
 * \details This package provides a set of tools and utilities for benchmarking
 * and performance measurement. It includes functions for
 * measuring loop overhead, checking hardware influence, signaling measurement
 * start and stop events, and asserting system preconditions.
 * @{
 * \file
 */

#ifndef BMT_HMS_H
#define BMT_HMS_H

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "stdint.h"
#include "stddef.h"

#include "bmth_config.h"
#include "bmth_port.h"

/*******************************************************************************
 * Defines
 ******************************************************************************/

typedef volatile uint32_t
  BMTH_time_marker_t; /* volatile to prevent compiler optimizations */

typedef struct BMTH_measurement_series_t
{
  uint32_t  values_accumulated;
  uint32_t  last_value;
  uint32_t *values_buffer;
  size_t    values_buffer_size;
  uint32_t  values_max;
  uint32_t  values_min;
  float     values_average;
  uint32_t  values_outlier_count;
  uint32_t  values_static_overhead;
  uint32_t  iteration_count;
} BMTH_measurement_series_t;

__attribute__((always_inline)) static inline uint32_t BMTH_get_counter_value(
  void)
{
  return BMTH_GET_COUNTER();
}

__attribute__((always_inline)) static inline void BMTH_reset_counter(void)
{
  BMTH_RESET_COUNTER();
}

/*
    Precondition check for the benchmark package.

    Deliberately independent of <assert.h>. The MCUX HAL glue defines NDEBUG
   build-wide whenever CONFIG_ASSERT is off
   (zephyr/modules/hal_nxp/mcux/CMakeLists.txt, to squelch a warning in
    fsl_flexcan.c), which compiles every assert() in this package to nothing
   without a diagnostic. These checks only run outside a measured window, so
   they cost nothing where it matters and must therefore hold in every build -
   including the one the published numbers come from.

    The halt itself is a port primitive, see BMTH_haltOnFailedPrecondition.
*/
#define BMTH_ASSERT(cond)                                                      \
  do                                                                           \
  {                                                                            \
    if (!(cond))                                                               \
    {                                                                          \
      BMTH_doomed();                                                           \
    }                                                                          \
  } while (0)

#define BMTH_ENABLE_TIME(void) BMTH_enable_counter()

#define BMTH_RESET_TIME(void) BMTH_reset_counter()

#define BMTH_GET_START_TIME(t0_)                                               \
  do                                                                           \
  {                                                                            \
    __DSB();                                                                   \
    __ISB();                                                                   \
    __ASM volatile("" ::: "memory");                                           \
    (t0_) = BMTH_get_counter_value();                                          \
    __ASM volatile("" ::: "memory");                                           \
  } while (0)

#define BMTH_GET_STOP_TIME(t1_)                                                \
  do                                                                           \
  {                                                                            \
    __ASM volatile("" ::: "memory");                                           \
    (t1_) = BMTH_get_counter_value();                                          \
    __ASM volatile("" ::: "memory");                                           \
  } while (0)

#define BMTH_TOGGLE_SIGNAL_SUCCESS(void) BMTH_toggle_signal_success()
#define BMTH_TOGGLE_SIGNAL_FAILURE(void) BMTH_toggle_signal_failure()
#define BMTH_TOGGLE_SIGNAL_EVENT(void) BMTH_toggle_signal_event()

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/* Signalize */

extern void BMTH_signalize_mseries_start(void);

extern void BMTH_signalize_mseries_stop(bool success);

extern void BMTH_signalize_jitter_detected(void);

/* Tools */

/* Measurement series */

extern bool BMTH_mseries_iterate(BMTH_measurement_series_t *mseries,
                                 uint32_t t0, uint32_t t1);

extern void BMTH_mseries_set_static_overhead(BMTH_measurement_series_t *mseries,
                                             uint32_t                   oh);

/* Measurement */

extern bool BMTH_get_counter_overhead(uint32_t *cyccnt_assignment_overhead,
                                      uint32_t  iterations);

extern void BMTH_check_hw_influence(uint32_t                   loop_count,
                                    BMTH_measurement_series_t *mseries);

/* Global Time Storage */

#if (defined(BMTH_GLOBAL_TIME_STORAGE) && (BMTH_GLOBAL_TIME_STORAGE == 1))
extern void BMTH_global_start_time(BMTH_time_marker_t t0);

extern void BMTH_global_stop_time(BMTH_time_marker_t t1);

extern bool BMTH_global_get_time_difference(float *result);

extern void BMTH_global_activate(void);

extern void BMTH_global_deactivate(void);

#endif

/* Helper */

extern uint32_t BMTH_MS_to_Ticks(uint32_t ms);

/* Assert */

extern void BMTH_assert_quiet_system(void);

extern void BMTH_assert_needed_components(void);

extern __attribute__((noreturn)) void BMTH_doomed(void);

#endif /* BMT_HMS_H */

/*!@}*/
