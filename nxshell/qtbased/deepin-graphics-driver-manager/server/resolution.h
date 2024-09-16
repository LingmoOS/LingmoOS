// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RESOLUTION_H_
#define RESOLUTION_H_

#include <QString>
#include <QJsonObject>

class Resolution
{
public:
    Resolution();
    explicit Resolution(const QJsonObject &info);
    bool isNull();

    inline int id() const { return m_resId; }
    inline bool display() const { return m_display; }
    inline bool keep_gltest() const { return m_keepGLTest; }
    inline const QString name() const { return m_resName; }
    inline const QString driver() const { return m_resdriver; }
    inline const QString title() const { return m_resTitle; }
    inline const QString description() const { return m_resDescription; }
    inline const QString installScript() const { return m_installScript; }
    inline const QString removeScript() const { return m_removeScript; }

    inline bool operator==(const Resolution &rhs) const { return m_resName == rhs.m_resName; }

private:
    int m_resId;
    bool m_display;
    bool m_keepGLTest;
    QString m_resName;
    QString m_resdriver;
    QString m_resTitle;
    QString m_resDescription;
    QString m_prepareScript;
    QString m_installScript;
    QString m_removeScript;
    QString m_conditionScript;
    QString m_versionScript;
};

#endif
