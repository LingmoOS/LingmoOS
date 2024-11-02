#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include "kaboutdialog.h"

class AboutDialog : public kdk::KAboutDialog
{
    Q_OBJECT
public:
    explicit AboutDialog(QWidget * parent = nullptr);

private:
    QString getScreenshotVersion();
};

#endif // ABOUTDIALOG_H
