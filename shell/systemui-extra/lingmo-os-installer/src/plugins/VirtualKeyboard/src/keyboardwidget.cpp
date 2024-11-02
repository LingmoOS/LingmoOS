/**
 * Copyright (C) 2018 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
**/
#include "keyboardwidget.h"
#include "ui_keyboardwidget.h"
#include <QDebug>
#include <QMap>
#include <QVector>
#include <QFile>

#define SYMBOL_KEY_COUNT 29
#define SYMBOL_PAGE_COUNT 2

#define BUTTON_BG "QPushButton{background:#1E1B18}"
#define BUTTON_BG_PRESSED "QPushButton{background: #181613;}"
#define BUTTON_BG_HL "QPushButton{background:#80c342}"
#define BUTTON_BG_HL_PRESSED "QPushButton{background:#486E25}"


QChar symbols[SYMBOL_PAGE_COUNT][SYMBOL_KEY_COUNT] =
              { {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
                'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
                'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/'},
                {'!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
                 '`', '-', '=', '[', ']', '\\', '|', '{', '}',
                 '~','<', '>', ':', ';', '\'', '"', '-', '+', '?'}};


KeyboardWidget::KeyboardWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::KeyboardWidget),
    capsLock(false),
    isShift(false),
    page(0)
{

    QFile qssFile(":/qss/keyboard.qss");
    qDebug() << qssFile.exists();
    qssFile.open(QIODevice::ReadOnly);
    setStyleSheet(qssFile.readAll());
    qssFile.close();
    vKeyboard = new X11Keyboard(this);
    connect(this, SIGNAL(keyPressed(QChar)),
            vKeyboard, SLOT(onKeyPressed(QChar)));
    connect(this, SIGNAL(keyPressed(FuncKey::FUNCKEY)),
            vKeyboard, SLOT(onKeyPressed(FuncKey::FUNCKEY)));

    ui->setupUi(this);
    bindSingal();
    setDefaultIcon();
}

KeyboardWidget::~KeyboardWidget()
{
    delete ui;
}

void KeyboardWidget::resizeEvent(QResizeEvent */*event*/)
{
    int w = width();
    int h = height();
    int mainLeftMargin = ui->hl_main->contentsMargins().left();
    int mainRightMargin = ui->hl_main->contentsMargins().right();
    int mainTopMargin = ui->hl_main->contentsMargins().left();
    int mainBottomMargin = ui->hl_main->contentsMargins().right();
    int mainSpacing = ui->hl_main->spacing();
    int itemSpacing = ui->hl_1->spacing();

    int btnWidthCount = w - 11 * itemSpacing - mainSpacing- mainLeftMargin - mainRightMargin;
    int btnHeightCount = h - 3 * itemSpacing - mainTopMargin - mainBottomMargin;
    double btnWidth = btnWidthCount / 12;
    double btnHeight = btnHeightCount / 4;
    for(int i = 0; i <= 28; i++) {
        QString btnObjName = "btn_" + QString::number(i);
        QPushButton *btn = ui->page_letter->findChild<QPushButton*>(btnObjName);
        btn->setFixedSize(btnWidth, btnHeight);
    }
    ui->btn_ctrl_l->setFixedSize(btnWidth * 1.3, btnHeight);
    ui->btn_ctrl_r->setFixedSize(btnWidth * 1.3, btnHeight);
    ui->btn_alt_l->setFixedSize(btnWidth, btnHeight);
    ui->btn_alt_r->setFixedSize(btnWidth, btnHeight);
    ui->btn_super->setFixedSize(btnWidth, btnHeight);
    ui->btn_shift_l->setFixedSize(btnWidth, btnHeight);
    ui->spacer_2->changeSize(btnWidth / 2, 20);


    for(int i = 1; i <= 9; i++) {
        QString btnObjName = "btn_num_" + QString::number(i);
        QPushButton *btn = ui->page_number->findChild<QPushButton*>(btnObjName);
        btn->setFixedWidth(btnWidth);
    }
    ui->btn_backspace_num->setFixedSize(btnWidth,btnHeight);
    ui->btn_insert->setFixedWidth(btnWidth);
    ui->btn_delete->setFixedWidth(btnWidth);
    ui->btn_home->setFixedWidth(btnWidth);
    ui->btn_end->setFixedWidth(btnWidth);
    ui->btn_pgup->setFixedWidth(btnWidth);
    ui->btn_pgdn->setFixedWidth(btnWidth);
    ui->btn_up->setFixedSize(btnWidth,btnHeight);
    ui->btn_down->setFixedSize(btnWidth,btnHeight);
    ui->btn_left->setFixedSize(btnWidth,btnHeight);
    ui->btn_right->setFixedSize(btnWidth,btnHeight);

    ui->btn_close->setFixedHeight(btnHeight);
    ui->btn_letter->setFixedHeight(btnHeight);
    ui->btn_symbol->setFixedHeight(btnHeight);
    ui->btn_number->setFixedHeight(btnHeight);

    ui->btn_close->setFixedWidth(btnWidth);
    ui->btn_letter->setFixedWidth(btnWidth);
    ui->btn_symbol->setFixedWidth(btnWidth);
    ui->btn_number->setFixedWidth(btnWidth);
    setIconSize();
}

