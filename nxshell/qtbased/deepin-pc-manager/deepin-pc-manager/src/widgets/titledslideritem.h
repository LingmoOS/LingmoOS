// Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEF_WIDGETS_TITLEDSLIDERITEM_H
#define DEF_WIDGETS_TITLEDSLIDERITEM_H

#include "settingsitem.h"

class QLabel;
class QSlider;

namespace def {
namespace widgets {

class NormalLabel;
class DCCSlider;

class TitledSliderItem : public SettingsItem
{
    Q_OBJECT
public:
    TitledSliderItem(QString title, QWidget *parent = nullptr);

    DCCSlider *slider() const;
    void setAnnotations(const QStringList &annotations);

    QString valueLiteral() const;
    void setValueLiteral(const QString &valueLiteral);

    void setLeftIcon(const QIcon &leftIcon);
    void setRightIcon(const QIcon &rightIcon);
    void setIconSize(const QSize &size);
    void setTitle(QString str);

private:
    NormalLabel *m_titleLabel;
    NormalLabel *m_valueLabel;
    DCCSlider *m_slider;
    QString m_valueLiteral;
};

} // namespace widgets
} // namespace def

#endif // DEF_WIDGETS_TITLEDSLIDERITEM_H
