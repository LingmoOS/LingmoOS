/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_ENGINE_DOCUMENT_OPERATIONS_H
#define BALOO_ENGINE_DOCUMENT_OPERATIONS_H

namespace Baloo {

enum DocumentOperation {
    DocumentTerms =  0x1,
    FileNameTerms =  0x2,
    XAttrTerms    =  0x4,
    DocumentData  =  0x8,
    DocumentTime  = 0x10,
    DocumentUrl   = 0x20,
    Everything    = DocumentTerms | FileNameTerms | XAttrTerms | DocumentData | DocumentTime | DocumentUrl,
};
Q_DECLARE_FLAGS(DocumentOperations, DocumentOperation)
Q_DECLARE_OPERATORS_FOR_FLAGS(DocumentOperations)

}

#endif
