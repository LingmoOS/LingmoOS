// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cexportimagedialog.h"
#include "csvglabel.h"
#include "dialog.h"
#include "cvalidator.h"
#include "cexportimagedialog_p.h"
#include "cspinbox.h"
#include "setting.h"

#include <QFileDialog>
#include <QImageWriter>
#include <QStandardPaths>
#include <QPdfWriter>
#include <QPainter>
#include <QCloseEvent>
#include <QDebug>
#include <QDateTime>
#include <QAbstractButton>
#include <QApplication>
#include <QLineEdit>

#define NAME_MAX 255
const QSize DIALOG_SIZE = QSize(380, 280);
const QSize LINE_EDIT_SIZE = QSize(250, 35);
const QSize TIP_LABEL_MAXSIZE = QSize(103, 12);
const int MAX_PERCENT_VALUE = 100;
const int MIN_PERCENT_VALUE = 0;
enum {ECancel = -1, EReExec, EOK};

CExportImageDialog::CExportImageDialog(DrawBoard *drawbord, DWidget *parent): m_drawBoard(drawbord), EXPORTFATHER(drawbord),
    CExportImageDialog_d(new CExportImageDialog_private(this))

{
    initUI();
    initConnection();
}

CExportImageDialog::~CExportImageDialog()
{
    //delete _pPrivate;
}

int CExportImageDialog::getImageType() const
{
    return  m_formatCombox->currentIndex();
}

QString CExportImageDialog::getImageFormate() const
{
    return  m_saveFormat;
}

int CExportImageDialog::getQuality() const
{
    return m_quality;
}

int CExportImageDialog::exec()
{
    quitRet = 1;
    if (m_savePathCombox->count() == Other + 1) {
        m_savePathCombox->blockSignals(true);
        m_savePathCombox->removeItem(Other);
    }
    m_savePathCombox->blockSignals(false);

    //获取page页名称
    if (m_drawBoard != nullptr) {
        auto name = m_drawBoard->currentPage()->name();
        m_fileNameEdit->setText(name);
    }

    //设置路径信息
    int indexpath = Setting::instance()->defaultExportDialogPath().first;
    if (indexpath == Other) {
        QString fileDir = Setting::instance()->defaultExportDialogPath().second;
        if (m_savePathCombox->count() < Other + 1)
            m_savePathCombox->insertItem(Other, fileDir);
        else
            m_savePathCombox->setItemText(Other, fileDir);
        m_savePathCombox->setCurrentText(fileDir);
    }
    m_savePathCombox->setCurrentIndex(qMax(0, indexpath));


    //设置格式信息
    int index = Setting::instance()->defaultExportDialogFilteFormat();
    m_formatCombox->setCurrentIndex(qMax(0, index));

    //设置默认值
    saveSetting();
    m_qualitySlider->setValue(100);
    slotOnSavePathChange(m_savePathCombox->currentIndex());
    slotOnFormatChange(m_formatCombox->currentIndex());
    slotOnQualityChanged(m_qualitySlider->value());

Exec:
#ifdef USE_DTK
    quitRet = DDialog::exec();
#else
    quitRet = QDialog::exec();
#endif

    if (quitRet == 1) {
        int ret = execCheckFile(resultFile());

        if (ret == EReExec) {
            goto Exec;
        }
        quitRet = ret;

        saveSetting();
    }

    return quitRet;
}

int CExportImageDialog::execFor(const Page *page)
{
    d_CExportImageDialog()->resetImageSettingSizeTo(page->pageRect().size().toSize());

    if (exec() == 1) {
        if (page->saveToImage(resultFile(), desImageSize(), getQuality()))
            return 1;
        else {
            return 0;
        }
    }
    return -1;
}


QString CExportImageDialog::resultFile() const
{
    if (quitRet == 1)
        return getCompleteSavePath();

    return "";
}

QSize CExportImageDialog::desImageSize() const
{
    return QSize(d_CExportImageDialog()->_widthEditor->text().toInt(), d_CExportImageDialog()->_heightEditor->text().toInt());
}

