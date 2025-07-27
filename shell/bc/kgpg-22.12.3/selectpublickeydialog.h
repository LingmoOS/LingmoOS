/*
    SPDX-FileCopyrightText: 2002 Jean-Baptiste Mardelle <bj@altern.org>
    SPDX-FileCopyrightText: 2007, 2008, 2009, 2010, 2011, 2012, 2013 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SELECTPUBLICKEYDIALOG_H
#define SELECTPUBLICKEYDIALOG_H

#include "core/kgpgkey.h"

#include <QDialog>
#include <QKeySequence>
#include <QUrl>

#include <QVBoxLayout>

class QCheckBox;
class QTableView;

class QLineEdit;


class SelectKeyProxyModel;
class KGpgItemModel;

/**
 * @brief shows a dialog to select a public key for encryption
 */
class KgpgSelectPublicKeyDlg : public QDialog
{
    Q_OBJECT

public:
    /* TODO: the goDefaultKey shortcut should be stored in a way it can be accessed from everywhere. So we don't have to pass it as an argument.
     */

    KgpgSelectPublicKeyDlg(QWidget *parent, KGpgItemModel *model, const QKeySequence &goDefaultKey, const bool hideasciioption, const QList<QUrl> &files = QList<QUrl>());

    QStringList selectedKeys() const;
    QString getCustomOptions() const;
    bool getSymmetric() const;
    bool getUntrusted() const;
    bool getHideId() const;
    bool getArmor() const;
    /**
     * @brief return the files passed in the constructor
     */
    const QList<QUrl> &getFiles() const;

    QWidget *optionsbox;

private Q_SLOTS:
    void slotOk();
    void slotSelectionChanged();
    void slotSymmetric(const bool state);
    void slotUntrusted(const bool state);
    void slotGotoDefaultKey();

private:
    void toggleDetails();
private:
    QCheckBox *m_cbarmor;
    QCheckBox *m_cbuntrusted;
    QCheckBox *m_cbhideid;
    QCheckBox *m_cbsymmetric;

    QPushButton *m_okButton;
    QPushButton *m_detailsButton;

    QWidget *m_searchbar;
    QLineEdit *m_customoptions;
    QTableView *m_keyslist;
    QLineEdit *m_searchlineedit;
    SelectKeyProxyModel *iproxy;
    KGpgItemModel *imodel;
    const QList<QUrl> m_files;

    bool m_hideasciioption;
};

#endif // SELECTPUBLICKEYDIALOG_H
