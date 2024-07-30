# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2021 Anupam Basak <anupam.basak27@gmail.com>

import logging
import logging.handlers
import os
from enum import Enum
try:
    from systemd.journal import JournalHandler
    systemd = True
except Exception:
    systemd = False

import PicoWizard.utils.constants as Constants


class Logger:
    LOG_LEVEL = 'INFO'
    LOG_FILE_PATH = Constants.LOG_FILE_PATH

    class Mode(Enum):
        INFO = 0
        DEBUG = 1

    @staticmethod
    def setLogMode(mode: Mode):
        if mode == Logger.Mode.INFO:
            Logger.LOG_LEVEL = 'INFO'
            Logger.LOG_FILE_PATH = Constants.LOG_FILE_PATH
        elif mode == Logger.Mode.DEBUG:
            Logger.LOG_LEVEL = 'DEBUG'
            Logger.LOG_FILE_PATH = Constants.LOG_FILE_PATH

            log = Logger.getLogger(__name__)

            log.debug("Debug mode enabled")

    @staticmethod
    def getLogger(name):
        if not os.path.exists(os.path.dirname(Logger.LOG_FILE_PATH)):
            os.makedirs(os.path.dirname(Logger.LOG_FILE_PATH))

        # Logging settings
        formatter = logging.Formatter(
            fmt='%(asctime)s %(name)-35s [%(levelname)-5s] %(message)s',
            datefmt='%Y-%m-%d %H:%M:%S'
        )

        handler = logging.handlers.RotatingFileHandler(Logger.LOG_FILE_PATH, maxBytes=1000*1000*10, backupCount=10)
        handler.setFormatter(formatter)

        screenHandler = logging.StreamHandler()
        screenHandler.setFormatter(formatter)

        logger = logging.getLogger(name)
        logger.setLevel(logging.getLevelName(Logger.LOG_LEVEL))

        logger.addHandler(handler)
        logger.addHandler(screenHandler)

        global systemd
        if systemd:
            journalHandlerFormatter = logging.Formatter(
                fmt='%(name)-35s [%(levelname)-5s] %(message)s'
            )
            journalHandler = JournalHandler(SYSLOG_IDENTIFIER='pico-wizard')
            journalHandler.setFormatter(journalHandlerFormatter)

            logger.addHandler(journalHandler)

        return logger
