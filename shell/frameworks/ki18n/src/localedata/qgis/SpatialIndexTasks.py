# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: LGPL-2.0-or-later

from config import *
import datetime
import functools
import os
import pytz
import time
from qgis import *
from qgis.core import *

xStep = xRange / (1 << zDepth)
yStep = yRange / (1 << zDepth)

def z2x(z):
    x = 0
    for i in range(0, zDepth):
        x += (z & (1 << i * 2)) >> i
    return x

def z2y(z):
    y = 0
    for i in range(0, zDepth):
        y += (z & (1 << (1 + i * 2))) >> (i + 1)
    return y

def rectForZ(z, depth):
    mask = (1 << (2*(zDepth - depth))) - 1
    x = z2x(z & ~mask) * xStep + xStart
    y = z2y(z & ~mask) * yStep + yStart
    xSize = xRange / (1 << depth)
    ySize = yRange / (1 << depth)
    return QgsRectangle(x, y, x + xSize, y + ySize)

def tzIdToEnum(tzId):
    if not tzId:
        return 'Undefined'
    return tzId.replace('/', '_').replace('-', '_')

# Layer specific configuration for things considered in the spatial index
class LayerConfig:
    def readFeatureValue(self, feature):
        return feature[self.featureKey]

    def isValidFeature(self, feature):
        return feature != None

    def normalizeAndFilter(self, features):
        for key in features:
            if not self.isValidFeature(key):
                del(features[key])
        return features

    def resolveConflicts(self, features):
        out = []
        for key in features:
            out.append((key, features[key]));
        n = functools.reduce(lambda n, f: n + f[1], out, 0)
        if n > 0:
            out = [(k, v/n) for (k, v) in out]
        out = list(filter(lambda x: x[1] > featureAreaRatioThreshold, out))
        out.sort(key = lambda x: x[1], reverse = True)
        return out

class TzLayerConfig(LayerConfig):
    def __init__(self):
        self.layer = 'tzLayer'
        self.featureKey = 'tzid'

    def readFeatureValue(self, feature):
        tzId = feature[self.featureKey]
        if tzId in TZID_MAP:
            return TZID_MAP[tzId]
        return tzId

    def resolveConflicts(self, features):
        if len(features) > 1:
            tzs = list(features.keys())
            # check for conflicting timezones
            tz = pytz.timezone(tzs[0])
            if not all(self.isSameTimezone(tz, pytz.timezone(x)) for x in tzs[1:]):
                return None
            out = super().resolveConflicts(features)
            return [out[0]]
        return super().resolveConflicts(features)

    def isSameTimezone(self, lhs, rhs):
        try:
            # hacky tz comparison, lacking access to the rules for comparing actual DST transition times
            # TODO stabilize results and ensure we capture differences due to different week boundaries
            dt = datetime.datetime.today().toordinal()
            return all(lhs.utcoffset(datetime.datetime.fromordinal(dt + 30*x)) == rhs.utcoffset(datetime.datetime.fromordinal(dt + 30*x))
                   and lhs.tzname(datetime.datetime.fromordinal(dt + 30*x)) == rhs.tzname(datetime.datetime.fromordinal(dt + 30*x)) for x in range(0, 11))
        except:
            return False

class CountryLayerConfig(LayerConfig):
    def __init__(self):
        self.layer = 'countryLayer'
        self.featureKey = 'ISO3166-1'

    def readFeatureValue(self, feature):
        code = feature[self.featureKey]
        if code in ISO3166_1_MAP:
            return ISO3166_1_MAP[code]
        return code

    def normalizeAndFilter(self, features):
        if len(features) > 1:
            # apply country disambiguation
            for d in ISO3166_1_DISAMBIGUATION_MAP:
                if d[0] in features and d[1] in features:
                    del(features[d[1]])
        return super().normalizeAndFilter(features)

    def resolveConflicts(self, features):
        out = super().resolveConflicts(features)
        if len(out) > 1:
            return None
        return out

class SubdivLayerConfig(LayerConfig):
    def __init__(self):
        self.layer = 'subdivLayer'
        self.featureKey = 'ISO3166-2'

    def resolveConflicts(self, features):
        out = super().resolveConflicts(features)
        if len(out) > 1:
            return None
        return out
