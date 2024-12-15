/***************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (directui@nokia.com)
**
** This file is part of applauncherd
**
** If you have questions regarding the use of this file, please contact
** Nokia at directui@nokia.com.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file LICENSE.LGPL included in the packaging
** of this file.
**
****************************************************************************/

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

const uint32_t INVOKER_MSG_MAGIC                          = 0xb0070000;
const uint32_t INVOKER_MSG_MAGIC_VERSION_MASK             = 0x0000ff00;
const uint32_t INVOKER_MSG_MAGIC_VERSION                  = 0x00000300;
const uint32_t INVOKER_MSG_MAGIC_OPTION_MASK              = 0x000000ff;
const uint32_t INVOKER_MSG_MAGIC_OPTION_WAIT              = 0x00000001;
const uint32_t INVOKER_MSG_MAGIC_OPTION_DLOPEN_GLOBAL     = 0x00000002;
const uint32_t INVOKER_MSG_MAGIC_OPTION_DLOPEN_DEEP       = 0x00000004;
const uint32_t INVOKER_MSG_MAGIC_OPTION_SINGLE_INSTANCE   = 0x00000008;
/* 0x00000010 was INVOKER_MSG_MAGIC_OPTION_SPLASH_SCREEN */
const uint32_t INVOKER_MSG_MAGIC_OPTION_OOM_ADJ_DISABLE   = 0x00000020;
/* 0x00000040 was INVOKER_MSG_MAGIC_OPTION_LANDSCAPE_SPLASH_SCREEN */


const uint32_t INVOKER_MSG_MASK               = 0xffff0000;

const uint32_t INVOKER_MSG_NAME               = 0x5a5e0000;
const uint32_t INVOKER_MSG_EXEC               = 0xe8ec0000;
const uint32_t INVOKER_MSG_ARGS               = 0xa4650000;
const uint32_t INVOKER_MSG_ENV                = 0xe5710000;
const uint32_t INVOKER_MSG_PRIO               = 0xa1ce0000;
const uint32_t INVOKER_MSG_DELAY              = 0xb2de0012;
const uint32_t INVOKER_MSG_IDS                = 0xb2df4000;
const uint32_t INVOKER_MSG_IO                 = 0x10fd0000;
const uint32_t INVOKER_MSG_END                = 0xdead0000;
const uint32_t INVOKER_MSG_PID                = 0x1d1d0000;
const uint32_t INVOKER_MSG_SPLASH             = 0x5b1a0000;
const uint32_t INVOKER_MSG_LANDSCAPE_SPLASH   = 0x5b120000;
const uint32_t INVOKER_MSG_EXIT               = 0xe4170000;
const uint32_t INVOKER_MSG_ACK                = 0x600d0000;
// not used (Harmattan security stuff)
// const uint32_t INVOKER_MSG_BAD_CREDS          = 0x60035800;

#endif // PROTOCOL_H
