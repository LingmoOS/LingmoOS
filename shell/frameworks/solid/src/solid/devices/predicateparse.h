/*
    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PREDICATEPARSE_H
#define PREDICATEPARSE_H

void PredicateLexer_unknownToken(const char *text);

void PredicateParse_setResult(void *result);
void PredicateParse_errorDetected(const char *error);
void PredicateParse_destroy(void *pred);

void *PredicateParse_newAtom(char *interface, char *property, void *value);
void *PredicateParse_newMaskAtom(char *interface, char *property, void *value);
void *PredicateParse_newIsAtom(char *interface);
void *PredicateParse_newAnd(void *pred1, void *pred2);
void *PredicateParse_newOr(void *pred1, void *pred2);
void *PredicateParse_newStringValue(char *val);
void *PredicateParse_newBoolValue(int val);
void *PredicateParse_newNumValue(int val);
void *PredicateParse_newDoubleValue(double val);
void *PredicateParse_newEmptyStringListValue();
void *PredicateParse_newStringListValue(char *name);
void *PredicateParse_appendStringListValue(char *name, void *list);

#endif