#
# Parallelized spatial index computation of a single sub-tile
#
class SpatialIndexerSubTask(QgsTask):
    def __init__(self, context, zStart, zStartDepth):
        super().__init__('Compute spatial index sub-tile ' + str(zStart), QgsTask.CanCancel)
        self.context = context
        self.zStart = zStart
        self.zStartDepth = zStartDepth
        self.lastFeature = []
        self.exception = None
        self.result = []
        self.layerConfig = [TzLayerConfig(), CountryLayerConfig(), SubdivLayerConfig()]

    def run(self):
        try:
            self.computeTile(self.zStart, self.zStartDepth)
        except Exception as e:
            self.exception = e
            QgsMessageLog.logMessage('Exception in task "{}"'.format(self.exception), LOG_CATEGORY, Qgis.Info)
            return False
        return True

    def computeTile(self, zStart, depth):
        if self.isCanceled() or depth < 1:
            return
        z = zStart
        d = depth - 1
        zIncrement = 1 << (2*d)
        for i in range(0, 4):
            # find features in the input vector layer inside our current tile
            # we cannot take shortcuts here and avoid the expensive area computation
            # as we do get spurious 0-area results for some large and disjoint polygons (FR, US, NZ, etc)
            feature = []
            featureCount = 0
            for layerConfig in self.layerConfig:
                l = {}
                rect = rectForZ(z, zDepth - d)
                rectGeo = QgsGeometry.fromRect(rect)
                for f in self.context[layerConfig.layer].getFeatures(rect):
                    featureArea = f.geometry().intersection(rectGeo).area()
                    if featureArea > 0.0:
                        key = layerConfig.readFeatureValue(f)
                        if key in l:
                            l[key] += featureArea / rectGeo.area()
                        else:
                            l[key] = featureArea / rectGeo.area()
                feature.append(layerConfig.normalizeAndFilter(l))
                featureCount = max(featureCount, len(l))

            # recurse on conflicts
            if depth > 1 and featureCount > 1:
                self.computeTile(z, d)
            # leaf tile, process the result
            else:
                # try to clean up any remaining conflicts
                for i in range(len(feature)):
                    feature[i] = self.layerConfig[i].resolveConflicts(feature[i])
                self.resolveConflicts(feature)
                # if there's a change to the previous value, propagate to the result output
                if self.lastFeature != feature and feature != []:
                    self.result.append((z, feature))
                    self.lastFeature = feature

            z += zIncrement

    def resolveConflicts(self, feature):
        # if we have a unique subdivision but no country, use the subdivision's country
        # this happens in coastal regions when territorial waters overlap in the tile, but
        # actual land boundaries don't. As we primarily care about correct result on land,
        # we can ignore the overlap on water
        if (not feature[1] or len(feature[1]) == 0) and feature[2] and len(feature[2]) == 1:
            feature[1] = [(feature[2][0][0][:2], 1)]

        # if subdivision country and country code conflict, discard the subdivision
        # this is mainly a problem for French overseas territories, and there the country
        # code is actually more useful
        if feature[1] and len(feature[1]) == 1 and feature[2] and len(feature[2]) == 1 and feature[1][0][0] != feature[2][0][0][:2]:
            feature[2] = None

    def finished(self, result):
        if not result and self.exception != None:
            QgsMessageLog.logMessage('Task "{name}" Exception: {exception}'.format(name=self.description(), exception=self.exception), LOG_CATEGORY, Qgis.Critical)
            raise self.exception

