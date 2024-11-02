#include "colordialog.h"

ColorDialog::ColorDialog(QWidget *parent) : QDialog(parent)
{
    this->setFixedSize(316, 374);
    // 窗口属性
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);//开启窗口无边框
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setAttribute(Qt::WA_TranslucentBackground);

    m_Layout = new QVBoxLayout(this);

    m_frame1 = new QFrame(this);
    m_frame1->setFixedHeight(24);

    m_topLayout = new QHBoxLayout(m_frame1);
    m_topLayout->setSpacing(24);
    m_topLayout->setContentsMargins(-1, 0, 0, 0);
    m_TopSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    m_closeBtn = new QPushButton(m_frame1);
    m_closeBtn->setFixedSize(QSize(16, 16));
    m_closeBtn->setProperty("isWindowButton", 0x02);
    m_closeBtn->setProperty("useIconHighlightEffect", 0x08);
    m_closeBtn->setFlat(true);
    m_closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    m_closeBtn->setToolTip(tr("Close"));

    // 颜色选择区域
    m_colorSquare = new ColorSquare(this);
    QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_colorSquare->setSizePolicy(sizePolicy);

    // 遮罩
    MaskWidget * maskWidget = new MaskWidget(m_colorSquare);
    maskWidget->setGeometry(0, 0, m_colorSquare->width(), m_colorSquare->height());

    m_frame2 = new QFrame(this);
    m_midLayout = new QHBoxLayout(m_frame2);

    // 水平渐变滑动条
    m_gradientSlider = new GradientSlider(m_frame2);
    m_gradientSlider->setOrientation(Qt::Horizontal); //设置滑块方向

//    m_colorPicker = new QPushButton(m_frame2);
//    m_colorPicker->setFocusPolicy(Qt::NoFocus);
//    m_colorPicker->setFixedSize(24, 24);
//    m_colorPicker->setIconSize(QSize(16, 16));
//    m_colorPicker->setIcon(QIcon(":/res/res/ai/colorpicker.svg"));
//    m_colorPicker->setStyleSheet("QPushButton{border:1px solid rgb(190, 190, 190);"
//                                 "border-radius: 4px;}");

    m_frame3 = new QFrame(this);
    m_bottomLayout = new QHBoxLayout(m_frame3);

    m_hexLayout = new QVBoxLayout();
    m_redLayout = new QVBoxLayout();
    m_greenLayout = new QVBoxLayout();
    m_blueLayout = new QVBoxLayout();

    m_hexLabel = new QLabel(m_frame3);
    m_hexLabel->setText("HEX");
    m_hexLabel->setAlignment(Qt::AlignCenter);

    m_colorLineEdit = new QLineEdit(m_frame3);
    m_colorLineEdit->setFixedSize(90, 36);
    m_colorLineEdit->setFocusPolicy(Qt::StrongFocus);
    m_colorLineEdit->setAlignment(Qt::AlignCenter);

    m_labelR = new QLabel(m_frame3);
    m_labelR->setText("R");
    m_labelR->setAlignment(Qt::AlignCenter);
    m_spinBoxR = new QSpinBox(m_frame3);
    m_spinBoxR->setFixedSize(50, 36);
    m_spinBoxR->setAlignment(Qt::AlignCenter);
    m_spinBoxR->setButtonSymbols(QAbstractSpinBox::NoButtons);

    m_labelG = new QLabel(m_frame3);
    m_labelG->setText("G");
    m_labelG->setAlignment(Qt::AlignCenter);
    m_spinBoxG = new QSpinBox(m_frame3);
    m_spinBoxG->setFixedSize(50, 36);
    m_spinBoxG->setAlignment(Qt::AlignCenter);
    m_spinBoxG->setButtonSymbols(QAbstractSpinBox::NoButtons);

    m_labelB = new QLabel(m_frame3);
    m_labelB->setText("B");
    m_labelB->setAlignment(Qt::AlignCenter);
    m_spinBoxB = new QSpinBox(m_frame3);
    m_spinBoxB->setFixedSize(50, 36);
    m_spinBoxB->setAlignment(Qt::AlignCenter);
    m_spinBoxB->setButtonSymbols(QAbstractSpinBox::NoButtons);

    this->setupInit();
    this->initLayout();
    this->signalsBind();
}

