/*
 *  SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "qscreenoutput.h"

#include <edid.h>
#include <mode.h>
#include <output.h>

#include <QGuiApplication>

using namespace KScreen;

QScreenOutput::QScreenOutput(const QScreen *qscreen, QObject *parent)
    : QObject(parent)
    , m_qscreen(qscreen)
    , m_id(-1)
{
}

QScreenOutput::~QScreenOutput()
{
}

int QScreenOutput::id() const
{
    return m_id;
}

void QScreenOutput::setId(const int newId)
{
    m_id = newId;
}

const QScreen *QScreenOutput::qscreen() const
{
    return m_qscreen;
}

OutputPtr QScreenOutput::toKScreenOutput() const
{
    OutputPtr output(new Output);
    output->setId(m_id);
    output->setName(m_qscreen->name());
    updateKScreenOutput(output);
    return output;
}

void QScreenOutput::updateKScreenOutput(OutputPtr &output) const
{
    // Initialize primary output
    output->setEnabled(true);
    output->setConnected(true);

    // Rotation - translate QScreen::primaryOrientation() to Output::rotation()
    if (m_qscreen->primaryOrientation() == Qt::PortraitOrientation) {
        // 90 degrees
        output->setRotation(Output::Right);
    } else if (m_qscreen->primaryOrientation() == Qt::InvertedLandscapeOrientation) {
        // 180 degrees
        output->setRotation(Output::Inverted);
    } else if (m_qscreen->primaryOrientation() == Qt::InvertedPortraitOrientation) {
        // 270 degrees
        output->setRotation(Output::Left);
    }

    // Physical size, geometry, etc.
    QSize mm;
    qreal physicalWidth;
    physicalWidth = m_qscreen->size().width() / (m_qscreen->physicalDotsPerInchX() / 25.4);
    mm.setWidth(qRound(physicalWidth));
    qreal physicalHeight;
    physicalHeight = m_qscreen->size().height() / (m_qscreen->physicalDotsPerInchY() / 25.4);
    mm.setHeight(qRound(physicalHeight));
    output->setSizeMm(mm);
    output->setPos(m_qscreen->availableGeometry().topLeft());
    output->setSize(m_qscreen->availableGeometry().size());

    // Modes: we create a single default mode and go with that
    ModePtr mode(new Mode);
    const QString modeid = QStringLiteral("defaultmode");
    mode->setId(modeid);
    mode->setRefreshRate(m_qscreen->refreshRate());
    mode->setSize(m_qscreen->size());

    const QString modename = QString::number(m_qscreen->size().width()) + QLatin1String("x") + QString::number(m_qscreen->size().height()) + QLatin1String("@")
        + QString::number(m_qscreen->refreshRate());
    mode->setName(modename);

    ModeList modes;
    modes[modeid] = mode;
    output->setModes(modes);
    output->setCurrentModeId(modeid);
}

#include "moc_qscreenoutput.cpp"
