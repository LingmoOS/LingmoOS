#include "brushsettings.h"
#include <QFontDatabase>
#include "global/interactiveqml.h"
BrushSettings::BrushSettings(QWidget *parent) : QWidget(parent)
{
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setFocusPolicy(Qt::NoFocus);


    m_triangleWidth = TRIANGLE_WIDTH;
    m_triangleHeight = TRIANGLE_HEIGHT;

    this->init();
    this->setButtonGroup();
    this->initConnect();
    this->setComboxSize();
    this->initGsetting();
}
//初始化控件
void BrushSettings::init()
{
    //按钮组
    m_buttonGroupLeft = new QButtonGroup(this);
    m_buttonGroupRight = new QButtonGroup(this);
    //画笔粗细
    m_smallest = new PainterThick(3, false, QColor(), this);
    m_thin = new PainterThick(5, false, QColor(), this);
    m_medium = new PainterThick(9, false, QColor(), this);
    m_medium->setChecked(true);
    m_thick = new PainterThick(13, false, QColor(), this);
    //左边文字控件
    m_fontComBox = new QComboBox(this);
    m_fontSizeComBox = new QSpinBox(this);
    m_fontSizeComBox->setFocusPolicy(Qt::NoFocus);

    m_deleteLine = new QCheckBox(this);
    m_deleteLine->setFixedSize(34, 18);
    m_underLine = new QCheckBox(this);
    m_underLine->setFixedSize(33, 18);
    m_bold = new QCheckBox(this);
    m_bold->setFixedSize(32, 18);
    m_italics = new QCheckBox(this);
    m_italics->setFixedSize(32, 18);
    //分割线
    m_line = new QLabel(this);
    m_line->setFixedSize(TOOL_LINE.width(), TOOL_LINE.height() - 4);
    //画笔颜色
    m_red = new PainterThick(14, true, QColor(219, 2, 15), this);
    m_red->setChecked(true);
    m_blue = new PainterThick(14, true, QColor(36, 87, 217), this);
    m_green = new PainterThick(14, true, QColor(85, 209, 57), this);
    m_yellow = new PainterThick(14, true, QColor(211, 154, 44), this);
    m_pink = new PainterThick(14, true, QColor(251, 66, 136), this);
    m_black = new PainterThick(14, true, QColor(25, 25, 25), this);
    m_grey = new PainterThick(14, true, QColor(98, 102, 113), this);
    m_white = new PainterThick(14, true, QColor(255, 255, 255), this);
}
//布局
void BrushSettings::initLayout(bool isTextOrNot)
{
    int tempInterval = CONTROL_INTERVAL;
    int tempLineInterval = 0;
    if (platForm.contains(Variable::platForm)) {
        tempInterval = CONTROL_INTERVAL_TM;
        tempLineInterval = 8;
    }
    if (isTextOrNot) {
        m_fontComBox->move(tempInterval, (this->height() - 9) / 2 - m_fontComBox->height() / 2);
        m_fontSizeComBox->move(m_fontComBox->width() + m_fontComBox->x() + 2, m_fontComBox->y());
        m_deleteLine->move(m_fontSizeComBox->x() + m_fontSizeComBox->width() + 2,
                           (this->height() - 9) / 2 - m_deleteLine->height() / 2);
        m_underLine->move(m_deleteLine->width() + m_deleteLine->x(), m_deleteLine->y());
        m_bold->move(m_underLine->width() + m_underLine->x(), m_deleteLine->y());
        m_italics->move(m_bold->width() + m_bold->x(), m_deleteLine->y());

        m_line->move(m_italics->width() + m_italics->x() + 8, 10 + tempLineInterval);
        m_red->move(m_line->width() + tempInterval + m_line->x(), tempInterval);
        m_blue->move(m_red->width() + m_red->x(), m_red->y());
        m_green->move(m_blue->width() + m_blue->x(), m_blue->y());
        m_yellow->move(m_green->width() + m_green->x(), m_green->y());
        m_pink->move(m_yellow->x() + m_yellow->width(), m_red->y());
        m_black->move(m_pink->width() + m_pink->x(), m_pink->y());
        m_grey->move(m_black->width() + m_black->x(), m_black->y());
        m_white->move(m_grey->width() + m_grey->x(), m_grey->y());
    } else {
        m_smallest->move(tempInterval, 4);
        m_thin->move(m_smallest->width() + m_smallest->x(), m_smallest->y());
        m_medium->move(m_thin->width() + m_thin->x(), m_smallest->y());
        m_thick->move(m_medium->width() + m_medium->x(), m_smallest->y());

        m_line->move(m_thick->width() + tempInterval + m_thick->x(), 10 + tempLineInterval);
        m_red->move(m_line->width() + tempInterval + m_line->x(), m_smallest->y());
        m_blue->move(m_red->width() + m_red->x(), m_red->y());
        m_green->move(m_blue->width() + m_blue->x(), m_blue->y());
        m_yellow->move(m_green->width() + m_green->x(), m_green->y());
        m_pink->move(m_yellow->width() + m_yellow->x(), m_red->y());
        m_black->move(m_pink->width() + m_pink->x(), m_pink->y());
        m_grey->move(m_black->width() + m_black->x(), m_black->y());
        m_white->move(m_grey->width() + m_grey->x(), m_grey->y());
    }
}
//左边的布局
void BrushSettings::leftWidChange(bool isTextOrNot)
{
    //判断是画笔粗细还是字体字号
    if (isTextOrNot) {
        m_smallest->hide();
        m_thin->hide();
        m_medium->hide();
        m_thick->hide();
        m_fontComBox->show();
        m_fontSizeComBox->show();
        m_deleteLine->show();
        m_underLine->show();
        m_bold->show();
        m_italics->show();
    } else {
        m_smallest->show();
        m_thin->show();
        m_medium->show();
        m_thick->show();
        m_fontComBox->hide();
        m_fontSizeComBox->hide();
        m_deleteLine->hide();
        m_underLine->hide();
        m_bold->hide();
        m_italics->hide();
    }
}
//设置互斥按钮组
void BrushSettings::setButtonGroup()
{
    //左边按钮组
    m_buttonGroupLeft->addButton(m_smallest);
    m_buttonGroupLeft->addButton(m_thin);
    m_buttonGroupLeft->addButton(m_medium);
    m_buttonGroupLeft->addButton(m_thick);
    //右边按钮组
    m_buttonGroupRight->addButton(m_red);
    m_buttonGroupRight->addButton(m_blue);
    m_buttonGroupRight->addButton(m_green);
    m_buttonGroupRight->addButton(m_yellow);
    m_buttonGroupRight->addButton(m_pink);
    m_buttonGroupRight->addButton(m_black);
    m_buttonGroupRight->addButton(m_grey);
    m_buttonGroupRight->addButton(m_white);
}
//初始化信号槽
void BrushSettings::initConnect()
{
    connect(m_smallest, &PainterThick::toggled, this, &BrushSettings::smallestClicked);
    connect(m_thin, &PainterThick::toggled, this, &BrushSettings::thinClicked);
    connect(m_medium, &PainterThick::toggled, this, &BrushSettings::mediumClicked);
    connect(m_thick, &PainterThick::toggled, this, &BrushSettings::thickClicked);
    connect(m_deleteLine, &PainterThick::clicked, this, &BrushSettings::deleteLineClicked);
    connect(m_underLine, &PainterThick::clicked, this, &BrushSettings::underLineClicked);
    connect(m_bold, &PainterThick::clicked, this, &BrushSettings::boldClicked);
    connect(m_italics, &PainterThick::clicked, this, &BrushSettings::italicsClicked);
    connect(m_red, &PainterThick::toggled, this, &BrushSettings::redClicked);
    connect(m_blue, &PainterThick::toggled, this, &BrushSettings::blueClicked);
    connect(m_green, &PainterThick::toggled, this, &BrushSettings::greenClicked);
    connect(m_yellow, &PainterThick::toggled, this, &BrushSettings::yellowClicked);
    connect(m_pink, &PainterThick::toggled, this, &BrushSettings::pinkClicked);
    connect(m_black, &PainterThick::toggled, this, &BrushSettings::blackClicked);
    connect(m_grey, &PainterThick::toggled, this, &BrushSettings::greyClicked);
    connect(m_white, &PainterThick::toggled, this, &BrushSettings::whiteClicked);
    connect(m_fontComBox, &QComboBox::currentTextChanged, this, &BrushSettings::currFontCombox);

    //    connect(m_fontSizeComBox,&QSpinBox::valueChanged,this,&BrushSettings::currFontSizeCombox);
    //        connect(m_fontSizeComBox, SIGNAL(valueChanged(int)), this, SLOT(currFontSizeCombox(int)));
    //    connect(m_fontSizeComBox, &QSpinBox::valueChanged, this, &BrushSettings::currFontSizeCombox);
    connect(m_fontSizeComBox, SIGNAL(valueChanged(int)), this, SLOT(currFontSizeCombox(int)));
}
//极细--4px
void BrushSettings::smallestClicked()
{
    recordPaintThick(4, m_smallest->isChecked());
}
//细--8px
void BrushSettings::thinClicked()
{
    recordPaintThick(8, m_thin->isChecked());
}
//粗--12px
void BrushSettings::mediumClicked()
{
    recordPaintThick(12, m_medium->isChecked());
}
//最粗--16px
void BrushSettings::thickClicked()
{
    recordPaintThick(16, m_thick->isChecked());
}
//红色
void BrushSettings::redClicked()
{
    recordPaintColor(0, m_red->isChecked(), QColor(219, 2, 15));
}
//蓝色
void BrushSettings::blueClicked()
{
    recordPaintColor(1, m_blue->isChecked(), QColor(36, 87, 217));
}
//绿色
void BrushSettings::greenClicked()
{
    recordPaintColor(2, m_green->isChecked(), QColor(85, 209, 57));
}
//黄色
void BrushSettings::yellowClicked()
{
    recordPaintColor(3, m_yellow->isChecked(), QColor(211, 154, 44));
}
//粉色
void BrushSettings::pinkClicked()
{
    recordPaintColor(4, m_pink->isChecked(), QColor(251, 66, 136));
}
//黑色
void BrushSettings::blackClicked()
{
    recordPaintColor(5, m_black->isChecked(), QColor(25, 25, 25));
}
//灰色
void BrushSettings::greyClicked()
{
    recordPaintColor(6, m_grey->isChecked(), QColor(98, 102, 113));
}
//白色
void BrushSettings::whiteClicked()
{
    recordPaintColor(7, m_white->isChecked(), QColor(255, 255, 255));
}
//删除线
void BrushSettings::deleteLineClicked()
{

    recordDate.m_textSet.isDeleteLine = m_deleteLine->isChecked();
    InteractiveQml::getInstance()->setPainterTextDeleteLine(recordDate.m_textSet.isDeleteLine);
}
//下划线
void BrushSettings::underLineClicked()
{
    recordDate.m_textSet.isUnderLine = m_underLine->isChecked();
    InteractiveQml::getInstance()->setPainterTextUnderLine(recordDate.m_textSet.isUnderLine);
}
//加粗
void BrushSettings::boldClicked()
{
    recordDate.m_textSet.isBold = m_bold->isChecked();
    InteractiveQml::getInstance()->setPainterTextBold(recordDate.m_textSet.isBold);
}
//斜体
void BrushSettings::italicsClicked()
{
    recordDate.m_textSet.isItalics = m_italics->isChecked();
    InteractiveQml::getInstance()->setPainterTextItalics(recordDate.m_textSet.isItalics);
}
//当前字体
void BrushSettings::currFontCombox(QString currentFont)
{
    recordDate.m_textSet.fontType = currentFont;
    InteractiveQml::getInstance()->setPainterTextFontType(recordDate.m_textSet.fontType);
}
//当前字号
void BrushSettings::currFontSizeCombox(int currentFontSize)
{
    //    recordDate.m_textSet.fontSize = currentFontSize;
    recordDate.m_textSet.fontSize = m_fontSizeComBox->value();
    InteractiveQml::getInstance()->setPainterTextFontSize(recordDate.m_textSet.fontSize);
}
//小三角位置
void BrushSettings::setStartPos(int startX)
{
    m_startX = startX;
}
//气泡弹窗位置
void BrushSettings::setShowPos(int showPosX, int showPosY)
{
    this->move(showPosX, showPosY);
}
//设置弹窗尺寸
void BrushSettings::setSize(bool isTextOrNot)
{
    m_showType = isTextOrNot;
    QSize tempSize = BRUSHSETTING_SIZE;
    QSize tempSizeText = BRUSHSETTINGTEXT_SIZE;
    m_triangleWidth = TRIANGLE_WIDTH;
    m_triangleHeight = TRIANGLE_HEIGHT;
    m_trianglePointy = TRIANGLE_POINTY;
    m_brush = BRUSHSETTING_PAINT;
    m_brushText = BRUSHSETTINGTEXT_PAINT;

    if (platForm.contains(Variable::platForm)) {
        tempSize = BRUSHSETTING_SIZE_TM;
        tempSizeText = BRUSHSETTINGTEXT_SIZE_TM;
        m_triangleWidth = TRIANGLE_WIDTH_TM;
        m_triangleHeight = TRIANGLE_HEIGHT_TM;
        m_trianglePointy = TRIANGLE_POINTY_TM;
        m_brush = BRUSHSETTING_PAINT_TM;
        m_brushText = BRUSHSETTINGTEXT_PAINT_TM;
    }
    if (isTextOrNot) {
        this->setFixedSize(tempSizeText);
    } else {
        this->setFixedSize(tempSize);
    }
    this->resetFontComboxSize();
    this->resetBtnSize();
    this->initLayout(isTextOrNot);
    this->leftWidChange(isTextOrNot);
}
//字号字体设置
void BrushSettings::setComboxSize()
{
    m_font.setPixelSize(10);
    //添加字体
    QFontDatabase database;
    QStringList fontFamilies;
    fontFamilies = database.families();
    if (fontFamilies.length() > 0) {
        for (int i = 0; i < fontFamilies.length(); i++) {
            m_fontComBox->addItem(fontFamilies.at(i));
        }
    }
    m_fontComBox->setFont(m_font);
    m_fontComBox->setFixedSize(65, 18 * 48 / 37);
    m_fontComBox->view()->setFixedWidth(240);
    //设置字号
    m_fontSizeComBox->setFixedSize(60, 18 * 48 / 37);
    m_fontSizeComBox->setValue(12);
    m_fontSizeComBox->setMinimum(6);
    m_fontSizeComBox->setMaximum(99);
    m_fontSizeComBox->setSingleStep(1);
    m_fontSizeComBox->setFont(m_font);
}
//画气泡型弹窗
void BrushSettings::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    if ("lingmo-dark" == m_themeChange || "lingmo-black" == m_themeChange) {
        painter.setBrush(QColor(0, 0, 0, 0.68 * 255));
    } else {
        painter.setBrush(QColor(255, 255, 255, 0.62 * 255));
    }
    // 小三角区域;
    QPainterPath drawPath;
    QPolygon trianglePolygon;
    trianglePolygon << QPoint(m_startX, m_trianglePointy);
    trianglePolygon << QPoint(m_startX + m_triangleWidth / 2, m_trianglePointy + m_triangleHeight);
    trianglePolygon << QPoint(m_startX + m_triangleWidth, m_trianglePointy);
    //判断尺寸
    if (m_showType) {
        drawPath.addRoundedRect(m_brushText, BORDER_RADIUS, BORDER_RADIUS);
    } else {
        drawPath.addRoundedRect(m_brush, BORDER_RADIUS, BORDER_RADIUS);
    }
    drawPath.addPolygon(trianglePolygon);
    painter.drawPath(drawPath);
}
//监听主题
void BrushSettings::initGsetting()
{
    if (QGSettings::isSchemaInstalled(FITTHEMEWINDOWA)) {
        m_pGsettingThemeData = new QGSettings(FITTHEMEWINDOWA);
        connect(m_pGsettingThemeData, &QGSettings::changed, this, &BrushSettings::dealSystemGsettingChange);
    }
}
//记录画笔粗细
void BrushSettings::recordPaintThick(int type, bool isChecked)
{
    if (isChecked) {
        recordDate.thickness = type;
        InteractiveQml::getInstance()->setPainterThickness(type);
    }
}
//记录画笔颜色
void BrushSettings::recordPaintColor(int type, bool isChecked, QColor painterColor)
{
    if (isChecked) {
        recordDate.painterColor = type;
        InteractiveQml::getInstance()->setPainterColor(painterColor);
    }
}
// pc-tm的字号控件的大小-两种模式，主题控件样式不一样
void BrushSettings::resetFontComboxSize()
{
    if (platForm.contains(Variable::platForm)) {
        m_fontSizeComBox->setFixedSize(120, 18 * 48 / 37);
    } else {
        m_fontSizeComBox->setFixedSize(60, 18 * 48 / 37);
    }
}

