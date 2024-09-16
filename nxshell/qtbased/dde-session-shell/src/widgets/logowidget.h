// SPDX-FileCopyrightText: 2015 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOGOWIDGET
#define LOGOWIDGET

#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <DLabel>

DWIDGET_USE_NAMESPACE

class LogoWidget : public QFrame
{
    Q_OBJECT
public:
    LogoWidget(QWidget *parent = nullptr);
    ~LogoWidget() override;

    void updateLocale(const QString &locale);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void initUI();
    QPixmap loadSystemLogo();
    QString getVersion();

private:
    QLabel *m_logoLabel;
    DLabel *m_logoVersionLabel;
    QString m_locale;
};
#endif // LOGOFRAME
