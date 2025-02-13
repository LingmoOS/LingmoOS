#include "texteditor.h"

FileHelper::FileHelper(QObject *parent)
    : QObject(parent)
{

}

void FileHelper::addPath(const QString &path)
{
    QFileInfo file(path);
    if(file.exists())
        emit newPath(file.absoluteFilePath());
    else
        emit unavailable(path);
}
