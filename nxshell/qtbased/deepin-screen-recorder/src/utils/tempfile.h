// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEMPFILE_H
#define TEMPFILE_H

#include <QObject>
#include <QMap>
#include <QWindow>

class TempFile : public QObject
{
    Q_OBJECT
public:
    static TempFile *instance();

public slots:
    inline const QPixmap getFullscreenPixmap() const
    {
        return m_fullscreenPixmap;
    }
    inline const QPixmap getMosaicPixmap(const int radius) const
    {
        return m_mosaicPixmap[radius];
    }
    inline const QPixmap getBlurPixmap(const int radius) const
    {
        return m_blurPixmap[radius];
    }

    void setFullScreenPixmap(const QPixmap &pixmap);
    void setBlurPixmap(const QPixmap &pixmap, const int radius);
    void setMosaicPixmap(const QPixmap &pixmap, const int radius);
    bool isContainsPix(const QString effect, const int radius);
private:
    explicit TempFile(QObject *parent = 0);
    ~TempFile();

    QPixmap m_fullscreenPixmap;
    QMap<int, QPixmap> m_mosaicPixmap;
    QMap<int, QPixmap> m_blurPixmap;
};
#endif // TEMPFILE_H
