// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dpropertysetdlg.h"
#include "dprinterpropertytemplate.h"

#include <qsettingbackend.h>
#include <dsettings.h>
#include <DSettingsOption>
#include <DSettingsOption>
#include <DSettingsWidgetFactory>
#include <DLabel>
#include <DListView>
#include <DPushButton>
#include <DDialog>
#include <DComboBox>
#include <DLineEdit>

#include <QDir>
#include <QTemporaryFile>
#include <QDebug>
#include <QFont>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLocale>
#include <QPalette>
#include <QFile>
#include <QProcess>

#include <QJsonParseError>
#include <QJsonObject>

#include "../util/dprintermanager.h"
#include "../util/dprinter.h"
#include "common.h"

#define FLAGITEMSPACE "\b\b\b\b\b\b\b\b"
#define ITEMWIDTH 100
#define ITEMHEIGHT 56
#define UNSUPPORTED "-1"

enum COMBOITEMROLE {
    VALUEROLE = Qt::UserRole + 1,
    BACKCOLORROLE
};

DPropertySetDlg::DPropertySetDlg(const QString &strPrinter, QWidget *pParent)
    : DSettingsDialog(pParent)
{
    m_strPrinterName = strPrinter;
}

void DPropertySetDlg::initUI()
{
    widgetFactory()->registerWidget("custom-label", [](QObject * obj) -> QWidget * {
        if (DSettingsOption *option = qobject_cast<DSettingsOption *>(obj))
        {
            QString strVal = option->data("text").toString();
            qCDebug(COMMONMOUDLE) << "create custom label:" << option->value();
            DLabel *pLable = new DLabel(FLAGITEMSPACE + strVal);
            return pLable;
        }

        return nullptr;
    });

    widgetFactory()->registerWidget("custom-lineedit", [this](QObject * obj) -> QWidget * {
        if (DSettingsOption *option = qobject_cast<DSettingsOption *>(obj))
        {
            QString strName = option->name();
            QLineEdit *pLineEdit = new QLineEdit;
            pLineEdit->setObjectName(strName);
            m_mapOfListWidget[strName] = pLineEdit;
            return pLineEdit;
        }

        return nullptr;
    });

    widgetFactory()->registerWidget("custom-listview", [this](QObject * obj) -> QWidget * {
        if (DSettingsOption *option = qobject_cast<DSettingsOption *>(obj))
        {
            qCDebug(COMMONMOUDLE) << "create custom list:" << option->value();
            DListView *pListView = new DListView;
            //pListView->setMinimumSize(QSize(300,200));
            QStandardItemModel *pModel = new QStandardItemModel(pListView);
            QString strName = option->name();
            pListView->setObjectName(strName);

            auto itemdata = option->data(QString::fromStdString("items"));

            if (itemdata.type() == QVariant::StringList) {
                QStringList strItems = itemdata.toStringList();

                for (int i = 0; i < strItems.size(); i++) {
                    QString strItem = strItems[i];
                    QStandardItem *pItem = new QStandardItem(FLAGITEMSPACE + strItem);
                    pItem->setSizeHint(QSize(ITEMWIDTH, ITEMHEIGHT));
                    pModel->appendRow(pItem);
                }
            }

            pListView->setModel(pModel);
            m_mapOfListWidget[strName] = pListView;
            return pListView;
        }

        return nullptr;
    });

    widgetFactory()->registerWidget("custom-combobox", [this](QObject * obj) -> QWidget * {
        if (DSettingsOption *option = qobject_cast<DSettingsOption *>(obj))
        {
            qCDebug(COMMONMOUDLE) << "create custom combobox:" << option->value();
            DComboBox *pCombo = new DComboBox;
            QString strName = option->name();
            pCombo->setObjectName(strName);
            m_mapOfListWidget[strName] = pCombo;
            return pCombo;
        }

        return nullptr;
    });

    widgetFactory()->registerWidget("custom-buttons", [this](QObject * obj) -> QWidget * {
        if (DSettingsOption *option = qobject_cast<DSettingsOption *>(obj))
        {
            qCDebug(COMMONMOUDLE) << "create custom button:" << option->value();
            QWidget *pWidget = new QWidget;
            auto itemdata = option->data(QString::fromStdString("items"));

            if (itemdata.type() == QVariant::StringList) {
                QStringList strItems = itemdata.toStringList();
                QHBoxLayout *pHlayout = new QHBoxLayout;

                for (int i = 0; i < strItems.size(); i++) {
                    QString strItem = strItems[i];
                    DPushButton *pBtn = new DPushButton(strItem);
                    pBtn->setObjectName(strItem);
                    m_mapOfListWidget[strItem] = pBtn;
                    pHlayout->addWidget(pBtn);
                }

                pWidget->setLayout(pHlayout);
            }

            return pWidget;
        }

        return nullptr;
    });

    widgetFactory()->registerWidget("custom-pushbutton", [this](QObject * obj) -> QWidget * {
        if (DSettingsOption *option = qobject_cast<DSettingsOption *>(obj)) {
            QString strItem = option->name();
            DPushButton *pBtn = new DPushButton(strItem);
            pBtn->setObjectName(strItem);
            pBtn->setFixedSize(200, 36);
            m_mapOfListWidget[strItem] = pBtn;
            return pBtn;
        }

        return nullptr;
    });

    QTemporaryFile tmpFile;
    tmpFile.open();
    QString strTmpFileName = tmpFile.fileName();
    auto backend = new Dtk::Core::QSettingBackend(strTmpFileName, this);
    QPointer<DSettings> settings;
    QVector<QString> vecOption;
    vecOption.push_back(tr("Print Properties"));
    vecOption.push_back(tr("Driver"));
    vecOption.push_back(tr("URI"));
    vecOption.push_back(tr("Location"));
    vecOption.push_back(tr("Description"));
    vecOption.push_back(tr("Color Mode"));
    vecOption.push_back(tr("Orientation"));
    vecOption.push_back(tr("Page Order"));

    QString strJson = generatePropertyDialogJson(vecOption);
    QVector<GENERALOPTNODE> generalNodes = getGeneralNodes();

    if (!generalNodes.isEmpty()) {
        for (int i = 0; i < generalNodes.size(); i++) {
            GENERALOPTNODE node = generalNodes[i];

            if (node.strOptName == QString("ColorModel") || node.strOptName == QString("ColorMode")) {
                generalNodes.remove(i);
                break;
            }
        }

        QString strGroup = formatGroupString(generalNodes);
        strJson = appendGroupString(strJson, strGroup);
    }

    QVector<INSTALLABLEOPTNODE> nodes = getInstallableNodes();

    if (!nodes.isEmpty()) {
        QString strGroup = formatGroupString(nodes);
        strJson = appendGroupString(strJson, strGroup);
    }

    if (isBishengDriver()) {
        QString strGroup = formatPreferenceString();
        strJson = appendGroupString(strJson, strGroup);
    }
    settings = Dtk::Core::DSettings::fromJson(strJson.toUtf8());
    settings->setBackend(backend);
    updateSettings(settings);
    resize(682, 546);
    setFixedSize(682, 546);
    m_bShow = false;
    setResetVisible(false);
    setWindowIcon(QIcon(":/images/dde-printer.svg"));
    if (settings.data()) {
        settings.data()->deleteLater();
    }
    this->setAccessibleName("propertySetDlg_mainWindow");
}