void CExportImageDialog::initUI()
{
    setWgtAccesibleName(this, "Export dialog");
    resize(DIALOG_SIZE);
    setModal(true);

    setContentsMargins(0, 0, 0, 0);
    setIcon(QIcon::fromTheme("deepin-draw"));
    setWindowTitle(tr("Export"));

    m_fileNameEdit = new LINEEDITOR(this);
    setWgtAccesibleName(m_fileNameEdit, "Export name line editor");
    m_fileNameEdit->setFixedHeight(LINE_EDIT_SIZE.height());
    m_fileNameEdit->setClearButtonEnabled(false);
    QLineEdit *lineeditor = nullptr;
#ifdef USE_DTK
    lineeditor = m_fileNameEdit->lineEdit();
#else
    lineeditor = m_fileNameEdit;
#endif
    //编译器会对反斜杠进行转换，要想在正则表达式中包括一个\，需要输入两次，例如\\s。要想匹配反斜杠本身，需要输入4次，比如\\\\。
    lineeditor->setValidator(new QRegExpValidator(QRegExp("[^\\\\ /:*?\"<>|]+"), lineeditor));

    m_savePathCombox = new QComboBox(this);
    setWgtAccesibleName(m_savePathCombox, "Export path comboBox");

    m_savePathCombox->insertItem(Pictures, tr("Pictures"));
    m_savePathCombox->insertItem(Documents, tr("Documents"));
    m_savePathCombox->insertItem(Downloads, tr("Downloads"));
    m_savePathCombox->insertItem(Desktop, tr("Desktop"));
    m_savePathCombox->insertItem(Videos, tr("Videos"));
    m_savePathCombox->insertItem(Music, tr("Music"));
    m_savePathCombox->insertItem(UsrSelect, tr("Select other directories"));

    m_formatCombox = new QComboBox(this);
    setWgtAccesibleName(m_formatCombox, "Export format comboBox");
    m_formatCombox->insertItem(PNG, tr("png"));
    m_formatCombox->insertItem(JPG, tr("jpg"));
    m_formatCombox->insertItem(BMP, tr("bmp"));
    m_formatCombox->insertItem(TIF, tr("tif"));
    m_formatCombox->insertItem(PDF, tr("pdf"));
    m_formatCombox->insertItem(PPM, tr("ppm"));
    m_formatCombox->insertItem(XBM, tr("xbm"));
    m_formatCombox->insertItem(XPM, tr("xpm"));

    m_qualitySlider = new QSlider(Qt::Horizontal, this);
    setWgtAccesibleName(m_qualitySlider, "Export quality slider");
    m_qualitySlider->setMinimum(1);
    m_qualitySlider->setMaximum(100);
    m_qualitySlider->setValue(100);
    m_qualitySlider->setFixedSize(QSize(240, LINE_EDIT_SIZE.height()));

    m_qualityLabel = new QLabel(this);

    QHBoxLayout *qualityHLayout = new QHBoxLayout;
    qualityHLayout->setMargin(0);
    qualityHLayout->setSpacing(0);
    qualityHLayout->addSpacing(3);
    qualityHLayout->addWidget(m_qualitySlider);
    qualityHLayout->addSpacing(20);
    qualityHLayout->addWidget(m_qualityLabel);

    QWidget *contentWidget = new QWidget(this);
    setWgtAccesibleName(contentWidget, "Export content widget");
    contentWidget->setContentsMargins(0, 0, 0, 0);
    QFormLayout *fLayout = new QFormLayout(contentWidget);
    fLayout->setFormAlignment(Qt::AlignJustify);
    fLayout->setHorizontalSpacing(10);
    fLayout->setSpacing(10);
    fLayout->setMargin(0);

    //分割线
    auto Splitline = new QFrame(this);
    Splitline->setFrameShape(QFrame::HLine);

    fLayout->addRow(tr("Name:"), m_fileNameEdit);
    fLayout->addRow(tr("Save to:"), m_savePathCombox);
    //fLayout->addRow(tr("Save to:"), m_savePathCombox);
    fLayout->addRow(tr("Format:"), m_formatCombox);
    fLayout->addRow(Splitline);
    fLayout->addRow(tr("Quality:"), qualityHLayout);

    d_CExportImageDialog()->initSizeSettingLayoutUi(fLayout, contentWidget);

#ifdef USE_DTK
    addContent(contentWidget);
    addButton(tr("Cancel"), false, DDialog::ButtonNormal);
    m_saveBtnId = addButton(tr("Save"), true, DDialog::ButtonRecommend);
#endif
}

