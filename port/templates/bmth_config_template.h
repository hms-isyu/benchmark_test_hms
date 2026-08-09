

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
 * \brief Default config template for the benchmark.
 * @{
 * \file
 */

#ifndef BMTH_CONFIG_H
#define BMTH_CONFIG_H

/*******************************************************************************
 * Includes
 ******************************************************************************/

#define BMTH_COUNTER_FREQUENCY 0U /* in MHz */

#define BMTH_GET_COUNTER() DWT->CYCCNT
#define BMTH_RESET_COUNTER() DWT->CYCCNT = 0U

#define BMTH_SIGNALING_EVNT_DURATION (1000U)

#define BMTH_GLOBAL_TIME_STORAGE (0U) /* 1: store global time, 0: do not store global time */

#endif /* BMTH_CONFIG_H */

/*!@}*/
