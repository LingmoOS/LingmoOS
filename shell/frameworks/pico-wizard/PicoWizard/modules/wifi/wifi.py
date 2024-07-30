# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2021 Anupam Basak <anupam.basak27@gmail.com>

import os
import re

from PySide2.QtCore import QUrl, Slot, Signal, Property, QObject, QProcess
from PySide2.QtQml import qmlRegisterType

from PicoWizard.module import Module
from PicoWizard.modules.wifi.wifimodel import WifiModel
from PicoWizard.utils.logger import Logger


class Wifi(Module):
    log = Logger.getLogger(__name__)

    def __init__(self, parent=None):
        super().__init__(__file__, parent)

        self._wifiModel = WifiModel(parent)

        self.listWifi()

    @staticmethod
    def registerTypes() -> None:
        qmlRegisterType(Wifi, 'PicoWizard', 1, 0, 'WifiModule')

    @staticmethod
    def qmlPath() -> QUrl:
        return QUrl(os.path.join(os.path.dirname(os.path.realpath(__file__)), "Wifi.qml"))

    @Slot(None, result=str)
    def moduleName(self) -> str:
        return self.tr("Wifi")

    @Signal
    def modelChanged(self):
        pass

    @Property(QObject, notify=modelChanged)
    def model(self):
        return self._wifiModel

    def listWifi(self):
        self.log.info('Fetching list of wifi')

        args = [
            '/usr/bin/nmcli',
            '-c',
            'no',
            '-f',
            'BSSID,SSID,SIGNAL,SECURITY',
            '-t',
            'dev',
            'wifi',
            'list',
            '--rescan',
            'yes'
        ]

        process = QProcess(self)
        process.start('/usr/bin/pkexec', args)

        process.finished.connect(lambda exitCode, exitStatus: self.listWifiProcessFinished(process, exitCode, exitStatus))
        process.errorOccurred.connect(lambda err: self.listWifiProcessError(process, err))

    def listWifiProcessFinished(self, process, exitCode, exitStatus):
        self.log.debug(f'List Wifi process status : {exitStatus}[CODE {exitCode}]')
        self.log.info('Listing Wifi')

        output = process.readAll().data().decode()

        if exitCode != 0 or "Error:" in output:
            self.log.error(f'Failed to get wifi list : {exitStatus}')
            self.errorOccurred.emit("Error listing wifi connections")
        else:
            self.log.debug('Parsing wifi data')
            self.generateWifiList(output)

    def listWifiProcessError(self, process, err):
        self.log.error(f'Failed to get wifi list : {err}')

    def generateWifiList(self, output):
        self._wifiModel.reset()
        wifiItemRegex = '(.*):(.*):(.*):(.*)'
        matches = re.findall(wifiItemRegex, output)

        for match in matches:
            if match[1]:
                wifiItem = {
                    'bssid': match[0].replace('\\', ''),
                    'ssid': match[1],
                    'signal': match[2],
                    'security': match[3] if match[3] else 'Open',
                    'isSecured': True if match[3] else False
                }

                self._wifiModel.layoutAboutToBeChanged.emit()
                self._wifiModel.addWifiItem(wifiItem)
                self._wifiModel.layoutChanged.emit()

        self.log.info('Generated Wifi List')
        self.log.debug(self._wifiModel.getWifiList())

    @Slot(int, str, result=None)
    def setWifi(self, wifiIndex, password):
        bssid = self._wifiModel.data(self._wifiModel.index(wifiIndex, 0), WifiModel.BssidRole)
        ssid = self._wifiModel.data(self._wifiModel.index(wifiIndex, 0), WifiModel.SsidRole)
        self.log.debug(f'Selected SSID : {ssid}')

        process = QProcess(self)
        args = [
            '/usr/bin/nmcli',
            'dev',
            'wifi',
            'connect',
            f'{ssid}',
            'password',
            f'{password}',
            'bssid',
            f'{bssid}'
        ]

        process.start('/usr/bin/pkexec', args)

        process.finished.connect(lambda exitCode, exitStatus: self.setWifiCmdSuccess(process, exitCode, exitStatus, ssid))
        process.error.connect(lambda err: self.setWifiCmdFailed(process, err))

    def setWifiCmdSuccess(self, process, exitCode, exitStatus, ssid):
        self.log.debug(f'Connect Wifi process status : {exitStatus} [CODE {exitCode}]')

        output = process.readAll().data().decode()

        if exitCode != 0 or "Error:" in output:
            self.log.error(f'Failed to connect to wifi : {output} [{exitStatus}]')
            self.errorOccurred.emit("Failed to connect to wifi. Recheck the password and try again.")
            self.connectWifiFailed.emit()
        else:
            self.log.info("Successfully connected to wifi")
            Module.__ENV__.insert('PICOWIZARD_WIFI', ssid)
            self.connectWifiSuccess.emit()

    def setWifiCmdFailed(self, process, err):
        self.log.error(f'Failed to connect to wifi : {err}')
        self.errorOccurred.emit("Failed to connect to wifi")
        self.connectWifiFailed.emit()

    @Signal
    def connectWifiSuccess(self):
        pass

    @Signal
    def connectWifiFailed(self):
        pass
