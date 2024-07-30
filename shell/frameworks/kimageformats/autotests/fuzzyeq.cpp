/*
    SPDX-FileCopyrightText: 2014 Alex Merry <alex.merry@kdemail.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

template<class Trait>
static bool fuzzyeq(const QImage &im1, const QImage &im2, uchar fuzziness)
{
    Q_ASSERT(im1.format() == im2.format());
    Q_ASSERT(im1.depth() == 24 || im1.depth() == 32 || im1.depth() == 64);

    const int height = im1.height();
    const int width = im1.width();
    for (int i = 0; i < height; ++i) {
        const Trait *line1 = reinterpret_cast<const Trait *>(im1.scanLine(i));
        const Trait *line2 = reinterpret_cast<const Trait *>(im2.scanLine(i));
        for (int j = 0; j < width; ++j) {
            if (line1[j] > line2[j]) {
                if (line1[j] - line2[j] > fuzziness) {
                    return false;
                }
            } else {
                if (line2[j] - line1[j] > fuzziness) {
                    return false;
                }
            }
        }
    }
    return true;
}

// allow each byte to be different by up to 1, to allow for rounding errors
static bool fuzzyeq(const QImage &im1, const QImage &im2, uchar fuzziness)
{
    return (im1.depth() == 64) ? fuzzyeq<quint16>(im1, im2, fuzziness) : fuzzyeq<quint8>(im1, im2, fuzziness);
}
