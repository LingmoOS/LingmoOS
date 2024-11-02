#pragma once

#include "src/tools/abstractactiontool.h"

class CursorRecordTool : public AbstractActionTool {
    Q_OBJECT
public:
    explicit CursorRecordTool(QObject *parent = nullptr);

    bool closeOnButtonPressed() const;

    QString name() const override;

    bool isIsolated() const override;

    QIcon icon(const QColor &background, bool inEditor) const;
#ifdef SUPPORT_LINGMO
    QIcon icon(const QColor &background, bool inEditor,const CaptureContext &context) const override;
#endif
    QString description() const override;

    CaptureTool *copy(QObject *parent = nullptr) override;

public slots:
    void pressed(const CaptureContext &context) override;
//    void cursor_is_clicked();
};
