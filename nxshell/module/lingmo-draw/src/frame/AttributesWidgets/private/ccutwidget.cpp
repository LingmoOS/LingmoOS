// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ccutwidget.h"
#include "drawshape/cdrawscene.h"
#include "widgets/toolbutton.h"
#include "utils/cvalidator.h"
#include "drawshape/cdrawparamsigleton.h"
#include "frame/cviewmanagement.h"
#include "frame/cgraphicsview.h"
#include "drawshape/cdrawscene.h"
#include "application.h"
#include "cattributemanagerwgt.h"
#include "toolbutton.h"


#include <DLabel>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <DGuiApplicationHelper>
#include <QDesktopWidget>
#include <DFontSizeManager>


DGUI_USE_NAMESPACE

const int BTN_SPACING = 6;
const int SEPARATE_SPACING = 5;
const int PUSHBUTTON_FONT_SIZE = 12;
const int TEXT_SIZE = 12;

CCutWidget::CCutWidget(DWidget *parent)
    : DrawAttribution::CAttriBaseOverallWgt(parent)
{
    setAttribution(DrawAttribution::ECutToolAttri);
    qRegisterMetaType<ECutType>("ECutType");
    initUI();
    initConnection();
}

CCutWidget::~CCutWidget()
{
}

void CCutWidget::setCutSize(const QSize &sz, bool emitSig)
{
    if (sz != m_cutCutSize) {
        m_cutCutSize = sz;
        m_widthEdit->setText(QString::number(sz.width()));
        m_heightEdit->setText(QString::number(sz.height()));
        if (emitSig) {
            emit cutSizeChanged(sz);
        }
    }
}

QSize CCutWidget::cutSize()
{
    return m_cutCutSize;
}

void CCutWidget::clearAllChecked()
{
    m_scaleBtn1_1->setChecked(false);
    m_scaleBtn2_3->setChecked(false);
    m_scaleBtn8_5->setChecked(false);
    m_scaleBtn16_9->setChecked(false);
    m_freeBtn->setChecked(false);
    m_originalBtn->setChecked(false);
}

void CCutWidget::adjustSize(bool emitSig)
{
    if (m_curCutType < cut_free) {
        qreal rd = Radio[m_curCutType];
        QSize newSize = m_defultRadioSize;
        int newWidth = qRound(newSize.height() * rd);
        newSize.setWidth(newWidth);
        setCutSize(newSize, emitSig);
    }
}

//void CCutWidget::changeButtonTheme()
//{
//    m_sepLine->updateTheme();
//}

void CCutWidget::setCutType(ECutType current, bool emitSig, bool adjustSz)
{
    if (current != m_curCutType) {
        m_curCutType = current;
        if (emitSig) {
            emit cutTypeChanged(current);
        }
        if (adjustSz)
            adjustSize(emitSig);
    }

    clearAllChecked();
    // 设置按钮选中状态
    switch (m_curCutType) {

    case cut_1_1: {
        m_scaleBtn1_1->setChecked(true);
        break;
    }
    case cut_2_3: {
        m_scaleBtn2_3->setChecked(true);
        break;
    }
    case cut_8_5: {
        m_scaleBtn8_5->setChecked(true);
        break;
    }
    case cut_16_9: {
        m_scaleBtn16_9->setChecked(true);
        break;
    }
    case cut_free: {
        m_freeBtn->setChecked(true);
        break;
    }
    case cut_original: {
        m_originalBtn->setChecked(true);
        break;
    }
    default: break;
    }

    setFocus();
}

ECutType CCutWidget::cutType()
{
    return m_curCutType;
}

//void CCutWidget::setDefualtRaidoBaseSize(const QSize &sz)
//{
//    m_defultRadioSize = sz;
//}

void CCutWidget::setAutoCalSizeIfRadioChanged(bool b)
{
    m_autoCal = b;
}

int CCutWidget::totalNeedWidth()
{
    int totalW = 0;
    for (int i = 0; i < _allWgts.count(); ++i) {
        auto pw = _allWgts[i];
        int w = attriWidgetRecommendedSize(pw).width();

        totalW += w;
        if (i < _allWgts.count() - 1) {
            totalW += centerLayout()->spacing();
        }
    }

    return totalW;
}