float hScale = 0.6;
float wScale = hScale;
#define SET_ICON_SIZE_SCALE(btn) \
    ui->btn_##btn->setIconSize(QSize(ui->btn_##btn->width() * hScale, ui->btn_##btn->height() * wScale));

#define SET_ICON_SIZE(btn) \
    ui->btn_##btn->setIconSize(QSize(ui->btn_##btn->width(), ui->btn_##btn->height()));

void KeyboardWidget::setIconSize()
{
    SET_ICON_SIZE_SCALE(backspace);
    SET_ICON_SIZE_SCALE(enter);
    SET_ICON_SIZE_SCALE(close);
    SET_ICON_SIZE_SCALE(super);
    SET_ICON_SIZE_SCALE(shift_l);
    SET_ICON_SIZE_SCALE(shift_r);
    SET_ICON_SIZE(up);
    SET_ICON_SIZE(down);
    SET_ICON_SIZE(left);
    SET_ICON_SIZE(right);
}

void KeyboardWidget::bindSingal()
{
    for(auto obj : ui->page_letter->children()) {
        if(obj->metaObject()->className() == QString("QPushButton")) {
            QPushButton *btn = static_cast<QPushButton*>(obj);
            btn->setFocusPolicy(Qt::NoFocus);
            connect(btn, &QPushButton::clicked, this, &KeyboardWidget::onButtonClicked);
            connect(btn, &QPushButton::pressed, this, &KeyboardWidget::onButtonPressed);
            connect(btn, &QPushButton::released, this, &KeyboardWidget::onButtonReleased);
        }
    }
    for(auto obj : ui->page_number->children()) {
        if(obj->metaObject()->className() == QString("QPushButton")) {
            QPushButton *btn = static_cast<QPushButton*>(obj);
            btn->setFocusPolicy(Qt::NoFocus);
            connect(btn, &QPushButton::clicked, this, &KeyboardWidget::onButtonClicked);
            connect(btn, &QPushButton::pressed, this, &KeyboardWidget::onButtonPressed);
            connect(btn, &QPushButton::released, this, &KeyboardWidget::onButtonReleased);
        }
    }
    ui->btn_close->setFocusPolicy(Qt::NoFocus);
    ui->btn_letter->setFocusPolicy(Qt::NoFocus);
    ui->btn_symbol->setFocusPolicy(Qt::NoFocus);
    ui->btn_number->setFocusPolicy(Qt::NoFocus);

    connect(ui->btn_letter, &QPushButton::clicked, this, [&] {
        ui->stackedWidget->setCurrentWidget(ui->page_letter);
        page = 0;
        switchPage();
    });
    connect(ui->btn_symbol, &QPushButton::clicked, this, [&] {
        ui->stackedWidget->setCurrentWidget(ui->page_letter);
        page = 1;
        switchPage();
    });
    connect(ui->btn_number, &QPushButton::clicked, this, [&] {
        ui->stackedWidget->setCurrentWidget(ui->page_number);
    });
    connect(ui->btn_close, &QPushButton::clicked,
            this, &KeyboardWidget::aboutToClose);

    connect(ui->btn_close, &QPushButton::pressed,
            this, &KeyboardWidget::onButtonPressed);
    connect(ui->btn_close, &QPushButton::released,
            this, &KeyboardWidget::onButtonReleased);
}

void KeyboardWidget::setDefaultIcon()
{
    ui->btn_backspace->setIcon(QIcon(":/images/images/backspace.svg"));
    ui->btn_backspace_num->setIcon(QIcon(":/images/images/backspace.svg"));
    ui->btn_enter->setIcon(QIcon(":/images/images/enter.svg"));
    ui->btn_shift_l->setIcon(QIcon(":/images/images/capslock.svg"));
    ui->btn_shift_r->setIcon(QIcon(":/images/images/capslock.svg"));
    ui->btn_close->setIcon(QIcon(":/images/images/close.svg"));
    //ui->btn_super->setIcon(QIcon(":/images/images/super.svg"));
    ui->btn_super->setText("Super");
    ui->btn_up->setIcon(QIcon(":/images/images/up.svg"));
    ui->btn_down->setIcon(QIcon(":/images/images/down.svg"));
    ui->btn_left->setIcon(QIcon(":/images/images/left.svg"));
    ui->btn_right->setIcon(QIcon(":/images/images/right.svg"));
}

