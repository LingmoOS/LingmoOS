/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: yangyanwei <yangyanwei@kylinos.cn>
 *
 */

#include "tag-management.h"
#include "xatom-helper.h"
#include "file-label-model.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QWidget>
#include <QCheckBox>
#include <QPainter>
#include <QLineEdit>
#include <QColorDialog>
#include <QHeaderView>
#include <QMenu>

#include <QPainterPath>

using namespace Peony;
static TagManagement *global_instance = nullptr;

FileLabelEdit::FileLabelEdit(QColor color,QString name, QWidget *parent) : QWidget(parent)
{
    m_colorButton = new QPushButton(this);
    m_colorButton->setProperty("isRoundButton",true);
    m_colorButton->setFixedSize(12,12);
    QPalette pal = m_colorButton->palette();
    pal.setColor(QPalette::Button, color);
    pal.setColor(QPalette::Active,QPalette::Highlight, color);
    m_colorButton->setPalette(pal);

    QHBoxLayout *nameLayout = new QHBoxLayout(this);
    nameLayout->setMargin(0);
    nameLayout->setAlignment(Qt::AlignLeft);
    nameLayout->addWidget(m_colorButton);
    m_text = new QLabel(this);
    m_text->setText(name);
    nameLayout->addWidget(m_text);
    m_edit = new QLineEdit(this);
    nameLayout->addWidget(m_edit);
    m_edit->hide();
    setLayout(nameLayout);
    connect(m_edit, &QLineEdit::editingFinished, this, [=]() {
        m_edit->hide();
        m_text->show();
        m_text->setText(m_edit->text());
        Q_EMIT changeName(m_edit->text());
    });
}

void FileLabelEdit::setColor(QColor color)
{
    QPalette pal = m_colorButton->palette();
    pal.setColor(QPalette::Button, color);
    pal.setColor(QPalette::Active,QPalette::Highlight, color);
    m_colorButton->setPalette(pal);
}

void FileLabelEdit::setName(const QString& name)
{
    m_text->setText(name);
}

void FileLabelEdit::startEdit()
{
    m_text->hide();
    m_edit->show();
    m_edit->setFocus();
    m_edit->setText(m_text->text());
}

