// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "../common/comment_global.h"
#include <QString>
#include <QDebug>

namespace Comment {
    class COMMENT_EXPORT CommentDefinition
    {
      public:
      CommentDefinition();

      enum Style { NoStyle, CppStyle, HashStyle };
      void setComments(QString singleLineComment, QString multiLineCommentStart, QString multiLineCommentEnd);

      bool isValid() const;
      bool hasSingleLineStyle() const;
      bool hasMultiLineStyle() const;

      public:
      bool isAfterWhiteSpaces;
      QString singleLine;
      QString multiLineStart;
      QString multiLineEnd;
    };
}