void CCutWidget::paintEvent(QPaintEvent *event)
{
    DrawAttribution::CAttriBaseOverallWgt::paintEvent(event);
//    QPainter painter(this);

//    painter.setBrush(Qt::blue);
//    painter.drawRect(rect());

//    auto p = qobject_cast<DrawAttribution::CAttriBaseOverallWgt *>(parentWidget());
//    if (p != nullptr)
//    qDebug() << "margin = " << this->contentsMargins() << p->centerLayout()->contentsMargins();
}

bool CCutWidget::eventFilter(QObject *o, QEvent *e)
{

    if (o->isWidgetType()) {
        auto w = qobject_cast<QWidget *>(o);
        if (e->type() == QEvent::ParentChange) {
            if (w->parentWidget() != nullptr) {
                if (w->parentWidget()->isWindow()) {
                    auto toolBtn = qobject_cast<ToolButton *>(w);
                    if (nullptr != toolBtn) {
                        if (toolBtn == m_doneBtn || toolBtn == m_cancelBtn)
                            toolBtn->setShowText(true);
                        toolBtn->setBtnStyle(MENU_STYLE);
                    }
                    w->parentWidget()->setFocus();

                } else {
                    auto toolBtn = qobject_cast<ToolButton *>(w);
                    if (nullptr != toolBtn) {
                        if (toolBtn == m_doneBtn || toolBtn == m_cancelBtn)
                            toolBtn->setShowText(false);

                        toolBtn->setBtnStyle(BUTTON_STYLE);
                    }
                }
            }
        }

    }
    return DrawAttribution::CAttriBaseOverallWgt::eventFilter(o, e);
}

void CCutWidget::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event)
    m_widthEdit->lineEdit()->setReadOnly(true);
    m_heightEdit->lineEdit()->setReadOnly(true);
}

void CCutWidget::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event)
    m_widthEdit->lineEdit()->setReadOnly(false);
    m_heightEdit->lineEdit()->setReadOnly(false);
}

