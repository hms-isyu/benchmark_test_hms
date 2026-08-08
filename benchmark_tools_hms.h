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

/*******************************************************************************
 * Defines
 ******************************************************************************/

typedef volatile uint32_t
  BMTH_time_marker_t; /* volatile to prevent compiler optimizations */

#define TEST_SIGNALING_EVNT_DURATION (1000U) /* in ms */

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

#define BMTH_SIGNAL_SUCCESS(void) BMTH_signal_success()
#define BMTH_SIGNAL_FAILURE(void) BMTH_signal_failure()
#define BMTH_SIGNAL_EVENT(void) BMTH_signal_event()

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/* Signalize */

extern void BMTH_signal_success(void);

extern void BMTH_signal_failure(void);

extern void BMTH_signal_event(void);

extern void BMTH_signal_measurement_start(void);

extern void BMTH_signal_measurement_stop(bool success);

extern void BMTH_signal_jitter_detected(uint32_t *array, size_t size);

/* Hardware */

extern void BMTH_hardware_init(void);

extern void BMTH_disable_sys_tick(void);

extern void BMTH_enable_sys_tick(void);

extern void BMTH_enable_counter(void);

/* Tools */

extern bool BMTH_calc_overhead(uint32_t loop_count, uint32_t *loop_overhead,
                               uint32_t *cyccnt_assignment_overhead);

extern void BMTH_check_hw_influence(uint32_t loop_count);

extern uint32_t BMTH_MS_to_Ticks(uint32_t ms);

#if (defined(BMTH_GLOBAL_TIME_STORAGE) && (BMTH_GLOBAL_TIME_STORAGE == 1))
extern void BMTH_global_start_time(BMTH_time_marker_t t0);

extern void BMTH_global_stop_time(BMTH_time_marker_t t1);

extern bool BMTH_global_get_time_difference(float *result);
#endif

/* Assert */

extern void BMTH_assert_quiet_system(void);

extern void BMTH_assert_needed_components(void);

extern __attribute__((noreturn)) void BMTH_doomed(void);

#endif /* BMT_HMS_H */

/*!@}*/
