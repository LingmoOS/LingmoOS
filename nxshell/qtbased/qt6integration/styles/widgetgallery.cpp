// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#include <QtWidgets>
#include <QTreeView>
#include <QFileSystemModel>

#include "norwegianwoodstyle.h"
#include "widgetgallery.h"

inline static void setUnifiedMargin(QLayout* layout, int margin)
{
    layout->setContentsMargins(margin, margin, margin, margin);
}

//! [0]
WidgetGallery::WidgetGallery(QWidget *parent)
    : QMainWindow(parent)
{
    originalPalette = QApplication::palette();

    styleComboBox = new QComboBox;
    styleComboBox->addItem("NorwegianWood");
    styleComboBox->addItems(QStyleFactory::keys());
    styleComboBox->setToolTip("Hello");
    styleComboBox->setCurrentIndex(0);

    styleLabel = new QLabel(tr("&Style:"));
    styleLabel->setBuddy(styleComboBox);

    useStylePaletteCheckBox = new QCheckBox(tr("&Use style's standard palette"));
    useStylePaletteCheckBox->setChecked(false);

    disableWidgetsCheckBox = new QCheckBox(tr("&Disable widgets"));

    changeStyle(0);

    createTopLeftGroupBox();
    createTopRightGroupBox();
    createBottomLeftTabWidget();
    createBottomRightGroupBox();
    createProgressBar();
//! [0]

//! [1]
    connect(styleComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, static_cast<void (WidgetGallery::*)(int)>(&WidgetGallery::changeStyle));
//! [1] //! [2]
    connect(useStylePaletteCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(changePalette()));
    connect(disableWidgetsCheckBox, SIGNAL(toggled(bool)),
            topLeftGroupBox, SLOT(setDisabled(bool)));
    connect(disableWidgetsCheckBox, SIGNAL(toggled(bool)),
            topRightGroupBox, SLOT(setDisabled(bool)));
    connect(disableWidgetsCheckBox, SIGNAL(toggled(bool)),
            bottomLeftTabWidget, SLOT(setDisabled(bool)));
    connect(disableWidgetsCheckBox, SIGNAL(toggled(bool)),
            bottomRightGroupBox, SLOT(setDisabled(bool)));
//! [2]

//! [3]
    QWidget *mainWidget = new QWidget();
    QHBoxLayout *topLayout = new QHBoxLayout;
//! [3] //! [4]
    topLayout->addWidget(styleLabel);
    topLayout->addWidget(styleComboBox);
    topLayout->addStretch(1);
    topLayout->addWidget(useStylePaletteCheckBox);
    topLayout->addWidget(disableWidgetsCheckBox);

    QGridLayout *mainLayout = new QGridLayout(mainWidget);
    mainLayout->addLayout(topLayout, 0, 0, 1, 2);
    mainLayout->addWidget(topLeftGroupBox, 1, 0);
    mainLayout->addWidget(topRightGroupBox, 1, 1);
    mainLayout->addWidget(bottomLeftTabWidget, 2, 0);
    mainLayout->addWidget(bottomRightGroupBox, 2, 1);
    mainLayout->addWidget(progressBar, 3, 0, 1, 2);
    mainLayout->setRowStretch(1, 1);
    mainLayout->setRowStretch(2, 1);
    mainLayout->setColumnStretch(0, 1);
    mainLayout->setColumnStretch(1, 1);

    setWindowTitle(tr("Styles"));

    setCentralWidget(mainWidget);

    QMenu *firstLayer = new QMenu("first");
    menuBar()->addAction("chameleon")->setMenu(firstLayer);
    firstLayer->addAction("menu 1")->setIcon(QIcon::fromTheme("document-open"));
    firstLayer->addAction("menu 2")->setEnabled(false);
    firstLayer->addSection("Section");
    firstLayer->addAction("menu 3")->setCheckable(true);
    firstLayer->addSeparator();

    QMenu *secondLayer = new QMenu("second");
    firstLayer->addMenu(secondLayer);
    QActionGroup *group = new QActionGroup(this);
    group->setExclusive(true);
    QAction *sa1 = new QAction("submenu 1");
    sa1->setCheckable(true);
    group->addAction(sa1);
    QAction *sa2 = new QAction("submenu 2");
    sa2->setCheckable(true);
    sa2->setIcon(QIcon::fromTheme("application-exit"));
    group->addAction(sa2);
    QAction *sa3 = new QAction("submenu 3");
    sa3->setCheckable(true);
    sa3->setShortcut(QKeySequence::New);
    group->addAction(sa3);
    secondLayer->addActions(group->actions());
    secondLayer->setLayoutDirection(Qt::RightToLeft);

    menuBar()->addAction("dlight");
    menuBar()->addAction("ddark");
    menuBar()->addAction("dsemidark");
    menuBar()->addAction("dsemilight");

    connect(menuBar(), &QMenuBar::triggered, this, [this](const QAction * action) {
        changeStyle(action->text());
    });
}
//! [4]

