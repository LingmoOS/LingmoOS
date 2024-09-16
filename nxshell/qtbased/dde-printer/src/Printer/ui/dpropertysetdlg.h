// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QPROPERTYSETDLG_H
#define QPROPERTYSETDLG_H
//
#include <DSettingsDialog>
#include <DListView>
#include <QMap>
#include "dprinter.h"

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class DPropertySetDlg : public DSettingsDialog
{
    Q_OBJECT
public:
    DPropertySetDlg(const QString &strPrinter = QString(""), QWidget *pParent = nullptr);

public:
    void setPrinterName(const QString &strName);
    void updateViews();
    void moveToParentCenter();
    bool isDriveBroken();
    void initUI();
    void initConnection();

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    void updateComboByName(const QString &, const QString &, const QVector<QMap<QString, QString>> &);
    void updateDriverUI(const QString &);
    void updateUriUI(const QString &);
    void updateLocationUI(const QString &);
    void updateDescriptionUI(const QString &);
    void updatePageSizeCombo(const QString &, const QVector<QMap<QString, QString>> &);
    void updateOrientationCombo(const QString &, const QVector<QMap<QString, QString>> &);
    void updateDuplexPrintCombo(const QString &, const QVector<QMap<QString, QString>> &);
    void updateColorModeCombo(const QString &, const QVector<QMap<QString, QString>> &);
    void updatePrintOrderCombo(const QString &, const QVector<QMap<QString, QString>> &);
    void updatePaperOriginCombo(const QString &, const QVector<QMap<QString, QString>> &);
    void updatePaperTypeCombo(const QString &, const QVector<QMap<QString, QString>> &);
    void updateOutputQuanlityCombo(const QString &, const QVector<QMap<QString, QString>> &);
    void updateStapleLocationCombo(const QString &, const QVector<QMap<QString, QString>> &);
    void updateBindEdgeCombo(const QString &, const QVector<QMap<QString, QString>> &);
    void updateResolutionCombo(const QString &, const QVector<QMap<QString, QString>> &);
    void updateInstallAbleNodeCombo(const QVector<INSTALLABLEOPTNODE> &);
    void updateGeneralNodeCombo(const QVector<GENERALOPTNODE> &);
    QString changeComboSelectionByName(const QString &strComboName, int index);
    bool haveConflict(const QString &, const QString &, QVector<CONFLICTNODE> &);
    bool isConflict(const QString &, const QString &);
    void initConflictMap(const QVector<INSTALLABLEOPTNODE> &, const QVector<GENERALOPTNODE> &);
    void showConflictDlg(const vector<CONFLICTPAIR> &);
    void updateComboByConflits(const QSet<QString> &);
    void checkAllConflicts(QSet<QString> &, vector<CONFLICTPAIR> &);
    QVector<INSTALLABLEOPTNODE> getInstallableNodes();
    QVector<GENERALOPTNODE> getGeneralNodes();
    bool isBishengDriver();
    QString getBishengInfo(QString info);

private slots:
    void pageSizeCombo_clicked(int iIndex);
    void paperOrientation_clicked(int iIndex);
    void duplexPrintCombo_clicked(int iIndex);
    void colorModeCombo_clicked(int iIndex);
    void printOrderCombo_clicked(int iIndex);
    void paperOriginCombo_clicked(int iIndex);
    void paperTypeCombo_clicked(int iIndex);
    void printStapleLocationCombo_clicked(int index);
    void printBindEdgeCombo_clicked(int index);
    void printResolutionCombo_clicked(int index);
    void outputQuanlity_clicked(int index);
    void installAbleCombo_clicked(int index);
    void generalCombo_clicked(int index);
    void printUriUI_EditFinished();
    void printLocationUI_EditFinished();
    void printDescriptionUI_EditFinished();
    void confirmBtn_clicked();
    void cancelBtn_clicked();
    void preferenceClickSlot();

private:
    QMap<QString, QWidget *> m_mapOfListWidget;
    QMap<QString, QString> m_mapInitUIValue;
    QMap<QString, QString> m_mapDynamicUIValue;
    QMap<QString, QString> m_mapOfConflict;
    QSet<QString> m_setConflictOptions;
    QString m_strPrinterName;
    QString m_strPpdName;
    bool m_bShow;
};

#endif // QPROPERTYSETDLG_H
