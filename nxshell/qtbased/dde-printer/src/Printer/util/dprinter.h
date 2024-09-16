// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DPRINTER_H
#define DPRINTER_H

#include "cupsconnection.h"
#include "cupsppd.h"
#include "cupssnmp.h"
#include "ddestination.h"

#include <QStringList>
#include <QObject>
#include <QVector>
#include <QString>
#include <QMap>

typedef struct conflictNode {
    QString strOption;
    QString strValue;

    conflictNode()
    {
        strOption = "";
        strValue = "";
    }

    conflictNode(const conflictNode &other)
    {
        if (this != &other) {
            strOption = other.strOption;
            strValue = other.strValue;
        }
    }

    conflictNode &operator=(const conflictNode &other)
    {
        if (this != &other) {
            strOption = other.strOption;
            strValue = other.strValue;
        }

        return *this;
    }
} CONFLICTNODE;

typedef struct conflictPair {
    QString strOpt1;
    QString strOpt2;
} CONFLICTPAIR;

typedef struct OptNode {
    QString strOptName;
    QString strOptText;
    QString strDefaultValue;
    QVector<QMap<QString, QString>> vecChooseableValues;
} OPTNODE;

typedef OPTNODE INSTALLABLEOPTNODE;
typedef OPTNODE GENERALOPTNODE;

class DPrinter : public DDestination
{

public:
    DPrinter();
    virtual ~DPrinter() override;

    //operator ppd
    virtual bool initPrinterPPD() override;
    virtual bool isPpdFileBroken() override;

    //PageSize PPDFILE
    void setPageSize(const QString &strValue);
    QString getPageSize();
    QVector<QMap<QString, QString>> getPageSizeChooses();

    //PageRegion PPDFILE
    void setPageRegion(const QString &strValue);
    QString getPageRegion();
    QVector<QMap<QString, QString>> getPageRegionChooses();

    //MediaType PPDFILE
    void setMediaType(const QString &strValue);
    QString getMediaType();
    QVector<QMap<QString, QString>> getMediaTypeChooses();

    //ColorModel PPDFILE
    bool canSetColorModel();
    void setColorModel(const QString &strColorMode);
    QString getColorModel();
    QVector<QMap<QString, QString>> getColorModelChooses();

    //OutputMode/Print Quality PPDFILE
    void setPrintQuality(const QString &strValue);
    QString getPrintQuality();
    QVector<QMap<QString, QString>> getPrintQualityChooses();

    //InputSlot PPDFILE
    void setInputSlot(const QString &strValue);
    QString getInputSlot();
    QVector<QMap<QString, QString>> getInputSlotChooses();

    //Duplex: long, short, none PPDFILE
    bool canSetDuplex();
    void setDuplex(const QString &strValue);
    QString getDuplex();
    QVector<QMap<QString, QString>> getDuplexChooses();

    //Driver
    QString getDriverName();
    QString getPrinterMakeAndModel();
    //URI
    QString getPrinterUri();
    void setPrinterUri(const QString &strValue);

    //page orientation  ATTRIBUTES
    QString getPageOrientation();
    void setPageOrientationChoose(const QString &strValue);
    QVector<QMap<QString, QString>> getPageOrientationChooses();

    //page reverse ATTRIBUTES
    QString getPageOutputOrder();
    void setPageOutputOrder(const QString &strValue);
    QVector<QMap<QString, QString>> getPageOutputOrderChooses();

    //page BindEdge ATTRIBUTES
    QString getBindEdgeOption();
    void setBindEdgetOption(const QString &strValue);
    QVector<QMap<QString, QString>> getBindEdgeChooses();

    //page Finishings
    QString getFinishings();
    void setFinishings(const QString &strValue);
    QVector<QMap<QString, QString>> getFinishingsChooses();

    //stapleLocation
    QString getStapleLocation();
    void setStapleLoaction(const QString &strVal);
    QVector<QMap<QString, QString>> getStapLocationChooses();

    //print Resolution
    QString getResolution();
    void setResolution(const QString &strValue);
    QVector<QMap<QString, QString>> getResolutionChooses();

    //read marker
    QVector<SUPPLYSDATA> getSupplys();
    void updateSupplys();
    void disableSupplys();
    int getMinMarkerLevel();
    void SetSupplys(const QVector<SUPPLYSDATA> &vecMarkInfo);


    QStringList getDefaultPpdOpts();

    //Save Modify
    bool saveModify();
    bool needSavePpd();

    //conflicts check
    bool isConflict(const QString &, const QString &, QVector<CONFLICTNODE> &);

    //读取可安装选项
    QVector<INSTALLABLEOPTNODE> getInstallableNodes();
    void setInstallableNodeValue(const QString &strOpt, const QString &strValue);

    //读取常规选项
    QVector<GENERALOPTNODE> getGeneralNodes();
    void setGeneralNodeValue(const QString &strOpt, const QString &strValue);

    //通过关键词读取选项
    OPTNODE getOptionNodeByKeyword(const QString &strKey);

    QString getPPDName();

private:
    QString getOptionValue(const QString &strOptName);
    void setOptionValue(const QString &strOptName, const QString &strValue);
    QVector<QMap<QString, QString>> getOptionChooses(const QString &strOptName);
    QString getColorAttr();
    QString getMarkerType();
    QString getMarkerName();
    QString getMarkerLevel();

private:
    PPD m_ppd;
    bool m_bNeedSavePpd;
    bool m_bNeedUpdateInk;
    QVector<SUPPLYSDATA> m_vecMarkInfo;
};

#endif // DPRINTER_H