void CCutWidget::initUI()
{
    _allWgts.clear();
    m_sizeWidget = new QWidget(this);
    QDesktopWidget *desktopWidget = dApp->desktop();
    bool withNotVarble = desktopWidget->screenGeometry().width() < 1152;
    DLabel *sizeLabel = new DLabel(m_sizeWidget);
    sizeLabel->setText(tr("Dimensions"));
    QFont ft;
    ft.setPixelSize(withNotVarble ? TEXT_SIZE - 2 : TEXT_SIZE);
    sizeLabel->setFont(ft);


    m_widthEdit = new DLineEdit(m_sizeWidget);
    m_widthEdit->setObjectName("CutWidthLineEdit");
    m_widthEdit->setText(QString::number(800));
    m_widthEdit->setClearButtonEnabled(false);
    m_widthEdit->setFixedWidth(withNotVarble ? 47 : 60);
    m_widthEdit->lineEdit()->setValidator(new CIntValidator(10, /*INT_MAX*/999999, this));
    m_widthEdit->setFont(ft);

    DLabel *multiLabel = new DLabel(m_sizeWidget);
    multiLabel->setText(tr("x"));
    multiLabel->setAlignment(Qt::AlignCenter);

    m_heightEdit = new DLineEdit(m_sizeWidget);
    m_heightEdit->setObjectName("CutHeightLineEdit");
    m_heightEdit->setText(QString::number(600));
    m_heightEdit->setClearButtonEnabled(false);
    m_heightEdit->setFixedWidth(withNotVarble ? 47 : 60);
    m_heightEdit->lineEdit()->setValidator(new CIntValidator(10, /*INT_MAX*/999999, this));
    m_heightEdit->setFont(ft);

    m_sizeLayout = new QHBoxLayout();
    m_sizeLayout->setSpacing(5);
    m_sizeLayout->setContentsMargins(0, 0, 0, 0);
    m_sizeLayout->addWidget(sizeLabel);
    m_sizeLayout->addWidget(m_widthEdit);
    m_sizeLayout->addWidget(multiLabel);
    m_sizeLayout->addWidget(m_heightEdit);
    DFontSizeManager::instance()->bind(sizeLabel, DFontSizeManager::T7, QFont::Normal);
    DFontSizeManager::instance()->bind(m_widthEdit, DFontSizeManager::T7, QFont::Normal);
    DFontSizeManager::instance()->bind(multiLabel, DFontSizeManager::T7, QFont::Normal);
    DFontSizeManager::instance()->bind(m_heightEdit, DFontSizeManager::T7, QFont::Normal);

    m_sizeWidget->setObjectName("sizeWidget");
    //m_sizeWidget->setMinimumWidth(210);
    m_sizeWidget->setLayout(m_sizeLayout);
    _allWgts << m_sizeWidget;

    if (Application::isTabletSystemEnvir()) {
        sizeLabel->hide();
        m_widthEdit->hide();
        multiLabel->hide();
        m_heightEdit->hide();
        _allWgts.removeOne(sizeLabel);
        _allWgts.removeOne(m_widthEdit);
        _allWgts.removeOne(multiLabel);
        _allWgts.removeOne(m_heightEdit);
    }

    m_SizeAddAction = new QAction(this);
    m_SizeAddAction->setObjectName("CutSizeKeyUp");
    m_SizeAddAction->setShortcut(QKeySequence(Qt::Key_Up));
    this->addAction(m_SizeAddAction);

    m_SizeReduceAction = new QAction(this);
    m_SizeReduceAction->setObjectName("CutSizeKeyDown");
    m_SizeReduceAction->setShortcut(QKeySequence(Qt::Key_Down));
    this->addAction(m_SizeReduceAction);

    DLabel *scaleLabel = new DLabel(this);
    scaleLabel->setText(tr("Aspect ratio"));
    _allWgts << scaleLabel;
    scaleLabel->setProperty(WidgetShowInVerWindow, false);

    scaleLabel->setFont(ft);

    QFont pushBtnFont;
    pushBtnFont.setPixelSize(PUSHBUTTON_FONT_SIZE);

    //m_scaleBtn1_1 = new QPushButton(this);
    m_scaleBtn1_1 = new ToolButton(this, BUTTON_STYLE);
    setWgtAccesibleName(m_scaleBtn1_1, "Cut ratio(1:1) pushbutton");
    m_scaleBtn1_1->setText("1:1");
    m_scaleBtn1_1->setFont(pushBtnFont);
    _allWgts << m_scaleBtn1_1;


    //m_scaleBtn2_3 = new QPushButton(this);
    m_scaleBtn2_3 = new ToolButton(this, BUTTON_STYLE);
    setWgtAccesibleName(m_scaleBtn2_3, "Cut ratio(2:3) pushbutton");
    m_scaleBtn2_3->setText("2:3");
    m_scaleBtn2_3->setFont(pushBtnFont);
    _allWgts << m_scaleBtn2_3;

    //m_scaleBtn8_5 = new QPushButton(this);
    m_scaleBtn8_5 = new ToolButton(this, BUTTON_STYLE);
    setWgtAccesibleName(m_scaleBtn8_5, "Cut ratio(8:5) pushbutton");
    m_scaleBtn8_5->setText("8:5");
    m_scaleBtn8_5->setFont(pushBtnFont);
    _allWgts << m_scaleBtn8_5;

    //m_scaleBtn16_9 = new QPushButton(this);
    m_scaleBtn16_9 = new ToolButton(this, BUTTON_STYLE);
    setWgtAccesibleName(m_scaleBtn16_9, "Cut ratio(16:9) pushbutton");
    m_scaleBtn16_9->setText("16:9");
    m_scaleBtn16_9->setFont(pushBtnFont);
    _allWgts << m_scaleBtn16_9;

    //m_freeBtn = new QPushButton(this);
    m_freeBtn = new ToolButton(this, BUTTON_STYLE);
    setWgtAccesibleName(m_freeBtn, "Cut ratio(free) pushbutton");
    m_freeBtn->setText(tr("Free"));
    m_freeBtn->setFont(pushBtnFont);
    _allWgts << m_freeBtn;

    //m_originalBtn = new QPushButton(this);
    m_originalBtn = new ToolButton(this, BUTTON_STYLE);
    setWgtAccesibleName(m_originalBtn, "Cut ratio(Original) pushbutton");
    m_originalBtn->setText(tr("Original"));
    m_originalBtn->setFont(pushBtnFont);
    _allWgts << m_originalBtn;

    //修复切换维语和藏语后,裁剪模式按钮大小不一致
    int unifyHeight = m_originalBtn->height() + 8;
    m_scaleBtn1_1->setFixedHeight(unifyHeight);
    m_scaleBtn2_3->setFixedHeight(unifyHeight);
    m_scaleBtn8_5->setFixedHeight(unifyHeight);
    m_scaleBtn16_9->setFixedHeight(unifyHeight);
    m_freeBtn->setFixedHeight(unifyHeight);
    m_originalBtn->setFixedHeight(unifyHeight);

    m_scaleBtn1_1->setCheckable(true);
    m_scaleBtn2_3->setCheckable(true);
    m_scaleBtn8_5->setCheckable(true);
    m_scaleBtn16_9->setCheckable(true);
    m_freeBtn->setCheckable(true);
    m_originalBtn->setCheckable(true);
    m_freeBtn->setChecked(true);

    m_sepLine = new SeperatorLine(this);
    _allWgts.append(m_sepLine);

    m_doneBtn = new ToolButton(this, BUTTON_STYLE);
    qobject_cast<ToolButton *>(m_doneBtn)->setShowText(false);
    setWgtAccesibleName(m_doneBtn, "Cut done pushbutton");
    //m_doneBtn->setFixedSize(QSize(38, 38));
    m_doneBtn->setMaximumHeight(unifyHeight);
    m_doneBtn->setIcon(QIcon::fromTheme("ddc_cutting_normal"));
    m_doneBtn->setIconSize(QSize(48, 48));
    m_doneBtn->setText(QObject::tr("OK"));


    //m_cancelBtn = new QPushButton(this);
    m_cancelBtn = new ToolButton(this, BUTTON_STYLE);
    qobject_cast<ToolButton *>(m_cancelBtn)->setShowText(false);
    setWgtAccesibleName(m_cancelBtn, "Cut cancel pushbutton");
    //m_cancelBtn->setFixedSize(QSize(38, 38));
    m_cancelBtn->setMaximumHeight(unifyHeight);
    m_cancelBtn->setIcon(QIcon::fromTheme("ddc_cancel_normal"));
    m_cancelBtn->setIconSize(QSize(48, 48));
    m_cancelBtn->setText(QObject::tr("Cancel"));

    m_sizeWidget->setProperty(AttriWidgetReWidth, QSize(210, unifyHeight));
    m_scaleBtn1_1->setProperty(WidgetAlignInVerWindow, 0);
    m_scaleBtn2_3->setProperty(WidgetAlignInVerWindow, 0);
    m_scaleBtn8_5->setProperty(WidgetAlignInVerWindow, 0);
    m_scaleBtn16_9->setProperty(WidgetAlignInVerWindow, 0);
    m_freeBtn->setProperty(WidgetAlignInVerWindow, 0);
    m_originalBtn->setProperty(WidgetAlignInVerWindow, 0);
    m_doneBtn->setProperty(WidgetAlignInVerWindow, 0);
    m_cancelBtn->setProperty(WidgetAlignInVerWindow, 0);

    m_scaleBtn1_1->setProperty(AttriWidgetReWidth, m_scaleBtn1_1->sizeHint());
    m_scaleBtn2_3->setProperty(AttriWidgetReWidth, m_scaleBtn2_3->sizeHint());
    m_scaleBtn8_5->setProperty(AttriWidgetReWidth, m_scaleBtn8_5->sizeHint());
    m_scaleBtn16_9->setProperty(AttriWidgetReWidth, m_scaleBtn16_9->sizeHint());
    m_freeBtn->setProperty(AttriWidgetReWidth, m_freeBtn->sizeHint());
    m_originalBtn->setProperty(AttriWidgetReWidth, m_originalBtn->sizeHint());
    m_doneBtn->setProperty(AttriWidgetReWidth, m_doneBtn->sizeHint());
    m_cancelBtn->setProperty(AttriWidgetReWidth, m_cancelBtn->sizeHint());

    m_scaleBtn1_1->setProperty(WidgetMarginInVerWindow, 0);
    m_scaleBtn2_3->setProperty(WidgetMarginInVerWindow, 0);
    m_scaleBtn8_5->setProperty(WidgetMarginInVerWindow, 0);
    m_scaleBtn16_9->setProperty(WidgetMarginInVerWindow, 0);
    m_freeBtn->setProperty(WidgetMarginInVerWindow, 0);
    m_originalBtn->setProperty(WidgetMarginInVerWindow, 0);
    m_doneBtn->setProperty(WidgetMarginInVerWindow, 0);
    m_cancelBtn->setProperty(WidgetMarginInVerWindow, 0);

    int minWidth = withNotVarble ? 47 : 60;
    m_scaleBtn1_1->setMinimumWidth(minWidth);
    m_scaleBtn2_3->setMinimumWidth(minWidth);
    m_scaleBtn8_5->setMinimumWidth(minWidth);
    m_scaleBtn16_9->setMinimumWidth(minWidth);
    m_freeBtn->setMinimumWidth(minWidth);
    m_originalBtn->setMinimumWidth(minWidth);
    m_doneBtn->setMinimumWidth(minWidth);
    m_cancelBtn->setMinimumWidth(minWidth);

    _allWgts << m_cancelBtn << m_doneBtn;

    QHBoxLayout *layout = static_cast<QHBoxLayout *>(centerLayout()) ;
    layout->setContentsMargins(0, 0, 0, 0);
    if (!Application::isTabletSystemEnvir()) {
        layout->addWidget(m_sizeWidget);
    }
    layout->addWidget(scaleLabel);
    layout->addWidget(m_scaleBtn1_1);
    layout->addWidget(m_scaleBtn2_3);
    layout->addWidget(m_scaleBtn8_5);
    layout->addWidget(m_scaleBtn16_9);
    layout->addWidget(m_freeBtn);
    layout->addWidget(m_originalBtn);

    layout->addWidget(m_sepLine);
    layout->addWidget(m_cancelBtn);
    layout->addWidget(m_doneBtn);

    m_scaleBtnGroup = new QButtonGroup(this);
    m_scaleBtnGroup->setExclusive(true);
    m_scaleBtnGroup->addButton(m_scaleBtn1_1, cut_1_1);
    m_scaleBtnGroup->addButton(m_scaleBtn2_3, cut_2_3);
    m_scaleBtnGroup->addButton(m_scaleBtn8_5, cut_8_5);
    m_scaleBtnGroup->addButton(m_scaleBtn16_9, cut_16_9);
    m_scaleBtnGroup->addButton(m_freeBtn, cut_free);
    m_scaleBtnGroup->addButton(m_originalBtn, cut_original);


    for (int i = 0; i < _allWgts.count(); ++i) {
        auto pw = _allWgts.at(i);
        DFontSizeManager::instance()->bind(pw, DFontSizeManager::T7, QFont::Normal);
        pw->installEventFilter(this);
    }
}

