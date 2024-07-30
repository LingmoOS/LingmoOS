/*
    Render a PipeWire stream into a QtQuick scene as a standard Item
    SPDX-FileCopyrightText: 2020 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include <QImage>
#include <QQuickItem>
#include <functional>
#include <optional>

#include <pipewire/pipewire.h>
#include <spa/param/format-utils.h>
#include <spa/param/props.h>
#include <spa/param/video/format-utils.h>

#include <kpipewire_export.h>

struct DmaBufAttributes;
class PipeWireSourceStream;
struct PipeWireFrame;
class PipeWireFrameData;
class QSGTexture;
class QOpenGLTexture;
typedef void *EGLImage;

class PipeWireSourceItemPrivate;

class KPIPEWIRE_EXPORT PipeWireSourceItem : public QQuickItem
{
    Q_OBJECT

    /// Returns where the streams current state
    Q_PROPERTY(StreamState state READ state NOTIFY stateChanged)

    /// Specify the pipewire node id that we want to play
    Q_PROPERTY(uint nodeId READ nodeId WRITE setNodeId NOTIFY nodeIdChanged)

    /**
     * Specifies the file descriptor we are connected to, if none 0 will be returned
     *
     * Transfers the ownership of the fd, will close it when it's done with it.
     */
    Q_PROPERTY(uint fd READ fd WRITE setFd NOTIFY fdChanged RESET resetFd)

    /**
     * Returns the size of the source being rendered
     * @note: This won't be updated until the first frame is recieved
     */
    Q_PROPERTY(QSize streamSize READ streamSize NOTIFY streamSizeChanged)

    /**
     * Allows disabling the dmabuf streams
     */
    Q_PROPERTY(bool allowDmaBuf READ allowDmaBuf WRITE setAllowDmaBuf)

    Q_PROPERTY(bool usingDmaBuf READ usingDmaBuf NOTIFY usingDmaBufChanged)

    /**
     * Specifies whether the first frame has been received and there's something to display.
     */
    Q_PROPERTY(bool ready READ isReady NOTIFY readyChanged)

public:
    enum class StreamState { Error, Unconnected, Connecting, Paused, Streaming };
    Q_ENUM(StreamState);

    PipeWireSourceItem(QQuickItem *parent = nullptr);
    ~PipeWireSourceItem() override;

    QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *data) override;
    Q_SCRIPTABLE QString error() const;

    void setNodeId(uint nodeId);
    uint nodeId() const;

    void setFd(uint fd);
    void resetFd();
    uint fd() const;

    QSize streamSize() const;

    bool usingDmaBuf() const;
    bool allowDmaBuf() const;
    void setAllowDmaBuf(bool allowed);

    bool isReady() const;

    void componentComplete() override;
    void releaseResources() override;

    StreamState state() const;

Q_SIGNALS:
    void nodeIdChanged(uint nodeId);
    void fdChanged(uint fd);
    void streamSizeChanged();
    void stateChanged();
    void usingDmaBufChanged();
    void readyChanged();

private:
    void itemChange(ItemChange change, const ItemChangeData &data) override;
    void processFrame(const PipeWireFrame &frame);
    void updateTextureDmaBuf(const DmaBufAttributes &attribs, spa_video_format format);
    void updateTextureImage(const std::shared_ptr<PipeWireFrameData> &data);
    void refresh();
    void setReady(bool ready);

    QScopedPointer<PipeWireSourceItemPrivate> d;
};
