# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2021 Anupam Basak <anupam.basak27@gmail.com>

import os
from queue import SimpleQueue

import PySide2
from PySide2.QtCore import QUrl, Slot, Signal, Property, QTimer, QProcess
from PySide2.QtQml import qmlRegisterType
from PySide2.QtWidgets import QApplication

from PicoWizard.module import Module
from PicoWizard.utils.constants import SCRIPTS_DIR
from PicoWizard.utils.logger import Logger


class Finish(Module):
    log = Logger.getLogger(__name__)

    _isComplete = False
    _runningScriptIndex = -1
    _totalScriptsCount = -1

    def __init__(self, parent=None):
        super().__init__(__file__, parent)
        self._scriptsQueue = SimpleQueue()

    @staticmethod
    def registerTypes() -> None:
        qmlRegisterType(Finish, 'PicoWizard', 1, 0, 'FinishModule')

    @staticmethod
    def qmlPath() -> QUrl:
        return QUrl(os.path.join(os.path.dirname(os.path.realpath(__file__)), "Finish.qml"))

    def _getIsComplete(self):
        return self._isComplete

    def _getRunningScriptIndex(self):
        return self._runningScriptIndex

    def _getTotalScriptsCount(self):
        return self._totalScriptsCount

    def _processScripts(self):
        if self._scriptsQueue.empty():
            self._isComplete = True
            self.isCompleteChanged.emit()
        else:
            script = self._scriptsQueue.get()
            self._runningScriptIndex += 1
            self.runningScriptIndexChanged.emit()

            self.log.info(f'Running Script : {script.arguments()[2]}')

            script.finished.connect(lambda exitCode: self._scriptFinishedHandler(script, exitCode))
            script.errorOccurred.connect(lambda error: self._scriptErroredHandler(script, error))
            script.readyReadStandardOutput.connect(lambda: self.log.info(f'Output: {script.readAllStandardOutput()}'))

            script.start()

    def _scriptFinishedHandler(self, script, exitCode):
        self.log.info(f'Script [{script.arguments()[2]}] Finished: Exit code ({exitCode})')
        self._processScripts()

    def _scriptErroredHandler(self, script, error):
        self.log.info(f'Script [{script.arguments()[2]}] Errored: {error}')
        self._processScripts()

    @Slot(None, result=str)
    def moduleName(self) -> str:
        return "Finish"

    @Slot(None)
    def runScripts(self):
        if os.path.exists(SCRIPTS_DIR):
            with open("/tmp/pico-wizard.env", "w") as env:
                for key in Module.__ENV__.keys():
                    env.writelines(f'export {key}="{Module.__ENV__.value(key)}"\n')

            scripts = os.listdir(SCRIPTS_DIR)
            scripts.sort()

            self._totalScriptsCount = len(scripts)
            self.totalScriptsCountChanged.emit()

            self.log.debug(Module.__ENV__.toStringList())

            for script in scripts:
                if ".hook" not in script:
                    args = [
                        "/usr/bin/pico-wizard-script-runner",
                        "/tmp/pico-wizard.env",
                        os.path.join(SCRIPTS_DIR, script)
                    ]

                    process = QProcess(self)
                    process.setProgram('/usr/bin/pkexec')
                    process.setArguments(args)

                    self._scriptsQueue.put(process)

        self._processScripts()

    @Slot(None)
    def runCompleteHook(self):
        process = QProcess(self)
        args = None

        if os.path.isfile("/etc/pico-wizard/scripts.d/finish.hook"):
            self.log.info("Executing finish hook")
            args = [
                "/usr/bin/pico-wizard-script-runner",
                "/tmp/pico-wizard.env",
                os.path.join(SCRIPTS_DIR, 'finish.hook')
            ]
        else:
            self.log.info("Rebooting")
            args = [
                "/bin/shutdown",
                "-r",
                "now"
            ]

        process.start('/usr/bin/pkexec', args)

    @Signal
    def isCompleteChanged(self):
        pass

    @Signal
    def runningScriptIndexChanged(self):
        pass

    @Signal
    def totalScriptsCountChanged(self):
        pass

    isComplete = Property(bool, _getIsComplete, notify=isCompleteChanged)
    runningScriptIndex = Property(int, _getRunningScriptIndex, notify=runningScriptIndexChanged)
    totalScriptsCount = Property(int, _getTotalScriptsCount, notify=totalScriptsCountChanged)