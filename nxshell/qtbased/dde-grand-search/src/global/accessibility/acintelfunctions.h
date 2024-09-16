// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ACINTELFUNCTIONS_H
#define ACINTELFUNCTIONS_H

#ifdef ENABLE_ACCESSIBILITY
    #define AC_SET_ACCESSIBLE_NAME(classObj,accessiblename) classObj->setAccessibleName(accessiblename);
    #define AC_SET_ACCESSIBLE_TEXT(classObj,accessibletext) classObj->setAccessibleText(accessibletext);
    #define AC_SET_OBJECT_NAME(classObj,objectname)
#else
    #define AC_SET_ACCESSIBLE_NAME(classObj,accessiblename)
    #define AC_SET_ACCESSIBLE_TEXT(classObj,accessibletext)
    #define AC_SET_OBJECT_NAME(classObj,objectname)
#endif

#endif // ACINTELFUNCTIONS_H
