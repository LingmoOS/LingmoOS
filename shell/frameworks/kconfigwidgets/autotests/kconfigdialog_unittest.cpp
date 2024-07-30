/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2012 Albert Astals Cid <aacid@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QTest>
#include <QTestEvent>

#include <QComboBox>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalSpy>
#include <QSpinBox>

#include <KColorCombo>
#include <KConfigSkeleton>
#include <kconfigdialog.h>
#include <kconfigdialogmanager.h>

#include "signaltest.h"

static const auto CONFIG_FILE = QStringLiteral("kconfigdialog_unittestrc");

class TextEditUserPropertyWidget : public QWidget
{
    Q_OBJECT
    // with USER parameter
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged USER true)
public:
    TextEditUserPropertyWidget(QWidget *parent = nullptr)
        : QWidget(parent)
    {
    }
    void setText(const QString &text)
    {
        m_text = text;
        Q_EMIT textChanged(m_text);
    }
    QString text() const
    {
        return m_text;
    }
Q_SIGNALS:
    void textChanged(const QString &text);

private:
    QString m_text;
};

class TextEditNoUserPropertyWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QString other READ other WRITE setOther NOTIFY otherChanged USER true)
public:
    TextEditNoUserPropertyWidget(QWidget *parent = nullptr)
        : QWidget(parent)
    {
    }
    void setText(const QString &text)
    {
        m_text = text;
        Q_EMIT textChanged(m_text);
    }
    QString text() const
    {
        return m_text;
    }
    void setOther(const QString &other)
    {
        m_other = other;
        Q_EMIT textChanged(m_other);
    }
    QString other() const
    {
        return m_other;
    }
Q_SIGNALS:
    void textChanged(const QString &text);
    void otherChanged(const QString &other);

private:
    QString m_text;
    QString m_other;
};

class TextEditNoUserPropertyNoNotifyWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QString other READ other WRITE setOther NOTIFY otherChanged USER true)
public:
    TextEditNoUserPropertyNoNotifyWidget(QWidget *parent = nullptr)
        : QWidget(parent)
    {
    }
    void setText(const QString &text)
    {
        m_text = text;
        Q_EMIT textChanged(m_text);
    }
    QString text() const
    {
        return m_text;
    }
    void setOther(const QString &other)
    {
        m_other = other;
        Q_EMIT textChanged(m_other);
    }
    QString other() const
    {
        return m_other;
    }
Q_SIGNALS:
    void textChanged(const QString &text);
    void otherChanged(const QString &other);

private:
    QString m_text;
    QString m_other;
};

class ComboBoxPage : public QWidget
{
public:
    ComboBoxPage()
    {
        colorCombo = new KColorCombo(this);
        colorCombo->setObjectName(QStringLiteral("kcfg_Color"));
        colorCombo->setColor(Qt::red);

        enumCombo = new QComboBox(this);
        enumCombo->setObjectName(QStringLiteral("kcfg_Enum"));
        enumCombo->addItems(QStringList() << QStringLiteral("A") << QStringLiteral("B") << QStringLiteral("C"));

        textCombo = new QComboBox(this);
        textCombo->setObjectName(QStringLiteral("kcfg_Text"));
        textCombo->setEditable(true);
        textCombo->addItems(QStringList() << QStringLiteral("A") << QStringLiteral("B") << QStringLiteral("C"));

        numInput = new QSpinBox(this);
        numInput->setValue(1);
        numInput->setObjectName(QStringLiteral("kcfg_IntNumInput"));
    }

    KColorCombo *colorCombo;
    QComboBox *enumCombo;
    QComboBox *textCombo;
    QSpinBox *numInput;
};

class ComboSettings : public KConfigSkeleton
{
public:
    ComboSettings()
        : KConfigSkeleton(CONFIG_FILE)
    {
        colorItem = new ItemColor(currentGroup(), QStringLiteral("Color"), color, Qt::white);
        addItem(colorItem, QStringLiteral("Color"));

        QList<ItemEnum::Choice> textValues;
        {
            ItemEnum::Choice choice;
            choice.name = QStringLiteral("A");
            textValues.append(choice);
        }
        {
            ItemEnum::Choice choice;
            choice.name = QStringLiteral("B");
            textValues.append(choice);
        }
        {
            ItemEnum::Choice choice;
            choice.name = QStringLiteral("C");
            textValues.append(choice);
        }
        enumItem = new ItemEnum(currentGroup(), QStringLiteral("Enum"), enumIndex, textValues, 1);
        addItem(enumItem, QStringLiteral("Enum"));

        stringItem = new ItemString(currentGroup(), QStringLiteral("Text"), string, QStringLiteral("hh:mm"));
        addItem(stringItem, QStringLiteral("Text"));

        intValueItem = new ItemInt(currentGroup(), QStringLiteral("IntNumInput"), intValue, 42);
        addItem(intValueItem, QStringLiteral("IntNumInput"));
    }