void DPropertySetDlg::initConnection()
{
    //PageSize_Combo
    //QComboBox
    //Direction_Combo
    QComboBox *pCombo = qobject_cast<DComboBox *>(m_mapOfListWidget[QString::fromStdString("Orientation_Combo")]);
    QObject::connect(pCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(paperOrientation_clicked(int)));

    //ColorMode_Combo
    pCombo = qobject_cast<DComboBox *>(m_mapOfListWidget[QString::fromStdString("ColorMode_Combo")]);
    QObject::connect(pCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(colorModeCombo_clicked(int)));

    //PrintOrder_Combo
    pCombo = qobject_cast<DComboBox *>(m_mapOfListWidget[QString::fromStdString("PrintOrder_Combo")]);
    QObject::connect(pCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(printOrderCombo_clicked(int)));

    //Uri_LineEdit
    QLineEdit *pLineEdit = qobject_cast<QLineEdit *>(m_mapOfListWidget[QString::fromStdString("URI_LineEdit")]);
    QObject::connect(pLineEdit, SIGNAL(editingFinished()), this, SLOT(printUriUI_EditFinished()));

    //Location_LineEdit
    pLineEdit = qobject_cast<QLineEdit *>(m_mapOfListWidget[QString::fromStdString("Location_LineEdit")]);
    QObject::connect(pLineEdit, SIGNAL(editingFinished()), this, SLOT(printLocationUI_EditFinished()));

    //Description_LineEdit
    pLineEdit = qobject_cast<QLineEdit *>(m_mapOfListWidget[QString::fromStdString("Description_LineEdit")]);
    QObject::connect(pLineEdit, SIGNAL(editingFinished()), this, SLOT(printDescriptionUI_EditFinished()));

    //InstallAble_Combo
    QVector<INSTALLABLEOPTNODE> nodes = getInstallableNodes();

    for (int i = 0; i < nodes.size(); i++) {
        QString strOptName = nodes[i].strOptName;
        QString strComboName = QString::fromStdString("%1_Combo").arg(strOptName);
        pCombo = qobject_cast<DComboBox *>(m_mapOfListWidget[strComboName]);
        QObject::connect(pCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(installAbleCombo_clicked(int)));
    }

    //GeneralOpt_Combo
    QVector<GENERALOPTNODE> genNode = getGeneralNodes();

    for (int i = 0; i < genNode.size(); i++) {
        QString strOptName = genNode[i].strOptName;
        QString strComboName = QString::fromStdString("%1_Combo").arg(strOptName);
        pCombo = qobject_cast<DComboBox *>(m_mapOfListWidget[strComboName]);
        QObject::connect(pCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(generalCombo_clicked(int)));
    };

    if (isBishengDriver()) {
        QString strButtonName = PREFERENCE;
        DPushButton *pBtn = qobject_cast<DPushButton *>(m_mapOfListWidget[strButtonName]);
        QObject::connect(pBtn, SIGNAL(clicked()), this,  SLOT(preferenceClickSlot()));
    }
}

void DPropertySetDlg::updateComboByName(const QString &strWidgetName, const QString &strDefault, const QVector<QMap<QString, QString>> &vecChoices)
{
    QComboBox *pCombo = qobject_cast<DComboBox *>(m_mapOfListWidget[strWidgetName]);

    if (nullptr == pCombo) {
        return;
    }

    if (vecChoices.size() > 0) {
        int iIndex = -1;

        for (int i = 0; i < vecChoices.size(); i++) {
            QMap<QString, QString> mapValues = vecChoices[i];
            QString strText = mapValues[QString::fromStdString("text")];
            QString strChoice = mapValues[QString::fromStdString("choice")];
            DPrinterManager *pManger = DPrinterManager::getInstance();
            strText = strText.trimmed();
            strText = pManger->translateLocal(strWidgetName, strChoice, strText);

            if ((!strText.isEmpty()) && (!strChoice.isEmpty())) {
                if (strChoice == strDefault) {
                    iIndex = i;
                }

                pCombo->addItem(strText);
                pCombo->setItemData(i, strChoice, COMBOITEMROLE::VALUEROLE);
            }
        }

        pCombo->setCurrentIndex(iIndex);
    } else {
        DPrinterManager *pManger = DPrinterManager::getInstance();
        QString strText = pManger->translateLocal(strWidgetName, "None", "None");
        pCombo->addItem(strText);
        pCombo->setItemData(0, UNSUPPORTED, COMBOITEMROLE::VALUEROLE);
    }
}

