#include "aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent)
   : kdk::KAboutDialog(parent)
{
    setAppIcon(QIcon::fromTheme("lingmo-screenshot"));
    setAppName(tr("screenshot"));
    setAppVersion(tr("Version: ") + getScreenshotVersion());
    this->closeButton()->setToolTip(tr("Close"));
}

QString AboutDialog::getScreenshotVersion()
{
    FILE *pp = NULL;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    char *q = NULL;
    QString version = tr("none");
    pp = popen("dpkg -l lingmo-screenshot", "r");
    if(NULL == pp)
        return version;

    while((read = getline(&line, &len, pp)) != -1){
        q = strrchr(line, '\n');
        *q = '\0';
        QString content = line;
        QStringList list = content.split(" ");
        list.removeAll("");
        if (list.size() >= 3)
            version = list.at(2);
    }
    free(line);
    pclose(pp);
    return version;
}
