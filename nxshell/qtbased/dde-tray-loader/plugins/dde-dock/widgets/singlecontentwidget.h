// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SINGLE_CONTENT_WIDGET_H
#define SINGLE_CONTENT_WIDGET_H

#include <QWidget>
#include <QHBoxLayout>
#include <QPointer>
#include <QList>
#include <QDebug>

/**
 * @brief 类似于QStackedWidget的作用，不同的是，这个控件的大小会跟随显示的内容变化，而QStackedWidget大小为显示内容的最大size
 * 使用方法同QStackedWidget一样。
 */
class SingleContentWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SingleContentWidget(QWidget* parent = nullptr)
        : m_mainLayout(new QHBoxLayout(this))
    {
        m_mainLayout->setMargin(0);
        m_mainLayout->setSpacing(0);
    }

    void insertWidget(int index, QWidget *w, Qt::Alignment alignment = Qt::Alignment())
    {
        if (!w || m_mainLayout->children().contains(w)) {
            return;
        }
        m_widgets.append(w);
        m_mainLayout->insertWidget(index, w, 0, alignment);
        if (!m_currentWidget) {
            m_currentWidget = w;
        }
    }

    void setCurrentIndex(int index)
    {
        for (const auto w : findChildren<QWidget*>()) {
            if (m_mainLayout->indexOf(w) == index) {
                m_currentWidget = w;
                w->setVisible(true);
            } else if (m_widgets.contains(w)) {
                w->setVisible(false);
            }
        }
    }

    int currentIndex() const
    {
        return m_mainLayout->indexOf(m_currentWidget);
    }

    QWidget* currentWidget()
    {
        if (m_currentWidget) {
            return m_currentWidget.data();
        }
        return this;
    }

    QWidget* widget(int index)
    {
        for (const auto w : findChildren<QWidget*>()) {
            if (m_mainLayout->indexOf(w) == index) {
                return w;
            }
        }
        return this;
    }

    int count() const
    {
        return m_mainLayout->count();
    }

private:
    QHBoxLayout *m_mainLayout;
    QList<QPointer<QWidget>> m_widgets;
    QPointer<QWidget> m_currentWidget;
};

#endif