void DPropertySetDlg::updateDriverUI(const QString &strDrive)
{
    QLineEdit *pLineEdit = qobject_cast<QLineEdit *>(m_mapOfListWidget[QString::fromStdString("Driver_LineEdit")]);

    if (nullptr == pLineEdit) {
        return;
    }

    /*
    int iPos = strDrive.lastIndexOf(QDir::separator());
    QString strText = strDrive.right(strDrive.length() - iPos -1);
    iPos = strText.lastIndexOf(".");
    QString strLast = strText.right(strText.length() - iPos);

    if(strLast.contains(".ppd"))
    {
        strText = strText.left(iPos);
    }
    */

    pLineEdit->setText(strDrive);
    pLineEdit->setReadOnly(true);
}

void DPropertySetDlg::updateUriUI(const QString &strUri)
{
    QLineEdit *pLineEdit = qobject_cast<QLineEdit *>(m_mapOfListWidget[QString::fromStdString("URI_LineEdit")]);

    if (nullptr == pLineEdit) {
        return;
    }

    pLineEdit->setText(strUri);
    pLineEdit->setReadOnly(false);
}

void DPropertySetDlg::updateLocationUI(const QString &strLocation)
{
    QLineEdit *pLineEdit = qobject_cast<QLineEdit *>(m_mapOfListWidget[QString::fromStdString("Location_LineEdit")]);

    if (nullptr == pLineEdit) {
        return;
    }

    pLineEdit->setText(strLocation);
    pLineEdit->setReadOnly(false);
}

void DPropertySetDlg::updateDescriptionUI(const QString &strDescription)
{
    QLineEdit *pLineEdit = qobject_cast<QLineEdit *>(m_mapOfListWidget[QString::fromStdString("Description_LineEdit")]);

    if (nullptr == pLineEdit) {
        return;
    }

    pLineEdit->setText(strDescription);
    pLineEdit->setReadOnly(false);
}

void DPropertySetDlg::updatePageSizeCombo(const QString &strDefault, const QVector<QMap<QString, QString>> &vecChoices)
{
    m_mapInitUIValue[QString::fromStdString("PageSize_Combo")] = strDefault;
    m_mapDynamicUIValue[QString::fromStdString("PageSize_Combo")] = strDefault;
    updateComboByName(QString::fromStdString("PageSize_Combo"), strDefault, vecChoices);
}

void DPropertySetDlg::setPrinterName(const QString &strName)
{
    m_strPrinterName = strName;
}

void DPropertySetDlg::updateViews()
{
    DPrinterManager *pManager = DPrinterManager::getInstance();
    DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

    if (pDest != nullptr) {
        if (DESTTYPE::PRINTER == pDest->getType()) {
            DPrinter *pPrinter = static_cast<DPrinter *>(pDest);

            //Update Driver UI
            QString strDriver = pPrinter->getPrinterMakeAndModel();
            updateDriverUI(strDriver);

            //Update Uri UI
            QString strUri = pPrinter->getPrinterUri();
            updateUriUI(strUri);

            //Update Location UI
            QString strLoc = pPrinter->printerLocation();
            updateLocationUI(strLoc);

            //Update Description UI
            QString strDes = pPrinter->printerInfo();
            updateDescriptionUI(strDes);

            //Update ColorModel UI
            QVector<QMap<QString, QString>> vecChoice = pPrinter->getColorModelChooses();
            QString strDefault = pPrinter->getColorModel();
            updateColorModeCombo(strDefault, vecChoice);

            //Update Orientation UI
            strDefault = pPrinter->getPageOrientation();
            vecChoice = pPrinter->getPageOrientationChooses();
            updateOrientationCombo(strDefault, vecChoice);

            //Update PrintOrder UI
            strDefault = pPrinter->getPageOutputOrder();
            vecChoice = pPrinter->getPageOutputOrderChooses();
            updatePrintOrderCombo(strDefault, vecChoice);

            //Update Installable UI
            QVector<INSTALLABLEOPTNODE> nodes = pPrinter->getInstallableNodes();
            updateInstallAbleNodeCombo(nodes);

            //Update General Option UI
            QVector<INSTALLABLEOPTNODE> node2 = pPrinter->getGeneralNodes();
            updateGeneralNodeCombo(node2);

            //Init ConflictOption with Combobox
            initConflictMap(nodes, node2);
        }
    }
}

void DPropertySetDlg::updateOrientationCombo(const QString &strDefault, const QVector<QMap<QString, QString>> &vecChoices)
{
    updateComboByName(QString::fromStdString("Orientation_Combo"), strDefault, vecChoices);
}

void DPropertySetDlg::updateDuplexPrintCombo(const QString &strDefault, const QVector<QMap<QString, QString>> &vecChoices)
{
    m_mapInitUIValue[QString::fromStdString("Duplex_Combo")] = strDefault;
    m_mapDynamicUIValue[QString::fromStdString("Duplex_Combo")] = strDefault;
    updateComboByName(QString::fromStdString("Duplex_Combo"), strDefault, vecChoices);
}

void DPropertySetDlg::updateColorModeCombo(const QString &strDefault, const QVector<QMap<QString, QString>> &vecChoices)
{
    m_mapInitUIValue[QString::fromStdString("ColorMode_Combo")] = strDefault;
    m_mapDynamicUIValue[QString::fromStdString("ColorMode_Combo")] = strDefault;
    updateComboByName(QString::fromStdString("ColorMode_Combo"), strDefault, vecChoices);
}

void DPropertySetDlg::updatePrintOrderCombo(const QString &strDefault, const QVector<QMap<QString, QString>> &vecChoices)
{
    updateComboByName(QString::fromStdString("PrintOrder_Combo"), strDefault, vecChoices);
}