void CExportImageDialog::initConnection()
{
    connect(m_savePathCombox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotOnSavePathChange(int)), Qt::QueuedConnection);
    connect(m_formatCombox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotOnFormatChange(int)));
    connect(m_qualitySlider, SIGNAL(valueChanged(int)), this, SLOT(slotOnQualityChanged(int)));

    //设置的文件名为空时应该要设置保存按钮为disable
#ifdef USE_DTK
    connect(m_fileNameEdit, &LINEEDITOR::textChanged, this, [ = ](const QString & text) {

        QString newText = text;
        bool isEmpty = newText.isEmpty();

        if (m_saveBtnId != -1) {

            QAbstractButton *pBtn = getButton(m_saveBtnId);
            if (pBtn != nullptr) {
                pBtn->setEnabled(!isEmpty);
            }
        }

    });
#endif

}

void CExportImageDialog::slotOnSavePathChange(int index)
{
    switch (index) {
    case Pictures:
    case Documents:
    case Downloads:
    case Desktop:
    case Videos:
    case Music:
        m_savePath = Setting::instance()->SavePathChange(index);
        break;
    case UsrSelect:
        showDirChoseDialog();
        break;
    case Other:
        m_savePath = m_savePathCombox->itemText(index);
        break;
    default:
        m_savePath = "";
        break;
    }
}

void CExportImageDialog::slotOnFormatChange(int index)
{
    if (PDF == index) {
        m_qualitySlider->setValue(100);
        m_qualitySlider->setEnabled(false);
    } else {
        m_qualitySlider->setEnabled(true);
    }

    m_saveFormat = m_formatCombox->itemText(index);

    QString name = m_fileNameEdit->text().trimmed();
    m_fileNameEdit->setText(name);
}

int CExportImageDialog::execCheckFile(const QString &text)
{
    //  [BUG: 30843] 希望在导出时，名字以点开头的图片，也能给到提示：以点开始会被隐藏
    QString fileName = m_fileNameEdit->text().trimmed();
    if (fileName.startsWith(".")) {
        int status = MessageDlg::execMessage(tr("This file will be hidden if the file name starts with a dot (.). Do you want to hide it?"), EWarningMsg,
                                             QStringList() << tr("Cancel") << tr("Confirm"),
                                             QList<EButtonType>() << ENormalMsgBtn << EWarningMsgBtn, this);
        if (status != 1) {
            return EReExec;
        }
    }

    // 判断路径是否超过255字符
    QString completePath = getCompleteSavePath();
    QFileInfo info(completePath);
    // NAME_MAX 文件名字最大长度
    if (info.fileName().toLocal8Bit().length() > NAME_MAX) {
        MessageDlg::execMessage(tr("The file name is too long"), this);
        return ECancel;
    }

    if (completePath == "") {
        return ECancel;
    }
    if (QFileInfo::exists(completePath)) {
        int ret = execFileIsExists(completePath);
        if (ret != 1) {
            return ECancel;
        }
    }
    return EOK;
}


void CExportImageDialog::slotOnQualityChanged(int value)
{
    m_qualityLabel->setText(QString("%1%").arg(value));
    m_quality = value;
}

void CExportImageDialog::showDirChoseDialog()
{
    QFileDialog dialog(this);
    dialog.setDirectory(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setFileMode(QFileDialog::DirectoryOnly);
    if (dialog.exec()) {
        auto dirs = dialog.selectedFiles();
        QString fileDir = dirs.isEmpty() ? "" : dirs.first();
        if (!fileDir.isEmpty()) {
            if (m_savePathCombox->count() < Other + 1) {
                m_savePathCombox->insertItem(Other, fileDir);
            } else {
                m_savePathCombox->setItemText(Other, fileDir);
            }
            m_savePathCombox->setCurrentText(fileDir);
            //保存设置
            saveSetting();
        }
    }
}

int CExportImageDialog::execFileIsExists(const QString &path)
{
    return MessageDlg::execMessage(QString(tr("%1 \n already exists, do you want to replace it?")).arg(path),
                                   EWarningMsg,
                                   QStringList() << tr("Cancel") << tr("Replace"),
                                   QList<EButtonType>() << ENormalMsgBtn << EWarningMsgBtn,
                                   this);
}

QString CExportImageDialog::getCompleteSavePath() const
{
    QString fileName = m_fileNameEdit->text().trimmed();

    if (fileName.isEmpty() || fileName == "") {
        return "";
    }

    fileName = fileName + "." + m_saveFormat/*m_formatCombox->currentText()*/;

    return m_savePath + "/" + fileName;
}

void CExportImageDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == (Qt::Key_Return) || event->key() == (Qt::Key_Enter)) {
        return;
    }
    return EXPORTFATHER::keyPressEvent(event);
}

