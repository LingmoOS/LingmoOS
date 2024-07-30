// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#ifndef SMARTCTL_H
#define SMARTCTL_H

#include <QJsonDocument>
#include <QObject>

#include <queue>

class AbstractSMARTCtl : public QObject
{
    Q_OBJECT
public:
    ~AbstractSMARTCtl() override = default;
    virtual void run(const QString &devicePath) = 0;

Q_SIGNALS:
    /**
     * @param devicePath the device the request was finished for (same as was passed into run())
     * @param document the JSON presentation of the report
     * @param the text presentation of the report (ordinary CLI output from smartctl)
     */
    void finished(const QString &devicePath, const QJsonDocument &document, const QString &textDocument) const;

protected:
    AbstractSMARTCtl() = default;

private:
    Q_DISABLE_COPY(AbstractSMARTCtl)
};

class SMARTCtl : public AbstractSMARTCtl
{
public:
    void run(const QString &devicePath) override;

private:
    bool m_busy = false;
    std::queue<QString> m_requestQueue;
};

#endif // SMARTCTL_H