void DPropertySetDlg::updatePaperOriginCombo(const QString &strDefault, const QVector<QMap<QString, QString>> &vecChoices)
{
    m_mapInitUIValue[QString::fromStdString("PaperOrigin_Combo")] = strDefault;
    m_mapDynamicUIValue[QString::fromStdString("PaperOrigin_Combo")] = strDefault;
    updateComboByName(QString::fromStdString("PaperOrigin_Combo"), strDefault, vecChoices);
}

void DPropertySetDlg::updatePaperTypeCombo(const QString &strDefault, const QVector<QMap<QString, QString>> &vecChoices)
{
    m_mapInitUIValue[QString::fromStdString("PaperType_Combo")] = strDefault;
    m_mapDynamicUIValue[QString::fromStdString("PaperType_Combo")] = strDefault;
    updateComboByName(QString::fromStdString("PaperType_Combo"), strDefault, vecChoices);
}

void DPropertySetDlg::updateOutputQuanlityCombo(const QString &strDefault, const QVector<QMap<QString, QString>> &vecChoices)
{
    m_mapInitUIValue[QString::fromStdString("OutputQuanlity_Combo")] = strDefault;
    m_mapDynamicUIValue[QString::fromStdString("OutputQuanlity_Combo")] = strDefault;
    updateComboByName(QString::fromStdString("OutputQuanlity_Combo"), strDefault, vecChoices);
}

void DPropertySetDlg::updateStapleLocationCombo(const QString &strDefault, const QVector<QMap<QString, QString>> &vecChoices)
{
    updateComboByName(QString::fromStdString("StapleLocation_Combo"), strDefault, vecChoices);
}

void DPropertySetDlg::updateBindEdgeCombo(const QString &strDefault, const QVector<QMap<QString, QString>> &vecChoices)
{
    m_mapInitUIValue[QString::fromStdString("BindEdge_Combo")] = strDefault;
    m_mapDynamicUIValue[QString::fromStdString("BindEdge_Combo")] = strDefault;
    updateComboByName(QString::fromStdString("BindEdge_Combo"), strDefault, vecChoices);
}

void DPropertySetDlg::updateResolutionCombo(const QString &strDefault, const QVector<QMap<QString, QString>> &vecChoices)
{
    m_mapInitUIValue[QString::fromStdString("Resolution_Combo")] = strDefault;
    m_mapDynamicUIValue[QString::fromStdString("Resolution_Combo")] = strDefault;
    updateComboByName(QString::fromStdString("Resolution_Combo"), strDefault, vecChoices);
}

void DPropertySetDlg::updateInstallAbleNodeCombo(const QVector<INSTALLABLEOPTNODE> &nodes)
{
    for (int i = 0; i < nodes.size(); i++) {
        QString strOptName = nodes[i].strOptName;
        QString strComboName = QString::fromStdString("%1_Combo").arg(strOptName);
        m_mapInitUIValue[strComboName] = nodes[i].strDefaultValue;
        m_mapDynamicUIValue[strComboName] = nodes[i].strDefaultValue;
        updateComboByName(strComboName, nodes[i].strDefaultValue, nodes[i].vecChooseableValues);
    }
}

void DPropertySetDlg::updateGeneralNodeCombo(const QVector<GENERALOPTNODE> &nodes)
{
    for (int i = 0; i < nodes.size(); i++) {
        QString strOptName = nodes[i].strOptName;
        QString strComboName = QString::fromStdString("%1_Combo").arg(strOptName);
        m_mapInitUIValue[strComboName] = nodes[i].strDefaultValue;
        m_mapDynamicUIValue[strComboName] = nodes[i].strDefaultValue;
        updateComboByName(strComboName, nodes[i].strDefaultValue, nodes[i].vecChooseableValues);
    }
}

QString DPropertySetDlg::changeComboSelectionByName(const QString &strComboName, int iIndex)
{
    DComboBox *pCombo = qobject_cast<DComboBox *>(m_mapOfListWidget[strComboName]);

    if (nullptr == pCombo) {
        return UNSUPPORTED;
    }

    QString strChoice = pCombo->itemData(iIndex, COMBOITEMROLE::VALUEROLE).toString();
    QString strText = pCombo->currentText();

    if (strChoice == UNSUPPORTED) {
        return UNSUPPORTED;
    }

    //依据模块进行冲突判断
    vector<CONFLICTPAIR> vecConflictOptionPairs;
    checkAllConflicts(m_setConflictOptions, vecConflictOptionPairs);

    if (m_setConflictOptions.size() > 0) {
        showConflictDlg(vecConflictOptionPairs);
        m_mapDynamicUIValue[strComboName] = strChoice;
    } else {
        m_mapDynamicUIValue[strComboName] = strChoice;
    }

    updateComboByConflits(m_setConflictOptions);
    return strChoice;
}

bool DPropertySetDlg::haveConflict(const QString &strModule, const QString &strValue,
                                   QVector<CONFLICTNODE> &vecConflictNode)
{
    bool bRet = true;
    DPrinterManager *pManager = DPrinterManager::getInstance();
    DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

    if (DESTTYPE::PRINTER == pDest->getType()) {
        DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
        bRet = pPrinter->isConflict(strModule, strValue, vecConflictNode);
    }

    return bRet;
}

bool DPropertySetDlg::isConflict(const QString &strConflictOption, const QString &strConflictValue)
{
    QString strWidgetName = m_mapOfConflict.value(strConflictOption);

    if (strWidgetName.isEmpty()) {
        return false;
    }

    QString strCurrentValue;
    QComboBox *pCombo = qobject_cast<QComboBox *>(m_mapOfListWidget[strWidgetName]);
    int iIndex = pCombo->currentIndex();
    strCurrentValue = pCombo->itemData(iIndex, COMBOITEMROLE::VALUEROLE).toString();
    return (strCurrentValue == strConflictValue);
}

