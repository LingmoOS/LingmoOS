#include "kcomboboxtest.h"

#include <assert.h>

#include <KConfig>
#include <KConfigGroup>

#include <QApplication>
#include <QBoxLayout>
#include <QDebug>
#include <QLabel>
#include <QLayout>
#include <QPixmap>
#include <QPushButton>
#include <QTimer>

#include <khistorycombobox.h>

KComboBoxTest::KComboBoxTest(QWidget *widget)
    : QWidget(widget)
{
    QVBoxLayout *vbox = new QVBoxLayout(this);

    // Qt combobox
    QHBoxLayout *hbox = new QHBoxLayout();
    QLabel *lbl = new QLabel(QStringLiteral("&QCombobox:"), this);
    lbl->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    hbox->addWidget(lbl);

    m_qc = new QComboBox(this);
    m_qc->setObjectName(QStringLiteral("QtReadOnlyCombo"));
    lbl->setBuddy(m_qc);
    connectComboSignals(m_qc);
    hbox->addWidget(m_qc);

    vbox->addLayout(hbox);

    // Read-only combobox
    hbox = new QHBoxLayout();
    lbl = new QLabel(QStringLiteral("&Read-Only Combo:"), this);
    lbl->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    hbox->addWidget(lbl);

    m_ro = new KComboBox(this);
    m_ro->setObjectName(QStringLiteral("ReadOnlyCombo"));
    lbl->setBuddy(m_ro);
    m_ro->setCompletionMode(KCompletion::CompletionAuto);
    connectComboSignals(m_ro);
    hbox->addWidget(m_ro);

    vbox->addLayout(hbox);

    // Read-write combobox
    hbox = new QHBoxLayout();
    lbl = new QLabel(QStringLiteral("&Editable Combo:"), this);
    lbl->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    hbox->addWidget(lbl);

    m_rw = new KComboBox(true, this);
    m_rw->setObjectName(QStringLiteral("ReadWriteCombo"));
    lbl->setBuddy(m_rw);
    m_rw->setDuplicatesEnabled(true);
    m_rw->setInsertPolicy(QComboBox::NoInsert);
    m_rw->setTrapReturnKey(true);
    connectComboSignals(m_rw);
    hbox->addWidget(m_rw);
    vbox->addLayout(hbox);

    // History combobox...
    hbox = new QHBoxLayout();
    lbl = new QLabel(QStringLiteral("&History Combo:"), this);
    lbl->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    hbox->addWidget(lbl);

    m_hc = new KHistoryComboBox(this);
    m_hc->setObjectName(QStringLiteral("HistoryCombo"));
    lbl->setBuddy(m_hc);
    m_hc->setDuplicatesEnabled(true);
    m_hc->setInsertPolicy(QComboBox::NoInsert);
    connectComboSignals(m_hc);
    m_hc->setTrapReturnKey(true);
    hbox->addWidget(m_hc);
    vbox->addLayout(hbox);

    // Read-write combobox that is a replica of code in konqueror...
    hbox = new QHBoxLayout();
    lbl = new QLabel(QStringLiteral("&Konq's Combo:"), this);
    lbl->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    hbox->addWidget(lbl);

    m_konqc = new KComboBox(true, this);
    m_konqc->setObjectName(QStringLiteral("KonqyCombo"));
    lbl->setBuddy(m_konqc);
    m_konqc->setMaxCount(10);
    connectComboSignals(m_konqc);
    hbox->addWidget(m_konqc);
    vbox->addLayout(hbox);

    // Create an exit button
    hbox = new QHBoxLayout();
    m_btnExit = new QPushButton(QStringLiteral("E&xit"), this);
    QObject::connect(m_btnExit, &QAbstractButton::clicked, this, &KComboBoxTest::quitApp);
    hbox->addWidget(m_btnExit);

    // Create a disable button...
    m_btnEnable = new QPushButton(QStringLiteral("Disa&ble"), this);
    QObject::connect(m_btnEnable, &QAbstractButton::clicked, this, &KComboBoxTest::slotDisable);
    hbox->addWidget(m_btnEnable);

    vbox->addLayout(hbox);

    // Populate the select-only list box
    QStringList list;
    list << QStringLiteral("Stone") << QStringLiteral("Tree") << QStringLiteral("Peables") << QStringLiteral("Ocean") << QStringLiteral("Sand")
         << QStringLiteral("Chips") << QStringLiteral("Computer") << QStringLiteral("Mankind");
    list.sort();

    // Setup the qcombobox
    m_qc->addItems(list);

    // Setup read-only combo
    m_ro->addItems(list);
    m_ro->completionObject()->setItems(list);

    // Setup read-write combo
    m_rw->addItems(list);
    m_rw->completionObject()->setItems(list);

    // Setup history combo
    m_hc->addItems(list);
    m_hc->completionObject()->setItems(list + QStringList() << QStringLiteral("One") << QStringLiteral("Two") << QStringLiteral("Three"));

    // Setup konq's combobox
    KConfig historyConfig(QStringLiteral("konq_history"), KConfig::SimpleConfig);
    KConfigGroup cg(&historyConfig, QStringLiteral("Location Bar"));
    KCompletion *s_pCompletion = new KCompletion;
    s_pCompletion->setOrder(KCompletion::Weighted);
    s_pCompletion->setItems(cg.readEntry("ComboContents", QStringList()));
    s_pCompletion->setCompletionMode(KCompletion::CompletionPopup);
    m_konqc->setCompletionObject(s_pCompletion);

    QPixmap pix(16, 16);
    pix.fill(Qt::blue);
    m_konqc->addItem(pix, QStringLiteral("http://www.kde.org"));
    m_konqc->setCurrentIndex(m_konqc->count() - 1);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &KComboBoxTest::slotTimeout);
}

