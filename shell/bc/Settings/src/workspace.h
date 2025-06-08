#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <QObject>
#include <QString>

class Workspace : public QObject
{
    Q_OBJECT

public:
    explicit Workspace(QObject *parent = nullptr);

    Q_INVOKABLE void openKwinscreenedges();
    Q_INVOKABLE void openWebsearch();
    Q_INVOKABLE void openTaskSwitcher();
    Q_INVOKABLE void openDisplayeffect();
    Q_INVOKABLE void openFontmanager();
    Q_INVOKABLE void openFilesearch();
    Q_INVOKABLE void openFileAssociations();
    Q_INVOKABLE void openKeyboard();

private:
};

#endif // WORKSPACE_H
