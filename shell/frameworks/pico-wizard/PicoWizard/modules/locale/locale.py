# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2021 Anupam Basak <anupam.basak27@gmail.com>

import os

from PySide2.QtCore import QUrl, Slot, Property, QObject, Signal, QSortFilterProxyModel, Qt, QProcess
from PySide2.QtQml import qmlRegisterType

from PicoWizard.module import Module
from PicoWizard.modules.locale.localemodel import LocaleModel
from PicoWizard.modules.locale.localeslist import locales
from PicoWizard.utils.logger import Logger


class Locale(Module):
    log = Logger.getLogger(__name__)
    __filterText__ = ''

    def __init__(self, parent=None):
        super().__init__(__file__, parent)

        self.__localeModel__ = LocaleModel(parent)
        self.__localeProxyModel__ = QSortFilterProxyModel(parent)

        self.__localeProxyModel__.setSourceModel(self.__localeModel__)
        self.__localeProxyModel__.setFilterRole(LocaleModel.Roles.NameRole)
        self.__localeProxyModel__.setFilterCaseSensitivity(Qt.CaseInsensitive)

        self.__localeProxyModel__.setSortRole(LocaleModel.Roles.NameRole)
        self.__localeProxyModel__.setSortCaseSensitivity(Qt.CaseInsensitive)
        self.__localeProxyModel__.sort(0, Qt.AscendingOrder)

        for locale in locales:
            self.__localeModel__.addLocaleItem(locale)

    @staticmethod
    def registerTypes() -> None:
        qmlRegisterType(Locale, 'PicoWizard', 1, 0, 'LocaleModule')
        qmlRegisterType(LocaleModel, 'PicoWizard', 1, 0, 'LocaleModel')

    @staticmethod
    def qmlPath() -> QUrl:
        return QUrl(os.path.join(os.path.dirname(os.path.realpath(__file__)), "Locale.qml"))

    @Slot(None, result=str)
    def moduleName(self) -> str:
        return self.tr("Locale")

    @Slot(None)
    def writeLocaleGenConfig(self):
        self.log.debug(f'Selected locales : {self.__localeModel__.getSelectedLocales()}')

        process = QProcess(self)
        args = [os.path.join(os.path.dirname(os.path.realpath(__file__)), "writelocalegenconfig.sh")]

        for locale in self.__localeModel__.getSelectedLocales():
            args.append(f"{locale[0]} {locale[1]}")

        self.log.debug(f"writelocalegenconfig.sh arguments : {args}")

        process.start('/usr/bin/pkexec', args)

        process.finished.connect(lambda exitCode, exitStatus: self.writeLocaleScriptSuccess(exitCode, exitStatus, process))
        process.error.connect(lambda err: self.writeLocaleScriptError(err))

    def writeLocaleScriptSuccess(self, exitCode, exitStatus, process):
        if exitCode != 0:
            self.log.error('Failed to write `localegen` config')
            self.localeSetupFailed.emit()
            self.errorOccurred.emit("Failed to write `localegen` config")
        else:
            self.log.info('`localegen` config written successfully')
            self.log.info("Running `locale-gen` command to generate locales")

            process = QProcess(self)
            args = ['locale-gen']

            process.start('/usr/bin/pkexec', args)
            process.finished.connect(lambda exitCode, exitStatus: self.localeGenCmdSuccess(exitCode, exitStatus, process))
            process.error.connect(lambda err: self.localeGenCmdError(err))


    def writeLocaleScriptError(self, err):
        self.log.error('Failed to write `localegen` config')
        self.log.error(err)
        self.localeSetupFailed.emit()
        self.errorOccurred.emit("Failed to write `localegen` config")

    def localeGenCmdSuccess(self, exitCode, exitStatus, process):
        if exitCode != 0:
            self.log.error('`locale-gen` command failed')
            self.localeSetupFailed.emit()
            self.errorOccurred.emit("`locale-gen` command failed")
        else:
            self.log.info('`locale-gen` command complete')
            self.log.debug(process.readAll())
            self.localeSetupSuccess.emit()

    def localeGenCmdError(self, err):
        self.log.error('`locale-gen` command failed')
        self.log.error(err)
        self.localeSetupFailed.emit()
        self.errorOccurred.emit("`locale-gen` command failed")

    def __getModel__(self):
        return self.__localeProxyModel__

    def __getFilterText__(self):
        return self.__filterText__

    def __setFilterText__(self, text):
        self.__filterText__ = text
        self.__localeProxyModel__.setFilterRegExp(text)

    filterTextChanged = Signal()
    modelChanged = Signal()
    localeSetupSuccess = Signal()
    localeSetupFailed = Signal()

    model = Property(QObject, __getModel__, notify=modelChanged)
    filterText = Property(str, __getFilterText__, __setFilterText__, notify=filterTextChanged)