void DPropertySetDlg::initConflictMap(const QVector<INSTALLABLEOPTNODE> &nodes1, const QVector<GENERALOPTNODE> &nodes2)
{
    m_mapOfConflict.clear();
    /*
    m_mapOfConflict.insert(QString::fromStdString("Duplex"), QString::fromStdString("Duplex_Combo"));
    m_mapOfConflict.insert(QString::fromStdString("MediaType"), QString::fromStdString("PaperType_Combo"));
    m_mapOfConflict.insert(QString::fromStdString("PageSize"), QString::fromStdString("PageSize_Combo"));
    m_mapOfConflict.insert(QString::fromStdString("InputSlot"), QString::fromStdString("PaperOrigin_Combo"));
    m_mapOfConflict.insert(QString::fromStdString("BindEdge"), QString::fromStdString("BindEdge_Combo"));
    m_mapOfConflict.insert(QString::fromStdString("StapleLocation"), QString::fromStdString("StapleLocation_Combo"));
    */

    for (int i = 0; i < nodes1.size(); i++) {
        QString strWidgetName = QString::fromStdString("%1_Combo").arg(nodes1[i].strOptName);
        m_mapOfConflict.insert(nodes1[i].strOptName, strWidgetName);
    }

    for (int i = 0; i < nodes2.size(); i++) {
        QString strWidgetName = QString::fromStdString("%1_Combo").arg(nodes2[i].strOptName);
        m_mapOfConflict.insert(nodes2[i].strOptName, strWidgetName);
    }
}

void DPropertySetDlg::showConflictDlg(const vector<CONFLICTPAIR> &vecConflictPairs)
{
    QWidget *pWidget = new QWidget;
    DLabel *pLabel1 = new DLabel(tr("Options conflict!"));
    DFontSizeManager::instance()->bind(pLabel1, DFontSizeManager::T6, QFont::DemiBold);
    pLabel1->setAlignment(Qt::AlignCenter);
    DLabel *pLabel2 = new DLabel(tr("Please resolve the conflict first, and then save the changes."));
    DFontSizeManager::instance()->bind(pLabel2, DFontSizeManager::T6, QFont::DemiBold);
    pLabel2->setAlignment(Qt::AlignCenter);
    DPrinterManager *pManager = DPrinterManager::getInstance();
    DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);
    DPrinter *pPrinter = nullptr;
    QString strValue1, strValue2;

    if (pDest != nullptr) {
        if (DESTTYPE::PRINTER == pDest->getType()) {
            pPrinter = static_cast<DPrinter *>(pDest);
            OPTNODE node = pPrinter->getOptionNodeByKeyword(vecConflictPairs[0].strOpt1);
            strValue1 = node.strOptText;
            node = pPrinter->getOptionNodeByKeyword(vecConflictPairs[0].strOpt2);
            strValue2 = node.strOptText;
        }
    }
    QString strOpt1 = pManager->translateLocal(m_mapOfConflict.value(vecConflictPairs[0].strOpt1), vecConflictPairs[0].strOpt1, strValue1);
    QString strOpt2 = pManager->translateLocal(m_mapOfConflict.value(vecConflictPairs[0].strOpt2), vecConflictPairs[0].strOpt2, strValue2);
    QLabel *pLabel3 = new DLabel(tr("Conflict:") + strOpt1 + "\t,\t" + strOpt2);
    DFontSizeManager::instance()->bind(pLabel3, DFontSizeManager::T6, QFont::DemiBold);
    pLabel3->setAlignment(Qt::AlignCenter);
    QVBoxLayout *pLayout = new QVBoxLayout;
    pLayout->addWidget(pLabel1);
    pLayout->addWidget(pLabel2);
    pLayout->addWidget(pLabel3);
    pWidget->setLayout(pLayout);
    DDialog dialog(this);
    //dialog.setFixedSize(450, 172);
    dialog.addContent(pWidget);
    dialog.addButton(tr("OK"));
    dialog.setIcon(QPixmap(":/images/warning_logo.svg"));
    QPoint ptCen1 = this->geometry().center();
    QPoint ptCen2 = dialog.geometry().center();
    QPoint mvPhasor = ptCen1 - ptCen2;
    QRect geo = dialog.geometry();
    geo.moveCenter(mvPhasor + geo.center());
    dialog.setGeometry(geo);
    dialog.exec();
}

void DPropertySetDlg::updateComboByConflits(const QSet<QString> &nodes)
{
    for (auto iter = m_mapOfConflict.begin(); iter != m_mapOfConflict.end(); iter++) {
        if (nodes.contains(iter.key())) {
            continue;
        }

        QString strWidgetName = iter.value();
        QComboBox *pCombo = qobject_cast<QComboBox *>(m_mapOfListWidget.value(strWidgetName));
        QPalette pal = pCombo->palette();
        pal.setColor(QPalette::ButtonText, Qt::black);
        pCombo->setPalette(pal);
    }

    foreach (QString strOpt, nodes) {
        QString strWidgetName = m_mapOfConflict.value(strOpt);
        QComboBox *pCombo = qobject_cast<QComboBox *>(m_mapOfListWidget.value(strWidgetName));
        QPalette pal = pCombo->palette();
        pal.setColor(QPalette::ButtonText, Qt::red);
        pCombo->setPalette(pal);
    }
}

void DPropertySetDlg::checkAllConflicts(QSet<QString> &setOptions, vector<CONFLICTPAIR> &vecOptionPairs)
{
    bool bHaveConflict = false;
    CONFLICTNODE node;
    m_setConflictOptions.clear();

    for (auto iter = m_mapOfConflict.begin(); iter != m_mapOfConflict.end(); iter++) {
        QComboBox *pCombo = qobject_cast<QComboBox *>(m_mapOfListWidget.value(iter.value()));

        if (pCombo == nullptr) {
            continue;
        }

        int iCurIndex = pCombo->currentIndex();
        QString strCheckValue = pCombo->itemData(iCurIndex, COMBOITEMROLE::VALUEROLE).toString();
        QString strCheckOption = iter.key();
        QVector<CONFLICTNODE> vecConflicts;
        bHaveConflict = haveConflict(strCheckOption, strCheckValue, vecConflicts);

        if (bHaveConflict) {
            bHaveConflict = false;

            for (int i = 0; i < vecConflicts.size(); i++) {
                node = vecConflicts[i];
                bHaveConflict = isConflict(node.strOption, node.strValue);

                if (bHaveConflict) {
                    setOptions.insert(strCheckOption);
                    setOptions.insert(node.strOption);
                    CONFLICTPAIR pair;
                    pair.strOpt1 = strCheckOption;
                    pair.strOpt2 = node.strOption;
                    vecOptionPairs.push_back(pair);
                }
            }
        }
    }
}

