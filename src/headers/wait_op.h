/* Copyright (C) 2015-2019, Wazuh Inc.
 * Copyright (C) 2009 Trend Micro Inc.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation
 */

#ifndef WAIT_OP_H
#define WAIT_OP_H

void os_setwait(void);
void os_delwait(void);
void os_wait(void);

#endif /* WAIT_OP_H */
