/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include "index-monitor-test.h"
#include <QDebug>

IndexMonitorTest::IndexMonitorTest(QObject *parent) : QObject(parent)
{
    m_monitor = LingmoUISearch::IndexMonitor::self();
    connect(m_monitor, &LingmoUISearch::IndexMonitor::serviceReady, this, [&](){
        qDebug() << "==serviceReady==";
        qDebug() << "==currentIndexPaths==" << m_monitor->currentIndexPaths();
        qDebug() << "==indexState==" << m_monitor->indexState();
        qDebug() << "==basicIndexSize==" << m_monitor->basicIndexSize();
        qDebug() << "==contentIndexSize==" << m_monitor->contentIndexSize();
        qDebug() << "==ocrContentIndexSize==" << m_monitor->ocrContentIndexSize();
        qDebug() << "==basicIndexProgress==" << m_monitor->basicIndexProgress();
        qDebug() << "==contentIndexProgress==" << m_monitor->contentIndexProgress();
        qDebug() << "==ocrContentIndexProgress==" << m_monitor->ocrContentIndexProgress();
        qDebug() << "==basicIndexDocNum==" << m_monitor->basicIndexDocNum();
        qDebug() << "==contentIndexDocNum==" << m_monitor->contentIndexDocNum();
        qDebug() << "==ocrContentIndexDocNum==" << m_monitor->ocrContentIndexDocNum();
        qDebug() << "==basicIndexUpdating==" << m_monitor->basicIndexUpdating();
        qDebug() << "==contentIndexUpdating==" << m_monitor->contentIndexUpdating();
        qDebug() << "==ocrContentIndexUpdating==" << m_monitor->ocrContentIndexUpdating();
    });
    connect(m_monitor, &LingmoUISearch::IndexMonitor::serviceOffline, this, [&](){
        qDebug() << "==serviceReady==";
    });
    connect(m_monitor, &LingmoUISearch::IndexMonitor::currentIndexPathsChanged, this, [&](const QStringList& currentIndexPaths){
        qDebug() << "==currentIndexPathsChanged==" << currentIndexPaths;
    });
    connect(m_monitor, &LingmoUISearch::IndexMonitor::indexStateChanged, this, [&](const QString& indexState){
        qDebug() << "==indexStateChanged==" << indexState;
    });
    connect(m_monitor, &LingmoUISearch::IndexMonitor::basicIndexSizeChanged, this, [&](uint basicIndexSize){
        qDebug() << "==basicIndexSizeChanged==" << basicIndexSize;
    });
    connect(m_monitor, &LingmoUISearch::IndexMonitor::contentIndexSizeChanged, this, [&](uint contentIndexSize){
        qDebug() << "==contentIndexSizeChanged==" << contentIndexSize;
    });
    connect(m_monitor, &LingmoUISearch::IndexMonitor::ocrContentIndexSizeChanged, this, [&](uint ocrIndexSize){
        qDebug() << "==ocrContentIndexSizeChanged==" << ocrIndexSize;
    });
    connect(m_monitor, &LingmoUISearch::IndexMonitor::basicIndexProgressChanged, this, [&](uint basicIndexProgress){
        qDebug() << "==basicIndexProgressChanged==" << basicIndexProgress;
    });
    connect(m_monitor, &LingmoUISearch::IndexMonitor::contentIndexProgressChanged, this, [&](uint contentIndexProgress){
        qDebug() << "==contentIndexProgressChanged==" << contentIndexProgress;
    });
    connect(m_monitor, &LingmoUISearch::IndexMonitor::ocrContentIndexProgressChanged, this, [&](uint ocrIndexProgress){
        qDebug() << "==ocrContentIndexProgressChanged==" << ocrIndexProgress;
    });
    connect(m_monitor, &LingmoUISearch::IndexMonitor::basicIndexDocNumChanged, this, [&](uint basicIndexDocNum){
        qDebug() << "==basicIndexDocNumChanged==" << basicIndexDocNum;
    });
    connect(m_monitor, &LingmoUISearch::IndexMonitor::contentIndexDocNumChanged, this, [&](uint contentIndexDocNum){
        qDebug() << "==contentIndexDocNumChanged==" << contentIndexDocNum;
    });
    connect(m_monitor, &LingmoUISearch::IndexMonitor::ocrContentIndexDocNumChanged, this, [&](uint ocrContentIndexDocNum){
        qDebug() << "==ocrContentIndexDocNumChanged==" << ocrContentIndexDocNum;
    });
    connect(m_monitor, &LingmoUISearch::IndexMonitor::basicIndexStart, this, [&](){
        qDebug() << "==basicIndexStart==";
    });
    connect(m_monitor, &LingmoUISearch::IndexMonitor::contentIndexStart, this, [&](){
        qDebug() << "==contentIndexStart==";
    });
    connect(m_monitor, &LingmoUISearch::IndexMonitor::ocrContentIndexStart, this, [&](){
        qDebug() << "==ocrContentIndexStart==";
    });
    connect(m_monitor, &LingmoUISearch::IndexMonitor::basicIndexDone, this, [&](bool success){
        qDebug() << "==basicIndexDone==" << success;
    });
    connect(m_monitor, &LingmoUISearch::IndexMonitor::contentIndexDone, this, [&](bool success){
        qDebug() << "==contentIndexDone==" << success;
    });
    connect(m_monitor, &LingmoUISearch::IndexMonitor::ocrContentIndexDone, this, [&](bool success){
        qDebug() << "==ocrContentIndexDone==" << success;
    });
    connect(m_monitor, &LingmoUISearch::IndexMonitor::basicIndexUpdatingChanged, this, [&](bool updating){
        qDebug() << "==basicIndexUpdatingChanged==" << updating;
    });
    connect(m_monitor, &LingmoUISearch::IndexMonitor::contentIndexUpdatingChanged, this, [&](bool updating){
        qDebug() << "==contentIndexUpdatingChanged==" << updating;
    });
    connect(m_monitor, &LingmoUISearch::IndexMonitor::ocrContentIndexUpdatingChanged, this, [&](bool updating){
        qDebug() << "==ocrContentIndexUpdatingChanged==" << updating;
    });
}

IndexMonitorTest::~IndexMonitorTest()
{
    LingmoUISearch::IndexMonitor::stopMonitor();
}