QVector<INSTALLABLEOPTNODE> DPropertySetDlg::getInstallableNodes()
{
    QVector<INSTALLABLEOPTNODE> nodes;
    DPrinterManager *pManager = DPrinterManager::getInstance();
    DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

    if (pDest != nullptr) {
        if (DESTTYPE::PRINTER == pDest->getType()) {
            DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
            nodes = pPrinter->getInstallableNodes();
        }
    }

    return nodes;
}

QVector<GENERALOPTNODE> DPropertySetDlg::getGeneralNodes()
{
    QVector<GENERALOPTNODE> nodes;
    DPrinterManager *pManager = DPrinterManager::getInstance();
    DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

    if (pDest != nullptr) {
        if (DESTTYPE::PRINTER == pDest->getType()) {
            DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
            nodes = pPrinter->getGeneralNodes();
        }
    }

    return nodes;
}

void DPropertySetDlg::pageSizeCombo_clicked(int iIndex)
{
    if (!m_bShow) {
        return;
    }

    QString strChoice = changeComboSelectionByName(QString::fromStdString("PageSize_Combo"), iIndex);

    if (strChoice == UNSUPPORTED) {
        return;
    }

    if (!strChoice.isEmpty()) {
        DPrinterManager *pManager = DPrinterManager::getInstance();
        DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

        if (pDest != nullptr) {
            if (DESTTYPE::PRINTER == pDest->getType()) {
                DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
                pPrinter->setPageSize(strChoice);
            }
        }
    }
}

void DPropertySetDlg::paperOrientation_clicked(int index)
{
    if (!m_bShow) {
        return;
    }

    QString strChoice = changeComboSelectionByName(QString::fromStdString("Orientation_Combo"), index);

    if (strChoice == UNSUPPORTED) {
        return;
    }

    if (!strChoice.isEmpty()) {
        DPrinterManager *pManager = DPrinterManager::getInstance();
        DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

        if (pDest != nullptr) {
            if (DESTTYPE::PRINTER == pDest->getType()) {
                DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
                //need set direction
                strChoice = strChoice.trimmed();
                pPrinter->setPageOrientationChoose(strChoice);
            }
        }
    }
}

void DPropertySetDlg::duplexPrintCombo_clicked(int index)
{
    if (!m_bShow) {
        return;
    }

    QString strChoice = changeComboSelectionByName(QString::fromStdString("Duplex_Combo"), index);

    if (strChoice == UNSUPPORTED) {
        return;
    }

    if (!strChoice.isEmpty()) {
        DPrinterManager *pManager = DPrinterManager::getInstance();
        DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

        if (pDest != nullptr) {
            if (DESTTYPE::PRINTER == pDest->getType()) {
                DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
                pPrinter->setDuplex(strChoice);
            }
        }
    }
}

void DPropertySetDlg::colorModeCombo_clicked(int index)
{
    if (!m_bShow) {
        return;
    }

    QString strChoice = changeComboSelectionByName(QString::fromStdString("ColorMode_Combo"), index);

    if (strChoice == UNSUPPORTED) {
        return;
    }

    if (!strChoice.isEmpty()) {
        DPrinterManager *pManager = DPrinterManager::getInstance();
        DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

        if (pDest != nullptr) {
            if (DESTTYPE::PRINTER == pDest->getType()) {
                DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
                pPrinter->setColorModel(strChoice);
            }
        }
    }
}

void DPropertySetDlg::printOrderCombo_clicked(int index)
{
    if (!m_bShow) {
        return;
    }

    QString strChoice = changeComboSelectionByName(QString::fromStdString("PrintOrder_Combo"), index);

    if (strChoice == UNSUPPORTED) {
        return;
    }

    if (!strChoice.isEmpty()) {
        DPrinterManager *pManager = DPrinterManager::getInstance();
        DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

        if (pDest != nullptr) {
            if (DESTTYPE::PRINTER == pDest->getType()) {
                DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
                //need set Print Order;
                pPrinter->setPageOutputOrder(strChoice);
            }
        }
    }
}

void DPropertySetDlg::paperOriginCombo_clicked(int index)
{
    if (!m_bShow) {
        return;
    }

    QString strChoice = changeComboSelectionByName(QString::fromStdString("PaperOrigin_Combo"), index);

    if (strChoice == UNSUPPORTED) {
        return;
    }

    if (!strChoice.isEmpty()) {
        DPrinterManager *pManager = DPrinterManager::getInstance();
        DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

        if (pDest != nullptr) {
            if (DESTTYPE::PRINTER == pDest->getType()) {
                DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
                pPrinter->setInputSlot(strChoice);
            }
        }
    }
}

void DPropertySetDlg::paperTypeCombo_clicked(int index)
{
    if (!m_bShow) {
        return;
    }

    QString strChoice = changeComboSelectionByName(QString::fromStdString("PaperType_Combo"), index);

    if (strChoice == UNSUPPORTED) {
        return;
    }

    if (!strChoice.isEmpty()) {
        DPrinterManager *pManager = DPrinterManager::getInstance();
        DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

        if (pDest != nullptr) {
            if (DESTTYPE::PRINTER == pDest->getType()) {
                DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
                pPrinter->setMediaType(strChoice);
            }
        }
    }
}