LabelSettings::LabelSettings(QWidget *parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    m_fileLabel = new QTableWidget(this);
    m_fileLabel->setColumnCount(3);
    FileLabelModel *model = FileLabelModel::getGlobalModel();
    auto items = model->getAllFileLabelItems();
    m_fileLabel->setContextMenuPolicy(Qt::CustomContextMenu);
    m_fileLabel->setRowCount(items.size());
    m_fileLabel->setContentsMargins(0, 0, 0, 0);
    m_fileLabel->setSelectionBehavior(QAbstractItemView::SelectRows);
    //m_fileLabel->setSelectionMode(QAbstractItemView::NoSelection);
    m_fileLabel->setAlternatingRowColors(true);
    m_fileLabel->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_fileLabel->verticalHeader()->setVisible(false);
    m_fileLabel->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_fileLabel->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_fileLabel->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_fileLabel->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_fileLabel->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    m_fileLabel->setShowGrid(false);

    QStringList titles;
    titles.append(tr("Name"));
    titles.append(tr("SideBar"));
    titles.append(tr("Menu"));
    m_fileLabel->setHorizontalHeaderLabels(titles);

    loadAllFileLabels();
    mainLayout->addWidget(m_fileLabel);

    QHBoxLayout *layout = new QHBoxLayout(this);
    QPushButton *add = new QPushButton(QIcon::fromTheme("list-add-symbolic"), tr("Create New Label"), this);
    connect(add, &QPushButton::clicked, this, &LabelSettings::addFileLabel);
    QPushButton *del = new QPushButton(QIcon::fromTheme("list-remove-symbolic"), tr("Delete Label"), this);
    connect(del, &QPushButton::clicked, this, &LabelSettings::delFileLabel);

    layout->setContentsMargins(0, 26, 18, 0);
    layout->addStretch();
    layout->addWidget(add);
    layout->addWidget(del);

    QLabel *instruction = new QLabel(this);
    instruction->setWordWrap(true);
    instruction->setText(tr("Display the following items in the identification area: (maximum of 6)"));
    mainLayout->setContentsMargins(8, 18, 8, 26);
    mainLayout->addWidget(instruction);
    mainLayout->addSpacing(18);
    mainLayout->addWidget(m_fileLabel);
    mainLayout->addLayout(layout);
    setLayout(mainLayout);
    connect(m_fileLabel, &QTableWidget::cellClicked, this, [=](int row, int col) {
        if (0 == col) {
            FileLabelEdit *item = static_cast<FileLabelEdit*>(m_fileLabel->cellWidget(row,col));
            item->startEdit();
        }
    });
    connect(m_fileLabel, &QWidget::customContextMenuRequested, this, [=](const QPoint &pos) {
        auto index = m_fileLabel->indexAt(pos);
        QMenu menu(this);
        if (index.isValid()) {
            menu.addAction(tr("Rename"), [=]() {
                //FIXME: edit
                int row = m_fileLabel->currentRow();
                FileLabelEdit *item = static_cast<FileLabelEdit*>(m_fileLabel->cellWidget(row,0));
                item->startEdit();
            });

            menu.addAction(tr("Edit Color"), [=]() {
                QColorDialog d;
                d.setStyleSheet("QSpinBox{"
                                "min-width: 2em;"
                                "}");
                if (d.exec()) {
                    auto color = d.selectedColor();
                    auto item = FileLabelModel::getGlobalModel()->itemFormIndex(index);
                    FileLabelModel::getGlobalModel()->setLabelColor(item->id(), color);
                    int row = m_fileLabel->currentRow();
                    FileLabelEdit *edit = static_cast<FileLabelEdit*>(m_fileLabel->cellWidget(row,0));
                    edit->setColor(color);
                }
            });

            auto a = menu.addAction(tr("Delete This Label"));
            connect(a, &QAction::triggered, this, &LabelSettings::delFileLabel);
        } else {
            auto a = menu.addAction(tr("Create New Label"));
            connect(a, &QAction::triggered, this, &LabelSettings::addFileLabel);
        }
        menu.exec(mapToGlobal(pos));
    });
}

void LabelSettings::loadAllFileLabels()
{
    FileLabelModel *model = FileLabelModel::getGlobalModel();
    auto items = model->getAllFileLabelItems();
    for (int i = 0; i < items.size(); i++) {
        bool valids[2] = {0};
        valids[0] = items[i]->isValidInSidebar();
        valids[1] = items[i]->isValidInMenu();
        for(int j = 0 ; j < 2; j++) {
            m_fileLabel->setCellWidget(i, j + 1, nullptr);
            QWidget *w = new QWidget(m_fileLabel);
            QHBoxLayout *l = new QHBoxLayout(w);
            l->setMargin(0);
            w->setLayout(l);
            l->setAlignment(Qt::AlignCenter);
            QCheckBox *box = new QCheckBox(w);
            box->setChecked(valids[j]);
            connect(box, &QCheckBox::clicked, this, [=](bool status) {
                if(0 == j) {
                    items[i]->setValidInSidebar(status);
                } else {
                    items[i]->setValidInMenu(status);
                }
            });

            l->addWidget(box);
            m_fileLabel->setCellWidget(i, j + 1, w);
        }
        FileLabelEdit *labelName = new FileLabelEdit(items[i]->color(), items[i]->name(), this);
        m_fileLabel->setCellWidget(i, 0, labelName);
        connect(labelName, &FileLabelEdit::changeName, model, [=](const QString &name){
            model->setLabelName(items[i]->id(), name);
        });
        connect(items[i], &FileLabelItem::nameChanged, labelName, &FileLabelEdit::setName);
        connect(items[i], &FileLabelItem::colorChanged, labelName, &FileLabelEdit::setColor);
    }
}