ColorDialog::~ColorDialog()
{

}

void ColorDialog::initLayout()
{
    m_topLayout->addItem(m_TopSpacer);
    m_topLayout->addWidget(m_closeBtn);

    m_midLayout->addWidget(m_gradientSlider);
//    m_midLayout->setSpacing(20);
//    m_midLayout->addWidget(m_colorPicker);
    m_midLayout->setContentsMargins(8, 0, 8, 0);

    m_hexLayout->addWidget(m_colorLineEdit);
    m_hexLayout->addWidget(m_hexLabel);
    m_redLayout->addWidget(m_spinBoxR);
    m_redLayout->addWidget(m_labelR);
    m_greenLayout->addWidget(m_spinBoxG);
    m_greenLayout->addWidget(m_labelG);
    m_blueLayout->addWidget(m_spinBoxB);
    m_blueLayout->addWidget(m_labelB);

    m_bottomLayout->addLayout(m_hexLayout);
    m_bottomLayout->addLayout(m_redLayout);
    m_bottomLayout->addLayout(m_greenLayout);
    m_bottomLayout->addLayout(m_blueLayout);
    m_bottomLayout->setContentsMargins(8, 0, 8, 0);

    m_Layout->addWidget(m_frame1);
    m_Layout->addWidget(m_colorSquare, 0, Qt::AlignCenter);
    m_Layout->addWidget(m_frame2);
    m_Layout->addWidget(m_frame3);
    m_Layout->setContentsMargins(16, 16, 16, 16);

    this->setLayout(m_Layout);
}

void ColorDialog::setupInit()
{
    // spinbox
    // α:
    spinAlpha = new QSpinBox(this);
    spinAlpha->setVisible(false);
    spinAlpha->setMaximum(100);
    sliderAlpha = new GradientSlider(this);
    sliderAlpha->setVisible(false);
    sliderAlpha->setMaximum(100);
    sliderAlpha->setOrientation(Qt::Vertical);
    // H:
    spinHue = new QSpinBox(this);
    spinHue->setVisible(false);
    spinHue->setMaximum(359);
    sliderHue = new GradientSlider(this);
    sliderHue->setVisible(false);
    sliderHue->setMaximum(359);
    sliderHue->setOrientation(Qt::Vertical);

    QVector<QColor> rainbow;
    for ( int i = 0; i < 360; i+= 360/6 )
        rainbow.push_back(QColor::fromHsv(i,255,255));
    rainbow.push_back(Qt::red);
    sliderHue->setColors(rainbow);
    // S:
    spinSat = new QSpinBox(this);
    spinSat->setVisible(false);
    spinSat->setMaximum(100);
    sliderSat = new GradientSlider(this);
    sliderSat->setVisible(false);
    sliderSat->setMaximum(100);
    sliderSat->setOrientation(Qt::Vertical);
    // V:
    spinVal = new QSpinBox(this);
    spinVal->setVisible(false);
    spinVal->setMaximum(100);
    sliderVal = new GradientSlider(this);
    sliderVal->setVisible(false);
    sliderVal->setMaximum(100);
    sliderVal->setOrientation(Qt::Vertical);
    // R:
    spinRed = m_spinBoxR;
    spinRed->setMaximum(255);
    sliderRed = new GradientSlider(this);
    sliderRed->setVisible(false);
    sliderRed->setMaximum(255);
    sliderRed->setOrientation(Qt::Vertical);
    // G:
    spinGreen = m_spinBoxG;
    spinGreen->setMaximum(255);
    sliderGreen = new GradientSlider(this);
    sliderGreen->setVisible(false);
    sliderGreen->setMaximum(255);
    sliderGreen->setOrientation(Qt::Vertical);
    // B:
    spinBlue = m_spinBoxB;
    spinBlue->setMaximum(255);
    sliderBlue = new GradientSlider(this);
    sliderBlue->setVisible(false);
    sliderBlue->setMaximum(255);
    sliderBlue->setOrientation(Qt::Vertical);

    // 绘制滑动条
    drawSlider();
}

void ColorDialog::signalsBind()
{
    connect(m_closeBtn, &QPushButton::clicked, [=](bool checked){
        Q_UNUSED(checked)
        hide();
    });

    connect(sliderAlpha,&GradientSlider::valueChanged,spinAlpha,&QSpinBox::setValue);
    connect(spinAlpha,static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            sliderAlpha,&GradientSlider::setValue);
    connect(sliderHue,&GradientSlider::valueChanged,spinHue,&QSpinBox::setValue);
    connect(spinHue,static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            sliderHue,&GradientSlider::setValue);
    connect(sliderSat,&GradientSlider::valueChanged,spinSat,&QSpinBox::setValue);
    connect(spinSat,static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            sliderSat,&GradientSlider::setValue);
    connect(sliderVal,&GradientSlider::valueChanged,spinVal,&QSpinBox::setValue);
    connect(spinVal,static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            sliderVal,&GradientSlider::setValue);
    connect(sliderRed,&GradientSlider::valueChanged,spinRed,&QSpinBox::setValue);
    connect(spinRed,static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            sliderRed,&GradientSlider::setValue);
    connect(sliderGreen,&GradientSlider::valueChanged,spinGreen,&QSpinBox::setValue);
    connect(spinGreen,static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            sliderGreen,&GradientSlider::setValue);
    connect(sliderBlue,&GradientSlider::valueChanged,spinBlue,&QSpinBox::setValue);
    connect(spinBlue,static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            sliderBlue,&GradientSlider::setValue);

    connect(sliderHue,&GradientSlider::valueChanged,this,&ColorDialog::SetHsvSlot);
    connect(sliderSat,&GradientSlider::valueChanged,this,&ColorDialog::SetHsvSlot);
    connect(sliderVal,&GradientSlider::valueChanged,this,&ColorDialog::SetHsvSlot);
    connect(sliderRed,&GradientSlider::valueChanged,this,&ColorDialog::setRgbSlot);
    connect(sliderGreen,&GradientSlider::valueChanged,this,&ColorDialog::setRgbSlot);
    connect(sliderBlue,&GradientSlider::valueChanged,this,&ColorDialog::setRgbSlot);

    connect(sliderAlpha,&GradientSlider::valueChanged,this,&ColorDialog::updateWidgetsSlot);
    connect(m_colorSquare,&ColorSquare::colorSelected,this,&ColorDialog::updateWidgetsSlot);
    connect(m_colorSquare,&ColorSquare::colorSelected,this,&ColorDialog::colorSelectedSolt);
    connect(this,&ColorDialog::checkedChanged,m_colorSquare,&ColorSquare::setCheckedColorSlot);

    connect(m_colorLineEdit,&QLineEdit::textChanged,this,[=](){
        QColor mcolor;
        mcolor.setNamedColor(m_colorLineEdit->text());
        m_spinBoxR->setValue(mcolor.red());
        m_spinBoxB->setValue(mcolor.blue());
        m_spinBoxG->setValue(mcolor.green());
    });
}

void ColorDialog::drawSlider()
{
    QVector<QColor> rainbow;
    for ( int i = 0; i < 360; i+= 360/6 )
        rainbow.push_back(QColor::fromHsv(i,255,255));
    rainbow.push_back(Qt::red);
    m_gradientSlider->setMaximum(sliderHue->maximum());
    m_gradientSlider->setColors(rainbow);
    m_gradientSlider->setValue(sliderHue->value());
    connect(m_gradientSlider, &GradientSlider::valueChanged, sliderHue, &GradientSlider::setValue);
    Q_EMIT checkedChanged('H');
}

QColor ColorDialog::color() const
{
    QColor color = m_colorSquare->color();
    return color;
}

void ColorDialog::SetColor(QColor color)
{
    m_colorSquare->setColor(color);
    sliderAlpha->setValue(color.alpha() / 2.55);
    updateWidgetsSlot();
}


/****************Slots*********************/

void ColorDialog::colorSelectedSolt()
{
    QColor color = this->color();
    Q_EMIT colorSelected(color);
}

void ColorDialog::setRgbSlot()
{
    if(!signalsBlocked())
    {
        QColor color(sliderRed->value(), sliderGreen->value(), sliderBlue->value());
        if(color.saturation() == 0)
        {
            color = QColor::fromHsv(sliderHue->value(), 0, color.value());
        }
        m_colorSquare->setColor(color);
        updateWidgetsSlot();
    }
}

void ColorDialog::SetHsvSlot()
{
    if(!signalsBlocked())
    {
        QColor color = QColor::fromHsv(m_gradientSlider->value() * 360/m_gradientSlider->maximum(), 100, 100);
        m_colorSquare->setColor(color);
        updateWidgetsSlot();
    }
}

void ColorDialog::updateWidgetsSlot()
{
    blockSignals(true);
    for(QWidget* w: findChildren<QWidget*>())
        w->blockSignals(true);

    QColor col = color();

    sliderRed->setValue(col.red());
    spinRed->setValue(sliderRed->value());
    sliderRed->setFirstColor(QColor(0,col.green(),col.blue()));
    sliderRed->setLastColor(QColor(255,col.green(),col.blue()));

    sliderGreen->setValue(col.green());
    spinGreen->setValue(sliderGreen->value());
    sliderGreen->setFirstColor(QColor(col.red(),0,col.blue()));
    sliderGreen->setLastColor(QColor(col.red(),255,col.blue()));

    sliderBlue->setValue(col.blue());
    spinBlue->setValue(sliderBlue->value());
    sliderBlue->setFirstColor(QColor(col.red(),col.green(),0));
    sliderBlue->setLastColor(QColor(col.red(),col.green(),255));

    sliderHue->setValue(qRound(m_colorSquare->hue()*360.0));
    spinHue->setValue(sliderHue->value());

    sliderSat->setValue(qRound(m_colorSquare->saturation()*100.0));
    spinSat->setValue(sliderSat->value());
    sliderSat->setFirstColor(QColor::fromHsvF(m_colorSquare->hue(),0,m_colorSquare->value()));
    sliderSat->setLastColor(QColor::fromHsvF(m_colorSquare->hue(),1,m_colorSquare->value()));

    sliderVal->setValue(qRound(m_colorSquare->value()*100.0));
    spinVal->setValue(sliderVal->value());
    sliderVal->setFirstColor(QColor::fromHsvF(m_colorSquare->hue(),m_colorSquare->saturation(),0));
    sliderVal->setLastColor(QColor::fromHsvF(m_colorSquare->hue(),m_colorSquare->saturation(),1));

    if (!m_colorLineEdit->hasFocus()) {
        m_colorLineEdit->blockSignals(true);
        m_colorLineEdit->setText(col.name().toUpper());
        m_colorLineEdit->blockSignals(false);
    }
    setHorizontalSlider();

    blockSignals(false);
    for(QWidget* w: findChildren<QWidget*>())
        w->blockSignals(false);

    Q_EMIT colorChanged(col);
}

void ColorDialog::setHorizontalSlider()
{
    disconnect(m_gradientSlider, SIGNAL(valueChanged(int)), sliderHue, SLOT(setValue(int)));
    disconnect(m_gradientSlider, SIGNAL(valueChanged(int)), sliderSat, SLOT(setValue(int)));
    disconnect(m_gradientSlider, SIGNAL(valueChanged(int)), sliderVal, SLOT(setValue(int)));
    disconnect(m_gradientSlider, SIGNAL(valueChanged(int)), sliderRed, SLOT(setValue(int)));
    disconnect(m_gradientSlider, SIGNAL(valueChanged(int)), sliderGreen, SLOT(setValue(int)));
    disconnect(m_gradientSlider, SIGNAL(valueChanged(int)), sliderBlue, SLOT(setValue(int)));


    QVector<QColor> rainbow;
    for ( int i = 0; i < 360; i+= 360/6 )
        rainbow.push_back(QColor::fromHsv(i,255,255));
    rainbow.push_back(Qt::red);
    m_gradientSlider->setMaximum(sliderHue->maximum());
    m_gradientSlider->setColors(rainbow);
    m_gradientSlider->setValue(sliderHue->value());

    connect(m_gradientSlider, SIGNAL(valueChanged(int)), sliderHue, SLOT(setValue(int)));
    Q_EMIT checkedChanged('H');
}

void ColorDialog::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QPainterPath path;
    path.addRoundedRect(rect(), 10, 10);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillPath(path, QColor(250, 250, 250));
}