#
# Task for spawning the sub-tasks doing the actual work, and accumulating the result
#
class SpatialIndexerTask(QgsTask):
    def __init__(self, context, loadLayersTask):
        super().__init__('Compute spatial index', QgsTask.CanCancel)
        self.context = context
        self.tasks = []
        self.conflictTiles = [0, 0, 0]
        self.startTime = time.time()
        self.propertyTable = []

        startDepth = 4
        startIncrement = 1 << (2 * (zDepth - startDepth))
        for i in range(0, (1 << (2 * startDepth))):
            task = SpatialIndexerSubTask(context, i * startIncrement, zDepth - startDepth)
            self.addSubTask(task, [loadLayersTask], QgsTask.ParentDependsOnSubTask)
            self.tasks.append(task)

    def run(self):
        try:
            QgsMessageLog.logMessage('Aggregating results...', LOG_CATEGORY, Qgis.Info)

            # compact the spatial index and prepare the property map
            spatialIndex = []
            propertyMap = {}
            prevProperty = (None, None, None)
            propertyMap[prevProperty] = 0
            for task in self.tasks:
                for (z,res) in task.result:
                    res = self.normalize(res);
                    tmp = []
                    for i in range(len(res)):
                        if res[i] == None or len(res[i]) == 0:
                            tmp.append(None)
                        else:
                            tmp.append(res[i][0][0])
                        if res[i] == None:
                            self.conflictTiles[i] += 1
                    prop = (tmp[0], tmp[1], tmp[2])

                    if prevProperty == prop:
                        continue
                    prevProperty = prop
                    spatialIndex.append((z, prop))
                    propertyMap[prop] = 0

            for prop in propertyMap:
                self.propertyTable.append(prop)
            self.propertyTable.sort(key = lambda x: (x[0] if x[0] else "", x[1] if x[1] else "", x[2] if x[2] else ""))
            for i in range(len(self.propertyTable)):
                propertyMap[self.propertyTable[i]] = i

            # write spatial index
            out = open('../../data/spatial_index_data.cpp', 'w')
            out.write("""/*
 * SPDX-License-Identifier: ODbL-1.0
 * SPDX-FileCopyrightText: OpenStreetMap contributors
 *
 * Autogenerated spatial index generated using QGIS.
 */

#include "spatial_index_entry_p.h"

static constexpr const SpatialIndexEntry spatial_index[] = {
    // clang-format off
""")
            prevProperties = (None, None, None)
            for (z, prop) in spatialIndex:
                out.write(f"    {{{z}, {propertyMap[prop]}}},\n")
            out.write("    // clang-format on\n};\n")
            out.close()

            # write property table
            out = open('../../data/spatial_index_properties.cpp', 'w')
            out.write("""/*
 * SPDX-License-Identifier: ODbL-1.0
 * SPDX-FileCopyrightText: OpenStreetMap contributors
 *
 * Autogenerated spatial index generated using QGIS.
 */

#include "spatial_index_property_p.h"
#include "timezone_names_p.h"

static constexpr const SpatialIndexProperty spatial_index_properties[] = {
""")
            for p in self.propertyTable:
                if not p[1] and not p[2]:
                    out.write(f"    {{Tz::{tzIdToEnum(p[0])}}},\n")
                elif not p[2]:
                    out.write(f"    {{Tz::{tzIdToEnum(p[0])}, \"{p[1]}\"}},\n")
                else:
                    out.write(f"    {{Tz::{tzIdToEnum(p[0])}, \"{p[2]}\"}},\n")
            out.write("};\n")
            out.close()

            # write zindex parameters
            out = open('../../data/spatial_index_parameters_p.h', 'w')
            out.write("""/*
 * SPDX-License-Identifier: CC0-1.0
 * SPDX-FileCopyrightText: none
 *
 * Autogenerated spatial index generated using QGIS.
 */

#include <cstdint>

""")
            out.write(f"constexpr const float XStart = {xStart};\n")
            out.write(f"constexpr const float XRange = {xRange};\n")
            out.write(f"constexpr const float YStart = {yStart};\n")
            out.write(f"constexpr const float YRange = {yRange};\n")
            out.write(f"constexpr const uint8_t ZDepth = {zDepth};\n")
            out.close()

            return True

        except Exception as e:
            self.exception = e
            QgsMessageLog.logMessage('Exception in task "{}"'.format(self.exception), LOG_CATEGORY, Qgis.Info)
            return False

    def normalize(self, feature):
        # normalization we couldn't do in the sub tasks as they rely on results not available yet at that point
        # fill in missing timezones from country/subdiv to tz maps
        # this is needed for the same reason we do the subdivision to country back filling above, conflicts in
        # territorial waters but a unique result on land
        tz = None
        if feature[2] and len(feature[2]) == 1:
            subdivCode = feature[2][0][0]
            subdivToTz = self.context['subdivisionToTimezoneMap']
            if subdivCode in subdivToTz and len(subdivToTz[subdivCode]) == 1:
                tz = list(subdivToTz[subdivCode])[0]
        if not tz and feature[1] and len(feature[1]) == 1:
            countryCode = feature[1][0][0]
            countryToTz = self.context['countryToTimezoneMap']
            if countryCode in countryToTz and len(countryToTz[countryCode]) == 1:
                tz = list(countryToTz[countryCode])[0]
        if tz:
            feature[0] = [(tz, 1)]

        return feature

    def finished(self, result):
        QgsMessageLog.logMessage('Finished task "{}"'.format(self.description()), LOG_CATEGORY, Qgis.Info)
        tileCount = 1 << (2 * zDepth)
        for i in range(len(self.conflictTiles)):
            QgsMessageLog.logMessage(f" {self.conflictTiles[i] / tileCount} of feature {i} area is conflicted", LOG_CATEGORY, Qgis.Info)
        QgsMessageLog.logMessage(f" collected {len(self.propertyTable)} distinct features", LOG_CATEGORY, Qgis.Info)
        QgsMessageLog.logMessage(f" computation took {time.time() - self.startTime} seconds", LOG_CATEGORY, Qgis.Info)
        if not result and self.exception != None:
            QgsMessageLog.logMessage('Task "{name}" Exception: {exception}'.format(name=self.description(), exception=self.exception), LOG_CATEGORY, Qgis.Critical)
            raise self.exception

