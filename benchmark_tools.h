/*******************************************************************************
 **    Copyright (C) 1999-2026 HMS Technology Center Ravensburg GmbH
 **    All rights reserved
 **
 **        File: benchmark_tools.h
 **     Summary: Benchmarking tools for Cycle-Count benchmarking methods.
 **      Author: Isaac L. L. Yuki
 ** Responsible: T. Drexel
 **
 ******************************************************************************/

#ifndef BENCHMARK_TOOLS_H_
#define BENCHMARK_TOOLS_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "stdint.h"
#include "stdbool.h"

/*******************************************************************************
 * Defines
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

extern bool BENCHMARK_calc_overhead(uint32_t loop_count, uint32_t *loop_overhead, uint32_t *cyccnt_assignment_overhead);
extern void BENCHMARK_check_hw_influence(uint32_t loop_count);

#endif /* BENCHMARK_TOOLS_H_ */