void CExportImageDialog::CExportImageDialog_private::initSizeSettingLayoutUi(QFormLayout *fLayout, QWidget *contentWidget)
{
    _formLayout = fLayout;

    //add radio setting ui.
    QHBoxLayout *lay1 = new QHBoxLayout;
    lay1->setContentsMargins(0, 0, 0, 0);
    _radioRadioBtn = new QRadioButton(tr("Percentage"), contentWidget);
    lay1->addWidget(_radioRadioBtn);


    QHBoxLayout *pecent_layout = new QHBoxLayout;
    PercentageWgt = new QWidget(contentWidget);
    auto spinBox = new CSpinBox(contentWidget);
    _precentpix = new QLabel(contentWidget);
    QPalette temp;
    QColor c;
    c.setAlphaF(0.4);
    temp.setColor(QPalette::WindowText, c);
    _precentpix->setPalette(temp);
    pecent_layout->addWidget(spinBox);
    pecent_layout->addWidget(_precentpix);
    spinBox->setSpinRange(MIN_PERCENT_VALUE, MAX_PERCENT_VALUE);
    spinBox->setSuffix("%");
    PercentageWgt->setLayout(pecent_layout);

#ifdef USE_DTK
    spinBox->setEnabledEmbedStyle(true);
    spinBox->lineEdit()->setValidator(new CIntValidator(MIN_PERCENT_VALUE, MAX_PERCENT_VALUE, spinBox));
#endif
    spinBox->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
    spinBox->setMaximumSize(QSize(100, 36));
    _radioSpinBox = spinBox;

    lay1->addSpacing(-70);
    _radioPiexlBtn = new QRadioButton(tr("Pixels"), contentWidget);
    lay1->addWidget(_radioPiexlBtn);
    fLayout->addRow(tr("Dimensions:"), lay1);
    fLayout->addRow("", PercentageWgt);

    QHBoxLayout *lay2 = new QHBoxLayout;
    piexlWgt = new QWidget(contentWidget);
    piexlWgt->setLayout(lay2);
    lay2->setContentsMargins(0, 0, 0, 0);
    {
        //set w ui.
        _keepRaidoCheckBox = new QCheckBox(tr("Lock aspect ratio"), contentWidget);

        connect(_keepRaidoCheckBox, &QCheckBox::toggled, _q, [ = ](bool checked) {
            if (settingModel == EPixelModel)
                keepRadioIfPixelModel = checked;
        });

        {
            lay2->addWidget(new QLabel(tr("W"), contentWidget));

            auto linespinBox = new CSpinBox(contentWidget);
            linespinBox->setSpinRange(0, 999999);
            lay2->addWidget(linespinBox);
#ifdef USE_DTK
            linespinBox->setEnabledEmbedStyle(true);
            linespinBox->lineEdit()->setValidator(new CIntValidator(0, 999999, linespinBox));
#endif
            linespinBox->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
            linespinBox->setMaximumSize(QSize(100, 36));
            _widthEditor = linespinBox;
        }

        {
            lay2->addSpacing(10);
            lay2->addWidget(new QLabel(tr("H"), contentWidget));
            auto linespinBox = new CSpinBox(contentWidget);
            linespinBox->setSpinRange(0, 999999);
            lay2->addWidget(linespinBox);
            lay2->addStretch();

#ifdef USE_DTK
            linespinBox->setEnabledEmbedStyle(true);
            linespinBox->lineEdit()->setValidator(new CIntValidator(0, 999999, linespinBox));
#endif
            linespinBox->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
            linespinBox->setMaximumSize(QSize(100, 36));
            _heightEditor = linespinBox;
        }
        {
            _tipLabelForOutOfBounds = new QLabel("tip label");//设置内容让其适应字体，然后再设置为空，避免字体与系统字体不一样，导致高度变化
            _tipLabelForOutOfBounds->setMaximumSize(TIP_LABEL_MAXSIZE);
            QPalette palette = _tipLabelForOutOfBounds->palette();
            palette.setColor(QPalette::WindowText, QColor("#8AA1B4"));
            QFont font;
            font.setFamily("SourceHanSansSC");
            font.setPointSize(9);
            _tipLabelForOutOfBounds->setText("");
            _tipLabelForOutOfBounds->setPalette(palette);
            _tipLabelForOutOfBounds->setFont(font);
        }

    }

    fLayout->addRow("", piexlWgt);
    fLayout->addRow("", _keepRaidoCheckBox);
    fLayout->addRow("", _tipLabelForOutOfBounds);

    {
        auto validtor = new CIntValidator(0, 999999, _widthEditor);
        validtor->setEmptyStrToBottom(false);
        QLineEdit *lineeditor = nullptr;
#ifdef USE_DTK
        lineeditor = _widthEditor->lineEdit();
#else
        lineeditor = _widthEditor;
#endif
        lineeditor->setValidator(validtor);
        connect(_widthEditor, &CSpinBox::valueChanged, _q, [ = ]() {
            if (_widthEditor->text().toInt() != curShowIntSize.width())
                autoKeepSize(EKeepBaseW);
        });
    }
    {
        auto validtor = new CIntValidator(0, 999999, _heightEditor);
        validtor->setEmptyStrToBottom(false);
        QLineEdit *lineeditor = nullptr;
#ifdef USE_DTK
        lineeditor = _heightEditor->lineEdit();
#else
        lineeditor = _heightEditor;
#endif
        lineeditor->setValidator(validtor);
        connect(_heightEditor, &CSpinBox::valueChanged, _q, [ = ]() {
            if (_heightEditor->text().toInt() != curShowIntSize.height())
                autoKeepSize(EKeepBaseH);
        });
    }
    connect(_radioSpinBox, &CSpinBox::valueChanged, _q, [ = ](int value, EChangedPhase phase) {
        Q_UNUSED(value)
        Q_UNUSED(phase)
        autoKeepSize(EKeepBaseRadioValue);
    });

    QButtonGroup *group = new QButtonGroup(_q);
    group->addButton(_radioRadioBtn, ERadioModel);
    group->addButton(_radioPiexlBtn, EPixelModel);
    connect(group, QOverload<int, bool>::of(&QButtonGroup::buttonToggled), _q, [ = ](int model, bool checked) {
        if (checked) {
            settingModel = ESizeSettingModel(model);
            updateSettingModelUi();
        }
    });

    _radioRadioBtn->setChecked(true);
}

