/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2000, 2007 David Faure <faure@kde.org>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
*/

#ifndef FILETYPEDETAILS_H
#define FILETYPEDETAILS_H

#include <QTabWidget>

class KIconButton;
class MimeTypeData;
class TypesListItem;
class QLabel;
class QListWidget;
class QGroupBox;
class QButtonGroup;
class QCheckBox;
class QRadioButton;
class KLineEdit;
class QPushButton;
class KServiceListWidget;

/**
 * This widget contains the right part of the file type configuration
 * dialog, that shows the details for a file type.
 * It is implemented as a separate class so that it can be used by
 * the keditfiletype program to show the details of a single mimetype.
 */
class FileTypeDetails : public QWidget
{
    Q_OBJECT
public:
    explicit FileTypeDetails(QWidget *parent = nullptr);

    /**
     * Set a non-gui "mimetype data" to work on,
     * and optionally a gui "treeview item", to update its icon if set.
     */
    void setMimeTypeData(MimeTypeData *mimeTypeData, TypesListItem *item = nullptr);

    /**
     * Called when ksycoca has changed
     */
    void refresh();

    void allowMultiApply(bool allow);

protected:
    void updateRemoveButton();
    void updateAskSave();

Q_SIGNALS:
    void embedMajor(const QString &major, bool &embed); // To adjust whether major type is being embedded
    void changed(bool);
    void multiApply(int kind);

protected Q_SLOTS:
    void updateIcon(const QString &icon);
    void updateDescription(const QString &desc);
    void addExtension();
    void removeExtension();
    void enableExtButtons();
    void slotAutoEmbedClicked(int button);
    void slotAskSaveToggled(bool);

private:
    MimeTypeData *m_mimeTypeData;
    TypesListItem *m_item; // can be 0, in keditfiletype!

    QLabel *m_mimeTypeLabel;

    QTabWidget *m_tabWidget;

    // First tab - General
    KIconButton *iconButton;
    QLabel *iconLabel; // if icon cannot be changed

    QListWidget *extensionLB;
    QPushButton *addExtButton, *removeExtButton;
    KLineEdit *description;
    KServiceListWidget *serviceListWidget;

    // Second tab - Embedding
    QGroupBox *m_autoEmbedBox;
    QButtonGroup *m_autoEmbedGroup;
    KServiceListWidget *embedServiceListWidget;
    QRadioButton *m_rbOpenSeparate;
    QCheckBox *m_chkAskSave;
    QRadioButton *m_rbGroupSettings;
};

#endif
