// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DPRINTERPROPERTYTEMPLATE_H
#define DPRINTERPROPERTYTEMPLATE_H
#include <QString>
#include <QVector>
#include <QObject>
#include <QDebug>
#include <QFile>

#include "util/dprinter.h"
#include "util/dprintermanager.h"

#define PROPERTYOPTIONNUM 8
#define PREFERENCE QObject::tr("Preferences")

const QString generatePropertyDialogJson(const QVector<QString> &vecOption)
{
    if (vecOption.size() != PROPERTYOPTIONNUM) {
        return "";
    }

    return QString::fromStdString("\
    {\
        \"groups\": [\
            {\
                \"key\": \"Print_Setting\",\
                \"name\": \"%1\",\
                \"groups\": [\
                    {\
                        \"key\": \"Driver_Lable\",\
                        \"name\": \"%2\",\
\
                        \"options\": [\
                            {\
                                \"key\": \"Driver_LineEdit\",\
                                \"name\": \"Driver_LineEdit\",\
                                \"type\": \"custom-lineedit\"\
                            }\
                        ]\
                    },\
\
                    {\
                      \"key\": \"URI_Lable\",\
                      \"name\": \"%3\",\
\
                      \"options\": [\
                          {\
                              \"key\": \"URI_LineEdit\",\
                              \"name\": \"URI_LineEdit\",\
                              \"type\": \"custom-lineedit\"\
                          }\
                      ]\
                    },\
\
                    {\
                      \"key\": \"Location_Lable\",\
                      \"name\": \"%4\",\
\
                      \"options\": [\
                          {\
                              \"key\": \"Location_LineEdit\",\
                              \"name\": \"Location_LineEdit\",\
                              \"type\": \"custom-lineedit\"\
                          }\
                      ]\
                    },\
\
                    {\
                      \"key\": \"Description_Lable\",\
                      \"name\": \"%5\",\
\
                      \"options\": [\
                          {\
                              \"key\": \"Description_LineEdit\",\
                              \"name\": \"Description_LineEdit\",\
                              \"type\": \"custom-lineedit\"\
                          }\
                      ]\
                    },\
\
                    {\
                      \"key\": \"ColorMode_Label\",\
                      \"name\": \"%6\",\
                      \"options\": [\
                          {\
                              \"key\": \"ColorMode_Combo\",\
                              \"name\": \"ColorMode_Combo\",\
                              \"type\": \"custom-combobox\"\
                          }\
                      ]\
                    },\
\
                     {\
                       \"key\": \"Orientation_Label\",\
                       \"name\": \"%7\",\
                       \"options\": [\
                          {\
                              \"key\": \"Orientation_Combo\",\
                              \"name\": \"Orientation_Combo\",\
                              \"type\": \"custom-combobox\"\
                          }\
                      ]\
                    },\
\
                    {\
                      \"key\": \"PrintOrder_Label\",\
                      \"name\": \"%8\",\
                      \"options\": [\
                          {\
                              \"key\": \"PrintOrder_Combo\",\
                              \"name\": \"PrintOrder_Combo\",\
                              \"type\": \"custom-combobox\"\
                          }\
                      ]\
                    }\
                ]\
            }\
        ]\
    }\
                                  ")
        .arg(vecOption[0])
        .arg(vecOption[1])
        .arg(vecOption[2])
        .arg(vecOption[3])
        .arg(vecOption[4])
        .arg(vecOption[5])
        .arg(vecOption[6])
        .arg(vecOption[7]);
}

const QString formatGroupString(const QVector<OptNode> &nodes)
{
    QString strAll;
    DPrinterManager *pManager = DPrinterManager::getInstance();

    for (int i = 0; i < nodes.size(); i++) {
        QString strOptName = nodes[i].strOptName;
        QString strComboName = strOptName + QString::fromStdString("_Combo");
        QString strLableText = pManager->translateLocal(strComboName, strOptName, nodes[i].strOptText);
        QString strNode = QString("\
          {\
            \"key\": \"%1_Label\",\
            \"name\": \"%2\",\
            \"options\": [\
                {\
                    \"key\": \"%3_Combo\",\
                    \"name\": \"%4_Combo\",\
                    \"type\": \"custom-combobox\"\
                }\
            ]\
          }\
        ")
                              .arg(strOptName)
                              .arg(strLableText)
                              .arg(strOptName)
                              .arg(strOptName);

        strAll += ",";
        strAll += strNode;
    }

    qCDebug(UTIL) << strAll;
    return strAll;
}

const QString formatPreferenceString()
{
    QString strAll;

        QString strOptName = PREFERENCE;
        QString strNode = QString("\
          {\
            \"key\": \"%1_Label\",\
            \"name\": \"%2\",\
            \"options\": [\
                {\
                    \"name\": \"%3\",\
                    \"type\": \"custom-pushbutton\"\
                }\
            ]\
          }\
        ")
                              .arg(strOptName)
                              .arg(strOptName)
                              .arg(strOptName);

        strAll += ",";
        strAll += strNode;

    return strAll;
}

const QString appendGroupString(QString &strBase, const QString &strGroup)
{
    int iIndex = strBase.lastIndexOf("]");

    if (iIndex < 0)
        return "";

    iIndex = strBase.lastIndexOf("]", iIndex - 1);

    if (iIndex < 0)
        return "";

    strBase.insert(iIndex, strGroup);
    return strBase;
}

#endif
