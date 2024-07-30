/*  Convenience header for conditional use of GNU <libintl.h>.
    SPDX-FileCopyrightText: 1995-1998, 2000-2002, 2004-2006 Free Software Foundation, Inc.

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
// clang-format off
#ifndef _LIBGETTEXT_H
#define _LIBGETTEXT_H 1

/* Get declarations of GNU message catalog functions.  */
# include <libintl.h>
// libintl.h redefines inline which causes MSVC to abort compilation with the message
// fatal error C1189: #error :  The C++ Standard Library forbids macroizing keywords
#undef inline

/* You can set the DEFAULT_TEXT_DOMAIN macro to specify the domain used by
   the gettext() and ngettext() macros.  This is an alternative to calling
   textdomain(), and is useful for libraries.  */
# ifdef DEFAULT_TEXT_DOMAIN
#  undef gettext
#  define gettext(Msgid) \
    dgettext (DEFAULT_TEXT_DOMAIN, Msgid)
#  undef ngettext
#  define ngettext(Msgid1, Msgid2, N) \
    dngettext (DEFAULT_TEXT_DOMAIN, Msgid1, Msgid2, N)
# endif

/* The separator between msgctxt and msgid in a .mo file.  */
#define GETTEXT_CONTEXT_GLUE "\004"

/* Pseudo function calls, taking a MSGCTXT and a MSGID instead of just a
   MSGID.  MSGCTXT and MSGID must be string literals.  MSGCTXT should be
   short and rarely need to change.
   The letter 'p' stands for 'particular' or 'special'.  */
#ifdef DEFAULT_TEXT_DOMAIN
# define pgettext(Msgctxt, Msgid) \
    pgettext_aux (DEFAULT_TEXT_DOMAIN, Msgctxt GETTEXT_CONTEXT_GLUE Msgid, Msgid)
#else
# define pgettext(Msgctxt, Msgid) \
    pgettext_aux (NULL, Msgctxt GETTEXT_CONTEXT_GLUE Msgid, Msgid)
#endif
#define dpgettext(Domainname, Msgctxt, Msgid) \
    pgettext_aux (Domainname, Msgctxt GETTEXT_CONTEXT_GLUE Msgid, Msgid)

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static const char *
pgettext_aux(const char *domain,
             const char *msg_ctxt_id, const char *msgid)
{
    const char *translation = dgettext(domain, msg_ctxt_id);
    if (translation == msg_ctxt_id) {
        return msgid;
    } else {
        return translation;
    }
}

/* The same thing extended for non-constant arguments.  Here MSGCTXT and MSGID
   can be arbitrary expressions.  But for string literals these macros are
   less efficient than those above.  */

#include <string.h>

#ifndef __STRICT_ANSI__
#define __STRICT_ANSI__
#define __STRICT_ANSI_FORCED__
#endif

#define _LIBGETTEXT_HAVE_VARIABLE_SIZE_ARRAYS \
    (__STRICT_ANSI__ - 0 == 0) && (__GNUC__ >= 3 || __GNUG__ >= 2 /* || __STDC_VERSION__ >= 199901L */ )

#if !_LIBGETTEXT_HAVE_VARIABLE_SIZE_ARRAYS
#include <stdlib.h>
#endif

#define pgettext_expr(Msgctxt, Msgid) \
    dpgettext_expr (NULL, Msgctxt, Msgid)
#define dpgettext_expr(Domainname, Msgctxt, Msgid) \
    dpgettext_expr (Domainname, Msgctxt, Msgid)

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static const char *
dpgettext_expr(const char *domain,
                const char *msgctxt, const char *msgid)
{
    size_t msgctxt_len = strlen(msgctxt) + 1;
    size_t msgid_len = strlen(msgid) + 1;
    const char *translation;
    int translation_found;
#if _LIBGETTEXT_HAVE_VARIABLE_SIZE_ARRAYS
    char msg_ctxt_id[msgctxt_len + msgid_len];
#else
    char buf[1024];
    char *msg_ctxt_id =
        (msgctxt_len + msgid_len <= sizeof(buf)
         ? buf
         : (char *) malloc(msgctxt_len + msgid_len));
    if (msg_ctxt_id != nullptr)
#endif
    {
        memcpy(msg_ctxt_id, msgctxt, msgctxt_len - 1);
        msg_ctxt_id[msgctxt_len - 1] = '\004';
        memcpy(msg_ctxt_id + msgctxt_len, msgid, msgid_len);
        translation = dgettext(domain, msg_ctxt_id);
        /* Test must occur before msg_ctxt_id freed */
        translation_found = translation != msg_ctxt_id;
#if !_LIBGETTEXT_HAVE_VARIABLE_SIZE_ARRAYS
        if (msg_ctxt_id != buf) {
            free(msg_ctxt_id);
        }
#endif
        if (translation_found) {
            return translation;
        }
    }
    return msgid;
}

#define npgettext_expr(Msgctxt, Msgid, MsgidPlural, N) \
    dnpgettext_expr (NULL, Msgctxt, Msgid, MsgidPlural, N)
#define dnpgettext_expr(Domainname, Msgctxt, Msgid, MsgidPlural, N) \
    dnpgettext_expr (Domainname, Msgctxt, Msgid, MsgidPlural, N)

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static const char *
dnpgettext_expr(const char *domain,
                 const char *msgctxt, const char *msgid,
                 const char *msgid_plural, unsigned long int n)
{
    size_t msgctxt_len = strlen(msgctxt) + 1;
    size_t msgid_len = strlen(msgid) + 1;
    const char *translation;
    int translation_found;
#if _LIBGETTEXT_HAVE_VARIABLE_SIZE_ARRAYS
    char msg_ctxt_id[msgctxt_len + msgid_len];
#else
    char buf[1024];
    char *msg_ctxt_id =
        (msgctxt_len + msgid_len <= sizeof(buf)
         ? buf
         : (char *) malloc(msgctxt_len + msgid_len));
    if (msg_ctxt_id != nullptr)
#endif
    {
        memcpy(msg_ctxt_id, msgctxt, msgctxt_len - 1);
        msg_ctxt_id[msgctxt_len - 1] = '\004';
        memcpy(msg_ctxt_id + msgctxt_len, msgid, msgid_len);
        translation = dngettext(domain, msg_ctxt_id, msgid_plural, n);
        /* Test must occur before msg_ctxt_id freed */
        translation_found = !(translation == msg_ctxt_id || translation == msgid_plural);
#if !_LIBGETTEXT_HAVE_VARIABLE_SIZE_ARRAYS
        if (msg_ctxt_id != buf) {
            free(msg_ctxt_id);
        }
#endif
        if (translation_found) {
            return translation;
        }
    }
    return (n == 1 ? msgid : msgid_plural);
}

#ifdef __STRICT_ANSI_FORCED__
#undef __STRICT_ANSI__
#undef __STRICT_ANSI_FORCED__
#endif

#endif /* _LIBGETTEXT_H */
// clang-format on
