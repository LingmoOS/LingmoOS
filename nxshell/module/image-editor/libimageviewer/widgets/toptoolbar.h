// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TOPTOOLBAR_H
#define TOPTOOLBAR_H

#include "abstracttoptoolbar.h"

#include <QJsonObject>
#include <QPointer>
#include <DTitlebar>
#include <DLabel>
#include <QGraphicsDropShadowEffect>
#include <DFontSizeManager>
#include <DApplicationHelper>
#include <DMenu>
#include <DBlurEffectWidget>

DWIDGET_USE_NAMESPACE

class SettingsWindow;
class QHBoxLayout;
class QProcess;

class LibTopToolbar : public AbstractTopToolbar
{
    Q_OBJECT
public:
    LibTopToolbar(bool manager, QWidget *parent);
    void setMiddleContent(const QString &path) override;
public slots:
    void setTitleBarTransparent(bool a) override;
protected:
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void paintEvent(QPaintEvent *e) override;
    void resizeEvent(QResizeEvent *event) override;
    void leaveEvent(QEvent *event) override;
signals:
    void updateMaxBtn();
private:
    enum MenuItemId {
        IdCreateAlbum,
        IdSwitchTheme,
        IdSetting,
        IdImport,
        IdHelp,
        IdAbout,
        IdQuick,
        IdSeparator
    };
    void initMenu();
    void initWidgets();
    QString  geteElidedText(QFont font, QString str, int MaxWidth);

private slots:
    //void onThemeTypeChanged();
    //void onUpdateFileName(const QString &filename);

private:
    QColor m_coverBrush;
    QColor m_topBorderColor;
    QColor m_bottomBorderColor;
    QPointer<QProcess> m_manualPro;
    QHBoxLayout *m_layout = nullptr;
    QHBoxLayout *m_lLayout = nullptr;
    QHBoxLayout *m_mLayout = nullptr;
    QHBoxLayout *m_rLayout = nullptr;
    DTitlebar *m_titlebar = nullptr;
    DLabel *m_titletxt = nullptr;
    QString m_filename;
#ifndef LITE_DIV
    SettingsWindow *m_settingsWindow;
#endif
    DMenu *m_menu;
    bool m_manager;

    bool m_viewChange;
    QGraphicsDropShadowEffect *shadowEffect {nullptr};
};

#endif // TOPTOOLBAR_H