QString KeyboardWidget::getKeyName(QPushButton *btn)
{
    QString objName = btn->objectName();
    int lastUnderline = objName.lastIndexOf('_');
    int start = strlen("btn_");
    int keyLength = lastUnderline - start;
    QString keyName = objName.mid(start, keyLength);
    return (QString)keyName;
}

void KeyboardWidget::changeFuncKeyStyle(QPushButton *btn, bool isPressed)
{
    QString modName = getKeyName(btn);
    Modifier::MOD mod = Modifier::getModifier(modName);

    if(vKeyboard->hasModifier(mod)) {
        if(isPressed)
            btn->setStyleSheet(BUTTON_BG_HL_PRESSED);
        else
            btn->setStyleSheet(BUTTON_BG_HL);
    } else {
        if(isPressed)
            btn->setStyleSheet(BUTTON_BG_PRESSED);
        else
            btn->setStyleSheet(BUTTON_BG);
    }
}

void KeyboardWidget::changeShitKeyStyle(QPushButton *btn, bool isPressed)
{
    if(page == 0){
        if(isShift) {
            if(capsLock){
                if(isPressed) {
                    btn->setStyleSheet(BUTTON_BG_HL_PRESSED);
                    btn->setIcon(QIcon(":/images/images/capslock_click.svg"));
                } else {
                    btn->setStyleSheet(BUTTON_BG_HL);
                    btn->setIcon(QIcon(":/images/images/capslock.svg"));
                }
            }
            else {
                if(isPressed)
                    btn->setIcon(QIcon(":/images/images/capslock_hl_click.svg"));
                else
                    btn->setIcon(QIcon(":/images/images/capslock_hl.svg"));
            }
        } else {
            if(isPressed)
                btn->setIcon(QIcon(":/images/images/capslock_click.svg"));
            else
                btn->setIcon(QIcon(":/images/images/capslock.svg"));
        }
    }
}


void KeyboardWidget::changeDirectKeyStyle(QPushButton *btn, bool isPressed)
{
    QString keyName = getKeyName(btn);
    FuncKey::FUNCKEY key = FuncKey::getKey(keyName);
    if(key == FuncKey::UNKNOWN)
        return;

    QString iconName = QString(":/images/images/%1.svg").arg(keyName);
    QString iconNamePressed = QString(":/images/images/%1_click.svg").arg(keyName);

    if(isPressed)
        btn->setIcon(QIcon(iconNamePressed));
    else
        btn->setIcon(QIcon(iconName));
}

/**
 * @brief 修改按键样式
 * @param obj 按键
 * @param isPressed 按下或者松开
 */
void KeyboardWidget::changeKeyStyle(QPushButton *btn, bool isPressed)
{
    if(btn == ui->btn_ctrl_l || btn == ui->btn_ctrl_r ||
            btn == ui->btn_alt_l || btn == ui->btn_alt_r ||
            btn == ui->btn_super) {
        changeFuncKeyStyle(btn, isPressed);
    }

    if(btn == ui->btn_shift_l)
        changeShitKeyStyle(ui->btn_shift_l, isPressed);
    if(btn == ui->btn_shift_r)
        changeShitKeyStyle(ui->btn_shift_r, isPressed);

    changeDirectKeyStyle(btn, isPressed);
}

void KeyboardWidget::onButtonPressed()
{
    QPushButton *btn = static_cast<QPushButton*>(sender());
    changeKeyStyle(btn, true);
}

void KeyboardWidget::onButtonReleased()
{
    QPushButton *btn = static_cast<QPushButton*>(sender());
    changeKeyStyle(btn, false);
}