//! [5]
void WidgetGallery::changeStyle(int index)
//! [5] //! [6]
{
    changeStyle(styleComboBox->itemText(index));
}
//! [6]

void WidgetGallery::changeStyle(const QString &styleName)
{
    if (styleName == "NorwegianWood") {
        QApplication::setStyle(new NorwegianWoodStyle);
    } else {
        QApplication::setStyle(QStyleFactory::create(styleName));
    }
    changePalette();
}

//! [7]
void WidgetGallery::changePalette()
//! [7] //! [8]
{
    if (useStylePaletteCheckBox->isChecked()) {
        QApplication::setPalette(QApplication::style()->standardPalette());
     } else {
//        QApplication::setPalette(originalPalette);
        QApplication::setAttribute(Qt::AA_SetPalette, false);
    }
}
//! [8]

//! [9]
void WidgetGallery::advanceProgressBar()
//! [9] //! [10]
{
    int curVal = progressBar->value();
    int maxVal = progressBar->maximum();
    progressBar->setValue(curVal + (maxVal - curVal) / 100);
}
//! [10]

//! [11]
void WidgetGallery::createTopLeftGroupBox()
//! [11] //! [12]
{
    topLeftGroupBox = new QGroupBox(tr("Group 1"));

    radioButton1 = new QRadioButton(tr("Radio button 1"));
    radioButton2 = new QRadioButton(tr("Radio button 2"));
    radioButton3 = new QRadioButton(tr("Radio button 3"));
    radioButton1->setChecked(true);

    checkBox = new QCheckBox(tr("Tri-state check box"));
    checkBox->setTristate(true);
    checkBox->setCheckState(Qt::PartiallyChecked);

    connect(radioButton1, &QRadioButton::clicked, this, [&] {
        bottomLeftTabWidget->setDocumentMode(!bottomLeftTabWidget->documentMode());
    });
    connect(radioButton2, &QRadioButton::clicked, this, [&] {
        bottomLeftTabWidget->setTabShape(QTabWidget::Rounded);
        bottomLeftTabWidget->update();
    });
    connect(radioButton3, &QRadioButton::clicked, this, [&] {
        bottomLeftTabWidget->setTabShape(QTabWidget::Triangular);
        bottomLeftTabWidget->update();
    });

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(radioButton1);
    layout->addWidget(radioButton2);
    layout->addWidget(radioButton3);
    layout->addWidget(checkBox);
    layout->addStretch(1);
    topLeftGroupBox->setLayout(layout);
}
//! [12]

