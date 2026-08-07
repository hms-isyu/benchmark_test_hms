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
 * \brief Port layer for the benchmark.
 * @{
 * \file
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "benchmark_tools_hms.h"
#include "board.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

void BMTH_hardware_init(void)
{
  LED_RED_INIT(LOGIC_LED_OFF);
  LED_GREEN_INIT(LOGIC_LED_OFF);
  LED_BLUE_INIT(LOGIC_LED_OFF);
}

void BMTH_signal_success(void)
{
  LED_GREEN_TOGGLE();
}

void BMTH_signal_failure(void)
{
  LED_RED_TOGGLE();
}

void BMTH_signal_event(void)
{
  LED_BLUE_TOGGLE();
}

/*!@}*/
