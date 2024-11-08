#ifndef HIGHLIGHTMODEL_H
#define HIGHLIGHTMODEL_H

#include <QObject>

#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Definition>

class HighlightModel : public QObject
{
    Q_OBJECT

public:
    explicit HighlightModel(QObject *parent = nullptr);

private:
    KSyntaxHighlighting::Repository m_repository;
};

#endif // HIGHLIGHTMODEL_H
