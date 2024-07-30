/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1998, 1999 Torben Weis <weis@kde.org>
    SPDX-FileCopyrightText: 1998, 1999  Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSYCOCATYPE_H
#define KSYCOCATYPE_H

/**
 * \relates KSycocaEntry
 * A KSycocaType is a code (out of the KSycocaType enum) assigned to each
 * class type derived from KSycocaEntry .
 * To use it, call the macro K_SYCOCATYPE( your_typecode, parent_class )
 * at the top of your class definition.
 */
enum KSycocaType {
    KST_KSycocaEntry = 0,
    KST_KService = 1,
    KST_KServiceType = 2,
    KST_KMimeType = 3,
    KST_KMimeTypeEntry = 6 /*internal*/,
    KST_KServiceGroup = 7,
    // 8 was KST_KImageIOFormat in kdelibs4
    // 9 was KST_KProtocolInfo in kdelibs4
    KST_KServiceSeparator = 10,
    KST_KCustom = 1000,
};

/**
 * \relates KSycocaFactory
 * A KSycocaFactoryId is a code (out of the KSycocaFactoryId enum)
 * assigned to each class type derived from KSycocaFactory.
 * To use it, call the macro K_SYCOCAFACTORY( your_factory_id )
 * at the top of your class definition.
 */
enum KSycocaFactoryId {
    KST_KServiceFactory = 1,
    KST_KServiceTypeFactory = 2,
    KST_KServiceGroupFactory = 3,
    // 4 was KST_KImageIO in kdelibs4
    // 5 was KST_KProtocolInfoFactory in kdelibs4
    KST_KMimeTypeFactory = 6,
    KST_CTimeInfo = 100,
};

#define K_SYCOCAFACTORY(factory_id)                                                                                                                            \
public:                                                                                                                                                        \
    KSycocaFactoryId factoryId() const override                                                                                                                \
    {                                                                                                                                                          \
        return factory_id;                                                                                                                                     \
    }                                                                                                                                                          \
                                                                                                                                                               \
private:

#endif
