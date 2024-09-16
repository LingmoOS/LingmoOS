// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <DApplicationHelper>
#include <QWidget>
DWIDGET_USE_NAMESPACE

class MemoryWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(int roundedCornerRadius READ roundedCornerRadius WRITE setRoundedCornerRadius NOTIFY roundedCornerRadiusChanged FINAL)
public:
    explicit MemoryWidget(QWidget *parent = nullptr);
    virtual ~MemoryWidget() override;

    void updateMemoryInfo(const QString &memPercent,
                          const QString &swapPercent);

    int roundedCornerRadius() const;
    void setRoundedCornerRadius(int newRoundCornerRadius);

signals:
    void roundedCornerRadiusChanged();

protected:
    virtual void paintEvent(QPaintEvent *event) override;

private:
    void changeTheme(DApplicationHelper::ColorType themeType);
    void changeFont(const QFont &font);

private:
    QColor summaryColor;
    QFont m_memTxtFont;

    QColor memoryBackgroundColor;
    QColor memoryColor {"#00C5C0"};
    QColor memoryForegroundColor {"#00C5C0"};
    QColor numberColor;
    QColor swapBackgroundColor;
    QColor swapColor {"#FEDF19"};
    QColor swapForegroundColor {"#FEDF19"};
    QColor backgroundBase {"FFFFFF"};

    qreal memoryBackgroundOpacity = 0.1;
    qreal memoryForegroundOpacity = 1.0;
    qreal swapBackgroundOpacity = 0.1;
    qreal swapForegroundOpacity = 1.0;
    const int ringWidth = 6;

    QFont m_memPercentFont;

    QString m_memPercent;
    //交换内存
    QString m_swapPercent;
    int m_roundedCornerRadius;
};