void DPropertySetDlg::printStapleLocationCombo_clicked(int index)
{
    if (!m_bShow) {
        return;
    }

    QString strChoice = changeComboSelectionByName(QString::fromStdString("StapleLocation_Combo"), index);

    if (strChoice == UNSUPPORTED) {
        return;
    }

    if (!strChoice.isEmpty()) {
        DPrinterManager *pManager = DPrinterManager::getInstance();
        DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

        if (pDest != nullptr) {
            if (DESTTYPE::PRINTER == pDest->getType()) {
                DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
                pPrinter->setStapleLoaction(strChoice);
            }
        }
    }
}

void DPropertySetDlg::printBindEdgeCombo_clicked(int index)
{
    if (!m_bShow) {
        return;
    }

    QString strChoice = changeComboSelectionByName(QString::fromStdString("BindEdge_Combo"), index);

    if (strChoice == UNSUPPORTED) {
        return;
    }

    if (!strChoice.isEmpty()) {
        DPrinterManager *pManager = DPrinterManager::getInstance();
        DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

        if (pDest != nullptr) {
            if (DESTTYPE::PRINTER == pDest->getType()) {
                DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
                pPrinter->setBindEdgetOption(strChoice);
            }
        }
    }
}

void DPropertySetDlg::printResolutionCombo_clicked(int index)
{
    if (!m_bShow) {
        return;
    }

    QString strChoice = changeComboSelectionByName(QString::fromStdString("Resolution_Combo"), index);

    if (strChoice == UNSUPPORTED) {
        return;
    }

    if (!strChoice.isEmpty()) {
        DPrinterManager *pManager = DPrinterManager::getInstance();
        DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

        if (pDest != nullptr) {
            if (DESTTYPE::PRINTER == pDest->getType()) {
                DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
                pPrinter->setResolution(strChoice);
            }
        }
    }
}

void DPropertySetDlg::outputQuanlity_clicked(int index)
{
    if (!m_bShow) {
        return;
    }

    QString strChoice = changeComboSelectionByName(QString::fromStdString("OutputQuanlity_Combo"), index);

    if (strChoice == UNSUPPORTED) {
        return;
    }

    if (!strChoice.isEmpty()) {
        DPrinterManager *pManager = DPrinterManager::getInstance();
        DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

        if (pDest != nullptr) {
            if (DESTTYPE::PRINTER == pDest->getType()) {
                DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
                pPrinter->setPrintQuality(strChoice);
            }
        }
    }
}

void DPropertySetDlg::installAbleCombo_clicked(int index)
{
    if (!m_bShow) {
        return;
    }

    QWidget *pFocusedWidget = nullptr;

    for (auto iter = m_mapOfListWidget.begin(); iter != m_mapOfListWidget.end(); iter++) {
        QWidget *pWidget = iter.value();

        if (pWidget->hasFocus()) {
            pFocusedWidget = pWidget;
            break;
        }
    }

    QString strWidgetName = pFocusedWidget->objectName();
    QString strChoice = changeComboSelectionByName(strWidgetName, index);

    if (strChoice == UNSUPPORTED) {
        return;
    }

    if (!strChoice.isEmpty()) {
        DPrinterManager *pManager = DPrinterManager::getInstance();
        DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

        if (pDest != nullptr) {
            if (DESTTYPE::PRINTER == pDest->getType()) {
                QString strOptName = strWidgetName.remove("_Combo");
                DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
                pPrinter->setInstallableNodeValue(strOptName, strChoice);
            }
        }
    }
}

void DPropertySetDlg::generalCombo_clicked(int index)
{
    if (!m_bShow) {
        return;
    }

    QWidget *pFocusedWidget = nullptr;

    for (auto iter = m_mapOfListWidget.begin(); iter != m_mapOfListWidget.end(); iter++) {
        QString strName = iter.key();
        QWidget *pWidget = iter.value();

        if (pWidget->hasFocus()) {
            pFocusedWidget = pWidget;
            break;
        }
    }

    QString strWidgetName = pFocusedWidget->objectName();
    QString strChoice = changeComboSelectionByName(strWidgetName, index);

    if (strChoice == UNSUPPORTED) {
        return;
    }

    if (!strChoice.isEmpty()) {
        DPrinterManager *pManager = DPrinterManager::getInstance();
        DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

        if (pDest != nullptr) {
            if (DESTTYPE::PRINTER == pDest->getType()) {
                QString strOptName = strWidgetName.remove("_Combo");
                DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
                pPrinter->setGeneralNodeValue(strOptName, strChoice);
            }
        }
    }
}

void DPropertySetDlg::printUriUI_EditFinished()
{
    if (!m_bShow) {
        return;
    }

    QLineEdit *pLineEdit = qobject_cast<QLineEdit *>(m_mapOfListWidget[QString::fromStdString("URI_LineEdit")]);

    if (nullptr == pLineEdit) {
        return;
    }

    QString strVal = pLineEdit->text().trimmed();

    if (strVal.isEmpty()) {
        return;
    }

    QRegExp reg("([^# ]{1,})(:[/]{1,})([^# ]{1,})");
    QRegExpValidator v(reg);
    int pos = 0;
    QValidator::State state = v.validate(strVal, pos);

    if (state != QValidator::Acceptable) {
        DDialog dialog;
        dialog.setFixedSize(QSize(400, 150));
        dialog.setMessage(tr("Invalid URI"));
        dialog.addButton(tr("OK"));
        dialog.setIcon(QPixmap(":/images/warning_logo.svg"));
        dialog.exec();
        return;
    }

    DPrinterManager *pManager = DPrinterManager::getInstance();
    DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

    if (pDest != nullptr) {
        if (DESTTYPE::PRINTER == pDest->getType()) {
            DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
            pPrinter->setPrinterUri(strVal);
        }
    }
}

