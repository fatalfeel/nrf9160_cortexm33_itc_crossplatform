/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**@file
 *
 * @brief   Watchdog module for asset tracker
 */

#ifndef APP_WATCHDOG_H
#define APP_WATCHDOG_H

extern int watchdog_init_and_start(struct k_work_q *work_q);

#endif /* WATCHDOG_H */
