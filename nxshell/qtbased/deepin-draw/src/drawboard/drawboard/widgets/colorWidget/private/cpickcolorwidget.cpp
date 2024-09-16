// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cpickcolorwidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QApplication>
#include <QDBusInterface>
#include <QDebug>
#include <QSizePolicy>
#include <QRegExpValidator>

#include "utils/global.h"
#include "service/colorpickerinterface.h"
#include "ciconbutton.h"
#include "calphacontrolwidget.h"

#ifdef USE_DTK
#include <DGuiApplicationHelper>
#include <DArrowLineExpand>
DGUI_USE_NAMESPACE
#endif

//const QSize PICKCOLOR_WIDGET_SIZE = QSize(314, 285);
//const QSize PICKCOLOR_LABEL_SIZE = QSize(45, 36);
const QColor PICK_TITLE_COLOR = QColor(0, 0, 0);
const qreal PICK_TITLE_COLOR_ALPHA = 0.4;
const int CONST_LABEL_HEIGHT = 36;
const int OFFSET_EXPEND_HEADER = 15;
PickColorWidget::PickColorWidget(QWidget *parent, bool bUseOldUi)
    : QWidget(parent)
{
    if (bUseOldUi) {
        initOldUi();
    } else {
        initUI();
    }
    initConnects();
}

void PickColorWidget::updateColor(const QColor &color)
{
    QColor c = color.isValid() ? color : curColor;
    m_redEditLabel->setText(QString("%1").arg(c.red()));
    m_greenEditLabel->setText(QString("%1").arg(c.green()));
    m_blueEditLabel->setText(QString("%1").arg(c.blue()));

    if (color.isValid()) {
        //证明是预览发出信号通知外界
        emit previewedColorChanged(color);
    }

    QString colorName = "";
    if (c.name().contains("#")) {
        colorName = c.name().split("#").last();
    }

    m_hexLineEdit->blockSignals(true);
    m_hexLineEdit->setText(colorName);
    m_hexLineEdit->blockSignals(false);
}

void PickColorWidget::initUI()
{
    //this->setStyleSheet("background-color:red;");
    //颜色选择lable
    m_colorLabel = new ColorLabel(this);
    m_colorLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    //颜色滑块
    m_colorSlider = new ColorSlider(this);

    //透明度
    m_alphaControlWidget = new CAlphaControlWidget(this);
    m_alphaControlWidget->setObjectName("CAlphaControlWidget");
    m_alphaControlWidget->setFocusPolicy(Qt::NoFocus);

    //颜色数字输入
    m_hexLineEdit = new DLineEdit(this);
    m_hexLineEdit->setObjectName("ColorLineEdit");
    m_hexLineEdit->setClearButtonEnabled(false);
    m_hexLineEdit->setFocusPolicy(Qt::FocusPolicy::StrongFocus);

#ifdef USE_DTK
    m_hexLineEdit->lineEdit()->setValidator(new QRegExpValidator(QRegExp("[0-9A-Fa-f]{6}"), this));
#else
    m_hexLineEdit->setValidator(new QRegExpValidator(QRegExp("[0-9A-Fa-f]{6}"), this));
#endif
    m_hexLineEdit->setText("#ffffff");
    m_redEditLabel = new EditLabel(this);
    m_greenEditLabel = new EditLabel(this);
    m_blueEditLabel = new EditLabel(this);
    //取色器使用系统托管icon方式设置图标
    QMap<int, QMap<CIconButton::EIconButtonSattus, QString>> pictureMap;
    m_picker = new CIconButton(pictureMap, QSize(55, 36), this, false);
    m_picker->setIconMode();
    m_picker->setIconSize(QSize(36, 36));
    m_picker->setIcon(QIcon::fromTheme("dorpper_normal"));

    QList<QWidget *> editList;
    editList << m_hexLineEdit << m_redEditLabel << m_greenEditLabel << m_blueEditLabel << m_picker;
    //m_hexEditLabel->setStyleSheet("background-color:red;");
    //颜色R G B显示
    QLabel *hexLabel = new QLabel("HEX", this);
    QLabel *rLabel = new QLabel("R", this);
    QLabel *gLabel = new QLabel("G", this);
    QLabel *bLabel = new QLabel("B", this);
    QLabel *strawLabel = new QLabel(tr(""), this);
    strawLabel->setFixedSize(55, 36);
    QList<QLabel *> labelList;
    labelList << hexLabel << rLabel << gLabel << bLabel << strawLabel;

    QGridLayout *rgbLayout = new QGridLayout();
    rgbLayout->setContentsMargins(0, 0, 0, 0);
    QLabel *l = nullptr;
    QFont labelFont;

    for (int i = 0; i < labelList.size(); ++i) {
        l = labelList.at(i);
        labelFont = l->font();
        labelFont.setPixelSize(13);
        l->setFont(labelFont);
        l->setAlignment(Qt::AlignCenter);

        EditLabel *p = dynamic_cast<EditLabel *>(editList.at(i));
        if (p != nullptr) {
#ifdef USE_DTK
            p->lineEdit()->setAlignment(Qt::AlignCenter);
#else
            p->setAlignment(Qt::AlignCenter);
#endif
        }
        rgbLayout->addWidget(editList.at(i), 0, i);
        rgbLayout->addWidget(l, 1, i);
    }

    QVBoxLayout *mLayout = new QVBoxLayout;
    mLayout->setContentsMargins(0, 0, 0, 0);
    mLayout->addWidget(m_colorLabel, 1);
    mLayout->addWidget(m_colorSlider);
    mLayout->addWidget(m_alphaControlWidget);
    mLayout->addLayout(rgbLayout);

    setLayout(mLayout);
}

