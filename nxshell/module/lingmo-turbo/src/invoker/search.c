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

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "report.h"
#include "search.h"

static char* merge_paths(const char *base_path, const char *rel_path)
{
    char *path;

    if (asprintf(&path, "%s%s%s", base_path,
                 (base_path[strlen(base_path) - 1] == '/' ? "" : "/"),
                 rel_path) < 0)
    {
        die(1, "allocating merge path buffer");
    }
    return path;
}

char* search_program(const char *progname)
{
    char *launch = NULL;
    char *cwd;

    if (progname[0] == '/')
    {
        launch = strdup(progname);
        if (!launch)
        {
            die(1, "allocating program name buffer");
        }
    }
    else if (strchr(progname, '/') != NULL)
    {
        cwd = get_current_dir_name();
        launch = merge_paths(cwd, progname);
        free(cwd);
    }
    else
    {
        char *path = getenv("PATH");
        char *saveptr = NULL;
        char *token;

        if (path == NULL)
        {
            die(1, "could not get PATH environment variable");
        }
        path = strdup(path);

        for (token = strtok_r(path, ":", &saveptr); token != NULL; token = strtok_r(NULL, ":", &saveptr))
        {
            launch = merge_paths(token, progname);

            if (access(launch, X_OK) == 0)
                break;

            free(launch);
            launch = NULL;
        }

        free(path);

        if (launch == NULL)
        {
            die(1, "could not locate program \"%s\" to launch \n", progname);
        }

        if (launch[0] != '/')
        {
            char *relative = launch;

            cwd = get_current_dir_name();
            launch = merge_paths(cwd, relative);

            free(cwd);
            free(relative);
        }
    }
    return launch;
}

