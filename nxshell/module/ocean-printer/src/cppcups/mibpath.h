// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MIBPATH_H
#define MIBPATH_H

/* Host MIB */
#define CUPS_OID_mib2				1,3,6,1,2,1

#define CUPS_OID_system				CUPS_OID_mib2,1
#define CUPS_OID_sysLocation			CUPS_OID_system,6

#define CUPS_OID_host				CUPS_OID_mib2,25

#define CUPS_OID_hrSystem			CUPS_OID_host,1

#define CUPS_OID_hrStorage			CUPS_OID_host,2

#define CUPS_OID_hrDevice			CUPS_OID_host,3
#define CUPS_OID_hrDeviceTable			CUPS_OID_hrDevice,2
#define CUPS_OID_hrDeviceEntry			CUPS_OID_hrDeviceTable,1
#define CUPS_OID_hrDeviceIndex			CUPS_OID_hrDeviceEntry,1
#define CUPS_OID_hrDeviceType			CUPS_OID_hrDeviceEntry,2
#define CUPS_OID_hrDeviceDescr			CUPS_OID_hrDeviceEntry,3

#define CUPS_OID_hrPrinterTable			CUPS_OID_hrDevice,5
#define CUPS_OID_hrPrinterEntry			CUPS_OID_hrPrinterTable,1
#define CUPS_OID_hrPrinterStatus		CUPS_OID_hrPrinterEntry,1
#define CUPS_OID_hrPrinterDetectedErrorState	CUPS_OID_hrPrinterEntry,2

/* Printer MIB */
#define CUPS_OID_printmib			CUPS_OID_mib2,43

#define CUPS_OID_prtGeneral			CUPS_OID_printmib,5
#define CUPS_OID_prtGeneralTable		CUPS_OID_prtGeneral,1
#define CUPS_OID_prtGeneralEntry		CUPS_OID_prtGeneralTable,1
#define CUPS_OID_prtGeneralCurrentLocalization	CUPS_OID_prtGeneralEntry,2
#define CUPS_OID_prtGeneralPrinterName		CUPS_OID_prtGeneralEntry,16
#define CUPS_OID_prtGeneralSerialNumber		CUPS_OID_prtGeneralEntry,17

#define CUPS_OID_prtCover			CUPS_OID_printmib,6
#define CUPS_OID_prtCoverTable			CUPS_OID_prtCover,1
#define CUPS_OID_prtCoverEntry			CUPS_OID_prtCoverTable,1
#define CUPS_OID_prtCoverDescription		CUPS_OID_prtCoverEntry,2
#define CUPS_OID_prtCoverStatus			CUPS_OID_prtCoverEntry,3

#define CUPS_OID_prtLocalization		CUPS_OID_printmib,7
#define CUPS_OID_prtLocalizationTable		CUPS_OID_prtLocalization,1
#define CUPS_OID_prtLocalizationEntry		CUPS_OID_prtLocalizationTable,1
#define CUPS_OID_prtLocalizationCharacterSet	CUPS_OID_prtLocalizationEntry,4

#define CUPS_OID_prtMarker			CUPS_OID_printmib,10
#define CUPS_OID_prtMarkerTable			CUPS_OID_prtMarker,2
#define CUPS_OID_prtMarkerEntry			CUPS_OID_prtMarkerTable,1
#define CUPS_OID_prtMarkerLifeCount		CUPS_OID_prtMarkerEntry,4

#define CUPS_OID_prtMarkerSupplies		CUPS_OID_printmib,11
#define CUPS_OID_prtMarkerSuppliesTable		CUPS_OID_prtMarkerSupplies,1
#define CUPS_OID_prtMarkerSuppliesEntry		CUPS_OID_prtMarkerSuppliesTable,1
#define CUPS_OID_prtMarkerSuppliesIndex		CUPS_OID_prtMarkerSuppliesEntry,1
#define CUPS_OID_prtMarkerSuppliesMarkerIndex	CUPS_OID_prtMarkerSuppliesEntry,2
#define CUPS_OID_prtMarkerSuppliesColorantIndex	CUPS_OID_prtMarkerSuppliesEntry,3
#define CUPS_OID_prtMarkerSuppliesClass		CUPS_OID_prtMarkerSuppliesEntry,4
#define CUPS_OID_prtMarkerSuppliesType		CUPS_OID_prtMarkerSuppliesEntry,5
#define CUPS_OID_prtMarkerSuppliesDescription	CUPS_OID_prtMarkerSuppliesEntry,6
#define CUPS_OID_prtMarkerSuppliesSupplyUnit	CUPS_OID_prtMarkerSuppliesEntry,7
#define CUPS_OID_prtMarkerSuppliesMaxCapacity	CUPS_OID_prtMarkerSuppliesEntry,8
#define CUPS_OID_prtMarkerSuppliesLevel		CUPS_OID_prtMarkerSuppliesEntry,9

#define CUPS_OID_prtMarkerColorant		CUPS_OID_printmib,12
#define CUPS_OID_prtMarkerColorantTable		CUPS_OID_prtMarkerColorant,1
#define CUPS_OID_prtMarkerColorantEntry		CUPS_OID_prtMarkerColorantTable,1
#define CUPS_OID_prtMarkerColorantIndex		CUPS_OID_prtMarkerColorantEntry,1
#define CUPS_OID_prtMarkerColorantMarkerIndex	CUPS_OID_prtMarkerColorantEntry,2
#define CUPS_OID_prtMarkerColorantRole		CUPS_OID_prtMarkerColorantEntry,3
#define CUPS_OID_prtMarkerColorantValue		CUPS_OID_prtMarkerColorantEntry,4
#define CUPS_OID_prtMarkerColorantTonality	CUPS_OID_prtMarkerColorantEntry,5

#define CUPS_OID_prtInterpreter			CUPS_OID_printmib,15
#define CUPS_OID_prtInterpreterTable		CUPS_OID_prtInterpreter,1
#define CUPS_OID_prtInterpreterEntry		CUPS_OID_prtInterpreterTable,1
#define CUPS_OID_prtInterpreterLangFamily	CUPS_OID_prtInterpreterEntry,2
#define CUPS_OID_prtInterpreterLangLevel	CUPS_OID_prtInterpreterEntry,3

#endif // MIBPATH_H