    ItemColor *colorItem;
    QColor color;

    ItemEnum *enumItem;
    int enumIndex;

    ItemString *stringItem;
    QString string;

    ItemInt *intValueItem;
    int intValue;
};

class KConfigDialog_UnitTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
        // Leftover configuration breaks combosTest
        const QString configFile = QStandardPaths::locate(QStandardPaths::GenericConfigLocation, CONFIG_FILE);
        if (!configFile.isEmpty()) {
            if (!QFile::remove(configFile)) {
                qWarning() << "Could not remove old config file:" << configFile;
            }
        }
    }

    void test()
    {
        ComboSettings *skeleton = new ComboSettings();
        KConfigDialog *dialog = new KConfigDialog(nullptr, QStringLiteral("settings"), skeleton);
        ComboBoxPage *page = new ComboBoxPage();

        QCOMPARE(page->colorCombo->color().name(), QColor(Qt::red).name());
        QCOMPARE(page->enumCombo->currentIndex(), 0);
        QCOMPARE(page->textCombo->currentText(), QString("A"));
        QCOMPARE(page->numInput->value(), 1);

        dialog->addPage(page, QStringLiteral("General"));

        QCOMPARE(page->colorCombo->color().name(), QColor(Qt::white).name());
        QCOMPARE(page->enumCombo->currentIndex(), 1);
        QCOMPARE(page->textCombo->currentText(), QLatin1String("hh:mm"));
        QCOMPARE(page->numInput->value(), 42);

        page->colorCombo->setColor(Qt::blue);
        page->enumCombo->setCurrentIndex(2);
        page->textCombo->setCurrentIndex(2);
        page->numInput->setValue(2);

        QDialogButtonBox *buttonBox = dialog->findChild<QDialogButtonBox *>();
        QVERIFY(buttonBox != nullptr);
        buttonBox->button(QDialogButtonBox::Apply)->click();
        QCOMPARE(skeleton->colorItem->property().value<QColor>().name(), QColor(Qt::blue).name());
        QCOMPARE(skeleton->enumItem->property().toInt(), 2);
        QCOMPARE(skeleton->stringItem->property().toString(), QLatin1String("C"));
        QCOMPARE(skeleton->intValueItem->property().toInt(), 2);

        delete dialog;
        delete skeleton;
    }

    void testKConfigCompilerSignalsKnownWidget()
    {
        QLineEdit *edit = new QLineEdit;

        testKConfigCompilerSignals<QLineEdit>(edit, QStringLiteral("settings2"));
    }

    void testKConfigCompilerSignalsWithUserProperty()
    {
        // make sure there is nothing registered for the property
        KConfigDialogManager::propertyMap()->remove("TextEditUserPropertyWidget");

        TextEditUserPropertyWidget *edit = new TextEditUserPropertyWidget;

        testKConfigCompilerSignals<TextEditUserPropertyWidget>(edit, QStringLiteral("settings3"));
    }

    void testKConfigCompilerSignalsWithoutUserPropertyByMap()
    {
        KConfigDialogManager::propertyMap()->insert("TextEditNoUserPropertyWidget", QByteArray("text"));

        TextEditNoUserPropertyWidget *edit = new TextEditNoUserPropertyWidget;

        testKConfigCompilerSignals<TextEditNoUserPropertyWidget>(edit, QStringLiteral("settings4"));
    }

    void testKConfigCompilerSignalsWithoutUserPropertyByProperty()
    {
        // make sure there is nothing registered for the property
        KConfigDialogManager::propertyMap()->remove("TextEditNoUserPropertyWidget");

        TextEditNoUserPropertyWidget *edit = new TextEditNoUserPropertyWidget;
        edit->setProperty("kcfg_property", QByteArray("text"));

        testKConfigCompilerSignals<TextEditNoUserPropertyWidget>(edit, QStringLiteral("settings5"));
    }

    void testKConfigCompilerSignalsWithUserPropertyAutoSignal()
    {
        // make sure there is nothing registered
        KConfigDialogManager::propertyMap()->remove("TextEditUserPropertyWidget");

        TextEditUserPropertyWidget *edit = new TextEditUserPropertyWidget;

        testKConfigCompilerSignals<TextEditUserPropertyWidget>(edit, QStringLiteral("settings6"));
    }

    void testKConfigCompilerSignalsWithoutUserPropertyByMapAutoSignal()
    {
        KConfigDialogManager::propertyMap()->insert("TextEditNoUserPropertyWidget", QByteArray("text"));

        TextEditNoUserPropertyWidget *edit = new TextEditNoUserPropertyWidget;

        testKConfigCompilerSignals<TextEditNoUserPropertyWidget>(edit, QStringLiteral("settings7"));
    }

    void testKConfigCompilerSignalsWithoutUserPropertyByPropertyAutoSignal()
    {
        // next to USER on "other" property, this one should also be ignored
        KConfigDialogManager::propertyMap()->insert("TextEditNoUserPropertyWidget", QByteArray("other"));

        TextEditNoUserPropertyWidget *edit = new TextEditNoUserPropertyWidget;
        edit->setProperty("kcfg_property", QByteArray("text"));

        testKConfigCompilerSignals<TextEditNoUserPropertyWidget>(edit, QStringLiteral("settings8"));
    }

    void testKConfigCompilerSignalsWithoutUserPropertyByPropertyBySignal()
    {
        // next to USER being on "other" property, this one should also be ignored
        KConfigDialogManager::propertyMap()->insert("TextEditNoUserPropertyNoNotifyWidget", QByteArray("other"));

        TextEditNoUserPropertyNoNotifyWidget *edit = new TextEditNoUserPropertyNoNotifyWidget;
        edit->setProperty("kcfg_property", QByteArray("text"));
        edit->setProperty("kcfg_propertyNotify", SIGNAL(textChanged(QString)));

        testKConfigCompilerSignals<TextEditNoUserPropertyNoNotifyWidget>(edit, QStringLiteral("settings9"));
    }

