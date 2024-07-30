# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2021 Aditya Mehra <aix.m@outlook.com>

import os
import time
import cec
import subprocess
import threading

from PySide2.QtCore import QUrl, Slot, Signal, QProcess, QByteArray, Property, QObject
from PySide2.QtQml import qmlRegisterType

from PicoWizard.module import Module
from PicoWizard.utils.logger import Logger


class Cec(Module):
    log = Logger.getLogger(__name__)
            
    def __init__(self, parent=None):
        super().__init__(__file__, parent)
        self.cec_available = False
        self.keystate = None
        
        # 0 = initializing, 1 = cec available, 2 = cec not available, 3 = key mapping stage, 4 = done
        self.setup_stage = 0
        self.key_stage = 0
        self.running = False
        self.threaded_cec = None
        
    @staticmethod
    def registerTypes() -> None:
        qmlRegisterType(Cec, 'PicoWizard', 1, 0, 'CecModule')

    @staticmethod
    def qmlPath() -> QUrl:
        return QUrl(os.path.join(os.path.dirname(os.path.realpath(__file__)), "Cec.qml"))

    @Slot(None, result=str)
    def moduleName(self) -> str:
        return self.tr("Cec")
    
    @Slot(None, result=int)
    def getCurrentSetupStage(self) -> str:
        self.log.info("getCurrentSetupStage requested")
        self.log.info("setup_stage: {}".format(self.setup_stage))
        return self.setup_stage
    
    @Slot(None, result=int)
    def getCurrentKeyStage(self) -> str:
        return self.key_stage

    @Slot(None, result=None)
    def initilize_cec(self):
        self.setup_stage = 0 # initializing
        self.setupStageChanged.emit()

        self.log.info("Initializing CEC")

        self.threaded_cec = threading.Thread(target=self.run_cec_in_thread)
        self.threaded_cec.start()

    # End cec mapping process successfully
    @Slot(None, result=None)
    def end_cec_mapping_success(self):
        process = QProcess(self)
        args = [os.path.join(os.path.dirname(os.path.realpath(__file__)), 'CopyConfig.sh')]
        process.start('/usr/bin/pkexec', args)

    # Restart cec mapping process
    @Slot(None, result=None)
    def restart_cec_mapping(self):
        self.log.info("Restarting CEC mapping")
        self.key_stage = 0
        self.keyStageChanged.emit()
        self.cleanUpConfig()
        self.appendStartToConfig()

    # cleanly close cec thread
    @Slot(None, result=None)
    def close_cec_thread(self):
        self.log.info("Closing CEC thread")
        self.running = False
        self.threaded_cec.join()
        self.threaded_cec = None
    
    # run cec in threaded mode
    def run_cec_in_thread(self):
        self.log.info("Running CEC in thread")
        try:
            cec.init()
            cec.add_callback(self.eventresponse, cec.EVENT_KEYPRESS)
            self.cec_available = True
            self.running = True
            self.setup_stage = 1
            self.setupStageChanged.emit()
            self.setup_stage = 3
            self.setupStageChanged.emit()
            self.keyStageChanged.emit()
            self.cleanUpConfig()
            self.appendStartToConfig()
            
            while self.running:
                time.sleep(1)
        
        except Exception as e:
            self.log.error("Error: {}".format(e))
            self.cec_available = False
            self.setup_stage = 2
            self.setupStageChanged.emit()

    def eventresponse(self, event, key, state):
        if state > 0 and self.keystate == None:
            for x in range(1):
                self.mapKeyToConfig(key)

    def cleanUpConfig(self):
        self.log.info("Cleaning up config")
        if os.path.exists("/tmp/plasma-remotecontrollersrc"):
            os.remove("/tmp/plasma-remotecontrollersrc")

    def appendStartToConfig(self):
        configFile = open("/tmp/plasma-remotecontrollersrc", "a")
        configFile.write("[General]\n")
        configFile.close()

    def mapKeyToConfig(self, key):
        if self.key_stage == 0:
            keyType = "ButtonLeft"
            self.key_stage = self.key_stage + 1
            self.keyStageChanged.emit()
        
        elif self.key_stage == 1:
            keyType = "ButtonRight"
            self.key_stage = self.key_stage + 1
            self.keyStageChanged.emit()

        elif self.key_stage == 2:
            keyType = "ButtonUp"
            self.key_stage = self.key_stage + 1
            self.keyStageChanged.emit()

        elif self.key_stage == 3:
            keyType = "ButtonDown"
            self.key_stage = self.key_stage + 1
            self.keyStageChanged.emit()
  
        elif self.key_stage == 4:
            keyType = "ButtonEnter"
            self.key_stage = self.key_stage + 1
            self.keyStageChanged.emit()
        
        elif self.key_stage == 5:
            keyType = "ButtonBack"
            self.key_stage = self.key_stage + 1
            self.keyStageChanged.emit()

        elif self.key_stage == 6:
            keyType = "ButtonHomepage"
            self.running = False
            self.setup_stage = 4
            self.setupStageChanged.emit()
            self.close_cec_thread()

        self.log.info("Mapping key {} to {}".format(key, keyType))
        
        configFile = open("/tmp/joyclickrc", "a")
        configFile.write("{}={}\n".format(keyType, key))
        configFile.close()

    @Signal
    def setupStageChanged(self):
        pass

    @Signal
    def keyStageChanged(self):
        pass