void CExportImageDialog::CExportImageDialog_private::updateSettingModelUi()
{
    if (_radioRadioBtn->isChecked()) {
        setSizeSettingModel(ERadioModel);
    } else if (_radioPiexlBtn->isChecked()) {
        setSizeSettingModel(EPixelModel);
    }
}
void CExportImageDialog::CExportImageDialog_private::resetImageSettingSizeTo(const QSize &sz, qreal raido, bool keepRadio, ESizeSettingModel settingModel)
{
    this->settingModel = settingModel;
    keepRadioIfPixelModel = true;

    originSize = sz;
    for (int i = 0; i < ESettingModelCount; ++i) {
        curSize[i] = sz;
    }
    curPrecent = raido;


    if (settingModel == ERadioModel) {
        _radioRadioBtn->setChecked(true);
    } else if (settingModel == EPixelModel) {
        _radioPiexlBtn->setChecked(true);
    }

    //_radioSpinBox->setRange(0, INT_MAX);
    _radioSpinBox->setValue(qMin(raido, 1.0) * 100.);
    _widthEditor->setValue(sz.width());
    _heightEditor->setValue(sz.height());
    _precentpix->setText(QString::number(sz.width()) + "*" + QString::number(sz.height()));
    _keepRaidoCheckBox->setChecked(keepRadio);
}

