#include "kcompletionuitest.h"

#include <QApplication>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QPushButton>

#include <khistorycombobox.h>
#include <klineedit.h>

Form1::Form1(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    resize(559, 465);
    setWindowTitle(QStringLiteral("Form1"));
    Form1Layout = new QVBoxLayout(this);

    GroupBox1 = new QGroupBox(this);
    GroupBox1->setLayout(new QVBoxLayout());
    GroupBox1->setTitle(QStringLiteral("Completion Test"));
    GroupBox1->layout()->setSpacing(0);
    GroupBox1->layout()->setContentsMargins(0, 0, 0, 0);
    GroupBox1Layout = new QVBoxLayout;
    GroupBox1Layout->setAlignment(Qt::AlignTop);
    GroupBox1->layout()->addItem(GroupBox1Layout);
    GroupBox1Layout->setParent(GroupBox1->layout());

    Layout9 = new QVBoxLayout;
    Layout9->setSpacing(6);
    Layout9->setContentsMargins(0, 0, 0, 0);

    Layout1 = new QHBoxLayout;
    Layout1->setSpacing(6);
    Layout1->setContentsMargins(0, 0, 0, 0);

    TextLabel1 = new QLabel(GroupBox1);
    TextLabel1->setObjectName(QStringLiteral("TextLabel1"));
    TextLabel1->setText(QStringLiteral("Completion"));
    Layout1->addWidget(TextLabel1);

    edit = new KLineEdit(GroupBox1);
    edit->setObjectName(QStringLiteral("edit"));
    Layout1->addWidget(edit);
    Layout9->addLayout(Layout1);
    edit->completionObject()->setItems(defaultItems());
    edit->completionObject()->setIgnoreCase(true);
    edit->setFocus();
    edit->setToolTip(QStringLiteral("right-click to change completion mode"));

    Layout2 = new QHBoxLayout;
    Layout2->setSpacing(6);
    Layout2->setContentsMargins(0, 0, 0, 0);

    combo = new KHistoryComboBox(GroupBox1);
    combo->setObjectName(QStringLiteral("history combo"));
    combo->setCompletionObject(edit->completionObject());
    // combo->setMaxCount( 5 );
    combo->setHistoryItems(defaultItems(), true);
    connect(combo, &QComboBox::textActivated, combo, &KHistoryComboBox::addToHistory);
    combo->setToolTip(QStringLiteral("KHistoryComboBox"));
    Layout2->addWidget(combo);

    LineEdit1 = new KLineEdit(GroupBox1);
    LineEdit1->setObjectName(QStringLiteral("LineEdit1"));
    Layout2->addWidget(LineEdit1);

    PushButton1 = new QPushButton(GroupBox1);
    PushButton1->setObjectName(QStringLiteral("PushButton1"));
    PushButton1->setText(QStringLiteral("Add"));
    connect(PushButton1, &QAbstractButton::clicked, this, &Form1::slotAdd);
    Layout2->addWidget(PushButton1);
    Layout9->addLayout(Layout2);

    Layout3 = new QHBoxLayout;
    Layout3->setSpacing(6);
    Layout3->setContentsMargins(0, 0, 0, 0);
    QSpacerItem *spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    Layout3->addItem(spacer);

    PushButton1_4 = new QPushButton(GroupBox1);
    PushButton1_4->setObjectName(QStringLiteral("PushButton1_4"));
    PushButton1_4->setText(QStringLiteral("Remove"));
    connect(PushButton1_4, &QAbstractButton::clicked, this, &Form1::slotRemove);
    Layout3->addWidget(PushButton1_4);
    Layout9->addLayout(Layout3);

    Layout8 = new QHBoxLayout;
    Layout8->setSpacing(6);
    Layout8->setContentsMargins(0, 0, 0, 0);

    ListBox1 = new QListWidget(GroupBox1);
    Layout8->addWidget(ListBox1);
    connect(ListBox1, &QListWidget::currentRowChanged, this, &Form1::slotHighlighted);
    ListBox1->setToolTip(QStringLiteral("Contains the contents of the completion object.\n:x is the weighting, i.e. how often an item has been inserted"));

    Layout7 = new QVBoxLayout;
    Layout7->setSpacing(6);
    Layout7->setContentsMargins(0, 0, 0, 0);

    PushButton1_3 = new QPushButton(GroupBox1);
    PushButton1_3->setObjectName(QStringLiteral("PushButton1_3"));
    PushButton1_3->setText(QStringLiteral("Completion items"));
    connect(PushButton1_3, &QAbstractButton::clicked, this, &Form1::slotList);
    Layout7->addWidget(PushButton1_3);

    PushButton1_2 = new QPushButton(GroupBox1);
    PushButton1_2->setObjectName(QStringLiteral("PushButton1_2"));
    PushButton1_2->setText(QStringLiteral("Clear"));
    connect(PushButton1_2, &QAbstractButton::clicked, edit->completionObject(), &KCompletion::clear);
    Layout7->addWidget(PushButton1_2);
    Layout8->addLayout(Layout7);
    Layout9->addLayout(Layout8);
    GroupBox1Layout->addLayout(Layout9);
    Form1Layout->addWidget(GroupBox1);

    slotList();
}

/*
 *  Destroys the object and frees any allocated resources
 */
Form1::~Form1()
{
    // no need to delete child widgets, Qt does it all for us
}

void Form1::slotAdd()
{
    qDebug("** adding: %s", LineEdit1->text().toLatin1().constData());
    edit->completionObject()->addItem(LineEdit1->text());

    QStringList matches = edit->completionObject()->allMatches(QStringLiteral("S"));
    QStringList::ConstIterator it = matches.constBegin();
    for (; it != matches.constEnd(); ++it) {
        qDebug("-- %s", (*it).toLatin1().constData());
    }
}

void Form1::slotRemove()
{
    edit->completionObject()->removeItem(LineEdit1->text());
}

void Form1::slotList()
{
    ListBox1->clear();
    QStringList items = edit->completionObject()->items();
    ListBox1->addItems(items);
}

void Form1::slotHighlighted(int row)
{
    if (row == -1) {
        return;
    }

    QListWidgetItem *i = ListBox1->item(row);
    Q_ASSERT(i != nullptr);

    QString text = i->text();

    // remove any "weighting"
    int index = text.lastIndexOf(QLatin1Char(':'));
    if (index > 0) {
        LineEdit1->setText(text.left(index));
    } else {
        LineEdit1->setText(text);
    }
}

QStringList Form1::defaultItems() const
{
    // clang-format off
    QStringList items;
    items << QStringLiteral("Super") << QStringLiteral("Sushi") << QStringLiteral("Samson") << QStringLiteral("Sucks") << QStringLiteral("Sumo") << QStringLiteral("Schumi");
    items << QStringLiteral("Slashdot") << QStringLiteral("sUpEr") << QStringLiteral("SUshi") << QStringLiteral("sUshi") << QStringLiteral("sUShi");
    items << QStringLiteral("sushI") << QStringLiteral("SushI");
    // clang-format on
    return items;
}

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("kcompletiontest"));

    QApplication app(argc, argv);

    Form1 *form = new Form1();
    form->show();

    return app.exec();
}

#include "moc_kcompletionuitest.cpp"
