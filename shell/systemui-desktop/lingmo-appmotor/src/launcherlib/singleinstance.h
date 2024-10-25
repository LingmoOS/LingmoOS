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

#ifndef SINGLEINSTANCE_H
#define SINGLEINSTANCE_H

#include "launcherlib.h"
#include <tr1/memory>

using std::tr1::shared_ptr;

// Function pointer type for lock()
typedef bool (*lock_func_t)(const char *);

// Function pointer type for unlock()
typedef void (*unlock_func_t)();

// Function pointer type for activateExistingInstance(const char * binaryName)
typedef bool (*activate_func_t)(const char *);

//! Single instance plugin entry
struct SingleInstancePluginEntry
{
    //! Lock function pointer
    lock_func_t lockFunc;

    //! Unlock function pointer
    unlock_func_t unlockFunc;

    //! Activate existing instance
    activate_func_t activateExistingInstanceFunc;

    //! Handle to the plugin
    void * handle;
};

//! Class that loads the single-instance binary as a plugin.
class DECL_EXPORT SingleInstance
{
public:

    /*! Validate given plugin library handle and register.
     *  Returns true if succeeded.
     */
    bool validateAndRegisterPlugin(void * handle);

    //! Return pointer to single-instance plugin. Return NULL if not loaded.
    SingleInstancePluginEntry * pluginEntry() const;

    //! dlclose() the plugin
    void closePlugin();

private:

    //! The plugin entry
    shared_ptr<SingleInstancePluginEntry> m_pluginEntry;
};

#endif // SINGLEINSTANCE_H
