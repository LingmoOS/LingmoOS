// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHLINEEDIT_H
#define SEARCHLINEEDIT_H

#include <QLineEdit>
#include <QLabel>
#include <QSpacerItem>
#include <QPropertyAnimation>
#include <QProxyStyle>

#include <DIconButton>

DWIDGET_USE_NAMESPACE

class SearchLineeditStyle : public QProxyStyle
{
    friend class SearchLineEdit;

public:
    explicit SearchLineeditStyle(QStyle *style = nullptr);

    int pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const;

private:
    bool hideCursor;
};

class SearchLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit SearchLineEdit(QWidget *parent = nullptr);
    ~SearchLineEdit() override;

public slots:
    void normalMode();
    void moveFloatWidget();

private slots:
    void editMode();
    void onTextChanged();

protected:
    bool event(QEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void themeChanged();

private:
    DIconButton *m_icon;
    DIconButton *m_clear;
    QLabel *m_placeholderText;
    QWidget *m_floatWidget;
#ifndef ARCH_MIPSEL
    QPropertyAnimation *m_floatAni;
#endif
    SearchLineeditStyle *m_editStyle;
};

#endif // SEARCHLINEEDIT_H