void LabelSettings::addFileLabel()
{
    QColorDialog dialog;
    dialog.setStyleSheet("QSpinBox{"
                    "min-width: 2em;"
                    "}");
    if (dialog.exec()) {
        auto color = dialog.selectedColor();
        auto name = color.name();
        FileLabelModel *model = FileLabelModel::getGlobalModel();
        if (!model->addLabel(name, color))
            return;
        int row = m_fileLabel->rowCount();
        m_fileLabel->insertRow(row);

        FileLabelItem *item = model->getItemByRow(row);

        FileLabelEdit *labelName = new FileLabelEdit(color, name, this);
        m_fileLabel->setCellWidget(row, 0, labelName);
        connect(labelName, &FileLabelEdit::changeName, model, [=](const QString &name){
            model->setLabelName(item->id(), name);
        });
        connect(item, &FileLabelItem::nameChanged, labelName, &FileLabelEdit::setName);
        connect(item, &FileLabelItem::colorChanged, labelName, &FileLabelEdit::setColor);

        for(int j = 0 ; j < 2; j++) {
            m_fileLabel->setCellWidget(row, j + 1, nullptr);
            QWidget *w = new QWidget(m_fileLabel);
            QHBoxLayout *l = new QHBoxLayout(w);
            l->setMargin(0);
            w->setLayout(l);
            l->setAlignment(Qt::AlignCenter);
            QCheckBox *box = new QCheckBox(w);
            connect(box, &QCheckBox::clicked, this, [=](bool status) {
                if(0 == j) {
                    item->setValidInSidebar(status);
                } else {
                    item->setValidInMenu(status);
                }
            });
            if (0 == j) {
                box->setChecked(true);
                model->getItemByRow(row)->setValidInSidebar(true);
            }
            l->addWidget(box);
            m_fileLabel->setCellWidget(row, j + 1, w);
        }      
    }
}
void LabelSettings::delFileLabel()
{
    int row = m_fileLabel->currentRow();
    if (row < 0) {
        return;
    }
    m_fileLabel->removeRow(row);
    auto item = FileLabelModel::getGlobalModel()->getItemByRow(row);
    int index = item->id();
    FileLabelModel::getGlobalModel()->removeLabel(index);
}

TagManagement *TagManagement::getInstance()
{
    if (!global_instance) {
        global_instance = new TagManagement;
    }
    return global_instance;
}

TagManagement::TagManagement(QWidget *parent) : QMainWindow(parent)
{
    //this->setAttribute(Qt::WA_DeleteOnClose);
    this->setContentsMargins(0, 25, 0, 0);
    this->setFixedSize(440, 568);

    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_ALL;
    hints.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(window()->winId(), hints);
    setWindowFlags(Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);

    QTabWidget *management = new QTabWidget(this);
    management->setStyle(new tabWidgetStyle);
    management->tabBar()->setStyle(new tabStyle);
    setCentralWidget(management);
    QWidget *general = new QWidget(this);
    LabelSettings *tag = new LabelSettings(this);

    QWidget *sidebar = new QWidget(this);
    QWidget *Advanced = new QWidget(this);

    //management->addTab(general, tr("General"));
    management->addTab(tag, tr("Mark"));
    //management->addTab(sidebar, tr("Sidebar"));
    //management->addTab(Advanced, tr("Advanced"));
}

//TagManagement* TagManagement::getInstance()
//{
//    if (!global_instance) {
//        global_instance = new TagManagement();
//    }
//    return global_instance;
//}

void tabStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter,
                           const QWidget *widget) const
{
    /**
     * FIX:需要修复颜色不能跟随主题的问题
     * \brief
     */
    if (element == CE_TabBarTab) {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            //设置按钮的左右上下偏移
            QRect rect = tab->rect;
            //左侧移动1px
            rect.setLeft(rect.x() + 1);
            //右侧移动1px
            rect.setRight((rect.x() + rect.width())-2);
            QPainterPath path;
            const qreal radius = 6;
            if(tab->position == QStyleOptionTab::Beginning)
            {
                path.moveTo(rect.topRight());
                path.lineTo(rect.topLeft() + QPointF(radius, 0));
                path.quadTo(rect.topLeft(), rect.topLeft() + QPointF(0, radius));
                path.lineTo(rect.bottomLeft() - QPointF(0, radius));
                path.quadTo(rect.bottomLeft(), rect.bottomLeft() + QPointF(radius, 0));
                path.lineTo(rect.bottomRight());
                path.lineTo(rect.topRight());

            }
            else if(tab->position == QStyleOptionTab::End)
            {
                path.moveTo(rect.topRight() - QPointF(radius, 0));
                path.lineTo(rect.topLeft());
                path.lineTo(rect.bottomLeft() );
                path.lineTo(rect.bottomRight() - QPointF(radius, 0));
                path.quadTo(rect.bottomRight(), rect.bottomRight() + QPointF(0, -radius));
                path.lineTo(rect.topRight() + QPointF(0, radius));
                path.quadTo(rect.topRight(), rect.topRight() + QPointF(-radius, -0));

            }
            else if(tab->position == QStyleOptionTab::OnlyOneTab)
            {
                path.addRoundedRect(rect, radius, radius);
            }
            else
            {
                path.addRect(rect);
            }
            const QPalette &palette = widget->palette();

            //未选中时文字颜色 - Text color when not selected
            painter->setPen(palette.color(QPalette::ButtonText));

            if (tab->state & QStyle::State_Selected) {
                painter->save();
                painter->setPen(Qt::NoPen);
                painter->setBrush(palette.brush(QPalette::Highlight));

                painter->setRenderHint(QPainter::Antialiasing);  // 反锯齿;
                painter->drawPath(path);
                painter->restore();

                //选中时文字颜色 - Text color when selected
                painter->setPen(palette.color(QPalette::HighlightedText));
            } else if (tab->state & QStyle::State_MouseOver) {
                painter->save();
                QColor color = palette.color(QPalette::Highlight).lighter(140);
                painter->setPen(Qt::NoPen);
                painter->setBrush(color);

                painter->setRenderHint(QPainter::Antialiasing);  // 反锯齿;
                painter->drawPath(path);
                painter->restore();
                //选中时文字颜色 - Text color when selected
                painter->setPen(palette.color(QPalette::HighlightedText));
            } else {
                painter->save();
                painter->setPen(Qt::NoPen);
                painter->setBrush(palette.color(QPalette::Button));

                painter->setRenderHint(QPainter::Antialiasing);  // 反锯齿;
                painter->drawPath(path);
                painter->restore();
            }

            painter->drawText(rect, tab->text, QTextOption(Qt::AlignCenter));

            return;
        }
    }
    if (element == CE_TabBarTabLabel) {
        QProxyStyle::drawControl(element, option, painter, widget);
    }
}

QSize tabStyle::sizeFromContents(QStyle::ContentsType ct, const QStyleOption *opt, const QSize &contentsSize,
                                 const QWidget *widget) const
{
    QSize barSize = QProxyStyle::sizeFromContents(ct, opt, contentsSize, widget);

    if (ct == QStyle::CT_TabBarTab) {
        barSize.transpose();
        const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(opt);
        //解决按钮不能自适应的问题
        int fontWidth = tab->fontMetrics.width(tab->text);
        //宽度统一加上30px
        barSize.setWidth(fontWidth + 30);

        //46 - 8 - 8 = 30;
        barSize.setHeight(36);
    }

    return barSize;
}

QRect tabWidgetStyle::subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const
{
    switch(element)
    {
        case SE_TabWidgetTabBar:
        {
            //解决QTabBar设置为居中
            if (const QStyleOptionTabWidgetFrame *twf = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option))
            {
                QRect rect = QRect(QPoint(0, 0), twf->tabBarSize);
                if (twf->rect.size().width() > twf->tabBarSize.width()) {
                    rect.moveLeft((twf->rect.size() - twf->leftCornerWidgetSize - twf->rightCornerWidgetSize - twf->tabBarSize).width() / 2);
                } else {
                    rect.setWidth(twf->rect.size().width());
                    rect.moveLeft(0);
                }
                return rect;
            }
            break;

        }
    default:
        break;

    }
    return QProxyStyle::subElementRect(element, option, widget);
}
