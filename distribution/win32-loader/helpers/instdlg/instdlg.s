/* Licensed under the zlib/libpng license (same as NSIS) */

/* Customized install files dialog

   Avoided windres because it messes strings up on big endian hosts
   https://sourceware.org/bugzilla/show_bug.cgi?id=17159
 */

.text
.globl	_WinMainCRTStartup
_WinMainCRTStartup:
	movl	$0, %eax
	ret

.set	DS_FIXEDSYS,		0x0008
.set    DS_SETFONT,		0x0040
.set	DS_CONTROL,		0x0400

.set	IDC_STATIC1,		1001
.set	IDC_PROGRESS,		1004
.set	IDC_PROGRESS1,		1005
.set	IDC_INTROTEXT,		1006
.set	IDC_LIST1,		1016
.set	IDC_SHOWDETAILS,	1027
.set	IDD_INSTFILES,		106

.set	IMAGE_RESOURCE_DATA_IS_DIRECTORY, 0x80000000

.set	LANG_ENGLISH_US,	0x409

.set	LVS_REPORT,		0x0001
.set	LVS_SINGLESEL,		0x0004
.set	LVS_NOCOLUMNHEADER,	0x4000

.set	RT_DIALOG,		5

.set	SS_LEFTNOWORDWRAP,	0x0000000C
.set	SS_NOPREFIX,		0x00000080

.set	WC_BUTTON,		0x0080
.set	WC_EDIT,		0x0081
.set	WC_STATIC,		0x0082
.set	WC_LISTBOX,		0x0083
.set	WC_SCROLLBAR,		0x0084
.set	WC_COMBOBOX,		0x0085

.set	WS_TABSTOP,		0x00010000
.set	WS_GROUP,		0x00020000
.set	WS_BORDER,		0x00800000
.set	WS_VISIBLE,		0x10000000
.set	WS_CHILD,		0x40000000

.macro WIDESTRING String
.irpc    Character,"\String"
.asciz  "\Character"
.endr
.short 	0
.endm

.macro	IMAGE_RESOURCE_DIRECTORY Characteristics=0, TimeDateStamp=0, MajorVersion=0, MinorVersion=0, NumberOfNamedEntries=0, NumberOfIdEntries=0
.long	\Characteristics
.long	\TimeDateStamp
.short	\MajorVersion
.short	\MinorVersion
.short	\NumberOfNamedEntries
.short	\NumberOfIdEntries
.endm

.macro	IMAGE_RESOURCE_DIRECTORY_ENTRY Name=0, OffsetToData=0
.long	\Name
.long	\OffsetToData
.endm

.macro	IMAGE_RESOURCE_DATA_ENTRY OffsetToData=0, Size=0, CodePage=0, Reserved=0
.long	\OffsetToData
.long	\Size
.long	\CodePage
.long	\Reserved
.endm

.macro	DLGTEMPLATEEX_WITH_FONT dlgVer=1, signature=0xFFFF, helpID=0, exStyle=0, style=0, cDlgItems=0, x=0, y=0, cx=0, cy=0, menu=0, windowClass=0, title="", pointsize=0, weight=0, italic=0, charset=0, typeface=""
.short	\dlgVer
.short	\signature
.long	\helpID
.long	\exStyle
.long	\style
.short	\cDlgItems
.short	\x
.short	\y
.short	\cx
.short	\cy
.short	\menu
.short	\windowClass
WIDESTRING "\title"
.short	\pointsize
.short	\weight
.byte	\italic
.byte	\charset
WIDESTRING "\typeface"
.endm

.macro	DLGITEMTEMPLATEEX helpID=0, exStyle=0, style=0, x=0, y=0, cx=0, cy=0, id=0, windowClass=0, title="", extraCount=0
.long	\helpID
.long	\exStyle
.long	\style
.short	\x
.short	\y
.short	\cx
.short	\cy
.long	\id
.short	0xFFFF
.short	\windowClass
WIDESTRING "\title"
.short	\extraCount
.endm

.macro	DLGITEMTEMPLATEEX_SZCLASS helpID=0, exStyle=0, style=0, x=0, y=0, cx=0, cy=0, id=0, windowClass="", title="", extraCount=0
.long	\helpID
.long	\exStyle
.long	\style
.short	\x
.short	\y
.short	\cx
.short	\cy
.long	\id
WIDESTRING "\windowClass"
WIDESTRING "\title"
.short	\extraCount
.endm