void BrushSettings::resetBtnSize()
{
    QSize tempBtnSize = QSize(32, 32);
    if (platForm.contains(Variable::platForm)) {
        tempBtnSize = QSize(48, 48);
    }
    //画笔粗细
    m_smallest->resetSize(tempBtnSize);
    m_thin->resetSize(tempBtnSize);
    m_medium->resetSize(tempBtnSize);
    m_thick->resetSize(tempBtnSize);
    m_red->resetSize(tempBtnSize);
    m_blue->resetSize(tempBtnSize);
    m_green->resetSize(tempBtnSize);
    m_yellow->resetSize(tempBtnSize);
    m_pink->resetSize(tempBtnSize);
    m_black->resetSize(tempBtnSize);
    m_grey->resetSize(tempBtnSize);
    m_white->resetSize(tempBtnSize);
}

void BrushSettings::dealSystemGsettingChange(const QString key)
{
    if (key == "styleName") {
        setTextSettings();
    }
}
//主题变化
void BrushSettings::setTextSettings()
{
    QString nowThemeStyle = m_pGsettingThemeData->get("styleName").toString();
    m_themeChange = nowThemeStyle;
    m_smallest->m_themeChange = nowThemeStyle;
    m_thin->m_themeChange = nowThemeStyle;
    m_medium->m_themeChange = nowThemeStyle;
    m_thick->m_themeChange = nowThemeStyle;
    if ("lingmo-dark" == nowThemeStyle || "lingmo-black" == nowThemeStyle) {
        m_line->setStyleSheet("QLabel{border: 1px solid rgba(255, 255, 255, 0.36);background-color: #393939;}");
        m_deleteLine->setStyleSheet("QCheckBox::indicator:unchecked{border:0px;border-radius:4px;background:"
                                    "transparent;image: url(:/res/res/mark/1deleteLine.png);}"
                                    "QCheckBox::indicator:checked{border:0px;border-radius:4px;background:transparent;"
                                    "image: url(:/res/res/mark/deleteLine_selected.png);}");
        m_underLine->setStyleSheet("QCheckBox::indicator:unchecked{border:0px;border-radius:4px;background:transparent;"
                                   "image: url(:/res/res/mark/1underLine.png);}"
                                   "QCheckBox::indicator:checked{border:0px;border-radius:4px;background:transparent;"
                                   "image: url(:/res/res/mark/underLine_selected.png);}");
        m_bold->setStyleSheet("QCheckBox::indicator:unchecked{border:0px;border-radius:4px;background:transparent;"
                              "image: url(:/res/res/mark/1bold.png);}"
                              "QCheckBox::indicator:checked{border:0px;border-radius:4px;background:transparent;image: "
                              "url(:/res/res/mark/bold_selected.png);}");
        m_italics->setStyleSheet("QCheckBox::indicator:unchecked{border:0px;border-radius:4px;background:transparent;"
                                 "image: url(:/res/res/mark/1italics.png);}"
                                 "QCheckBox::indicator:checked{border:0px;border-radius:4px;background:transparent;"
                                 "image: url(:/res/res/mark/italics_selected.png);}");
    } else {

        m_line->setStyleSheet("QLabel{border: 1px solid rgba(0, 0, 0, 1);background-color: #393939;}");
        m_deleteLine->setStyleSheet("QCheckBox::indicator:unchecked{border:0px;border-radius:4px;background:"
                                    "transparent;image: url(:/res/res/mark/deleteLine.png);}"
                                    "QCheckBox::indicator:checked{border:0px;border-radius:4px;background:transparent;"
                                    "image: url(:/res/res/mark/deleteLine_selected.png);}");
        m_underLine->setStyleSheet("QCheckBox::indicator:unchecked{border:0px;border-radius:4px;background:transparent;"
                                   "image: url(:/res/res/mark/underLine.png);}"
                                   "QCheckBox::indicator:checked{border:0px;border-radius:4px;background:transparent;"
                                   "image: url(:/res/res/mark/underLine_selected.png);}");
        m_bold->setStyleSheet("QCheckBox::indicator:unchecked{border:0px;border-radius:4px;background:transparent;"
                              "image: url(:/res/res/mark/bold.png);}"
                              "QCheckBox::indicator:checked{border:0px;border-radius:4px;background:transparent;image: "
                              "url(:/res/res/mark/bold_selected.png);}");
        m_italics->setStyleSheet("QCheckBox::indicator:unchecked{border:0px;border-radius:4px;background:transparent;"
                                 "image: url(:/res/res/mark/italics.png);}"
                                 "QCheckBox::indicator:checked{border:0px;border-radius:4px;background:transparent;"
                                 "image: url(:/res/res/mark/italics_selected.png);}");
    }
}
