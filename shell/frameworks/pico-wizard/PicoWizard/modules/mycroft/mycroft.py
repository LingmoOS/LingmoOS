# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2021 Aditya Mehra <aix.m@outlook.com>

import os
import subprocess
import sys

from PySide2.QtCore import QUrl, Slot, Signal, QProcess, QByteArray, Property
from PySide2.QtQml import qmlRegisterType

from PicoWizard.module import Module
from PicoWizard.utils.logger import Logger


class Mycroft(Module):
    log = Logger.getLogger(__name__)

    def __init__(self, parent=None):
        super().__init__(__file__, parent)

    @staticmethod
    def registerTypes() -> None:
        qmlRegisterType(Mycroft, 'PicoWizard', 1, 0, 'MycroftModule')

    @staticmethod
    def qmlPath() -> QUrl:
        return QUrl(os.path.join(os.path.dirname(os.path.realpath(__file__)), "Mycroft.qml"))

    @Slot(None, result=str)
    def moduleName(self) -> str:
        return self.tr("Mycroft")

    @Slot(None, result=None)
    def enableMycroftIntegration(self):
        process = QProcess(self)
        args = [os.path.join(os.path.dirname(os.path.realpath(__file__)), 'SetupMycroft.sh'), '--enable']
        process.start('/usr/bin/pkexec', args)

        process.finished.connect(lambda exitCode, exitStatus: self.enableMycroftSuccess(exitCode))
        process.error.connect(lambda err: self.enableMycroftFailed(err))

    def enableMycroftSuccess(self, exitCode):
        self.log.info('Mycroft Enabled Successfully')
        self.mycroftEnabledSuccess.emit()

    def enableMycroftFailed(self, exitCode):
        self.log.info('Mycroft Enabled Failed')
        self.mycroftEnabledFailed.emit()

    @Slot(None, result=None)
    def disableMycroftIntegration(self):
        process = QProcess(self)
        args = [os.path.join(os.path.dirname(os.path.realpath(__file__)), 'SetupMycroft.sh'), ' --disable']
        process.start('/usr/bin/pkexec', args)

        process.finished.connect(lambda exitCode, exitStatus: self.disableMycroftSuccess(exitCode))
        process.error.connect(lambda err: self.disableMycroftFailed(err))

    def disableMycroftSuccess(self, exitCode):
        self.log.info('Mycroft Enabled Successfully')
        self.mycroftDisabledSuccess.emit()

    def disableMycroftFailed(self, exitCode):
        self.log.info('Mycroft Enabled Failed')
        self.mycroftDisabledFailed.emit()

    @Signal
    def mycroftEnabledSuccess(self):
        pass

    @Signal
    def mycroftEnabledFailed(self):
        pass

    @Signal
    def mycroftDisabledSuccess(self):
        pass

    @Signal
    def mycroftDisabledFailed(self):
        pass
