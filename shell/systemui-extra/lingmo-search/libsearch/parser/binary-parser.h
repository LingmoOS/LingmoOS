/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *
 */
#ifndef SEARCHHELPER_H
#define SEARCHHELPER_H
#include <QtCore>
#include <QtConcurrent/QtConcurrent>

#define ULONG unsigned long
#define UCHAR unsigned char
#define USHORT unsigned short

typedef enum {
    Word = 0,
    Excel,
    Ppt
} TYPE;

/* Property Set Storage */
typedef struct pps_tag {
    ULONG	ulSB;
    ULONG	ulSize;
} ppsTag;

typedef struct pps_info_tag {
    ppsTag	tWordDocument;	/* Text stream */
    ppsTag	tWorkBook;
    ppsTag	tPPTDocument;
    ppsTag	tData;		/* Data stream */
    ppsTag	tTable;		/* Table stream */
    ppsTag	tSummaryInfo;	/* Summary Information */
    ppsTag	tDocSummaryInfo;/* Document Summary Information */
    ppsTag	t0Table;	/* Table 0 stream */
    ppsTag	t1Table;	/* Table 1 stream */
    ppsTag	tCurrentUser;
    TYPE		type;
} ppsInfoType;

/* Private type for Property Set Storage entries */
typedef struct pps_entry_tag {
    ULONG	ulNext;
    ULONG	ulPrevious;
    ULONG	ulDir;
    ULONG	ulSB;
    ULONG	ulSize;
    int	iLevel;
    char	szName[32];
    UCHAR	ucType;
} ppsEntryType;

/* Excel Record Struct*/
typedef struct excelRecord {
    excelRecord() {
        usLen = 0;
        usRichLen = 0;
        ulWLen = 0;
        bUni = false;
    }
    ushort usLen;
    ushort usRichLen;
    ulong ulWLen;
    bool bUni;
} excelRecord;

typedef struct readDataParam {
    readDataParam() {
        ulStBlk = 0;
        pFile = NULL;
        ulBBd = NULL;
        tBBdLen = 0;
        usBlkSize = 0;
    }
    ulong ulStBlk;
    FILE *pFile;
    ulong *ulBBd;
    size_t tBBdLen;
    ushort usBlkSize;
} rdPara;

class KBinaryParser : public QObject {
    Q_OBJECT
public:
    KBinaryParser(QObject *parent = 0);
    ~KBinaryParser();

public:
    bool RunParser(QString strFile, QString &content);

private:
    bool bGetPPS(FILE *pFile,
                 const ULONG *aulRootList, size_t tRootListLen, ppsInfoType *pPPS);

    int readData(rdPara &readParam, uchar *aucBuffer, ulong ulOffset, size_t tToRead);

    int InitDocOle(FILE *pFile, long lFilesize, QString &content);
    bool read8DocText(FILE *pFile, const ppsInfoType *pPPS,
                      const ULONG *aulBBD, size_t tBBDLen,
                      const ULONG *aulSBD, size_t tSBDLen,
                      const UCHAR *aucHeader, QString &content);

    int readSSTRecord(readDataParam &rdParam, ppsInfoType, ulong &ulOff, ushort usPartLen, QString &content);
    int read8BiffRecord(uchar uFlag, ulong ulOff, ulong &ulNext, readDataParam &rdParam, excelRecord &eR);

    ULONG readPPtRecord(FILE* pFile, ppsInfoType* PPS_info, ULONG* aulBBD,
                        size_t tBBDLen, ULONG ulPos, QString &content);

    QString m_strFileName;
};

#endif // SEARCHHELPER_H
