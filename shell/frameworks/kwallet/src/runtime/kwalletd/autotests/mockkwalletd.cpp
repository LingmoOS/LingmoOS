/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2022 Slava Aseev <nullptrnine@basealt.ru>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "static_mock.hpp"

#include "../kwalletd.h"
#include "../kwalletfreedesktopcollection.h"
#include "../kwalletfreedesktopitem.h"
#include "../kwalletfreedesktopprompt.h"
#include "../kwalletfreedesktopservice.h"
#include "../kwalletfreedesktopsession.h"

KWalletD::KWalletD()
    : _syncTime(0)
{
}
KWalletD::~KWalletD()
{
}

KWalletSessionStore::KWalletSessionStore()
{
}
KWalletSessionStore::~KWalletSessionStore()
{
}

KTimeout::KTimeout(QObject *)
{
}
KTimeout::~KTimeout()
{
}

MOCK_FUNCTION(KWalletD, encodeWalletName, 1, );
MOCK_FUNCTION(KWalletD, decodeWalletName, 1, );

MOCK_FUNCTION(KTimeout, clear, 0, );
MOCK_FUNCTION(KTimeout, resetTimer, 2, );
MOCK_FUNCTION(KTimeout, removeTimer, 1, );
MOCK_FUNCTION(KTimeout, addTimer, 2, );

void KTimeout::timerEvent(QTimerEvent *)
{
}

MOCK_FUNCTION(KWalletD, readEntry, 4, );
MOCK_FUNCTION(KWalletD, readMap, 4, );
MOCK_FUNCTION(KWalletD, readPassword, 4, );

MOCK_FUNCTION_RES(KWalletD, removeEntry, 4, 0, );
MOCK_FUNCTION_RES(KWalletD, writeMap, 5, 0, );
MOCK_FUNCTION_RES(KWalletD, writePassword, 5, 0, );

using OVWriteEntry_6 = int (KWalletD::*)(int, const QString &, const QString &, const QByteArray &, int, const QString &);
MOCK_FUNCTION_OVERLOADED_RES(KWalletD, writeEntry, 6, 0, OVWriteEntry_6);

using OVWriteEntry_5 = int (KWalletD::*)(int, const QString &, const QString &, const QByteArray &, const QString &);
MOCK_FUNCTION_OVERLOADED_RES(KWalletD, writeEntry, 5, 0, OVWriteEntry_5);

MOCK_FUNCTION(KWalletD, entryType, 4, );
MOCK_FUNCTION_RES(KWalletD, renameEntry, 5, 0, );

MOCK_FUNCTION(KWalletD, isEnabled, 0, const);
MOCK_FUNCTION(KWalletD, open, 3, );
MOCK_FUNCTION(KWalletD, openPath, 3, );
MOCK_FUNCTION(KWalletD, openPathAsync, 4, );

using OVOpenAsync4 = int (KWalletD::*)(const QString &, qlonglong, const QString &, bool);
MOCK_FUNCTION_OVERLOADED(KWalletD, openAsync, 4, OVOpenAsync4);

using OVOpenAsync6 = int (KWalletD::*)(const QString &, qlonglong, const QString &, bool, const QDBusConnection &, const QDBusMessage &);
MOCK_FUNCTION_OVERLOADED(KWalletD, openAsync, 6, OVOpenAsync6);

using OVClose4 = int (KWalletD::*)(int, bool, const QString &, const QDBusMessage &);
MOCK_FUNCTION_OVERLOADED(KWalletD, close, 4, OVClose4);

using OVClose2 = int (KWalletD::*)(const QString &, bool);
MOCK_FUNCTION_OVERLOADED(KWalletD, close, 2, OVClose2);

using OVClose3 = int (KWalletD::*)(int, bool, const QString &);
MOCK_FUNCTION_OVERLOADED(KWalletD, close, 3, OVClose3);

MOCK_FUNCTION(KWalletD, deleteWallet, 1, );

MOCK_FUNCTION_OVERLOADED(KWalletD, isOpen, 1, bool (KWalletD::*)(const QString &));
MOCK_FUNCTION_OVERLOADED(KWalletD, isOpen, 1, bool (KWalletD::*)(int));

MOCK_FUNCTION(KWalletD, users, 1, const);
MOCK_FUNCTION(KWalletD, wallets, 0, const);
MOCK_FUNCTION(KWalletD, folderList, 2, );
MOCK_FUNCTION(KWalletD, hasFolder, 3, );
MOCK_FUNCTION(KWalletD, createFolder, 3, );
MOCK_FUNCTION(KWalletD, removeFolder, 3, );
MOCK_FUNCTION(KWalletD, entryList, 3, );

#if KWALLET_BUILD_DEPRECATED_SINCE(5, 72)
MOCK_FUNCTION(KWalletD, readEntryList, 4, );
MOCK_FUNCTION(KWalletD, readMapList, 4, );
MOCK_FUNCTION(KWalletD, readPasswordList, 4, );
#endif

MOCK_FUNCTION(KWalletD, entriesList, 3, );
MOCK_FUNCTION(KWalletD, mapList, 3, );
MOCK_FUNCTION(KWalletD, passwordList, 3, );
MOCK_FUNCTION(KWalletD, renameWallet, 2, );
MOCK_FUNCTION(KWalletD, hasEntry, 4, );
MOCK_FUNCTION(KWalletD, disconnectApplication, 2, );
MOCK_FUNCTION(KWalletD, folderDoesNotExist, 2, );
MOCK_FUNCTION(KWalletD, keyDoesNotExist, 3, );
MOCK_FUNCTION(KWalletD, networkWallet, 0, );
MOCK_FUNCTION(KWalletD, localWallet, 0, );
MOCK_FUNCTION(KWalletD, pamOpen, 3, );
MOCK_FUNCTION(KWalletD, sync, 2, );
MOCK_FUNCTION(KWalletD, changePassword, 3, );
MOCK_FUNCTION(KWalletD, reconfigure, 0, );
MOCK_FUNCTION(KWalletD, closeAllWallets, 0, );
MOCK_FUNCTION(KWalletD, screenSaverChanged, 1, );

void KWalletD::slotServiceOwnerChanged(const QString &, const QString &, const QString &)
{
}
void KWalletD::emitWalletListDirty()
{
}
void KWalletD::timedOutClose(int)
{
}
void KWalletD::timedOutSync(int)
{
}
void KWalletD::notifyFailures()
{
}
void KWalletD::processTransactions()
{
}
void KWalletD::activatePasswordDialog()
{
}

#include <QLoggingCategory>
const QLoggingCategory &KWALLETD_LOG()
{
    static const QLoggingCategory category("kf.wallet.kwalletd", QtFatalMsg);
    return category;
}
