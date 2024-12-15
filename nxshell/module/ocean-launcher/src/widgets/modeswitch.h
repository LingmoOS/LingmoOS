// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WIDGET_H
#define WIDGET_H

#include <DGuiApplicationHelper>

#include <QWidget>

DGUI_USE_NAMESPACE

class QButtonGroup;
class QSettings;
class IconButton;
class QAbstractButton;

class ModeSwitch : public QWidget
{
    Q_OBJECT

public:
    enum WindowMode {
        TitleMode = 1,  // 标题分类模式
        LetterMode      // 字母排序模式
    };

    explicit ModeSwitch(QWidget *parent = Q_NULLPTR);
    ~ModeSwitch();

protected:
    void paintEvent(QPaintEvent *event);

signals:
    void buttonClicked(int);

private slots:
    void onButtonClick(int id);
    void onButtonToggle(QAbstractButton *button, bool checked);
    void onThemeTypeChange(DGuiApplicationHelper::ColorType themeType);

private:
    void initUi();
    void initConnection();
    void initAccessibleName();
    void updateIcon();
    void changeCategoryMode();

private:
    IconButton *m_titleModeButton;
    IconButton *m_letterModeButton;
    QButtonGroup *m_buttonGrp;
    QColor m_bgColor;
    QSettings *m_modeSettings;
};

#endif // WIDGET_H
