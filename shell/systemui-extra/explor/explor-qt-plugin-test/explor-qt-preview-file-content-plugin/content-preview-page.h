/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2019, Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef CONTENTPREVIEWPAGE_H
#define CONTENTPREVIEWPAGE_H

#include <QWidget>
#include "preview-page-plugin-iface.h"

class QLabel;

class ContentPreviewPage : public QWidget, public Peony::PreviewPageIface
{
    Q_OBJECT
public:
    explicit ContentPreviewPage(QWidget *parent = nullptr);
    ~ContentPreviewPage() override;

    void prepare(const QString &uri, PreviewType type) override {
        m_current_uri = uri;
        m_current_type = type;
    }
    void prepare(const QString &uri) override {
        m_current_uri = uri;
    }
    void startPreview() override;
    void cancel() override;
    void closePreviewPage() override {
        deleteLater();
    }

signals:

public slots:

private:
    QLabel *m_label = nullptr;

    QString m_current_uri = nullptr;
    Peony::PreviewPageIface::PreviewType m_current_type = Peony::PreviewPageIface::Other;
};

#endif // CONTENTPREVIEWPAGE_H
