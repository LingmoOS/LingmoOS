# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: LGPL-2.0-or-later

import os
import requests
from qgis import *
from qgis.core import *
import time
import zipfile
from config import *

# Download and unpack Shapefiles
class LayerDownloadTask(QgsTask):
    def __init__(self, url, dest):
        super().__init__('Download Shapefile', QgsTask.CanCancel)
        self.url = url
        self.dest = dest

    def run(self):
        try:
            QgsMessageLog.logMessage(f"Downloading and unpacking {self.dest}...", LOG_CATEGORY, Qgis.Info)
            if not os.path.exists(self.dest):
                r = requests.get(self.url)
                if r.status_code < 400:
                    with open(self.dest, 'wb') as f:
                        f.write(r.content)
            with zipfile.ZipFile(self.dest, 'r') as z:
                z.extractall('.')
            QgsMessageLog.logMessage(f"Downloaded and unpacked {self.dest}.", LOG_CATEGORY, Qgis.Info)
        except Exception as e:
            QgsMessageLog.logMessage(f"Exception in task: {e}", LOG_CATEGORY, Qgis.Critical)
        return True


# Load and simplify Shapefile layers
# Simplification is done to massively speed up geometry intersection computation
# (for reference: the original KItinerary tz spatial index took 8h to compute without simplification,
# and about 15 minutes with a Douglas Peucker simplification with a 0.001 threshold, with no practical
# loss of precision
class LoadLayerTask(QgsTask):
    def __init__(self, url, fileName, context, layerName):
        super().__init__(f"Loading layer {fileName}", QgsTask.CanCancel)
        self.layer = None
        self.url = url
        self.fileName = fileName
        self.context = context
        self.layerName = layerName
        self.downloadTask = LayerDownloadTask(url, fileName)
        self.addSubTask(self.downloadTask, [], QgsTask.ParentDependsOnSubTask)

    def run(self):
        QgsMessageLog.logMessage(f"Simplifying layer {self.fileName}", LOG_CATEGORY, Qgis.Info)
        fullLayer = QgsVectorLayer(self.fileName, f"{self.fileName}-full-resolution", 'ogr')
        if not fullLayer.isValid():
            QgsMessageLog.logMessage(f"Failed to load layer {self.fileName}!", LOG_CATEGORY, Qgis.Critical)
        result = processing.run('qgis:simplifygeometries', {'INPUT': fullLayer, 'METHOD': 0, 'TOLERANCE': 0.001, 'OUTPUT': 'TEMPORARY_OUTPUT' })
        self.layer = result['OUTPUT']
        self.layer.setName(f"{self.fileName}-simplified")
        self.context[self.layerName] = self.layer
        QgsMessageLog.logMessage(f"Simplified layer {self.fileName}", LOG_CATEGORY, Qgis.Info)
        return True

    def finished(self, result):
        QgsProject.instance().addMapLayer(self.layer)


# Filter out too small elements in the ISO 3166-2 layer
class Iso3166_2FilterTask(QgsTask):
    def __init__(self, context):
        super().__init__('Filtering ISO 3166-2 layer', QgsTask.CanCancel)
        self.context = context

    def run(self):
        QgsMessageLog.logMessage('Filtering ISO 3166-2 layer', LOG_CATEGORY, Qgis.Info)
        subdivLayer = self.context['subdivLayer']
        toBeRemoved = []
        for feature in subdivLayer.getFeatures():
            # sic: the key is really "admin_leve" in the input file, due to length restrictions in the Shapefile...
            level = feature['admin_leve']
            country = feature['ISO3166-2'][:2]
            if not isinstance(level, str) or not isinstance(country, str):
                continue
            for filter in ISO3166_2_FILTER:
                if int(level) == filter['admin_level'] and country == filter['country']:
                    toBeRemoved.append(feature.id())
                    break
        subdivLayer.dataProvider().deleteFeatures(toBeRemoved)
        return True


# Setup all data layers we need
class LoadLayersTask(QgsTask):
    def __init__(self, context):
        super().__init__('Loading layers...', QgsTask.CanCancel)
        self.context = context
        self.tasks = [
            LoadLayerTask(TZDATA_URL, f"timezones.shapefile-{TZDATA_VERSION}.zip", context, 'tzLayer'),
            LoadLayerTask(ISO3166_1_URL, f"iso3166-1-boundaries.shp-{ISO3166_1_VERSION}.zip", context, 'countryLayer'),
            LoadLayerTask(ISO3166_2_URL, f"iso3166-2-boundaries.shp-{ISO3166_2_VERSION}.zip", context, 'subdivLayer')
        ]
        for task in self.tasks:
            self.addSubTask(task, [], QgsTask.ParentDependsOnSubTask)

        self.filterTask = Iso3166_2FilterTask(context)
        self.addSubTask(self.filterTask, [self.tasks[2]], QgsTask.ParentDependsOnSubTask)

    def run(self):
        return True
