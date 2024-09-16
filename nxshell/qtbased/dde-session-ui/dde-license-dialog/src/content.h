// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONTENT_H
#define CONTENT_H

#include <QWidget>
#include <dtkwidget_global.h>
#include <DButtonBox>

DWIDGET_BEGIN_NAMESPACE
class DSuggestButton;
class DCommandLinkButton;
DWIDGET_END_NAMESPACE

class QScrollArea;
class QPushButton;
class QCheckBox;
class QLabel;
class Content : public QWidget
{
    Q_OBJECT
public:
    explicit Content(QWidget *parent = nullptr);
    void setHideBottom(const bool &status);
    /**
     * @brief calWidgetWidth 根据底下QCheckBox和QPushButton控件字宽计算窗口宽度
     * @return 返回计算后的窗口宽度
     */
    int calWidgetWidth();
signals:
    void sourceChanged(bool isCn);

public slots:
    void setSource(const QString& source);
    void setAllowCheckBoxText(const QString& text);
    void setEnAllowCheckBoxText(const QString& text);
    void setCnSource(const QString& source);
    void setEnSource(const QString& source);
    void updateLocaleSource();

private:
    void updateLanguageBtn();
    void updateContent();
    void updateWindowHeight();

private:
    QScrollArea* m_scrollArea;
    QCheckBox* m_acceptCheck;
    QPushButton* m_cancelBtn;
    DTK_WIDGET_NAMESPACE::DSuggestButton* m_acceptBtn;
    QLabel* m_source;
    DTK_WIDGET_NAMESPACE::DButtonBox* m_languageBtn;
    bool m_isCn;
    bool m_hasCn;
    bool m_hasEn;

    QString m_cn;
    QString m_en;
    QString m_allow;
    QString m_enallow;

    QWidget *m_bottom;
};

#endif // CONTENT_H