.macro	DLGITEMTEMPLATEEX_PROGRESSBAR helpID=0, exStyle=0, style=0, x=0, y=0, cx=0, cy=0, id=0, title="", extraCount=0
DLGITEMTEMPLATEEX_SZCLASS helpID=\helpID, exStyle=\exStyle, style=\style, x=\x, y=\y, cx=\cx, cy=\cy, id=\id, windowClass="MSCTLS_PROGRESS32", title=\title, extraCount=\extraCount
.endm

.macro	DLGITEMTEMPLATEEX_LISTVIEW helpID=0, exStyle=0, style=0, x=0, y=0, cx=0, cy=0, id=0, title="", extraCount=0
DLGITEMTEMPLATEEX_SZCLASS helpID=\helpID, exStyle=\exStyle, style=\style, x=\x, y=\y, cx=\cx, cy=\cy, id=\id, windowClass="SYSLISTVIEW32", title=\title, extraCount=\extraCount
.endm

.section .rsrc
rsrc:
	IMAGE_RESOURCE_DIRECTORY NumberOfIdEntries=((rsrc_directory_entries - rsrc_root_directory_entries) / (rsrc_root_directory_entries_first_end - rsrc_root_directory_entries))

rsrc_root_directory_entries:
	IMAGE_RESOURCE_DIRECTORY_ENTRY Name=RT_DIALOG, OffsetToData=(IMAGE_RESOURCE_DATA_IS_DIRECTORY | (rsrc_dialog_ID - rsrc))
rsrc_root_directory_entries_first_end:

rsrc_directory_entries:
rsrc_dialog_ID:
	IMAGE_RESOURCE_DIRECTORY NumberOfIdEntries=1
	IMAGE_RESOURCE_DIRECTORY_ENTRY Name=IDD_INSTFILES, OffsetToData=(IMAGE_RESOURCE_DATA_IS_DIRECTORY | (rsrc_dialog_directory - rsrc))

rsrc_dialog_directory:
	IMAGE_RESOURCE_DIRECTORY NumberOfIdEntries=1
	IMAGE_RESOURCE_DIRECTORY_ENTRY Name=LANG_ENGLISH_US, OffsetToData=(rsrc_dialog_content - rsrc)

rsrc_dialog_content:
	IMAGE_RESOURCE_DATA_ENTRY OffsetToData=(rsrc_dialog_data - IMAGE_BASE), Size=(rsrc_dialog_data_end - rsrc_dialog_data)

.align 4, 0
rsrc_dialog_data:
	DLGTEMPLATEEX_WITH_FONT style=(DS_FIXEDSYS|DS_SETFONT|DS_CONTROL|WS_CHILD), cDlgItems=6, x=0, y=0, cx=300, cy=140, pointsize=8, charset=1, typeface="MS Shell Dlg"
rsrc_dialog_items:
	DLGITEMTEMPLATEEX style=(WS_VISIBLE|WS_CHILD|WS_TABSTOP), x=1, y=26, cx=60, cy=14, id=IDC_SHOWDETAILS, windowClass=WC_BUTTON
	DLGITEMTEMPLATEEX_PROGRESSBAR style=(WS_VISIBLE|WS_CHILD|WS_BORDER), x=0, y=10, cx=300, cy=11, id=IDC_PROGRESS
	DLGITEMTEMPLATEEX style=(WS_VISIBLE|WS_CHILD|SS_LEFTNOWORDWRAP|SS_NOPREFIX), x=0, y=0, cx=300, cy=10, id=IDC_INTROTEXT, windowClass=WC_STATIC
	DLGITEMTEMPLATEEX_LISTVIEW style=(WS_CHILD|WS_BORDER|WS_TABSTOP|LVS_REPORT|LVS_SINGLESEL|LVS_NOCOLUMNHEADER), x=0, y=25, cx=300, cy=110, id=IDC_LIST1
	DLGITEMTEMPLATEEX_PROGRESSBAR style=(WS_CHILD|WS_BORDER), x=16, y=75, cx=266, cy=11, id=IDC_PROGRESS1
	DLGITEMTEMPLATEEX style=(WS_CHILD|SS_LEFTNOWORDWRAP|SS_NOPREFIX), x=16, y=65, cx=266, cy=10, id=IDC_STATIC1, windowClass=WC_STATIC
rsrc_dialog_data_end:
