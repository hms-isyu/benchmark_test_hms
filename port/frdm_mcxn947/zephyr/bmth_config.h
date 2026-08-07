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

#ifndef BMTH_CONFIG_H
#define BMTH_CONFIG_H

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "cmsis_core.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define BMTH_TIMER_FREQUENCY (SystemCoreClock) /* in MHz */

#ifdef __CORTEX_M
#define BMTH_GET_COUNTER() DWT->CYCCNT
#define BMTH_RESET_COUNTER() DWT->CYCCNT = 0U
#endif /* __CORTEX_M */

#endif /* BMTH_CONFIG_H */

/*!@}*/
