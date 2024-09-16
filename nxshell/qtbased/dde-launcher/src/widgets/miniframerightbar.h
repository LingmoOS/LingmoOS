// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MINIFRAMERIGHTBAR_H
#define MINIFRAMERIGHTBAR_H

#include <DGuiApplicationHelper>

#include <QWidget>
#include <QLabel>

DGUI_USE_NAMESPACE

class MiniFrameButton;
class QButtonGroup;

class MiniFrameRightBar : public QWidget
{
    Q_OBJECT

public:
    enum ButtonType {
        Setting,
        Power,
        Unknow
    };

    explicit MiniFrameRightBar(QWidget *parent = nullptr);
    ~MiniFrameRightBar() override;

    void setButtonChecked(const ButtonType buttonId) const;
    bool isButtonChecked(const ButtonType buttonId) const;

protected:
    void changeEvent(QEvent *event) override;

private:
    void initUi();
    void initConnection();
    void initAccessibleName();

signals:
    void requestFrameHide();

public slots:
    void showShutdown();
    void showSettings();

private slots:
    void updateIcon(DGuiApplicationHelper::ColorType themeType);

private:
    MiniFrameButton *m_settingsBtn;
    MiniFrameButton *m_powerBtn;
    QButtonGroup *m_buttonGroup;
    QStringList m_hideList;
};

#endif // MINIFRAMERIGHTBAR_H
