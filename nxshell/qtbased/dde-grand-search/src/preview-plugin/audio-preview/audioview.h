// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AUDIOVIEW_H
#define AUDIOVIEW_H

#include "previewplugin.h"

#include <QLabel>

namespace GrandSearch {
namespace audio_preview {

class NameLabel: public QLabel
{
    Q_OBJECT
public:
    explicit NameLabel(const QString &text = "", QWidget *parent = nullptr, Qt::WindowFlags f = {});
};

class SizeLabel: public QLabel
{
    Q_OBJECT
public:
    explicit SizeLabel(const QString &text = "", QWidget *parent = nullptr, Qt::WindowFlags f = {});
};

class AudioView : public QWidget
{
    Q_OBJECT
public:
    explicit AudioView(QWidget *parent = nullptr);
    void setItemInfo(const ItemInfo &item);
private:
    void initUI();
    QIcon defaultIcon(const QString &fileName);
private:
    QLabel *m_iconLabel = nullptr;
    NameLabel *m_nameLabel = nullptr;
    SizeLabel *m_sizeLabel = nullptr;
};

}}

#endif // AUDIOVIEW_H
