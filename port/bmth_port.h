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

#include <stdbool.h>

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/* Signalize */

extern void BMTH_toggle_signal_success(void);

extern void BMTH_toggle_signal_failure(void);

extern void BMTH_toggle_signal_event(void);

/* Hardware */

extern void BMTH_hardware_init(bool quiet);

extern void BMTH_disable_sys_tick(void);

extern void BMTH_enable_sys_tick(void);

extern void BMTH_disable_autoclockgating(void);

extern void BMTH_enable_autoclockgating(void);

extern void BMTH_disable_cache(void);

extern void BMTH_enable_cache(void);

extern void BMTH_disable_ram_ecc(void);

extern void BMTH_enable_ram_ecc(void);

extern void BMTH_disable_cpu1(void);

extern void BMTH_enable_cpu1(void);

extern void BMTH_disable_edma0(void);

extern void BMTH_enable_edma0(void);

extern void BMTH_disable_smartdma(void);

extern void BMTH_enable_smartdma(void);

extern void BMTH_disable_edma1(void);

extern void BMTH_enable_edma1(void);

extern void BMTH_disable_syspm(void);

extern void BMTH_enable_syspm(void);

extern void BMTH_disable_noise_sources(void);

extern void BMTH_enable_noise_sources(void);

extern void BMTH_enable_counter(void);

#endif /* BMTH_PORT_H */

/*!@}*/
