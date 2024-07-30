/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef SDF_SHADER_H
#define SDF_SHADER_H

#include <QSGMaterialShader>

struct UniformDataStream {
    inline UniformDataStream(QSGMaterialShader::RenderState &state) noexcept
        : bytes(state.uniformData()->data())
    {
    }

    ~UniformDataStream()
    {
    }

    template<typename Data>
    friend inline UniformDataStream &operator<<(UniformDataStream &stream, const Data &data)
    {
        constexpr uint dataSize = sizeof(Data);
        stream.align(dataSize);
        memcpy(stream.bytes, &data, dataSize);
        stream.bytes += dataSize;
        stream.offset += dataSize;
        return stream;
    }

    template<typename Data>
    inline void skip(const Data &data = {})
    {
        constexpr uint dataSize = sizeof(Data);

        align(dataSize);
        Q_UNUSED(data);
        bytes += dataSize;
        offset += dataSize;
    }

    inline void skipComponents(uint count)
    {
        const uint skipCount = count * 4;
        align(4);
        bytes += skipCount;
        offset += skipCount;
    }

    friend inline UniformDataStream &operator<<(UniformDataStream &stream, const QMatrix4x4 &m)
    {
        constexpr uint Matrix4x4Size = 4 * 4 * 4;

        stream.align(Matrix4x4Size);
        memcpy(stream.bytes, m.constData(), Matrix4x4Size);
        stream.bytes += Matrix4x4Size;
        stream.offset += Matrix4x4Size;
        return stream;
    }

    friend inline UniformDataStream &operator<<(UniformDataStream &stream, const QColor &color)
    {
        constexpr uint ColorSize = 4 * 4;

        stream.align(ColorSize);
        std::array<float, 4> colorArray;
        color.getRgbF(&colorArray[0], &colorArray[1], &colorArray[2], &colorArray[3]);
        memcpy(stream.bytes, colorArray.data(), ColorSize);
        stream.bytes += ColorSize;
        stream.offset += ColorSize;
        return stream;
    }

    template<typename T>
    friend inline UniformDataStream &operator<<(UniformDataStream &stream, const QList<T> &v)
    {
        for (const auto &item : v) {
            stream << item;
            // Using std140, array elements are padded to a size of 16 bytes per element.
            stream.align(16);
        }
        return stream;
    }

    char *bytes;
    size_t padding = 16;
    size_t offset = 0;

private:
    // Encode alignment rules for std140.
    // Minimum alignment is 4 bytes.
    // Vec2 alignment is 8 bytes.
    // Vec3 and Vec4 alignment is 16 bytes.
    inline void align(uint size)
    {
        if (size <= 4) {
            const auto padding = offset % 4 > 0 ? 4 - offset % 4 : 0;
            offset += padding;
            bytes += padding;
        } else if (size <= 8) {
            auto padding = offset % 8 > 0 ? 8 - offset % 8 : 0;
            offset += padding;
            bytes += padding;
        } else {
            auto padding = offset % 16 > 0 ? 16 - offset % 16 : 0;
            offset += padding;
            bytes += padding;
        }
    }
};

class SDFShader : public QSGMaterialShader
{
public:
    SDFShader();
    virtual ~SDFShader();

    void setShaders(const QString &vertex, const QString &fragment);
};

#endif // SDF_SHADER_H
