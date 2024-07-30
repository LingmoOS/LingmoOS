# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2021 Anupam Basak <anupam.basak27@gmail.com>

import typing

import PySide2
from PySide2.QtCore import QAbstractListModel, Qt, Slot, QProcess


class WifiModel(QAbstractListModel):
    __wifiList = []

    BssidRole = Qt.UserRole + 1
    SsidRole = BssidRole + 1
    SignalRole = SsidRole + 1
    SecurityRole = SignalRole + 1
    IsSecuredRole = SecurityRole + 1

    def __init__(self, parent=None):
        super().__init__(parent)

    def roleNames(self) -> typing.Dict:
        roles = dict()
        roles[self.BssidRole] = b'bssid'
        roles[self.SsidRole] = b'ssid'
        roles[self.SignalRole] = b'signal'
        roles[self.SecurityRole] = b'security'
        roles[self.IsSecuredRole] = b'isSecured'

        return roles

    def data(self, index: PySide2.QtCore.QModelIndex, role: int) -> typing.Any:
        if role == self.BssidRole:
            return self.__wifiList[index.row()]['bssid']
        elif role == self.SsidRole:
            return self.__wifiList[index.row()]['ssid']
        elif role == self.SignalRole:
            return self.__wifiList[index.row()]['signal']
        elif role == self.SecurityRole:
            return self.__wifiList[index.row()]['security']
        elif role == self.IsSecuredRole:
            return self.__wifiList[index.row()]['isSecured']

    def rowCount(self, parent: PySide2.QtCore.QModelIndex) -> int:
        return len(self.__wifiList)

    def addWifiItem(self, item):
        self.__wifiList.append(item)

    def reset(self):
        self.__wifiList.clear()

    def getWifiList(self):
        return self.__wifiList
