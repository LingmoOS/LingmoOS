#!/usr/bin/env python3

# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2021 Anupam Basak <anupam.basak27@gmail.com>

import sys
import os

# Add current dir to system path for module name resolution
from PicoWizard.utils.config import Config

sys.path.append(os.path.dirname(os.path.realpath(__file__ + '/../')))

import argparse
import signal

from PySide2.QtCore import QUrl
from PySide2.QtQml import QQmlApplicationEngine, qmlRegisterType, QQmlFileSelector
from PySide2.QtQuickControls2 import QQuickStyle
from PySide2.QtWidgets import QApplication

from PicoWizard.utils.logger import Logger
import PicoWizard.utils.constants as Constants


log = None


def registerTypes():
    log.info('Registering QML types')
    qmlRegisterType(ModuleLoader, 'PicoWizard', 1, 0, 'ModuleLoader')


def __main__():
    log.info('Initializing Application')

    app = QApplication(sys.argv)
    app.setApplicationName('pico-wizard')
    app.setApplicationDisplayName('Pico Wizard')

    registerTypes()
    ModuleLoader.registerModuleTypes()

    engine = QQmlApplicationEngine()
    selector = QQmlFileSelector(engine);
    engine.addImportPath(os.path.dirname(os.path.realpath(__file__)))
    engine.rootContext().setContextProperty('maximizeWindow', args.maximize)
    engine.load(QUrl(os.path.join(os.path.dirname(os.path.realpath(__file__)), 'start.qml')))

    if not engine.rootObjects():
        sys.exit(-1)

    signal.signal(signal.SIGINT, signal.SIG_DFL)

    sys.exit(app.exec_())


parser = argparse.ArgumentParser(prog='pico-wizard', description='Pico Wizard')
parser.add_argument('--debug', dest='debug', action='store_true', help='Enable debug mode')
parser.add_argument('--maximize', dest='maximize', action="store_true", help='Maximize window')
parser.set_defaults(debug=False)
parser.set_defaults(maximize=False)
args = parser.parse_args()

Logger.setLogMode(Config.getLogLevel())

if args.debug:
    Logger.setLogMode(Logger.Mode.DEBUG)

log = Logger.getLogger(__name__)

log.info(f'Logging to file {Constants.LOG_FILE_PATH}')

# Import ModuleLoader after setting debug mode
from PicoWizard.moduleloader import ModuleLoader

if __name__ == '__main__':
    # execute only if run as the entry point into the program
    __main__()
