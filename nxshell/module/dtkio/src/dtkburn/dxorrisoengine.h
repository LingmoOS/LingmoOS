// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DXORRISOENGINE_H
#define DXORRISOENGINE_H

#include <xorriso.h>

#include <QObject>

#include "dtkburn_global.h"
#include "dtkburntypes.h"

DBURN_BEGIN_NAMESPACE

class DXorrisoEngine : public QObject
{
    Q_OBJECT

public:
    enum class JolietSupported {
        False,
        True
    };

    enum class RockRageSupported {
        False,
        True
    };

    enum class KeepAppendable {
        False,
        True
    };

public:
    explicit DXorrisoEngine(QObject *parent = nullptr);
    ~DXorrisoEngine() override;

    bool acquireDevice(const QString &dev);
    void releaseDevice();
    void clearResult();
    MediaType mediaTypeProperty() const;
    void mediaStorageProperty(quint64 *usedSize, quint64 *availSize, quint64 *blocks) const;
    bool mediaFormattedProperty() const;
    QString mediaVolIdProperty() const;
    QStringList mediaSpeedProperty() const;
    QStringList takeInfoMessages();

    bool doErase();
    bool doWriteISO(const QString &isoPath, int speed);
    bool doDumpISO(quint64 dataBlocks, const QString &isoPath);
    bool doCheckmedia(quint64 dataBlocks, double *qgood, double *qslow, double *qbad);
    bool doBurn(const QPair<QString, QString> files, int speed, QString volId,
                JolietSupported joliet, RockRageSupported rockRage, KeepAppendable appendable);

public Q_SLOTS:
    void messageReceived(int type, char *text);

Q_SIGNALS:
    void jobStatusChanged(JobStatus status, int progress, QString speed);

private:
    void initialize();

private:
    XorrisO *m_xorriso;
    QString m_curDev;
    QStringList m_xorrisomsg;
    QString m_curspeed;
    quint64 m_curDatablocks;
};

DBURN_END_NAMESPACE

#endif   // DXORRISOENGINE_H
