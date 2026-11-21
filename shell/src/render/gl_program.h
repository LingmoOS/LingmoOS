// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <GLES3/gl3.h>
#include <map>
#include <string>
#include <string_view>

namespace Lingmo::Render {

class GlProgram
{
public:
    GlProgram() = default;
    ~GlProgram();
    GlProgram(GlProgram &&other) noexcept;
    GlProgram &operator=(GlProgram &&other) noexcept;
    GlProgram(const GlProgram &) = delete;
    GlProgram &operator=(const GlProgram &) = delete;
    static GlProgram create(const char *vertex, const char *fragment);

    operator GLuint() const { return m_name; }

    GLint uniform(const char *name) const;
    GLint attribute(const char *name) const;

    // Every shader here takes at most a position and a texture coordinate,
    // bound to these locations before linking.
    static constexpr GLuint kPositionAttribute = 0;
    static constexpr GLuint kTexCoordAttribute = 1;
    static constexpr GLuint kAttributeCount = 2;
    void reset();

private:
    GLuint m_name = 0;
    mutable std::map<std::string, GLint, std::less<>> m_uniforms;
    mutable std::map<std::string, GLint, std::less<>> m_attributes;
};

std::string clippedFragment(const char *body, const char *extensions = "");

} // namespace Lingmo::Render
