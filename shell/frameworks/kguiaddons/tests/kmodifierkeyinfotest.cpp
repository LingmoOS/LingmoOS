/*
    SPDX-FileCopyrightText: 2009 Michael Leupold <lemma@confuego.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include <QApplication>
#include <QCheckBox>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QMap>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include "kmodifierkeyinfo.h"

template<typename A, typename B, typename C>
class Triple
{
public:
    Triple()
    {
    }
    Triple(const A _first, const B _second, const C _third)
        : first(_first)
        , second(_second)
        , third(_third){};
    A first;
    B second;
    C third;
};

class TestWidget : public QWidget
{
    Q_OBJECT

public:
    TestWidget();

protected Q_SLOTS:
    void keyPressed(Qt::Key key, bool state);
    void keyLatched(Qt::Key key, bool state);
    void keyLocked(Qt::Key key, bool state);
    void mouseButtonPressed(Qt::MouseButton button, bool state);
    void latch();
    void lock();
    void keyAdded(Qt::Key)
    {
        qDebug() << "Key added";
    }
    void keyRemoved(Qt::Key)
    {
        qDebug() << "Key removed";
    }

private:
    KModifierKeyInfo m_lock;
    QMap<Qt::Key, Triple<QCheckBox *, QCheckBox *, QCheckBox *>> m_leds;
    QMap<Qt::MouseButton, QCheckBox *> m_mouseLeds;
};

TestWidget::TestWidget()
    : QWidget(nullptr)
    , m_lock(this)
{
    QMap<Qt::Key, QString> mods;
    mods.insert(Qt::Key_Shift, QStringLiteral("Shift"));
    mods.insert(Qt::Key_Control, QStringLiteral("Ctrl"));
    mods.insert(Qt::Key_Alt, QStringLiteral("Alt"));
    mods.insert(Qt::Key_Meta, QStringLiteral("Meta"));
    mods.insert(Qt::Key_Super_L, QStringLiteral("Super"));
    mods.insert(Qt::Key_Hyper_L, QStringLiteral("Hyper"));
    mods.insert(Qt::Key_AltGr, QStringLiteral("AltGr"));
    mods.insert(Qt::Key_NumLock, QStringLiteral("NumLock"));
    mods.insert(Qt::Key_CapsLock, QStringLiteral("CapsLock"));
    mods.insert(Qt::Key_ScrollLock, QStringLiteral("ScrollLock"));

    QMap<Qt::MouseButton, QString> buttons;
    buttons.insert(Qt::LeftButton, QStringLiteral("Left Button"));
    buttons.insert(Qt::RightButton, QStringLiteral("Right Button"));
    buttons.insert(Qt::MiddleButton, QStringLiteral("Middle Button"));
    buttons.insert(Qt::XButton1, QStringLiteral("First X Button"));
    buttons.insert(Qt::XButton2, QStringLiteral("Second X Button"));

    QVBoxLayout *layout = new QVBoxLayout(this);

    QMap<Qt::Key, QString>::const_iterator it;
    QMap<Qt::Key, QString>::const_iterator end = mods.constEnd();
    for (it = mods.constBegin(); it != end; ++it) {
        if (m_lock.knowsKey(it.key())) {
            QHBoxLayout *hlayout = new QHBoxLayout;
            QCheckBox *pressed = new QCheckBox(this);
            QCheckBox *latched = new QCheckBox(this);
            QCheckBox *locked = new QCheckBox(this);
            QPushButton *latch = new QPushButton(QStringLiteral("latch"), this);
            latch->setProperty("modifier", it.key());
            connect(latch, &QAbstractButton::clicked, this, &TestWidget::latch);
            QPushButton *lock = new QPushButton(QStringLiteral("lock"), this);
            lock->setProperty("modifier", it.key());
            connect(lock, &QAbstractButton::clicked, this, &TestWidget::lock);
            pressed->setChecked(m_lock.isKeyPressed(it.key()));
            latched->setChecked(m_lock.isKeyLatched(it.key()));
            locked->setChecked(m_lock.isKeyLocked(it.key()));
            m_leds.insert(it.key(), Triple<QCheckBox *, QCheckBox *, QCheckBox *>(pressed, latched, locked));
            hlayout->addWidget(pressed);
            hlayout->addWidget(latched);
            hlayout->addWidget(locked);
            hlayout->addWidget(new QLabel(it.value()));
            hlayout->addWidget(latch);
            hlayout->addWidget(lock);
            layout->addLayout(hlayout);
        }
    }

    QMap<Qt::MouseButton, QString>::const_iterator it2;
    QMap<Qt::MouseButton, QString>::const_iterator end2 = buttons.constEnd();
    for (it2 = buttons.constBegin(); it2 != end2; ++it2) {
        QHBoxLayout *hlayout = new QHBoxLayout;
        QCheckBox *pressed = new QCheckBox(this);
        pressed->setChecked(m_lock.isButtonPressed(it2.key()));
        m_mouseLeds.insert(it2.key(), pressed);
        hlayout->addWidget(pressed);
        hlayout->addWidget(new QLabel(it2.value()));
        layout->addLayout(hlayout);
    }

    connect(&m_lock, &KModifierKeyInfo::keyPressed, this, &TestWidget::keyPressed);
    connect(&m_lock, &KModifierKeyInfo::keyLatched, this, &TestWidget::keyLatched);
    connect(&m_lock, &KModifierKeyInfo::keyLocked, this, &TestWidget::keyLocked);
    connect(&m_lock, &KModifierKeyInfo::buttonPressed, this, &TestWidget::mouseButtonPressed);
    connect(&m_lock, &KModifierKeyInfo::keyAdded, this, &TestWidget::keyAdded);
    connect(&m_lock, &KModifierKeyInfo::keyRemoved, this, &TestWidget::keyRemoved);
}

void TestWidget::keyPressed(Qt::Key key, bool pressed)
{
    if (m_leds.contains(key)) {
        m_leds[key].first->setChecked(pressed);
    }
}

void TestWidget::keyLatched(Qt::Key key, bool latched)
{
    if (m_leds.contains(key)) {
        m_leds[key].second->setChecked(latched);
    }
}

void TestWidget::keyLocked(Qt::Key key, bool locked)
{
    if (m_leds.contains(key)) {
        m_leds[key].third->setChecked(locked);
    }
}

void TestWidget::mouseButtonPressed(Qt::MouseButton button, bool pressed)
{
    if (m_mouseLeds.contains(button)) {
        m_mouseLeds[button]->setChecked(pressed);
    }
}

void TestWidget::latch()
{
    Qt::Key key = (Qt::Key)sender()->property("modifier").toInt();
    m_lock.setKeyLatched(key, !m_lock.isKeyLatched(key));
}

void TestWidget::lock()
{
    Qt::Key key = (Qt::Key)sender()->property("modifier").toInt();
    m_lock.setKeyLocked(key, !m_lock.isKeyLocked(key));
}

int main(int argc, char *argv[])
{
    QApplication::setApplicationName(QStringLiteral("simple"));

    QApplication app(argc, argv);
    TestWidget mainWidget;
    mainWidget.show();

    return app.exec();
}

#include "kmodifierkeyinfotest.moc"
