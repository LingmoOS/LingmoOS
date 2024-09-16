// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import org.deepin.image.viewer 1.0 as IV
import org.deepin.album 1.0 as Album

Loader {
    id: pathViewItemLoader

    property alias frameCount: imageInfo.frameCount
    property int frameIndex: model.frameIndex
    property url imageSource: model.imageUrl

    function updateLoaderSource() {
        if (active && imageInfo.delegateSource) {
            setSource(imageInfo.delegateSource, {
                    "source": pathViewItemLoader.imageSource,
                    "type": imageInfo.type,
                    "frameIndex": pathViewItemLoader.frameIndex
                });
        }
    }

    active: {
        if (PathView.isCurrentItem) {
            return true;
        }
        return PathView.onPath;
    }
    asynchronous: true
    enabled: PathView.isCurrentItem
    height: PathView.view.height
    opacity: PathView.delegateOpacity
    visible: active
    width: PathView.view.width

    onActiveChanged: {
        updateLoaderSource();
    }

    // 当前图片路径变更时，图片类型 delegateSource 不一定变更，
    // 绑定 source 和 frameIndex 变更以在同类型下更新显示图片
    Binding {
        property: "source"
        target: pathViewItemLoader.item
        value: pathViewItemLoader.imageSource
        when: Loader.Ready === pathViewItemLoader.status
    }

    Binding {
        property: "frameIndex"
        target: pathViewItemLoader.item
        value: pathViewItemLoader.frameIndex
        when: Loader.Ready === pathViewItemLoader.status
    }

    Binding {
        property: "offset"
        target: pathViewItemLoader.item
        value: pathViewItemLoader.PathView.delegateOffset
        when: Loader.Ready === pathViewItemLoader.status
    }

    // TODO: 这部分组件也应移动到 Component 中，再抽象一层组件，不提前创建
    IV.ImageInfo {
        id: imageInfo

        property url delegateSource
        property bool isCurrentItem: pathViewItemLoader.PathView.isCurrentItem

        function checkDelegateSource() {
            if (IV.ImageInfo.Ready !== status && IV.ImageInfo.Error !== status) {
                delegateSource = "";
                return;
            }
            if (!imageInfo.exists) {
                delegateSource = "qrc:/qml/PreviewImageViewer/ImageDelegate/NonexistImageDelegate.qml";
                return;
            }
            switch (type) {
            case Album.Types.NormalImage:
                delegateSource = "qrc:/qml/PreviewImageViewer/ImageDelegate/NormalImageDelegate.qml";
                return;
            case Album.Types.DynamicImage:
                delegateSource = "qrc:/qml/PreviewImageViewer/ImageDelegate/DynamicImageDelegate.qml";
                return;
            case Album.Types.SvgImage:
                delegateSource = "qrc:/qml/PreviewImageViewer/ImageDelegate/SvgImageDelegate.qml";
                return;
            case Album.Types.MultiImage:
                // PathView 替换 ListView 后，不再单独展开多页图，而是每页单独更新
                // delegateSource = "qrc:/qml/PreviewImageViewer/ImageDelegate/MultiImageDelegate.qml";
                delegateSource = "qrc:/qml/PreviewImageViewer/ImageDelegate/NormalImageDelegate.qml";
                return;
            case Album.Types.NoPermissionImage:
                delegateSource = "qrc:/qml/PreviewImageViewer/ImageDelegate/NoPermissionImageDelegate.qml";
                return;
            default:
                // Default is damaged image.
                delegateSource = "qrc:/qml/PreviewImageViewer/ImageDelegate/DamagedImageDelegate.qml";
                return;
            }
        }

        // WARNING: 由于 Delegate 组件宽度关联的 view.width ，PathView 会计算 Delegate 大小
        // Loader 在构造时直接设置图片链接会导致数据提前加载，破坏了延迟加载策略
        // 调整机制，不在激活状态的图片信息置为空，在需要加载时设置图片链接
        source: pathViewItemLoader.active ? pathViewItemLoader.imageSource : ""

        onDelegateSourceChanged: {
            updateLoaderSource();
        }

        // InfoChange 在图片文件变更时触发，此时图片文件路径不变，文件内容被替换、删除
        onInfoChanged: {
            if (isCurrentItem) {
                GControl.currentFrameIndex = 0;
            }
            checkDelegateSource();
            var temp = delegateSource;
            delegateSource = "";
            delegateSource = temp;
        }
        onIsCurrentItemChanged: checkDelegateSource()
        onStatusChanged: checkDelegateSource()
    }
}
