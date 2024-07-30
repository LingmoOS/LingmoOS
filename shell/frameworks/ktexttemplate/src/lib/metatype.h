/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Michael Jansen <kde@michael-jansen.biz>
  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef KTEXTTEMPLATE_METATYPE_H
#define KTEXTTEMPLATE_METATYPE_H

#include "ktexttemplate_export.h"

#include "typeaccessor.h"

#include <QVariant>

/// @file

namespace KTextTemplate
{

/// @headerfile metatype.h <KTextTemplate/MetaType>

#ifndef K_DOXYGEN
/**
  @brief The **%MetaType** is the interface to the KTextTemplate introspection
  system.

  The **%MetaType** class is used as part of the type registration system of
  %KTextTemplate.

  @see @ref generic_types_and_templates
  @author Michael Jansen <kde@michael-jansen.biz>
  @author Stephen Kelly <steveire@gmail.com>
*/
class KTEXTTEMPLATE_EXPORT MetaType
{
public:
    /**
      @internal The signature for a property lookup method
     */
    typedef QVariant (*LookupFunction)(const QVariant &, const QString &);

    /**
      @internal Registers a property lookup method
     */
    static void registerLookUpOperator(int id, LookupFunction f);

    /**
      @internal
     */
    static void internalLock();

    /**
      @internal
     */
    static void internalUnlock();

    /**
      @internal
     */
    static QVariant lookup(const QVariant &object, const QString &property);

    /**
      @internal
     */
    static bool lookupAlreadyRegistered(int id);

private:
    MetaType();
};
#endif

namespace
{

/*
  This is a helper to select an appropriate overload of indexAccess
 */
template<typename RealType, typename HandleAs>
struct LookupTrait {
    static QVariant doLookUp(const QVariant &object, const QString &property)
    {
        typedef typename KTextTemplate::TypeAccessor<RealType> Accessor;
        return Accessor::lookUp(object.value<RealType>(), property);
    }
};

template<typename RealType, typename HandleAs>
struct LookupTrait<RealType &, HandleAs &> {
    static QVariant doLookUp(const QVariant &object, const QString &property)
    {
        typedef typename KTextTemplate::TypeAccessor<HandleAs &> Accessor;
        return Accessor::lookUp(object.value<HandleAs>(), property);
    }
};

template<typename RealType, typename HandleAs>
static int doRegister(int id)
{
    if (MetaType::lookupAlreadyRegistered(id))
        return id;

    QVariant (*lf)(const QVariant &, const QString &) = LookupTrait<RealType, HandleAs>::doLookUp;

    MetaType::registerLookUpOperator(id, reinterpret_cast<MetaType::LookupFunction>(lf));

    return id;
}

/*
  Register a type so KTextTemplate knows how to handle it.
 */
template<typename RealType, typename HandleAs>
struct InternalRegisterType {
    static int doReg()
    {
        const int id = qMetaTypeId<RealType>();
        return doRegister<RealType &, HandleAs &>(id);
    }
};

template<typename RealType, typename HandleAs>
struct InternalRegisterType<RealType *, HandleAs *> {
    static int doReg()
    {
        const int id = qMetaTypeId<RealType *>();
        return doRegister<RealType *, HandleAs *>(id);
    }
};
}

/**
  @brief Registers the type RealType with the metatype system.

  This method can take a second template parameter to specify a cast
  that should be invoked during registration. This is useful if a base type is
  already supported.

  @code
    class SomeType
    {
    public:
      QString someProp() const;
    };

    // define some introspectable API for SomeType

    KTEXTTEMPLATE_BEGIN_LOOKUP(SomeType)
      if (property == "someProp")
        return object.someProp();
    KTEXTTEMPLATE_END_LOOKUP


    class OtherType : public SomeType
    {
      // ...
    };

    registerMetaType<SomeType>();

    // Only the introspectable API from SomeType is needed, so we can reuse that
  registration.
    registerMetaType<OtherType, SomeType>();
  @endcode

  @see @ref generic_types_and_templates
 */
template<typename RealType, typename HandleAs>
int registerMetaType()
{
    MetaType::internalLock();

    const int id = InternalRegisterType<RealType, HandleAs>::doReg();

    MetaType::internalUnlock();

    return id;
}

#ifndef K_DOXYGEN
/**
  @internal
  Register a type so %KTextTemplate knows how to handle it.

  This is a convenience method.
 */
template<typename Type>
int registerMetaType()
{
    return registerMetaType<Type, Type>();
}

#endif
} // namespace KTextTemplate

/**
  Top boundary of a lookup function for Type.

  @see @ref generic_types
 */
#define KTEXTTEMPLATE_BEGIN_LOOKUP(Type)                                                                                                                       \
    namespace KTextTemplate                                                                                                                                    \
    {                                                                                                                                                          \
    template<>                                                                                                                                                 \
    inline QVariant TypeAccessor<Type &>::lookUp(const Type &object, const QString &property)                                                                  \
    {
/**
  Top boundary of a lookup function for Type*.

  @see @ref generic_types
 */
#define KTEXTTEMPLATE_BEGIN_LOOKUP_PTR(Type)                                                                                                                   \
    namespace KTextTemplate                                                                                                                                    \
    {                                                                                                                                                          \
    template<>                                                                                                                                                 \
    inline QVariant TypeAccessor<Type *>::lookUp(const Type *const object, const QString &property)                                                            \
    {
/**
  Bottom boundary of a lookup function for Type.

  @see @ref generic_types
 */
#define KTEXTTEMPLATE_END_LOOKUP                                                                                                                               \
    return QVariant();                                                                                                                                         \
    }                                                                                                                                                          \
    }

#endif // #define KTEXTTEMPLATE_METATYPE_H