void PickColorWidget::initConnects()
{
    m_cp = new ColorPickerInterface("com.deepin.Picker",
                                    "/com/deepin/Picker", QDBusConnection::sessionBus(), this);

    connect(m_cp, &ColorPickerInterface::colorPicked, this, [ = ](QString uuid, QString colorName) {
        if (uuid == QString("%1").arg(qApp->applicationPid())) {
            setColor(QColor(colorName));
        }
        m_picker->setChecked(false);
    });


    //设置透明度
    connect(m_alphaControlWidget, &CAlphaControlWidget::alphaChanged, this, [ = ](int apl, EChangedPhase phase) {
        QColor c = color();
        c.setAlpha(apl);
        qDebug() << "alphaChanged apl = " << c.alpha();
        this->setColor(c, true, phase);
    });

    connect(m_colorSlider, &ColorSlider::valueChanged, m_colorLabel, [ = ](int val) {
        m_colorLabel->setHue(val);
    });
    connect(m_colorLabel, &ColorLabel::pickedColor, this, [ = ](QColor color) {
        setColor(color);
    });

    connect(m_colorLabel, &ColorLabel::signalPreViewColor, this, [ = ](QColor color) {
        updateColor(color);
    });

    connect(m_picker, &CIconButton::mouseRelease, this, [ = ] {
        m_cp->StartPick(QString("%1").arg(qApp->applicationPid()));
    });

    connect(m_hexLineEdit, &DLineEdit::textChanged, this, [ = ](const QString & colorStr) {
        QColor c;
        QString colorStrname;
        if (colorStr.size() == 6) {
            colorStrname = "#" + colorStr;
        } else if (colorStr.size() == 7) {
            colorStrname = colorStr;
        }
        c.setNamedColor(colorStrname);
        if (c.isValid()) {
            // 去除透明度，当前颜色不同才会更新显示颜色
            if (c.rgb() == color().rgb()) {
                return;
            }
            setColor(c, true);
        }

    });

    connect(m_hexLineEdit, &DLineEdit::editingFinished, this, [ = ]() {
        QString colorStr = m_hexLineEdit->text();
        if (colorStr.size() == 6) {
            QColor c("#" + colorStr);
            if (c.isValid()) {
                int alpha = m_alphaControlWidget->alpha();
                c.setAlpha(alpha);
                this->setColor(c, true, EChanged);
            }
        }
    });
}

