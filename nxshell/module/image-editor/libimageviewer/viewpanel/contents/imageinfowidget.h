// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGEINFOWIDGET_H
#define IMAGEINFOWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QScrollArea>
#include <QVector>

#include <denhancedwidget.h>
#include <DScrollArea>
#include <DDrawer>
#include <DLabel>

DWIDGET_USE_NAMESPACE
typedef DLabel QLbtoDLabel;

class QFormLayout;
class QVBoxLayout;
class ViewSeparator;

class LibImageInfoWidget : public QFrame
{
    Q_OBJECT
    
public:
    explicit LibImageInfoWidget(const QString &darkStyle, const QString &lightStyle, QWidget *parent = nullptr);
    ~LibImageInfoWidget() Q_DECL_OVERRIDE;
    void setImagePath(const QString &path, bool forceUpdate = true);
    void updateInfo();
    int contentHeight() const;

    Q_SIGNAL void extensionPanelHeight(int height);

protected:
    void showEvent(QShowEvent *e) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    void timerEvent(QTimerEvent *e) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private:
    void clearLayout(QLayout *layout);
    const QString trLabel(const char *str);
    void updateBaseInfo(const QMap<QString, QString> &infos, bool CNflag);
    void updateDetailsInfo(const QMap<QString, QString> &infos, bool CNflag);
    QList<DDrawer *> addExpandWidget(const QStringList &titleList);
    void initExpand(QVBoxLayout *layout, DDrawer *expand);

private:
    int m_updateTid = 0;
    int m_maxTitleWidth;  // For align colon
    int m_maxFieldWidth;
    int m_currentFontSize;  // LMH0609上次显示的字体大小
    bool m_isBaseInfo = false;
    bool m_isDetailsInfo = false;
    QString m_path;
    QMap<QString, QString> m_metaData;
    QFrame *m_exif_base = nullptr;
    QFrame *m_exif_details = nullptr;
    QFormLayout *m_exifLayout_base = nullptr;
    QFormLayout *m_exifLayout_details = nullptr;
    ViewSeparator *m_separator = nullptr;
    QList<DDrawer *> m_expandGroup;
    QVBoxLayout *m_mainLayout = nullptr;
    QScrollArea *m_scrollArea = nullptr;
    QString m_closedString;
};

#endif  // IMAGEINFOWIDGET_H
