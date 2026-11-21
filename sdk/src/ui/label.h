// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "ui/color_mixer.h"
#include "ui/view.h"

namespace skia::textlayout {
class Paragraph;
}

namespace Lingmo::ui {

struct AdvanceRule;
struct TextLineBox;

// Text laid out at a whole logical pixel size with unhinted advances, each
// line as tall as the greatest ascent and descent of its fonts, rounded up.
// Glyphs are drawn at the device size.
class Label : public View
{
public:
    enum class TextAlignment {
        Left,
        Center,
        Right,
    };

    // Where a line too long for the label loses its text to an ellipsis.
    enum class Elide {
        // The line runs past the label.
        None,
        Right,
        Middle,
    };

    // A run of the text in its own weight or colour; 0 and unset take the
    // label's. As in styled text markup: <i> italic, <u> underline, <a href>
    // a link, underlined.
    struct Span
    {
        std::string text;
        int weight = 0;
        std::optional<ColorRecipe> color;
        bool italic = false;
        bool underline = false;
        std::string link;
    };

    explicit Label(std::string text = {});
    ~Label() override;

    const std::string& text() const { return m_text; }
    void setText(std::string text);
    // Logical pixels, rounded to whole ones; 0 follows the theme.
    void setFontSize(double size);
    // OpenType features, such as {"tnum", 1} for figures of one width.
    void setFontFeatures(std::vector<std::pair<std::string, int>> features);
    void setWeight(int weight);
    // Empty follows the theme's.
    void setFontFamily(std::string family);
    // Unset follows the theme's text colour, as does a recipe naming theme colours.
    void setColor(std::optional<ColorRecipe> color);
    // Disabled, a plain label fades to 0.6; a control's own label, which the
    // control colours, does not.
    void setFadesWhenDisabled(bool fades);
    // When false: greyscale glyphs whatever fontconfig's rgba says. Subpixel
    // (as fontconfig says) by default.
    void setSubpixelAntialiasing(bool subpixel);
    // Absolute letter spacing: logical pixels added after every glyph, the last
    // one too, so the width (and centring) includes it.
    void setLetterSpacing(double spacing);
    void setWrap(bool wrap);
    // Text.WrapAnywhere rather than Text.WordWrap: wrapped lines break at
    // any character, mid-word too. Takes effect with setWrap(true).
    void setWrapAnywhere(bool anywhere);
    // Text.lineHeight with lineHeightMode FixedHeight: every line box is
    // this many logical pixels, the text at its top; 0 for the font's own.
    void setFixedLineHeight(double height);
    // Unwrapped lines elide at the right unless set otherwise. Middle applies
    // to a single line of text without spans.
    void setElide(Elide elide);
    // With wrapping, at most this many lines, the last one elided; 0 for any.
    void setMaximumLineCount(int count);
    void setTextAlignment(TextAlignment alignment);
    void setVerticallyCentered(bool centered);
    // Replaces the text with runs of it, such as a bold month before a year.
    void setSpans(std::vector<Span> spans);

    // A line of the text as laid out at the label's width: its natural width
    // (its advance before alignment), and its top, baseline and height in the
    // label's logical coordinates. Nothing past the last line.
    struct LineGeometry
    {
        double width = 0;
        double top = 0;
        double baseline = 0;
        double height = 0;
    };
    std::optional<LineGeometry> lineGeometry(size_t line) const;
    size_t lineCount() const;

    // The link of the span under a point in the label's coordinates, as
    // Text.linkAt; empty for none. Wrapped text included; text elided in the
    // middle has none.
    std::string linkAt(PointF point) const;

    // Whether the text as last laid out lost some of itself to an ellipsis,
    // e.g. to show it whole in a tool tip.
    bool isElided() const;
    // The text as last laid out, ellipsis included, without the newlines
    // wrapping adds.
    std::string shownText() const;

    // From the label's top to its first baseline, in logical pixels, not
    // snapped.
    double baselineOffset() const;
    // The ascent, descent and x-height of the label's font, unrounded.
    double ascent() const;
    double descent() const;
    double xHeight() const;
    // The height of a tight "H".
    double capHeight() const;
    // The middle of the capitals below the label's top, snapped to the device
    // grid.
    double capCenter() const;
    // Pads above the text by the distance to y less the baseline offset,
    // snapped to the device grid, so the baseline lands near y below the
    // label's top (at the top of its layout, y in the parent). Negative turns
    // it off.
    void setBaseline(double y);
    // The ink bounds of the first line, relative to the
    // start of its baseline: whole pixels around the glyphs.
    RectF tightBoundingRect() const;
    // The exact ink bounds of the first line, relative to the start of its
    // baseline, at the window's scale.
    RectF inkBoundingRect() const;