void WidgetGallery::createTopRightGroupBox()
{
    topRightGroupBox = new QGroupBox(tr("Group 2"));

    defaultPushButton = new QPushButton(tr("Default Push Button"));
    defaultPushButton->setDefault(true);

    togglePushButton = new QPushButton(tr("Toggle Push Button"));
    togglePushButton->setCheckable(true);
    togglePushButton->setChecked(true);

    flatPushButton = new QPushButton(tr("Flat Push Button"));
    flatPushButton->setFlat(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(defaultPushButton);
    layout->addWidget(togglePushButton);
    layout->addWidget(flatPushButton);
    layout->addStretch(1);
    topRightGroupBox->setLayout(layout);
}

void WidgetGallery::createBottomLeftTabWidget()
{
    bottomLeftTabWidget = new QTabWidget;
    bottomLeftTabWidget->setSizePolicy(QSizePolicy::Preferred,
                                       QSizePolicy::Ignored);

    bottomLeftTabWidget->setTabsClosable(true);
    bottomLeftTabWidget->setTabShape(QTabWidget::Triangular);

    QWidget *tab1 = new QWidget;
    tableWidget = new QTableWidget(10, 10);
    tableWidget->setAlternatingRowColors(true);

    QHBoxLayout *tab1hbox = new QHBoxLayout;
    setUnifiedMargin(tab1hbox, 5);
    tab1hbox->addWidget(tableWidget);
    tab1->setLayout(tab1hbox);

    QWidget *tab2 = new QWidget;
    textEdit = new QTextEdit;

    textEdit->setPlainText(tr("Twinkle, twinkle, little star,\n"
                              "How I wonder what you are.\n"
                              "Up above the world so high,\n"
                              "Like a diamond in the sky.\n"
                              "Twinkle, twinkle, little star,\n"
                              "How I wonder what you are!\n"));

    QHBoxLayout *tab2hbox = new QHBoxLayout;
    setUnifiedMargin(tab2hbox, 5);
    tab2hbox->addWidget(textEdit);
    tab2->setLayout(tab2hbox);

    bottomLeftTabWidget->addTab(tab1, tr("&Table"));
    bottomLeftTabWidget->addTab(tab2, tr("Text &Edit"));

    QWidget *pTreeViewWidget = new QWidget;
    QFileSystemModel *model = new QFileSystemModel(this);
    model->setRootPath(QDir::currentPath());
    QHBoxLayout *pTabLayout = new QHBoxLayout;
    setUnifiedMargin(pTabLayout, 0);
    pTreeViewWidget->setLayout(pTabLayout);

    QTreeView *tree = new QTreeView;
    tree->setModel(model);
    pTabLayout->addWidget(tree);

    QWidget *pListViewWidget = new QWidget;
    QHBoxLayout *pListLayout = new QHBoxLayout;
    QStandardItemModel *listModel = new QStandardItemModel(this);
    setUnifiedMargin(pListLayout, 0);
    pListViewWidget->setLayout(pListLayout);

    QListView *lv = new QListView;
    lv->setDragEnabled(true);
    lv->setDragDropMode(QListView::DragDrop);
    lv->setDefaultDropAction(Qt::CopyAction);
    lv->setModel(listModel);
    pListLayout->addWidget(lv);

    for (uint8_t i = 0; i < 10; ++i) {
        QStandardItem *item = new QStandardItem;
        item->setIcon(this->style() ? this->style()->standardIcon(QStyle::StandardPixmap(QStyle::SP_DirIcon + i)) : QIcon());
        item->setText(QStringLiteral("Row %1...............").arg(i + 1));
        item->setEnabled(i % 2);
        item->setCheckable(true);
        item->setCheckState(Qt::Checked);
        item->setDragEnabled(true);
        listModel->appendRow(item);
    }

    QWidget *toolbtns = new QWidget;
    QVBoxLayout *tbVLayout = new QVBoxLayout(toolbtns);
    tbVLayout->addWidget(createToolButtons(nullptr, false));
    tbVLayout->addWidget(createToolButtons(nullptr, true));
    QScrollArea *toolArea = new QScrollArea;
    toolArea->setWidget(toolbtns);
    bottomLeftTabWidget->addTab(pTreeViewWidget, "&TreeView");
    bottomLeftTabWidget->addTab(pListViewWidget, "&ListView");
    bottomLeftTabWidget->addTab(toolArea, "toolbuttons");
    bottomLeftTabWidget->addTab(new QWidget(), "tab 3");
    bottomLeftTabWidget->addTab(new QWidget(), "tab 4");
}

void WidgetGallery::createBottomRightGroupBox()
{
    bottomRightGroupBox = new QGroupBox(tr("Group 3"));
    bottomRightGroupBox->setCheckable(true);
    bottomRightGroupBox->setChecked(true);

    lineEdit = new QLineEdit("s3cRe7");
    lineEdit->setEchoMode(QLineEdit::Password);
    lineEdit->setClearButtonEnabled(true);
    lineEdit->setFrame(false);
    QMenu *menu = lineEdit->createStandardContextMenu();
    menu->setParent(lineEdit);
    QAction *testActoin = new QAction("b");
    testActoin->setCheckable(true);
    testActoin->setProperty("_d_menu_item_redpoint", true);

    QMenu *subMenu = new QMenu("just test red point sub menu", menu);
    subMenu->setProperty("_d_menu_item_redpoint", true);
    subMenu->setProperty("_d_menu_item_info", "new");
    subMenu->addAction("111");
    subMenu->addAction("222");
    QAction *a3 = new QAction("a3");
    a3->setProperty("_d_menu_item_redpoint", true);
    subMenu->addAction(a3);
    menu->addMenu(subMenu);
    testActoin->setProperty("_d_menu_item_info", "99+");
    QObject::connect(testActoin, &QAction::triggered, testActoin, [testActoin](bool checked) {
        testActoin->setProperty("_d_menu_item_redpoint", checked);
    });
    menu->addAction(testActoin);
    QObject::connect(lineEdit, &QLineEdit::textChanged, lineEdit, [menu]() {
        menu->popup(QCursor::pos());
    });

    spinBox = new QSpinBox(bottomRightGroupBox);
    spinBox->setValue(50);
    spinBox->setButtonSymbols(QAbstractSpinBox::PlusMinus);
    spinBox->setPrefix(" Prefix ");
    spinBox->setSuffix(" Suffix ");
    spinBox->setAlignment(Qt::AlignCenter);
    spinBox->setFrame(false);

    dateTimeEdit = new QDateTimeEdit(bottomRightGroupBox);
    dateTimeEdit->setDateTime(QDateTime::currentDateTime());

    comboBoxEdit = new QComboBox(bottomRightGroupBox);
    comboBoxEdit->addItem(QIcon::fromTheme("dde-file-manager"), "dde-file-manager");
    comboBoxEdit->addItem(QIcon::fromTheme("dde-introduction"), "dde-introduction");
    comboBoxEdit->addItem(QIcon::fromTheme("deepin-deb-installer"), "deepin-deb-installer");
    comboBoxEdit->setEditable(true);

    slider = new QSlider(Qt::Horizontal, bottomRightGroupBox);
    slider->setRange(0, 100);
    slider->setTickInterval(10);
    slider->setTickPosition(QSlider::TicksBelow);
    slider->setValue(40);

    scrollBar = new QScrollBar(Qt::Horizontal, bottomRightGroupBox);
    scrollBar->setValue(60);
    scrollBar->setProperty("_d_dtk_slider_always_show", true);

    dial = new QDial(bottomRightGroupBox);
    dial->setValue(30);
    dial->setNotchesVisible(true);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(lineEdit, 0, 0, 1, 2);
    layout->addWidget(spinBox, 1, 0, 1, 2);
    layout->addWidget(dateTimeEdit, 2, 0, 1, 2);
    layout->addWidget(comboBoxEdit, 3, 0, 1, 2);
    layout->addWidget(slider, 4, 0);
    layout->addWidget(scrollBar, 5, 0);
    layout->addWidget(dial, 4, 1, 2, 1);
    layout->setRowStretch(6, 1);
    bottomRightGroupBox->setLayout(layout);
}

//! [13]
void WidgetGallery::createProgressBar()
{
    progressBar = new QProgressBar;
    progressBar->setRange(0, 10000);
    progressBar->setValue(0);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(advanceProgressBar()));
    timer->start(1000);
}
//! [13]

