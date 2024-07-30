# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2021 Anupam Basak <anupam.basak27@gmail.com>

import typing
from enum import IntEnum, auto

import PySide2
from PySide2.QtCore import QAbstractListModel, Qt, QEnum


class LocaleModel(QAbstractListModel):
    """
    A List of locales
    Array of {
        'locale': [column0, column1],
        'selected': bool
    }
    """
    __locales__ = []

    @QEnum
    class Roles(IntEnum):
        NameRole = Qt.UserRole + 1
        SelectedRole = auto()

    def __init__(self, parent=None):
        super().__init__(parent)

    def roleNames(self) -> typing.Dict:
        roles = dict()
        roles[self.Roles.NameRole] = b'name'
        roles[self.Roles.SelectedRole] = b'selected'

        return roles

    def data(self, index: PySide2.QtCore.QModelIndex, role: Roles) -> typing.Any:
        if role == self.Roles.NameRole:
            return self.__locales__[index.row()]['locale'][0]
        elif role == self.Roles.SelectedRole:
            return self.__locales__[index.row()]['selected']

    def setData(self, index: PySide2.QtCore.QModelIndex, value, role: Roles = None):
        if role == self.Roles.SelectedRole:
            self.__locales__[index.row()]['selected'] = value
            self.dataChanged.emit(index, index)

            return True
        else:
            return False

    def rowCount(self, parent: PySide2.QtCore.QModelIndex) -> int:
        return len(self.__locales__)

    def addLocaleItem(self, locale, selected=False):
        self.layoutAboutToBeChanged.emit()

        self.__locales__.append({
            'locale': locale,
            'selected': selected
        })

        self.layoutChanged.emit()

    def getSelectedLocales(self):
        selectedLocales = []
        for row in self.__locales__:
            if row['selected']:
                selectedLocales.append(row['locale'])

        return selectedLocales
