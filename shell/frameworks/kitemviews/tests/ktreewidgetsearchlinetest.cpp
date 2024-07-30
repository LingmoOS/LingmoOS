#include "ktreewidgetsearchlinetest.h"

#include <QApplication>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QPushButton>
#include <QString>
#include <QTreeWidget>

#include <ktreewidgetsearchline.h>
#include <ktreewidgetsearchlinewidget.h>

KTreeWidgetSearchLineTest::KTreeWidgetSearchLineTest()
    : QDialog()
{
    // to test KWhatsThisManager too:
    setWhatsThis(QStringLiteral("This is a test dialog for KTreeWidgetSearchLineTest"));
    tw = new QTreeWidget(this);
    tw->setColumnCount(4);
    tw->setHeaderLabels(QStringList() << QStringLiteral("Item") << QStringLiteral("Price") << QStringLiteral("HIDDEN COLUMN") << QStringLiteral("Source"));
    tw->hideColumn(2);

    KTreeWidgetSearchLineWidget *searchWidget = new KTreeWidgetSearchLineWidget(this, tw);
    m_searchLine = searchWidget->searchLine();

    QTreeWidgetItem *red = new QTreeWidgetItem(tw, QStringList() << QStringLiteral("Red"));
    red->setWhatsThis(0, QStringLiteral("This item is red"));
    red->setWhatsThis(1, QStringLiteral("This item is pricy"));
    tw->expandItem(red);
    QTreeWidgetItem *blue = new QTreeWidgetItem(tw, QStringList() << QStringLiteral("Blue"));
    tw->expandItem(blue);
    QTreeWidgetItem *green = new QTreeWidgetItem(tw, QStringList() << QStringLiteral("Green"));
    tw->expandItem(green);
    QTreeWidgetItem *yellow = new QTreeWidgetItem(tw, QStringList() << QStringLiteral("Yellow"));
    tw->expandItem(yellow);

    create2ndLevel(red);
    create2ndLevel(blue);
    create2ndLevel(green);
    create2ndLevel(yellow);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QHBoxLayout *hbox = new QHBoxLayout();

    QPushButton *caseSensitive = new QPushButton(QStringLiteral("&Case Sensitive"), this);
    hbox->addWidget(caseSensitive);
    caseSensitive->setCheckable(true);
    connect(caseSensitive, SIGNAL(toggled(bool)), SLOT(switchCaseSensitivity(bool)));

    QPushButton *keepParentsVisible = new QPushButton(QStringLiteral("Keep &Parents Visible"), this);
    hbox->addWidget(keepParentsVisible);
    keepParentsVisible->setCheckable(true);
    keepParentsVisible->setChecked(true);
    connect(keepParentsVisible, SIGNAL(toggled(bool)), m_searchLine, SLOT(setKeepParentsVisible(bool)));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    layout->addWidget(searchWidget);
    layout->addWidget(tw);
    layout->addLayout(hbox);
    layout->addWidget(buttonBox);

    m_searchLine->setFocus();

    resize(350, 600);
}

void KTreeWidgetSearchLineTest::create3rdLevel(QTreeWidgetItem *item)
{
    new QTreeWidgetItem(item, QStringList() << QStringLiteral("Growing") << QStringLiteral("$2.00") << QString("") << QStringLiteral("Farmer"));
    new QTreeWidgetItem(item, QStringList() << QStringLiteral("Ripe") << QStringLiteral("$8.00") << QString("") << QStringLiteral("Market"));
    new QTreeWidgetItem(item, QStringList() << QStringLiteral("Decaying") << QStringLiteral("$0.50") << QString("") << QStringLiteral("Ground"));
    new QTreeWidgetItem(item, QStringList() << QStringLiteral("Pickled") << QStringLiteral("$4.00") << QString("") << QStringLiteral("Shop"));
}

void KTreeWidgetSearchLineTest::create2ndLevel(QTreeWidgetItem *item)
{
    QTreeWidgetItem *beans = new QTreeWidgetItem(item, QStringList() << QStringLiteral("Beans"));
    tw->expandItem(beans);
    create3rdLevel(beans);

    QTreeWidgetItem *grapes = new QTreeWidgetItem(item, QStringList() << QStringLiteral("Grapes"));
    tw->expandItem(grapes);
    create3rdLevel(grapes);

    QTreeWidgetItem *plums = new QTreeWidgetItem(item, QStringList() << QStringLiteral("Plums"));
    tw->expandItem(plums);
    create3rdLevel(plums);

    QTreeWidgetItem *bananas = new QTreeWidgetItem(item, QStringList() << QStringLiteral("Bananas"));
    tw->expandItem(bananas);
    create3rdLevel(bananas);
}

void KTreeWidgetSearchLineTest::switchCaseSensitivity(bool cs)
{
    m_searchLine->setCaseSensitivity(cs ? Qt::CaseSensitive : Qt::CaseInsensitive);
}

void KTreeWidgetSearchLineTest::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);

    for (int i = 0; i < tw->header()->count(); ++i) {
        if (!tw->header()->isSectionHidden(i)) {
            tw->resizeColumnToContents(i);
        }
    }
}

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("KTreeWidgetSearchLineTest"));
    QApplication app(argc, argv);
    KTreeWidgetSearchLineTest dialog;

    dialog.exec();

    return 0;
}

#include "moc_ktreewidgetsearchlinetest.cpp"