QToolButton* WidgetGallery::toolBtn(QToolButton::ToolButtonPopupMode mode, const QString &text, bool hasMenu, bool hasIcon, Qt::ToolButtonStyle style)
{
    QToolButton *btn = new QToolButton;
    QMenu *menu = new QMenu;
    menu->addAction("action1");
    menu->addAction("action2");
    if (hasMenu)
        btn->setMenu(menu);
    if (hasIcon)
        btn->setIcon(QIcon::fromTheme("edit"));
    btn->setIconSize({16, 16});
    btn->setPopupMode(mode);
    if (!text.isEmpty()) {
        btn->setText(text);
        btn->setToolButtonStyle(style);
    }

    return btn;
}

QWidget* WidgetGallery::createToolButtons(QWidget *parent, bool hasMenu)
{
    QWidget *holder = new QWidget(parent);
    holder->resize(300, 500);
    QGridLayout *gridLayout = new QGridLayout(holder);
    gridLayout->addWidget(new QLabel(QString("ToolButtonPopupMode")), 0, 0);
    gridLayout->addWidget(new QLabel("IconOnly"), 0, 1);
    gridLayout->addWidget(new QLabel("    TextOnly"), 0, 2);
    gridLayout->addWidget(new QLabel("TextBesideIcon"), 0, 3);
    gridLayout->addWidget(new QLabel("TextUnderIcon"), 0, 4);
    gridLayout->addWidget(new QLabel("FollowStyle"), 0, 5);
    QString tmp =  + hasMenu ? QString("(hasMenu)") : QString("(NoMenu)");
    for (int i = 0; i < 3; ++i) {
        auto mode = static_cast<QToolButton::ToolButtonPopupMode>(i);
        QMetaEnum metaEnum = QMetaEnum::fromType<QToolButton::ToolButtonPopupMode>();

        gridLayout->addWidget(new QLabel(metaEnum.valueToKey(mode) + tmp), i + 1, 0);

        if (hasMenu) {
            QToolButton *menuTextIconBtnUnderIconOnly = toolBtn(mode, "ToolButton", true, true, Qt::ToolButtonStyle::ToolButtonIconOnly);
            gridLayout->addWidget(menuTextIconBtnUnderIconOnly, i + 1, 1);

            QToolButton *menuTextIconBtnUnderTextOnly = toolBtn(mode, "ToolButton", true, true, Qt::ToolButtonStyle::ToolButtonTextOnly);
            gridLayout->addWidget(menuTextIconBtnUnderTextOnly, i + 1, 2);

            QToolButton *menuTextIconBtn = toolBtn(mode, "ToolButton", true, true, Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
            gridLayout->addWidget(menuTextIconBtn, i + 1, 3);

            QToolButton *menuTextIconBtnUnder = toolBtn(mode, "ToolButton", true, true, Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
            gridLayout->addWidget(menuTextIconBtnUnder, i + 1, 4);

            QToolButton *menuTextIconBtnUnderFollow = toolBtn(mode, "ToolButton", true, true, Qt::ToolButtonStyle::ToolButtonFollowStyle);
            gridLayout->addWidget(menuTextIconBtnUnderFollow, i + 1, 5);
        } else {
            QToolButton *noMenuTextIconBtnIconOnly = toolBtn(mode, "ToolButton", false, true, Qt::ToolButtonStyle::ToolButtonIconOnly);
            gridLayout->addWidget(noMenuTextIconBtnIconOnly, i + 1, 1);

            QToolButton *noMenuTextIconBtnTextOnly = toolBtn(mode, "ToolButton", false, true, Qt::ToolButtonStyle::ToolButtonTextOnly);
            gridLayout->addWidget(noMenuTextIconBtnTextOnly, i + 1, 2);

            QToolButton *noMenuTextIconBtn = toolBtn(mode, "ToolButton", false, true, Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
            gridLayout->addWidget(noMenuTextIconBtn, i + 1, 3);

            QToolButton *noMenuTextIconBtnUnder = toolBtn(mode, "ToolButton", false, true, Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
            gridLayout->addWidget(noMenuTextIconBtnUnder, i + 1, 4);

            QToolButton *noMenuTextIconBtnFollow = toolBtn(mode, "ToolButton", false, true, Qt::ToolButtonStyle::ToolButtonFollowStyle);
            gridLayout->addWidget(noMenuTextIconBtnFollow, i + 1, 5);
        }
    }
    return holder;
}
