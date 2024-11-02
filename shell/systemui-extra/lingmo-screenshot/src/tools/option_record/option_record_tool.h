#pragma once

#include "src/tools/abstractactiontool.h"

class OptionRecordTool : public AbstractActionTool {
    Q_OBJECT
public:
    explicit OptionRecordTool(QObject *parent = nullptr);

    bool closeOnButtonPressed() const;

    QIcon icon(const QColor &background, bool inEditor) const override;
#ifdef SUPPORT_LINGMO
    QIcon icon(const QColor &background, bool inEditor,const CaptureContext &context) const override;
#endif
    QString name() const override;
    static QString nameID();
    QString description() const override;

    CaptureTool* copy(QObject *parent = nullptr) override;

public slots:
    void pressed(const CaptureContext &context) override;


};
