/*
    kiconengineplugin.cpp: Qt plugin providing the ability to create a KIconEngine

    This file is part of the KDE project, module kdeui.
    SPDX-FileCopyrightText: 2018 Fabian Vogt <fabian@ritter-vogt.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include <QIconEnginePlugin>

#include <KIconEngine>
#include <KIconLoader>

QT_BEGIN_NAMESPACE

class KIconEnginePlugin : public QIconEnginePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QIconEngineFactoryInterface" FILE "kiconengineplugin.json")

public:
    QIconEngine *create(const QString &file) override
    {
        return new KIconEngine(file, KIconLoader::global());
    }
};

QT_END_NAMESPACE

#include "kiconengineplugin.moc"
