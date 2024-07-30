# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2021 Anupam Basak <anupam.basak27@gmail.com>

import typing

import PySide2
from PySide2.QtCore import QAbstractListModel, Qt, Slot
from PicoWizard.modules.timezone.timezoneslist import TIMEZONES_LIST


class TimezoneModel(QAbstractListModel):
    TzRole = Qt.UserRole + 1

    def __init__(self, parent=None):
        super().__init__(parent)

    def roleNames(self) -> typing.Dict:
        roles = dict()
        roles[self.TzRole] = b'tz'

        return roles

    def data(self, index: PySide2.QtCore.QModelIndex, role: int) -> typing.Any:
        if role == self.TzRole:
            return TIMEZONES_LIST[index.row()]

    def rowCount(self, parent: PySide2.QtCore.QModelIndex) -> int:
        return len(TIMEZONES_LIST)
