// SPDX-FileCopyrightText: 2019 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "dtkwidget_global.h"
#include "utils/utils.h"

#include <DObject>

#include <QProgressBar>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

/*!
 * \class ColoredProgressBar
 * \brief ColoredProgressBar is the same as QProgressBar, except it can change its appearance depending on the value displayed.
 */
class ColoredProgressBar : public QProgressBar
{
    Q_OBJECT
public:
    explicit ColoredProgressBar(QWidget *parent = nullptr);

    /*!
     * \brief ColoredProgressBar::addThreshold adds a new threshold value and specifies the brush to use once that value is
     * reached. If a threshold of the same value already exists, it will be overwritten. \param threshold Minimum value for this
     * brush to be used. \param brush The brush to use when the currently displayed value is no less than \threshold and less than
     * the next threshold value.
     */
    void addThreshold(int threshold, QBrush brush);

    /*!
     * \brief ColoredProgressBar::removeThreshold removes a threshold.
     * \param threshold The threshold value to remove.
     */
    void removeThreshold(int threshold);
    QList<int> thresholds() const;

protected:
    /*!
     * \brief ColoredProgressBar::threadsholds gets all threshold values.
     * \return A list of threshold values.
     */
    void paintEvent(QPaintEvent *) override;

private:
    DGuiApplicationHelper::ColorType m_themeType;
    QMap<int, QBrush> threshmap;

private slots:
    void themeChanged();
};
