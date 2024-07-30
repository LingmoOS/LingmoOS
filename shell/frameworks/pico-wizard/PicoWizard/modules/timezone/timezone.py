# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2021 Anupam Basak <anupam.basak27@gmail.com>

import os

from PySide2.QtCore import QUrl, Slot, Property, Signal, QObject, QSortFilterProxyModel, Qt, QProcess
from PySide2.QtQml import qmlRegisterType

from PicoWizard.module import Module
from PicoWizard.modules.timezone.timezonemodel import TimezoneModel
from PicoWizard.utils.logger import Logger


class Timezone(Module):
    log = Logger.getLogger(__name__)
    _filterText = ''

    def __init__(self, parent=None):
        super().__init__(__file__, parent)

        self._timezoneModel = TimezoneModel(parent)
        self._timezoneProxyModel = QSortFilterProxyModel(parent)

        self._timezoneProxyModel.setSourceModel(self._timezoneModel)
        self._timezoneProxyModel.setFilterRole(TimezoneModel.TzRole)
        self._timezoneProxyModel.setFilterCaseSensitivity(Qt.CaseInsensitive)

        self._timezoneProxyModel.setSortRole(TimezoneModel.TzRole)
        self._timezoneProxyModel.setSortCaseSensitivity(Qt.CaseInsensitive)
        self._timezoneProxyModel.sort(0, Qt.AscendingOrder)

    @staticmethod
    def registerTypes() -> None:
        qmlRegisterType(Timezone, 'PicoWizard', 1, 0, 'TimezoneModule')

    @staticmethod
    def qmlPath() -> QUrl:
        return QUrl(os.path.join(os.path.dirname(os.path.realpath(__file__)), "Timezone.qml"))

    @Slot(None, result=str)
    def moduleName(self) -> str:
        return self.tr("Timezone")

    @Signal
    def modelChanged(self):
        pass

    @Signal
    def filterTextChanged(self):
        pass

    def _getFilterText(self):
        return self._filterText

    def _setFilterText(self, text):
        self._filterText = text
        self._timezoneProxyModel.setFilterRegExp(text)

    @Property(QObject, notify=modelChanged)
    def model(self):
        return self._timezoneProxyModel

    @Slot(int, result=None)
    def setTimezone(self, tzIndex):
        timezone = self._timezoneProxyModel.data(self._timezoneProxyModel.index(tzIndex, 0), TimezoneModel.TzRole)
        self.log.debug(f'Selected Timezone : {timezone}')
        Module.__ENV__.insert('PICOWIZARD_TZ', timezone)

        process = QProcess(self)
        args = [
            '/usr/bin/ln',
            '-sf',
            os.path.join(
                '/usr',
                'share',
                'zoneinfo',
                timezone
            ),
            '/etc/localtime'
        ]

        process.start('/usr/bin/pkexec', args)

        process.finished.connect(lambda exitCode, exitStatus: self.tzCmdSuccess(exitCode, exitStatus))
        process.error.connect(lambda err: self.tzCmdFailed(err))

    def tzCmdSuccess(self, exitCode, exitStatus):
        self.log.info('Setting Timezone')

        if exitCode != 0:
            self.log.error('Failed to set timezone')
            self.setTimezoneFailed.emit()
            self.errorOccurred.emit("Failed to set Timezone")
        else:
            self.setTimezoneSuccess.emit()

    def tzCmdFailed(self, err):
        self.log.error('Failed to set timezone')
        self.log.error(err)
        self.setTimezoneFailed.emit()
        self.errorOccurred.emit("Failed to set Timezone")

    @Signal
    def setTimezoneSuccess(self):
        pass

    @Signal
    def setTimezoneFailed(self):
        pass

    filterText = Property(str, _getFilterText, _setFilterText, notify=filterTextChanged)
