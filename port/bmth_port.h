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
 * \brief User defined interfaces for the benchmark.
 * @{
 * \file
 */

#ifndef BMTH_PORT_H
#define BMTH_PORT_H

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/* Signalize */

extern void BMTH_toggle_signal_success(void);

extern void BMTH_toggle_signal_failure(void);

extern void BMTH_toggle_signal_event(void);

/* Hardware */

extern void BMTH_hardware_init(void);

extern void BMTH_disable_sys_tick(void);

extern void BMTH_enable_sys_tick(void);

extern void BMTH_enable_counter(void);

#endif /* BMTH_PORT_H */

/*!@}*/
