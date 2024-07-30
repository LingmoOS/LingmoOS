# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: LGPL-2.0-or-later

import os
import qgis
from LayerTasks import *
from SpatialIndexTasks import *
from TimezoneTableTasks import *
from config import *

class MainTask(QgsTask):
    def __init__(self, context):
        super().__init__('Generating geographic data for KI18N', QgsTask.CanCancel)
        self.context = context
        self.loadLayersTask = LoadLayersTask(context)
        self.addSubTask(self.loadLayersTask, [], QgsTask.ParentDependsOnSubTask)

        self.tzStringTableTask = TimezoneStringTableTask(context)
        self.addSubTask(self.tzStringTableTask, [self.loadLayersTask], QgsTask.ParentDependsOnSubTask)
        self.regionToTzMapTask = RegionToTimezoneMapTask(context)
        self.addSubTask(self.regionToTzMapTask, [self.loadLayersTask], QgsTask.ParentDependsOnSubTask)

        self.spatialIndexTask = SpatialIndexerTask(context, self.loadLayersTask)
        self.addSubTask(self.spatialIndexTask, [self.loadLayersTask, self.regionToTzMapTask], QgsTask.ParentDependsOnSubTask)

    def run(self):
        QgsMessageLog.logMessage('Generation completed.', LOG_CATEGORY, Qgis.Info)
        return True

# main
os.chdir(os.path.join(os.path.dirname(QgsProject.instance().fileName()), 'data'))
context = {}
task = MainTask(context)
QgsApplication.taskManager().addTask(task)