void PickColorWidget::initOldUi()
{
    //颜色选择lable
    m_colorLabel = new ColorLabel(this);
    m_colorLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //颜色滑块
    m_colorSlider = new ColorSlider(this);

    //透明度
    m_alphaControlWidget = new CAlphaControlWidget(this);
    m_alphaControlWidget->setObjectName("CAlphaControlWidget");
    m_alphaControlWidget->setFocusPolicy(Qt::NoFocus);

    //颜色数字输入
    m_hexLineEdit = new DLineEdit(this);
    m_hexLineEdit->setFixedHeight(CONST_LABEL_HEIGHT);
    m_hexLineEdit->setObjectName("ColorLineEdit");
    m_hexLineEdit->setClearButtonEnabled(false);
    m_hexLineEdit->setFocusPolicy(Qt::FocusPolicy::StrongFocus);

#ifdef USE_DTK
    m_hexLineEdit->lineEdit()->setValidator(new QRegExpValidator(QRegExp("[0-9A-Fa-f]{6}"), this));
#else
    m_hexLineEdit->setValidator(new QRegExpValidator(QRegExp("[0-9A-Fa-f]{6}"), this));
#endif
    m_hexLineEdit->lineEdit()->setMaxLength(7);
    m_hexLineEdit->setText("#ffffff");
    m_redEditLabel = new EditLabel(this);
    m_greenEditLabel = new EditLabel(this);
    m_blueEditLabel = new EditLabel(this);
    //取色器使用系统托管icon方式设置图标
    QMap<int, QMap<CIconButton::EIconButtonSattus, QString>> pictureMap;
    m_picker = new CIconButton(pictureMap, QSize(55, 36), this, false);
    m_picker->setIconMode();
    m_picker->setIconSize(QSize(15, 15));
    m_picker->setIcon(QIcon::fromTheme("colorpicker"));

    QList<QWidget *> editList;
    editList << m_picker << m_hexLineEdit << m_redEditLabel << m_greenEditLabel << m_blueEditLabel ;
    //m_hexEditLabel->setStyleSheet("background-color:red;");
    //颜色R G B显示
    QLabel *hexLabel = new QLabel("#", this);
    QLabel *rLabel = new QLabel("R", this);
    QLabel *gLabel = new QLabel("G", this);
    QLabel *bLabel = new QLabel("B", this);
    QLabel *strawLabel = new QLabel(tr(""), this);
    strawLabel->setFixedSize(55, 36);
    QList<QLabel *> labelList;
    labelList << strawLabel << hexLabel << rLabel << gLabel << bLabel;

    QGridLayout *rgbLayout = new QGridLayout();
    rgbLayout->setContentsMargins(0, 0, 0, 0);
    QLabel *l = nullptr;
    QFont labelFont;
    for (int i = 0; i < labelList.size(); ++i) {
        l = labelList.at(i);
        l->setFixedHeight(CONST_LABEL_HEIGHT);
        labelFont = l->font();
        labelFont.setPixelSize(13);
        l->setFont(labelFont);
        l->setAlignment(Qt::AlignCenter);
        l->setEnabled(false);

        EditLabel *p = dynamic_cast<EditLabel *>(editList.at(i));
        if (p != nullptr) {
            p->setFixedHeight(CONST_LABEL_HEIGHT);
#ifdef USE_DTK
            p->lineEdit()->setAlignment(Qt::AlignCenter);
#else
            p->setAlignment(Qt::AlignCenter);
#endif
        }
        rgbLayout->addWidget(editList.at(i), 0, i);
        rgbLayout->addWidget(l, 1, i);
    }
    rgbLayout->setVerticalSpacing(0);

    DArrowLineExpand *expand = new DArrowLineExpand;
    expand->setExpandedSeparatorVisible(false);
    QWidget *w = new QWidget(this);
    w->setFixedHeight(150);//不设置expand content 下面会有空白
    //分割线
    auto Splitline = new QFrame(this);
    Splitline->setFrameShape(QFrame::HLine);

    QVBoxLayout *expandLayout = new QVBoxLayout(w);
    expandLayout->setContentsMargins(0, 0, 0, 10);
    w->setLayout(expandLayout);
    expandLayout->addWidget(m_colorLabel, 1);
    expandLayout->addWidget(m_colorSlider);
    expand->setContent(w);
    expand->setExpand(true);
    expand->setSeparatorVisible(false);
    expand->setAnimationDuration(100);

    QLabel *expand_header = new QLabel(this);
    QPalette pe;
    QColor expand_color(PICK_TITLE_COLOR);
    expand_color.setAlphaF(PICK_TITLE_COLOR_ALPHA);
    pe.setColor(QPalette::WindowText, expand_color);
    expand_header->setPalette(pe);
    expand_header->setFont(QFont("SourceHanSansSC", 10));
    expand_header->setText(tr("Color picker"));
    expand_header->move(pos().x(), w->height() - OFFSET_EXPEND_HEADER);


    QVBoxLayout *mLayout = new QVBoxLayout;
    mLayout->setContentsMargins(0, 10, 0, 0);
    mLayout->addWidget(m_alphaControlWidget);
    mLayout->addLayout(rgbLayout);
    mLayout->addWidget(Splitline);
    mLayout->addStretch(10);
    mLayout->addWidget(expand);
    setLayout(mLayout);
}

void PickColorWidget::setTheme(int theme)
{
    m_picker->setTheme(theme);
}

PickColorWidget::~PickColorWidget()
{
}

QColor PickColorWidget::color()
{
    return curColor;
}

void PickColorWidget::setColor(const QColor &c, bool internalChanged)
{
    bool changed = (c != curColor);

    if (changed) {
        curColor = c;

        updateColor();

        if (internalChanged) {
            emit colorChanged(curColor);
        }
    }

    m_colorLabel->setSelectColor(c);
    m_hexLineEdit->setText(c.name());
}

void PickColorWidget::setAlpha(int alpha, bool internalChanged)
{
    m_alphaControlWidget->setAlpha(alpha, internalChanged);
}

void PickColorWidget::setColor(const QColor &c,
                               bool internalChanged,
                               EChangedPhase phase)
{
    if (phase == EChangedFinished || phase == EChanged)
        curColor = c;

    updateColor(c);

    if (internalChanged) {
        emit colorChanged(c, phase);
    }

}

void PickColorWidget::hideEvent(QHideEvent *event)
{
    setFocus(Qt::NoFocusReason);
    QWidget::hideEvent(event);
}