void CExportImageDialog::CExportImageDialog_private::setSizeSettingModel(ESizeSettingModel model)
{
    if (model == ERadioModel) {
        piexlWgt->hide();
        _keepRaidoCheckBox->hide();
        _tipLabelForOutOfBounds->hide();
        _formLayout->removeWidget(piexlWgt);
        _formLayout->removeWidget(_keepRaidoCheckBox);
        _formLayout->removeWidget(_tipLabelForOutOfBounds);
        _formLayout->addRow("", PercentageWgt);
        PercentageWgt->show();

        _q->adjustSize();
        QSignalBlocker bloker(_radioSpinBox);
        _radioSpinBox->setValue(qRound((qreal(curSize[ERadioModel].width()) / originSize.width()) * 100.));

    } else if (model == EPixelModel) {
        PercentageWgt->hide();
        _formLayout->removeWidget(PercentageWgt);
        _formLayout->addRow("", piexlWgt);
        _formLayout->addRow("", _keepRaidoCheckBox);
        _formLayout->addRow("", _tipLabelForOutOfBounds);
        piexlWgt->show();
        _keepRaidoCheckBox->show();

        _q->adjustSize();
        QSignalBlocker bloker(_radioSpinBox);
        _radioSpinBox->setSpecialText();

        QSignalBlocker bloker1(_keepRaidoCheckBox);
        _keepRaidoCheckBox->setChecked(keepRadioIfPixelModel);
    }

    QSignalBlocker bloker1(_widthEditor);
    _widthEditor->setSpinValue(curSize[model].toSize().width());

    QSignalBlocker bloker2(_heightEditor);
    _heightEditor->setSpinValue(curSize[model].toSize().height());
}

bool CExportImageDialog::CExportImageDialog_private::autoKeepSize(EKeepBase base)
{
    EAlertReason alert = ENoAlert;

    QSignalBlocker bloker0(_widthEditor);
    QSignalBlocker bloker1(_heightEditor);
    QSignalBlocker bloker2(_radioSpinBox);

    if (base == EKeepBaseH || base == EKeepBaseW) {
        bool keepHW = _keepRaidoCheckBox->isChecked();
        if (!keepHW) {
            base = EFreeSetting;
        }
    }
    QSizeF resultSize = curSize[settingModel];
    qreal settingPrecent = _radioSpinBox->value() / 100.;
    if ((!_widthEditor->text().isEmpty()) && (!_heightEditor->text().isEmpty())) {
        int wantedWidth  = _widthEditor->text().isEmpty() ? qRound(curSize[settingModel].width()) : _widthEditor->text().toInt();
        int wantedHeight = _heightEditor->text().isEmpty() ? qRound(curSize[settingModel].height()) : _heightEditor->text().toInt();

        resultSize = autoKeepWHRadio(base, settingPrecent, QSize(wantedWidth, wantedHeight), curSize[settingModel],
                                     originSize, alert);
    }

    showTip(alert);

    int showWidth  = qRound(resultSize.width());
    int showHeight = qRound(resultSize.height());
    curShowIntSize = QSize(showWidth, showHeight);
    _widthEditor->setSpinPhaseValue(showWidth, EChanged);
    _heightEditor->setSpinPhaseValue(showHeight, EChanged);

    curSize[settingModel] = resultSize;
    if (base == EKeepBaseRadioValue) {
        if (alert != ENoAlert) {
            QSignalBlocker bloker(_radioSpinBox);
            qreal value = curPrecent * 100.;
            _radioSpinBox->setSpinPhaseValue(value, EChanged);
        } else {
            curPrecent = settingPrecent;
        }
        _precentpix->setText(QString::number(showWidth) + "*" + QString::number(showHeight));
    }
    return (alert == ENoAlert);
}