private:
    template<class T>
    void testKConfigCompilerSignals(T *edit, const QString &configDialogTitle)
    {
        const QString defaultValue = QStringLiteral("default value");
        const QString changedValue = QStringLiteral("changed value");
        const QString someOtherValue = QStringLiteral("some other value");
        // set to default to ensure no old stored values make things fail
        SignalTest::self()->setDefaults();
        KConfigDialog *dialog = new KConfigDialog(nullptr, configDialogTitle, SignalTest::self());
        QWidget *page = new QWidget;
        edit->setParent(page);
        edit->setObjectName(QStringLiteral("kcfg_foo"));
        edit->setText(QStringLiteral("some text"));

        QSignalSpy spy(SignalTest::self(), &SignalTest::fooChanged);
        QVERIFY(spy.isValid());
        // now all the magic happens
        dialog->addPage(page, QStringLiteral("General"));

        // check that default value gets loaded
        QCOMPARE(spy.size(), 0);
        QCOMPARE(edit->text(), defaultValue);
        QCOMPARE(SignalTest::foo(), defaultValue);

        edit->setText(changedValue);
        // change signal should not be emitted immediately (only on save)
        QCOMPARE(spy.size(), 0);
        QCOMPARE(SignalTest::foo(), defaultValue); // should be no change to skeleton

        QDialogButtonBox *buttonBox = dialog->findChild<QDialogButtonBox *>();
        QVERIFY(buttonBox != nullptr);
        buttonBox->button(QDialogButtonBox::Apply)->click(); // now signal should be emitted

        QCOMPARE(spy.size(), 1);
        QVariantList args = spy.last();
        QCOMPARE(args.size(), 1);
        QCOMPARE((QMetaType::Type)args[0].userType(), QMetaType::QString);
        QCOMPARE(args[0].toString(), changedValue);
        QCOMPARE(SignalTest::foo(), changedValue);

        // change it to a different value
        edit->setText(someOtherValue);
        QCOMPARE(spy.size(), 1);
        buttonBox->button(QDialogButtonBox::Apply)->click(); // now signal should be emitted

        QCOMPARE(spy.size(), 2);
        args = spy.last();
        QCOMPARE(args.size(), 1);
        QCOMPARE((QMetaType::Type)args[0].userType(), QMetaType::QString);
        QCOMPARE(args[0].toString(), someOtherValue);
        QCOMPARE(SignalTest::foo(), someOtherValue);
    }
};

QTEST_MAIN(KConfigDialog_UnitTest)

#include "kconfigdialog_unittest.moc"