KComboBoxTest::~KComboBoxTest()
{
    delete m_timer;
    m_timer = nullptr;
}

void KComboBoxTest::connectComboSignals(QComboBox *combo)
{
    QObject::connect(combo, qOverload<int>(&QComboBox::activated), this, &KComboBoxTest::slotActivated);
    QObject::connect(combo, &QComboBox::textActivated, this, &KComboBoxTest::slotTextActivated);

    QObject::connect(combo, qOverload<int>(&QComboBox::currentIndexChanged), this, &KComboBoxTest::slotCurrentIndexChanged);
    QObject::connect(combo, qOverload<int>(&QComboBox::currentIndexChanged), this, [this, combo](const int index) {
        slotCurrentTextChanged(combo->itemText(index));
    });

    QObject::connect(combo, &QComboBox::textActivated, this, qOverload<const QString &>(&KComboBoxTest::slotReturnPressed));
}

void KComboBoxTest::slotDisable()
{
    if (m_timer->isActive()) {
        return;
    }

    m_btnEnable->setEnabled(!m_btnEnable->isEnabled());

    m_timer->setSingleShot(true);
    m_timer->start(5000);
}

void KComboBoxTest::slotTimeout()
{
    bool enabled = m_ro->isEnabled();

    if (enabled) {
        m_btnEnable->setText(QStringLiteral("Ena&ble"));
    } else {
        m_btnEnable->setText(QStringLiteral("Disa&ble"));
    }

    m_qc->setEnabled(!enabled);
    m_ro->setEnabled(!enabled);
    m_rw->setEnabled(!enabled);
    m_hc->setEnabled(!enabled);
    m_konqc->setEnabled(!enabled);

    m_btnEnable->setEnabled(!m_btnEnable->isEnabled());
}

void KComboBoxTest::slotCurrentIndexChanged(int index)
{
    qDebug() << qPrintable(sender()->objectName()) << ", index:" << index;
}

void KComboBoxTest::slotCurrentTextChanged(const QString &item)
{
    qDebug() << qPrintable(sender()->objectName()) << ", item:" << item;
}

void KComboBoxTest::slotActivated(int index)
{
    qDebug() << "Activated Combo:" << qPrintable(sender()->objectName()) << ", index:" << index;
}

void KComboBoxTest::slotTextActivated(const QString &item)
{
    qDebug() << "Activated Combo:" << qPrintable(sender()->objectName()) << ", item:" << item;
}

void KComboBoxTest::slotReturnPressed(const QString &item)
{
    qDebug() << "Return Pressed:" << qPrintable(sender()->objectName()) << ", value =" << item;
}

void KComboBoxTest::quitApp()
{
    qApp->closeAllWindows();
}

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("kcomboboxtest"));

    QApplication a(argc, argv);

    KComboBoxTest *t = new KComboBoxTest;
    t->show();
    return a.exec();
}

#include "moc_kcomboboxtest.cpp"
