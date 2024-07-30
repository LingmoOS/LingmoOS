# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2021 Anupam Basak <anupam.basak27@gmail.com>

import os
import subprocess
import sys

from PySide2.QtCore import QUrl, Slot, Signal, QProcess, QByteArray, Property
from PySide2.QtQml import qmlRegisterType

from PicoWizard.module import Module
from PicoWizard.utils.config import Config
from PicoWizard.utils.logger import Logger


class User(Module):
    log = Logger.getLogger(__name__)

    def __init__(self, parent=None):
        super().__init__(__file__, parent)

        self._passwordType = Config.getPasswordType()
        self.passwordTypeChanged.emit()

    @staticmethod
    def registerTypes() -> None:
        qmlRegisterType(User, 'PicoWizard', 1, 0, 'UserModule')

    @staticmethod
    def qmlPath() -> QUrl:
        return QUrl(os.path.join(os.path.dirname(os.path.realpath(__file__)), "User.qml"))

    @Slot(None, result=str)
    def moduleName(self) -> str:
        return self.tr("User Configuration")

    @Slot(str, str, str, result=None)
    def createUser(self, fullname, username, password):
        process = QProcess(self)
        args = [
            '/usr/bin/useradd',
            '--create-home',
            '--comment',
            fullname,
            username
        ]
        process.start('/usr/bin/pkexec', args)

        process.finished.connect(lambda exitCode, exitStatus: self.createUserCmdSuccess(exitCode, username, password))
        process.error.connect(lambda err: self.createUserCmdFailed(err))

    def createUserCmdSuccess(self, exitCode, username, password):
        if exitCode != 0:
            self.log.error('Failed to create user')
            self.errorOccurred.emit("Failed to create user")
            self.createUserFailed.emit()
        else:
            self.log.info('User successfully created')
            Module.__ENV__.insert('PICOWIZARD_USERNAME', username)

            process = QProcess(self)
            args = [
                '/usr/bin/passwd',
                username
            ]
            process.start('/usr/bin/pkexec', args)

            inputPassword = QByteArray(f"{password}\n{password}\n".encode())
            process.write(inputPassword)

            process.finished.connect(lambda exitCode, exitStatus: self.passwordCmdSuccess(exitCode, exitStatus))
            process.error.connect(lambda err: self.passwordCmdFailed(err))

    def createUserCmdFailed(self, err):
        self.log.error('Failed to create user')
        self.log.error(err)
        self.errorOccurred.emit("Failed to create user")
        self.createUserFailed.emit()

    def passwordCmdSuccess(self, exitCode, exitStatus):
        if exitCode != 0:
            self.log.error('Failed to set password')
            self.errorOccurred.emit("Failed to set password")
            self.createUserFailed.emit()
        else:
            self.log.info('Password successfully set')
            self.createUserSuccess.emit()

    def passwordCmdFailed(self, err):
        self.log.error('Failed to set password')
        self.errorOccurred.emit("Failed to set password")
        self.log.error(err)
        self.createUserFailed.emit()

    @Signal
    def createUserSuccess(self):
        pass

    @Signal
    def createUserFailed(self):
        pass

    def getPasswordType(self):
        return self._passwordType

    @Signal
    def passwordTypeChanged(self):
        pass

    passwordType = Property(str, getPasswordType, notify=passwordTypeChanged)