QSizeF CExportImageDialog::CExportImageDialog_private::autoKeepWHRadio(int base, qreal precentRadio,
                                                                       const QSize &wantedSize,
                                                                       const QSizeF &preSize,
                                                                       const QSize &orgSz,
                                                                       EAlertReason &aler)
{
    const int c_MaxPiexl = 10000;
    const int c_MinPiexl = 1;
    QSizeF size = preSize;

    aler = ENoAlert;

    switch (base) {
    case EKeepBaseRadioValue: {
        size = QSizeF(orgSz.width() * precentRadio, orgSz.height() * precentRadio);
        if ((size.width() > c_MaxPiexl || size.width() < c_MinPiexl) || (size.height() > c_MaxPiexl || size.height() < c_MinPiexl)) {
            if (size.width() < c_MinPiexl || size.height() < c_MinPiexl) {
                aler = ETooSmall;
            } else if (size.width() > c_MaxPiexl || size.height() > c_MaxPiexl) {
                aler = ETooBig;
            }
            size = preSize;
        }
        break;
    }
    case EKeepBaseW:
    case EKeepBaseH: {
        if (wantedSize.width() < c_MinPiexl || wantedSize.height() < c_MinPiexl) {
            aler = ETooSmall;
        } else if (wantedSize.width() > c_MaxPiexl || wantedSize.height() > c_MaxPiexl) {
            aler = ETooBig;
        }

        const QSize tempSize = QSize(qMin(c_MaxPiexl, qMax(c_MinPiexl, wantedSize.width())), qMin(c_MaxPiexl, qMax(c_MinPiexl, wantedSize.height())));
        size = tempSize;

        qreal radioWH = qreal(preSize.width()) / preSize.height();
        if (base == EKeepBaseW) {
            qreal resultH = size.width() / radioWH;
            if (resultH >= c_MinPiexl && resultH <= c_MaxPiexl) {
                size.setHeight(resultH);
            } else {
                if (resultH > c_MaxPiexl)
                    aler = ETooBig;
                else if (resultH < c_MinPiexl)
                    aler = ETooSmall;
                size = preSize;
            }
        } else if (base == EKeepBaseH) {
            qreal resultW = size.height() * radioWH;
            if (resultW >= c_MinPiexl && resultW <= c_MaxPiexl) {
                size.setWidth(resultW);
            } else {
                if (resultW > c_MaxPiexl)
                    aler = ETooBig;
                else if (resultW < c_MinPiexl)
                    aler = ETooSmall;
                size = preSize;
            }
        }
        break;
    }
    case EFreeSetting: {
        size = wantedSize;
        if (size.width() > c_MaxPiexl) {
            size.rwidth() = c_MaxPiexl;
            aler = ETooBig;
        } else if (size.width() < c_MinPiexl) {
            aler = ETooSmall;
            size.rwidth() = c_MinPiexl;
        }

        if (size.height() > c_MaxPiexl) {
            aler = ETooBig;
            size.rheight() = c_MaxPiexl;
        } else if (size.height() < c_MinPiexl) {
            aler = ETooSmall;
            size.rheight() = c_MinPiexl;
        }
        break;
    }

    }
    return size;
}

void CExportImageDialog::CExportImageDialog_private::showTip(EAlertReason alertReson)
{
    if (alertReson == ENoAlert)
        return;
    //不是像素，不显示
    if (!_radioPiexlBtn->isChecked())
        return;

    QString tips;
    if (alertReson == ETooSmall) {
        tips = tr("At least one pixel please");
    } else if (alertReson == ETooBig) {
        tips = tr("It supports up to 10,000 pixels");
    }

    if (_tipLabelForOutOfBounds->isHidden())
        _tipLabelForOutOfBounds->show();
    _tipLabelForOutOfBounds->setText(tips);
    if (timer == nullptr) {
        timer = new QTimer(_q);
        timer->setSingleShot(true);
        connect(timer, &QTimer::timeout, _q, [ = ]() {
            _tipLabelForOutOfBounds->setText("");
        });
    }
    timer->start(2000);
}

bool CExportImageDialog::CExportImageDialog_private::isFocusInEditor() const
{
    QLineEdit *wlineeditor = nullptr;
    QLineEdit *hlineeditor = nullptr;
#ifdef USE_DTK
    wlineeditor = _widthEditor->lineEdit();
    hlineeditor = _heightEditor->lineEdit();
#else
    wlineeditor = _widthEditor;
    hlineeditor = _heightEditor;
#endif
    if (qApp->focusWidget() == wlineeditor || qApp->focusWidget() == hlineeditor) {
        return true;
    }
    return false;
}

void CExportImageDialog::showEvent(QShowEvent *event)
{
    DDialog::showEvent(event);
}

void CExportImageDialog::saveSetting()
{
    //保存路径和格式
    if (m_savePath != "") {
        Setting::instance()->setDefaultExportDialogPath(QPair<int, QString>(m_savePathCombox->currentIndex(), m_savePath));
        Setting::instance()->setDefaultExportDialogFilterFormat(m_formatCombox->currentIndex());
    }
}

void CExportImageDialog::resizeEvent(QResizeEvent *event)
{
    //获取到真实大小，移动位置
    this->moveToCenter();
    return DDialog::resizeEvent(event);
}
