/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1999 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KBUGREPORT_H
#define KBUGREPORT_H

#include <QDialog>
#include <kxmlgui_export.h>
#include <memory>

class KAboutData;
class KBugReportPrivate;

/**
 * @class KBugReport kbugreport.h KBugReport
 *
 * @short A dialog box for sending bug reports.
 *
 * All the information needed by the dialog box
 * (program name, version, bug-report address, etc.)
 * comes from the KAboutData class.
 * Make sure you create an instance of KAboutData and call
 * KAboutData::setApplicationData(<aboutData>).
 *
 * \image html kbugreport.png "KBugReport"
 *
 * @author David Faure <faure@kde.org>
 */
class KXMLGUI_EXPORT KBugReport : public QDialog
{
    Q_OBJECT

public:
    /**
     * Creates a bug-report dialog.
     * Note that you shouldn't have to do this manually,
     * since KHelpMenu takes care of the menu item
     * for "Report Bug..." and of creating a KBugReport dialog.
     */
    explicit KBugReport(const KAboutData &aboutData, QWidget *parent = nullptr);

    /**
     * Destructor
     */
    ~KBugReport() override;

    /**
     * The message body of the bug report
     * @return The message body of the bug report.
     */
    QString messageBody() const;

    /**
     * Sets the message body of the bug report.
     */
    void setMessageBody(const QString &messageBody);

    /**
     * OK has been clicked
     */
    void accept() override;

protected:
    /**
     * A complete copy of the bug report
     * @return QString copy of the bug report.
     */
    QString text() const;

    /**
     * Attempt to e-mail the bug report.
     * @return true on success
     */
    bool sendBugReport();

    void closeEvent(QCloseEvent *e) override;

private:
    friend class KBugReportPrivate;
    std::unique_ptr<KBugReportPrivate> const d;

    Q_DISABLE_COPY(KBugReport)
};

#endif