void DPropertySetDlg::printLocationUI_EditFinished()
{
    if (!m_bShow) {
        return;
    }

    QLineEdit *pLineEdit = qobject_cast<QLineEdit *>(m_mapOfListWidget[QString::fromStdString("Location_LineEdit")]);

    if (nullptr == pLineEdit) {
        return;
    }

    QString strVal = pLineEdit->text();

    if (strVal.isEmpty()) {
        return;
    }

    DPrinterManager *pManager = DPrinterManager::getInstance();
    DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

    if (pDest != nullptr) {
        if (DESTTYPE::PRINTER == pDest->getType()) {
            DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
            pPrinter->setPrinterLocation(strVal);
        }
    }
}

void DPropertySetDlg::printDescriptionUI_EditFinished()
{
    if (!m_bShow) {
        return;
    }

    QLineEdit *pLineEdit = qobject_cast<QLineEdit *>(m_mapOfListWidget[QString::fromStdString("Description_LineEdit")]);

    if (nullptr == pLineEdit) {
        return;
    }

    QString strVal = pLineEdit->text();

    if (strVal.isEmpty()) {
        return;
    }

    DPrinterManager *pManager = DPrinterManager::getInstance();
    DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

    if (pDest != nullptr) {
        if (DESTTYPE::PRINTER == pDest->getType()) {
            DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
            pPrinter->setPrinterInfo(strVal);
        }
    }
}

void DPropertySetDlg::confirmBtn_clicked()
{
}

void DPropertySetDlg::cancelBtn_clicked()
{
}

void DPropertySetDlg::moveToParentCenter()
{
    QWidget *pParent = parentWidget();

    if (pParent == nullptr) {
        return;
    } else {
        QRect parentRc = pParent->geometry();
        QPoint parentCen = parentRc.center();
        QRect selfRc = geometry();
        QPoint selfCen = selfRc.center();
        QPoint selfCenInParent = mapToParent(selfCen);
        QPoint mvPhasor = parentCen - selfCenInParent;
        move(mvPhasor.x(), mvPhasor.y());
    }
}

bool DPropertySetDlg::isDriveBroken()
{
    bool bRet = true;
    DPrinterManager *pManager = DPrinterManager::getInstance();
    DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

    if (pDest != nullptr) {
        if (DESTTYPE::PRINTER == pDest->getType()) {
            DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
            bRet = pPrinter->isPpdFileBroken();
        } else {
            bRet = false;
        }
    }

    return bRet;
}

void DPropertySetDlg::showEvent(QShowEvent *event)
{
    vector<CONFLICTPAIR> vecConflictOptionPairs;
    checkAllConflicts(m_setConflictOptions, vecConflictOptionPairs);

    if (m_setConflictOptions.size() > 0) {
        showConflictDlg(vecConflictOptionPairs);
        updateComboByConflits(m_setConflictOptions);
    }

    m_bShow = true;
    DSettingsDialog::showEvent(event);
}

void DPropertySetDlg::closeEvent(QCloseEvent *event)
{
    DPrinterManager *pManager = DPrinterManager::getInstance();
    DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);
    DPrinter *pPrinter = nullptr;

    if (pDest != nullptr) {
        if (DESTTYPE::PRINTER == pDest->getType()) {
            pPrinter = static_cast<DPrinter *>(pDest);

            if (!pPrinter->needSavePpd()) {
                return;
            } else {
                vector<CONFLICTPAIR> vecConflictOptionPairs;
                checkAllConflicts(m_setConflictOptions, vecConflictOptionPairs);

                if (m_setConflictOptions.size() > 0) {
                    return;
                }

                bool bSame = true;

                for (auto iter = m_mapInitUIValue.begin(); iter != m_mapInitUIValue.end(); iter++) {
                    QString strValue1 = iter.value();
                    QString strValue2 = m_mapDynamicUIValue[iter.key()];

                    if (strValue1 != strValue2) {
                        bSame = false;
                        break;
                    }
                }

                if ((!bSame) && (0 == m_setConflictOptions.size())) {
                    pPrinter->saveModify();
                }
            }
        }
    }

    DSettingsDialog::closeEvent(event);
}

bool DPropertySetDlg::isBishengDriver()
{
    DPrinterManager *pManager = DPrinterManager::getInstance();
    DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);
    QString strDriver;

    if (pDest != nullptr) {
        if (DESTTYPE::PRINTER == pDest->getType()) {
            DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
            strDriver = pPrinter->getPrinterMakeAndModel();
            m_strPpdName = pPrinter->getDriverName();
        }
    }

    if (!strDriver.contains("Bisheng")) {
        return false;
    }

    // 毕昇驱动支持首选项时pdd字段BishengPreferences为true
    QString preference = getBishengInfo("BishengPreferences");
    if (preference.isEmpty()) {
        return false;
    }

    preference.replace(QRegExp(" "), "").replace(QRegExp("\""), "");
    return preference.toLower() == "true";
}

void DPropertySetDlg::preferenceClickSlot()
{
    QStringList args;
    QString model = getBishengInfo("PrinterName").replace(QRegExp("^ "), "").replace("\"", "");
    QString prefix = getBishengInfo("PackageName").replace(QRegExp("^ "), "").replace(QRegExp("\""), "");

    args << "setprinter" << prefix << model;
    qCInfo(COMMONMOUDLE) << args;
    QProcess::startDetached("/bin/bash", args, "/usr/bin");
}

QString DPropertySetDlg::getBishengInfo(QString info)
{
    QString strOut, strErr;
    QString commond;
    commond += "cat " + m_strPpdName;
    if (shellCmd(commond, strOut, strErr) == 0) {
        QStringList list = strOut.split("\n", QString::SkipEmptyParts);
        int index = 0;
        for (; index < list.size(); index++) {
            if (list[index].contains(info))
            break;
        }
        if (index == list.size()) {
            return nullptr;
        }

        QStringList val = list[index].split(":", QString::SkipEmptyParts);
        if (val.count() <= 1) {
            return nullptr;
        }
        return val[1];
    }

    return nullptr;
}