    // The unwrapped size.
    SizeF sizeHint() const override;
    double heightForWidth(double width) const override;

protected:
    void paint(Painter& painter) override;
    // The text moved by a logical offset, all of it in the colour when one is given.
    void paintText(Painter& painter, PointF shift, std::optional<Color> color);
    // Whether the glyphs are drawn subpixel (when fontconfig asks for it).
    virtual bool usesSubpixel() const { return m_subpixel; }

private:
    void invalidate();
    double currentScale() const;
    // The window's Window::hintedTextLayout().
    bool hintedLayout() const;
    AdvanceRule advanceRule() const;
    double pixelSize() const;
    // With breaks, a newline goes before each of those UTF-8 offsets.
    std::unique_ptr<skia::textlayout::Paragraph> build(const std::string* text,
                                                       const std::vector<size_t>* breaks = nullptr) const;
    skia::textlayout::Paragraph* paragraph() const;
    // The paragraph as drawn at the logical width: the text, or the text
    // with its middle elided, or wrapped.
    skia::textlayout::Paragraph* layoutParagraph(double width) const;
    // In the lines the text breaks into at the width, the last one shown elided
    // when there are more than the maximum.
    skia::textlayout::Paragraph* wrappedParagraph(double width) const;
    // Trades the wrapping in use for the previous one.
    void swapWrapping() const;
    // `content` (the paragraph's text, laid out unbounded) from `from` to `to`
    // elided to the width by whole graphemes, the ellipsis counted in.
    std::string elidedText(const std::string& content, size_t from, size_t to, double width, Elide mode) const;
    // Logical pixels from the label's top to the top of the text.
    double textOffset(double textHeight) const;
    // Whether advances are rounded at the window's scale.
    bool roundsAdvances() const;
    // How much wider the paragraph's own advances make the text than the
    // truncated ones.
    double slack() const;
    double baselinePadding() const;
    double snapped(double value) const;
    // Forgets sizes measured in an earlier measurement generation or theme.
    void checkMeasurements() const;
    std::vector<TextLineBox> lineBoxes(skia::textlayout::Paragraph* text, double scale) const;
    // Where the underlined and linked spans lie as laid out at `width`: the
    // span's index, its line's top, baseline and height, and its x range,
    // in the label's logical coordinates.
    struct SpanBox
    {
        size_t span = 0;
        double left = 0;
        double right = 0;
        double top = 0;
        double baseline = 0;
        double height = 0;
    };
    std::vector<SpanBox> spanBoxes(skia::textlayout::Paragraph* text, double scale) const;

    std::string m_text;
    std::vector<Span> m_spans;
    std::vector<std::pair<std::string, int>> m_features;
    double m_fontSize = 0;
    int m_weight = 400;
    std::string m_family;
    bool m_fadesWhenDisabled = true;
    bool m_subpixel = true;
    double m_letterSpacing = 0;
    std::optional<ColorRecipe> m_color;
    bool m_wrap = false;
    bool m_wrapAnywhere = false;
    double m_fixedLineHeight = 0;
    Elide m_elide = Elide::Right;
    int m_maximumLineCount = 0;
    TextAlignment m_alignment = TextAlignment::Left;
    bool m_verticallyCentered = false;
    double m_baseline = -1;

    mutable std::unique_ptr<skia::textlayout::Paragraph> m_paragraph;
    mutable bool m_paragraphEnabled = true;
    mutable uint64_t m_paragraphTheme = 0;
    mutable double m_layoutWidth = -1;
    mutable std::unique_ptr<skia::textlayout::Paragraph> m_elided;
    mutable std::string m_elidedText;
    mutable double m_elidedWidth = -1;
    mutable std::string m_wrappedText;
    // Wrapped: the text with a newline where each line starts.
    mutable std::unique_ptr<skia::textlayout::Paragraph> m_wrapped;
    mutable std::vector<size_t> m_wrappedBreaks;
    mutable double m_wrappedWidth = -1;
    mutable bool m_wrappedRounded = false;
    mutable bool m_wrappedElided = false;
    // The wrapping at the width asked for before, as layouts ask for two
    // widths in turn.
    struct Wrapping;
    mutable std::unique_ptr<Wrapping> m_previousWrapping;
    // Measured unbounded, for the rounding they were measured with.
    mutable double m_naturalWidth = -1;
    mutable double m_intrinsicWidth = 0;
    mutable bool m_naturalRounded = false;
    // Layouts ask for the same sizes many times in one pass.
    mutable uint64_t m_measuredGeneration = 0;
    mutable uint64_t m_measuredTheme = 0;
    mutable std::optional<SizeF> m_measuredSize;
    mutable std::vector<std::pair<double, double>> m_measuredHeights;
};

} // namespace Lingmo::ui
