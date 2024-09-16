// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef _DMR_TIP_H
#define _DMR_TIP_H
#include <QFrame>
#include <QGuiApplication>
#include <DPalette>
#include <DApplicationHelper>
#include <DFontSizeManager>
#include <QApplication>
#include <QDesktopWidget>
#include <QDBusInterface>

namespace dmr {
class TipPrivate;
class Tip : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(int radius READ radius WRITE setRadius)
    Q_PROPERTY(QBrush background READ background WRITE setBackground)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor)
public:
    explicit Tip(const QPixmap &icon,
                 const QString &text,
                 QWidget *parent = 0);
    ~Tip();

    //单元测试在使用
    void pop(QPoint center);

    int radius() const;
    QColor borderColor() const;
    QBrush background() const;

public slots:
    void setText(const QString text);
    void setBackground(QBrush background);
    void setRadius(int radius);
    void setBorderColor(QColor borderColor);
    void slotWMChanged(QString msg);

protected:
    virtual void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;
    void enterEvent(QEvent *e) override;
    virtual void resizeEvent(QResizeEvent *ev) Q_DECL_OVERRIDE;

public:
    void resetSize(const int maxWidth);

private:
    QScopedPointer<TipPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), Tip)
    QString m_strText;
    QDBusInterface* m_pWMDBus {nullptr};
    bool bIsWM {true};
};
}
#endif /* ifndef _DMR_TIP_H */
