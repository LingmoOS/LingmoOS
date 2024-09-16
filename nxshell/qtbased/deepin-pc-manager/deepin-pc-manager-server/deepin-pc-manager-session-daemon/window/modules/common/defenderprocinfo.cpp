// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "defenderprocinfo.h"

bool DefenderProcInfo::operator!=(const DefenderProcInfo &procInfo)
{
    return !(this->nPid == procInfo.nPid);
}

QDBusArgument &operator<<(QDBusArgument &argument, const DefenderProcInfo &procInfo)
{
    argument.beginStructure();
    argument << procInfo.nPid << procInfo.sExecPath << procInfo.isbSysApp << procInfo.sProcName
             << procInfo.sDesktopPath << procInfo.sPkgName << procInfo.sAppName << procInfo.sThemeIcon
             << procInfo.sID << procInfo.nCategoryID << procInfo.nTimeInstalled
             << procInfo.sTimeyMdh << procInfo.dDownloadSpeed << procInfo.dUploadSpeed << procInfo.dDownloads << procInfo.dUploads
             << procInfo.sStatus << procInfo.sDefaultStatus << procInfo.sOtherParam;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DefenderProcInfo &procInfo)
{
    argument.beginStructure();
    argument >> procInfo.nPid >> procInfo.sExecPath >> procInfo.isbSysApp >> procInfo.sProcName
             >> procInfo.sDesktopPath >> procInfo.sPkgName >> procInfo.sAppName >> procInfo.sThemeIcon
             >> procInfo.sID >> procInfo.nCategoryID >> procInfo.nTimeInstalled
             >> procInfo.sTimeyMdh >> procInfo.dDownloadSpeed >> procInfo.dUploadSpeed >> procInfo.dDownloads >> procInfo.dUploads
             >> procInfo.sStatus >> procInfo.sDefaultStatus >> procInfo.sOtherParam;
    argument.endStructure();
    return argument;
}

void registerDefenderProcInfoMetaType()
{
    qRegisterMetaType<DefenderProcInfo>("DefenderProcInfo");
    qDBusRegisterMetaType<DefenderProcInfo>();
}