void CCutWidget::initConnection()
{
    connect(m_scaleBtnGroup, QOverload<int, bool>::of(&QButtonGroup::buttonToggled),
    this, [ = ](int tp, bool checked) {
        if (checked) {
            //修复最小化后,编辑后因焦点问题引起的,尺寸修改失败问题
            m_heightEdit->clearFocus();
            m_widthEdit->clearFocus();
            this->setCutType(ECutType(tp), true, m_autoCal);
        }
    });

    connect(m_widthEdit, &DLineEdit::editingFinished, this, [ = ]() {
        int newWidth = m_widthEdit->text().trimmed().toInt();
        int newHeight = m_heightEdit->text().trimmed().toInt();
        QSize maxSize = Application::drawApplication()->maxPicSize();
        if (newWidth > maxSize.width()) {
            newWidth = maxSize.width();
            m_widthEdit->blockSignals(true);
            m_widthEdit->setText(QString::number(newWidth));
            m_widthEdit->blockSignals(false);
        }

        qreal newRadio = qreal(newWidth) / qreal(newHeight);
        int count = sizeof(Radio) / sizeof(Radio[0]);
        ECutType tp = cut_free;
        for (int i = 0; i < count; ++i) {
            if (qFuzzyIsNull(Radio[i] - newRadio)) {
                tp = ECutType(i);
                break;
            }
        }
        //刷新画布，避免残影
        //CManageViewSigleton::GetInstance()->getCurView()->viewport()->update();
        this->setFocus();
        this->setCutType(ECutType::cut_free);
        this->setCutSize(QSize(newWidth, newHeight));
    });

    connect(m_heightEdit, &DLineEdit::editingFinished, this, [ = ]() {
        int newWidth = m_widthEdit->text().trimmed().toInt();
        int newHeight = m_heightEdit->text().trimmed().toInt();

        QSize maxSize = Application::drawApplication()->maxPicSize();
        if (newHeight > maxSize.height()) {
            newHeight = maxSize.height();
            m_heightEdit->blockSignals(true);
            m_heightEdit->setText(QString::number(newHeight));
            m_heightEdit->blockSignals(false);
        }

        qreal newRadio = qreal(newWidth) / qreal(newHeight);
        int count = sizeof(Radio) / sizeof(Radio[0]);
        ECutType tp = cut_free;
        for (int i = 0; i < count; ++i) {
            if (qFuzzyIsNull(Radio[i] - newRadio)) {
                tp = ECutType(i);
                break;
            }
        }
        //刷新画布，避免残影
        //CManageViewSigleton::GetInstance()->getCurView()->viewport()->update();

        this->setFocus();
        this->setCutType(ECutType::cut_free);
        this->setCutSize(QSize(newWidth, newHeight));
    });

    connect(m_doneBtn, &QPushButton::clicked, this, [ = ]() {
        if (m_widthEdit->lineEdit()->hasFocus()) {
            m_widthEdit->lineEdit()->clearFocus();
        }

        if (m_heightEdit->lineEdit()->hasFocus()) {
            m_heightEdit->lineEdit()->clearFocus();
        }
        emit finshed(true);
    });

    connect(m_cancelBtn, &QPushButton::clicked, this, [ = ]() {
        emit finshed(false);
    });

    connect(m_SizeAddAction, &QAction::triggered, this, [ = ](bool) {
        if (m_widthEdit->lineEdit()->hasFocus()) {
            int widthSizeNum = m_widthEdit->lineEdit()->text().trimmed().toInt();
            widthSizeNum++;
            if (widthSizeNum > 4096) {
                return;
            }
            QString text = QString::number(widthSizeNum);
            m_widthEdit->setText(text);
            emit m_widthEdit->lineEdit()->textEdited(text);
        } else if (m_heightEdit->lineEdit()->hasFocus()) {
            int heightSizeNum = m_heightEdit->lineEdit()->text().trimmed().toInt();
            heightSizeNum++;
            if (heightSizeNum > 4096) {
                return;
            }
            QString text = QString::number(heightSizeNum);
            m_heightEdit->setText(text);
            emit m_heightEdit->lineEdit()->textEdited(text);
        }
    });

    connect(m_SizeReduceAction, &QAction::triggered, this, [ = ](bool) {
        if (m_widthEdit->lineEdit()->hasFocus()) {
            int widthSizeNum = m_widthEdit->lineEdit()->text().trimmed().toInt();
            widthSizeNum--;
            if (widthSizeNum < 10) {
                return;
            }
            QString text = QString::number(widthSizeNum);
            m_widthEdit->setText(text);
            emit m_widthEdit->lineEdit()->textEdited(text);
        } else if (m_heightEdit->lineEdit()->hasFocus()) {
            int heightSizeNum = m_heightEdit->lineEdit()->text().trimmed().toInt();
            heightSizeNum--;
            if (heightSizeNum < 10) {
                return;
            }
            QString text = QString::number(heightSizeNum);
            m_heightEdit->setText(text);
            emit m_heightEdit->lineEdit()->textEdited(text);
        }
    });
}