void KeyboardWidget::onButtonClicked()
{
    QObject *obj = sender();
    if(obj->metaObject()->className() != QString("QPushButton"))
        return;

    QPushButton *btn = static_cast<QPushButton*>(obj);
    QString keyName = getKeyName(btn);
    qDebug() << "keyName: " << keyName;

    Modifier::MOD mod = Modifier::getModifier(keyName);
    FuncKey::FUNCKEY funcKey = FuncKey::getKey(keyName);

    if(keyName == "shift") {
        if(page == 0) {
            isShift = !isShift;
            if(isShift) {     //第一次被按下
                capsLock = false;
                shiftLastClicked = QTime::currentTime();
                ui->btn_shift_l->setIcon(QIcon(":/images/images/capslock_hl.svg"));
                ui->btn_shift_r->setIcon(QIcon(":/images/images/capslock_hl.svg"));
            }
            else {
                int doubleClickInterval = QApplication::doubleClickInterval();
                if(shiftLastClicked.msecsTo(QTime::currentTime()) <= doubleClickInterval) {
                    //shift键双击，锁定大写
                    capsLock = true;
                    isShift = true;
                    ui->btn_shift_l->setIcon(QIcon(":/images/images/capslock.svg"));
                    ui->btn_shift_r->setIcon(QIcon(":/images/images/capslock.svg"));
                    ui->btn_shift_l->setStyleSheet("QPushButton{background:#80c342}");
                    ui->btn_shift_r->setStyleSheet("QPushButton{background:#80c342}");
                } else {
                    ui->btn_shift_l->setIcon(QIcon(":/images/images/capslock.svg"));
                    ui->btn_shift_r->setIcon(QIcon(":/images/images/capslock.svg"));
                    ui->btn_shift_l->setStyleSheet("QPushButton{background:#1E1B18}");
                    ui->btn_shift_r->setStyleSheet("QPushButton{background:#1E1B18}");
                }
            }
            toggleCase();
        } else {
            page = page % (SYMBOL_PAGE_COUNT - 1) + 1;
            switchPage();
        }
    } else if(mod != Modifier::UNKNOWN) {
        if(vKeyboard->hasModifier(mod)) {
            vKeyboard->removeModifier(mod);
            btn->setStyleSheet(BUTTON_BG);
            btn->setStyleSheet(BUTTON_BG);
        } else {
            vKeyboard->addModifier(mod);
            btn->setStyleSheet(BUTTON_BG_HL);
            btn->setStyleSheet(BUTTON_BG_HL);
        }
    } else if(funcKey != FuncKey::UNKNOWN) {
        Q_EMIT keyPressed(funcKey);
    } else {    //字符键
        QChar c;
        QString text = btn->text();
        qDebug() << "clicked button text: " << text;
        if(text == "&&")
            c = '&';
        else if(text.length() == 1)
            c = text.at(0);

        Q_EMIT keyPressed(c);

        //如果shift键被单击，按一个键后就恢复为小写
        if(isShift && !capsLock) {
            isShift = false;
            toggleCase();
            changeShitKeyStyle(ui->btn_shift_l, false);
            changeShitKeyStyle(ui->btn_shift_r, false);
        }
        clearModifier();
    }
}

void KeyboardWidget::clearModifier()
{
    for(auto mod : vKeyboard->getAllModifier()) {
        QString modName = Modifier::getModifierName(mod);
        if(mod == Modifier::SUPER) {
            QString objName = QString("btn_%1").arg(modName);
            QPushButton *btn = ui->page_letter->findChild<QPushButton*>(objName);
            btn->setStyleSheet(BUTTON_BG);
        } else {
            QString objName = QString("btn_%1_l").arg(modName);
            QPushButton *btn = ui->page_letter->findChild<QPushButton*>(objName);
            btn->setStyleSheet(BUTTON_BG);
            objName = QString("btn_%1_r").arg(modName);
            btn = ui->page_letter->findChild<QPushButton*>(objName);
            btn->setStyleSheet(BUTTON_BG);
        }
    }
    vKeyboard->clearModifier();
}

void KeyboardWidget::toggleCase()
{
    for(int i = 0; i < 26; i++) {
        QString objName = "btn_" + QString::number(i);
        QPushButton *btn = findChild<QPushButton*>(objName);
        QChar ch = '\0';
        if(isShift) {   //切换到大写
            ch = symbols[0][i].toUpper();
        } else {
            ch = symbols[0][i];
        }
        btn->setText(ch);
    }
}

void KeyboardWidget::switchPage()
{
    if(page == 0) {
        ui->btn_shift_l->setText("");
        ui->btn_shift_r->setText("");
        ui->btn_shift_l->setIcon(QIcon(":/images/images/capslock.svg"));
        ui->btn_shift_r->setIcon(QIcon(":/images/images/capslock.svg"));
    } else {
        ui->btn_shift_l->setText("");
        ui->btn_shift_r->setText("");
        ui->btn_shift_l->setIcon(QIcon(":/images/images/capslock.svg"));
        ui->btn_shift_r->setIcon(QIcon(":/images/images/capslock.svg"));
    }

    for(int i = 0; i < SYMBOL_KEY_COUNT; i++) {
        QString btnObjName = "btn_" + QString::number(i);
        QPushButton *btn = ui->page_letter->findChild<QPushButton*>(btnObjName);
        QChar c = symbols[page][i];
        if(c == '&')
            btn->setText("&&");
        else
            btn->setText(c);
    }
}

