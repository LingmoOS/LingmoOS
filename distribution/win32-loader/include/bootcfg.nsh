; Licensed under the zlib/libpng license (same as NSIS)

!ifndef BOOTCFG_INCLUDED
!define BOOTCFG_INCLUDED

!include include\stdmacros.nsh
!include LogicLib.nsh
!include WinCore.nsh
!include Win\COM.nsh
!include include\security.nsh

${STDMACROS_DefIfNotDef} IID_IWbemLocator \
  "{dc12a687-737f-11cf-884d-00aa004b2e24}"
${NSISCOMIFACEDECL}IWbemLocator ConnectServer 3 (w,p,p,p,i,p,p,*p)i

${NSISCOMIFACEDECL}IWbemServices GetObject 6 (w,i,p,*p,p)i
${NSISCOMIFACEDECL}IWbemServices ExecMethod 24 (w,w,i,p,p,*p,p)i

${NSISCOMIFACEDECL}IWbemClassObject Get 4 (w,i,p,p,p)i
${NSISCOMIFACEDECL}IWbemClassObject Put 5 (w,i,p,i)i
${NSISCOMIFACEDECL}IWbemClassObject SpawnInstance 15 (i,*p)i
${NSISCOMIFACEDECL}IWbemClassObject GetMethod 19 (w,i,*p,p)i

!define BOOTCFG_BCDE_AUTORECOVERYENABLED      0x16000009
!define BOOTCFG_BCDE_RECOVERYSEQUENCE         0x14000008
!define BOOTCFG_BCDE_DESCRIPTION              0x12000004
!define BOOTCFG_BCDE_DEVICE                   0x11000001
!define BOOTCFG_BCDE_PATH                     0x12000002
!define BOOTCFG_BOOT_MANAGER                  0x10100002
!define BOOTCFG_BOOT_SECTOR                   0x10400008
!define BOOTCFG_DISPLAY_ORDER                 0x24000001
!define BOOTCFG_BOOTSEQUENCE                  0x24000002
!define BOOTCFG_BOOTMGR_GUID   "{9dea862c-5cdd-4e70-acc1-f32b344d4795}"
!define BOOTCFG_CURRENT_GUID   "{fa926493-6f1c-4193-a414-58f0b2456d1e}"
!define BOOTCFG_FWBOOTMGR_GUID "{a5a30fa2-3d06-4e9f-b5f4-a01df9d1fcba}"
!define BOOTCFG_PARTIONDEVICE_TYPE     2
!define BOOTCFG_VOLUME_BLOCK_RECORD_SIZE 512
!define BOOTCFG_VOLUME_SERIAL_NUMBER_OFFSET 72

${STDMACROS_DefIfNotDef} CLSID_WbemLocator \
  "{4590f811-1d3a-11d0-891f-00aa004b2e24}"
${STDMACROS_DefIfNotDef} ERROR_INVALID_PARAMETER       87
${STDMACROS_DefIfNotDef} ERROR_NO_ACCESS              998
${STDMACROS_DefIfNotDef} ERROR_NO_MORE_ITEMS          259

${STDMACROS_DefIfNotDef} FIRMWARE_TYPE_UEFI             2

${STDMACROS_DefIfNotDef} IOCTL_DISK_GET_PARTITION_INFO_EX 0x00070048
${STDMACROS_DefIfNotDef} PARTITION_INFORMATION_EX_SIZEOF 144
${STDMACROS_DefIfNotDef} PARTITION_STYLE_GPT 1

${STDMACROS_DefIfNotDef} OPEN_EXISTING                  3

${STDMACROS_DefIfNotDef} WBEM_FLAG_FORWARD_ONLY       0x20
${STDMACROS_DefIfNotDef} WBEM_FLAG_RETURN_IMMEDIATELY 0x10
${STDMACROS_DefIfNotDef} WBEM_INFINITE                0xffffffff

${STDMACROS_DefIfNotDef} VT_ARRAY                    8192 ; 0x2000
${STDMACROS_DefIfNotDef} VT_ARRAY_BSTR               8200 ; 0x2008
${STDMACROS_DefIfNotDef} VT_BOOL                       11 ; 0x000B
${STDMACROS_DefIfNotDef} VT_BSTR                        8 ; 0x0008
${STDMACROS_DefIfNotDef} VT_EMPTY                       0 ; 0x0000
${STDMACROS_DefIfNotDef} VT_I4                          3 ; 0x0003
${STDMACROS_DefIfNotDef} VT_UNKNOWN                    13 ; 0x000D

; Function parameters are ordered from first pushed parameter
; to the last pushed parameter (top most on the stack)

!define BOOTCFG_PREFIX "BOOTCFG_"
!define BOOTCFG_UNFUNC "un."

!define BOOTCFG_FUNCDEF \
  `!insertmacro STDMACROS_FUNCDEF ${BOOTCFG_PREFIX}`
!define BOOTCFG_FUNCINC \
  `!insertmacro STDMACROS_FUNCINC ${BOOTCFG_PREFIX}`
!define BOOTCFG_FUNCPROLOG \
  `!insertmacro STDMACROS_FUNCPROLOG ${BOOTCFG_PREFIX}`

; ${BOOTCFG_AllocEmptyVariant} variant
${BOOTCFG_FUNCDEF} AllocEmptyVariant
!macro BOOTCFG_AllocEmptyVariant_Call UN VARIANT
  Call `${UN}${BOOTCFG_PREFIX}AllocEmptyVariant`
  Pop ${VARIANT}
!macroend

; ${BOOTCFG_AllocI4Variant} value variant
${BOOTCFG_FUNCDEF} AllocI4Variant
!macro BOOTCFG_AllocI4Variant_Call UN VALUE VARIANT
  Push ${VALUE}
  Call `${UN}${BOOTCFG_PREFIX}AllocI4Variant`
  Pop ${VARIANT}
!macroend

; ${BOOTCFG_AllocStringArrayVariant} array variant
${BOOTCFG_FUNCDEF} AllocStringArrayVariant
!macro BOOTCFG_AllocStringArrayVariant_Call UN ARRAY VARIANT
  Push ${ARRAY}
  Call `${UN}${BOOTCFG_PREFIX}AllocStringArrayVariant`
  Pop ${VARIANT}
!macroend

; ${BOOTCFG_AllocStringVariant} string variant
${BOOTCFG_FUNCDEF} AllocStringVariant
!macro BOOTCFG_AllocStringVariant_Call UN STRING VARIANT
  Push "${STRING}"
  Call `${UN}${BOOTCFG_PREFIX}AllocStringVariant`
  Pop ${VARIANT}
!macroend

; ${BOOTCFG_FreeStringVariant} variant
${BOOTCFG_FUNCDEF} FreeStringVariant
!macro BOOTCFG_FreeStringVariant_Call UN VARIANT
  Push ${VARIANT}
  Call `${UN}${BOOTCFG_PREFIX}FreeStringVariant`
!macroend

; ${BOOTCFG_GetVariantType} variant type
${BOOTCFG_FUNCDEF} GetVariantType
!macro BOOTCFG_GetVariantType_Call UN VARIANT TYPE
  Push ${VARIANT}
  Call `${UN}${BOOTCFG_PREFIX}GetVariantType`
  Pop ${TYPE}
!macroend

; ${BOOTCFG_GetBoolOfVariant} variant value err
${BOOTCFG_FUNCDEF} GetBoolOfVariant
!macro BOOTCFG_GetBoolOfVariant_Call UN VARIANT VALUE ERR
  Push ${VARIANT}
  Call `${UN}${BOOTCFG_PREFIX}GetBoolOfVariant`
  Pop ${ERR}
  Pop ${VALUE}
!macroend

; ${BOOTCFG_GetI4OfVariant} variant value err
${BOOTCFG_FUNCDEF} GetI4OfVariant
!macro BOOTCFG_GetI4OfVariant_Call UN VARIANT VALUE ERR
  Push ${VARIANT}
  Call `${UN}${BOOTCFG_PREFIX}GetI4OfVariant`
  Pop ${ERR}
  Pop ${VALUE}
!macroend

; ${BOOTCFG_GetObjectOfVariant} variant obj err
${BOOTCFG_FUNCDEF} GetObjectOfVariant
!macro BOOTCFG_GetObjectOfVariant_Call UN VARIANT OBJ ERR
  Push ${VARIANT}
  Call `${UN}${BOOTCFG_PREFIX}GetObjectOfVariant`
  Pop ${ERR}
  Pop ${OBJ}
!macroend

; ${BOOTCFG_GetStringOfVariant} variant string err
${BOOTCFG_FUNCDEF} GetStringOfVariant
!macro BOOTCFG_GetStringOfVariant_Call UN VARIANT STRING ERR
  Push ${VARIANT}
  Call `${UN}${BOOTCFG_PREFIX}GetStringOfVariant`
  Pop ${ERR}
  Pop ${STRING}
!macroend

; ${BOOTCFG_GetStringArrayOfVariant} variant array err
${BOOTCFG_FUNCDEF} GetStringArrayOfVariant
!macro BOOTCFG_GetStringArrayOfVariant_Call UN VARIANT ARRAY ERR
  Push ${VARIANT}
  Call `${UN}${BOOTCFG_PREFIX}GetStringArrayOfVariant`
  Pop ${ERR}
  Pop ${ARRAY}
!macroend

; ${BOOTCFG_IsUEFI} uefi
${BOOTCFG_FUNCDEF} IsUEFI
!macro BOOTCFG_IsUEFI_Call UN UEFI
  Call `${UN}${BOOTCFG_PREFIX}IsUEFI`
  Pop ${UEFI}
!macroend

; ${BOOTCFG_ConnectWMI} locator services err
${BOOTCFG_FUNCDEF} ConnectWMI
!macro BOOTCFG_ConnectWMI_Call UN LOCATOR SERVICES ERR
  Call `${UN}${BOOTCFG_PREFIX}ConnectWMI`
  Pop ${ERR}
  Pop ${SERVICES}
  Pop ${LOCATOR}
!macroend

; ${BOOTCFG_GetObject} services classname obj err
${BOOTCFG_FUNCDEF} GetObject
!macro BOOTCFG_GetObject_Call UN SERVICES CLASSNAME OBJ ERR
  Push ${SERVICES}
  Push ${CLASSNAME}
  Call `${UN}${BOOTCFG_PREFIX}GetObject`
  Pop ${ERR}
  Pop ${OBJ}
!macroend

; ${BOOTCFG_GetMethod} baseobject method inparams err
${BOOTCFG_FUNCDEF} GetMethod
!macro BOOTCFG_GetMethod_Call UN BASEOBJECT METHOD INPARAMS ERR
  Push ${BASEOBJECT}
  Push ${METHOD}
  Call `${UN}${BOOTCFG_PREFIX}GetMethod`
  Pop ${ERR}
  Pop ${INPARAMS}
!macroend

; ${BOOTCFG_SpawnInstance} inparams instance err
${BOOTCFG_FUNCDEF} SpawnInstance
!macro BOOTCFG_SpawnInstance_Call UN INPARAMS INSTANCE ERR
  Push ${INPARAMS}
  Call `${UN}${BOOTCFG_PREFIX}SpawnInstance`
  Pop ${ERR}
  Pop ${INSTANCE}
!macroend

; ${BOOTCFG_PrepareMethod} baseobject method instance err
${BOOTCFG_FUNCDEF} PrepareMethod
!macro BOOTCFG_PrepareMethod_Call UN BASEOBJECT METHOD INSTANCE ERR
  Push ${BASEOBJECT}
  Push ${METHOD}
  Call `${UN}${BOOTCFG_PREFIX}PrepareMethod`
  Pop ${ERR}
  Pop ${INSTANCE}
!macroend

; ${BOOTCFG_Put} instance key value err
${BOOTCFG_FUNCDEF} Put
!macro BOOTCFG_Put_Call UN INSTANCE KEY VALUE ERR
  Push ${INSTANCE}
  Push ${KEY}
  Push ${VALUE}
  Call `${UN}${BOOTCFG_PREFIX}Put`
  Pop ${ERR}
!macroend

; ${BOOTCFG_ExecMethod} services obj instance method result err
${BOOTCFG_FUNCDEF} ExecMethod
!macro BOOTCFG_ExecMethod_Call UN SERVICES \
  OBJ INSTANCE METHOD RESULT ERR
  Push ${SERVICES}
  Push ${OBJ}
  Push ${INSTANCE}
  Push ${METHOD}
  Call `${UN}${BOOTCFG_PREFIX}ExecMethod`
  Pop ${ERR}
  Pop ${RESULT}
!macroend

; ${BOOTCFG_CallMethod} services baseobject obj method key \
;   value result err
${BOOTCFG_FUNCDEF} CallMethod
!macro BOOTCFG_CallMethod_Call UN SERVICES BASEOBJECT \
  OBJ METHOD KEY VALUE RESULT ERR
  Push ${SERVICES}
  Push ${BASEOBJECT}
  Push ${OBJ}
  Push ${METHOD}
  Push ${KEY}
  Push ${VALUE}
  Call `${UN}${BOOTCFG_PREFIX}CallMethod`
  Pop ${ERR}
  Pop ${RESULT}
!macroend

; ${BOOTCFG_OpenBcdStore} services baseobject filename obj err
${BOOTCFG_FUNCDEF} OpenBcdStore
!macro BOOTCFG_OpenBcdStore_Call UN SERVICES BASEOBJECT \
  FILENAME OBJ ERR
  Push ${SERVICES}
  Push ${BASEOBJECT}
  Push ${FILENAME}
  Call `${UN}${BOOTCFG_PREFIX}OpenBcdStore`
  Pop ${ERR}
  Pop ${OBJ}
!macroend

; ${BOOTCFG_OpenDefaultBcdStore} services baseobject obj err
${BOOTCFG_FUNCDEF} OpenDefaultBcdStore
!macro BOOTCFG_OpenDefaultBcdStore_Call UN SERVICES BASEOBJECT \
  OBJ ERR
  Push ${SERVICES}
  Push ${BASEOBJECT}
  Push ""
  Call `${UN}${BOOTCFG_PREFIX}OpenBcdStore`
  Pop ${ERR}
  Pop ${OBJ}
!macroend
!macro BOOTCFG_OpenDefaultBcdStore UN
${BOOTCFG_FUNCINC} "${UN}" OpenBcdStore
!macroend

; ${BOOTCFG_GetBcdObject} services baseobject bcdstore id obj err
${BOOTCFG_FUNCDEF} GetBcdObject
!macro BOOTCFG_GetBcdObject_Call UN SERVICES BASEOBJECT \
  BCDSTORE ID OBJ ERR
  Push ${SERVICES}
  Push ${BASEOBJECT}
  Push ${BCDSTORE}
  Push ${ID}
  Call `${UN}${BOOTCFG_PREFIX}GetBcdObject`
  Pop ${ERR}
  Pop ${OBJ}
!macroend

; ${BOOTCFG_GetBcdElement} services baseobject bcdobject elemtype \
;    elment err
${BOOTCFG_FUNCDEF} GetBcdElement
!macro BOOTCFG_GetBcdElement_Call UN SERVICES BASEOBJECT \
  BCDOBJECT ELEMTYPE ELEMENT ERR
  Push ${SERVICES}
  Push ${BASEOBJECT}
  Push ${BCDOBJECT}
  Push ${ELEMTYPE}
  Call `${UN}${BOOTCFG_PREFIX}GetBcdElement`
  Pop ${ERR}
  Pop ${ELEMENT}
!macroend

; ${BOOTCFG_GetObjectPropertyValue} obj property value err
${BOOTCFG_FUNCDEF} GetObjectPropertyValue
!macro BOOTCFG_GetObjectPropertyValue_Call UN OBJ PROPERTY \
  VALUE ERR
  Push ${OBJ}
  Push ${PROPERTY}
  Call `${UN}${BOOTCFG_PREFIX}GetObjectPropertyValue`
  Pop ${ERR}
  Pop ${VALUE}
!macroend

; ${BOOTCFG_GetElementFromBcd} services basebcdstore bcdstore \
;   basebcdobject id elemtype obj elem err
${BOOTCFG_FUNCDEF} GetElementFromBcd
!macro BOOTCFG_GetElementFromBcd_Call UN SERVICES \
  BASEBCDSTORE BCDSTORE BASEBCDOBJECT ID ELEMTYPE OBJ ELEM ERR
  Push ${SERVICES}
  Push ${BASEBCDSTORE}
  Push ${BCDSTORE}
  Push ${BASEBCDOBJECT}
  Push ${ID}
  Push ${ELEMTYPE}
  Call `${UN}${BOOTCFG_PREFIX}GetElementFromBcd`
  Pop ${ERR}
  Pop ${ELEM}
  Pop ${OBJ}
!macroend

; ${BOOTCFG_GetESPVolume} services basebcdstore bcdstore basebcdobject volume err
${BOOTCFG_FUNCDEF} GetESPVolume
!macro BOOTCFG_GetESPVolume_Call UN SERVICES BASEBCDSTORE BCDSTORE BASEBCDOBJECT VOLUME ERR
  Push ${SERVICES}
  Push ${BASEBCDSTORE}
  Push ${BCDSTORE}
  Push ${BASEBCDOBJECT}
  Call `${UN}${BOOTCFG_PREFIX}GetESPVolume`
  Pop ${ERR}
  Pop ${VOLUME}
!macroend

; ${BOOTCFG_GetFileSystemUUID} drive uuid
${BOOTCFG_FUNCDEF} GetFileSystemUUID
!macro BOOTCFG_GetFileSystemUUID_Call UN DRIVE UUID
  Push ${DRIVE}
  Call `${UN}${BOOTCFG_PREFIX}GetFileSystemUUID`
  Pop ${UUID}
!macroend

; ${BOOTCFG_GetSystemPartition} services basebcdstore partition err
${BOOTCFG_FUNCDEF} GetSystemPartition
!macro BOOTCFG_GetSystemPartition_Call UN SERVICES BASEBCDSTORE PARTITION ERR
  Push ${SERVICES}
  Push ${BASEBCDSTORE}
  Call `${UN}${BOOTCFG_PREFIX}GetSystemPartition`
  Pop ${ERR}
  Pop ${PARTITION}
!macroend

; ${BOOTCFG_GetVolumeUUID} volume uuid err
${BOOTCFG_FUNCDEF} GetVolumeUUID
!macro BOOTCFG_GetVolumeUUID_Call UN VOLUME UUID ERR
  Push ${VOLUME}
  Call `${UN}${BOOTCFG_PREFIX}GetVolumeUUID`
  Pop ${ERR}
  Pop ${UUID}
!macroend

; ${BOOTCFG_ObjectListGetSafeArray} objectlist safearray err
${BOOTCFG_FUNCDEF} ObjectListGetSafeArray
!macro BOOTCFG_ObjectListGetSafeArray_Call UN OBJECTLIST \
  SAFEARRAY ERR
  Push ${OBJECTLIST}
  Call `${UN}${BOOTCFG_PREFIX}ObjectListGetSafeArray`
  Pop ${ERR}
  Pop ${SAFEARRAY}
!macroend

; ${BOOTCFG_GetSizeOfSafeArray} safearray size
${BOOTCFG_FUNCDEF} GetSizeOfSafeArray
!macro BOOTCFG_GetSizeOfSafeArray_Call UN SAFEARRAY SIZE
  Push ${SAFEARRAY}
  Call `${UN}${BOOTCFG_PREFIX}GetSizeOfSafeArray`
  Pop ${SIZE}
!macroend

; ${BOOTCFG_GetElementDescription} element description
${BOOTCFG_FUNCDEF} GetElementDescription
!macro BOOTCFG_GetElementDescription_Call UN ELEMENT DESCRIPTION
  Push ${ELEMENT}
  Call `${UN}${BOOTCFG_PREFIX}GetElementDescription`
  Pop ${DESCRIPTION}
!macroend

; ${BOOTCFG_GetBcdObjectDescription} $services basebcdstore bcdstore \
;  basebcdobject id description
${BOOTCFG_FUNCDEF} GetBcdObjectDescription
!macro BOOTCFG_GetBcdObjectDescription_Call UN SERVICES BASEBCDSTORE \
  BCDSTORE BASEBCDOBJECT ID DESCRIPTION
  Push ${SERVICES}
  Push ${BASEBCDSTORE}
  Push ${BCDSTORE}
  Push ${BASEBCDOBJECT}
  Push ${ID}
  Call `${UN}${BOOTCFG_PREFIX}GetBcdObjectDescription`
  Pop ${DESCRIPTION}
!macroend

; ${BOOTCFG_EnumerateBcdObjectList} services basebcdstore bcdstore \
;   basebcdobject objectlist err
${BOOTCFG_FUNCDEF} EnumerateBcdObjectList
!macro BOOTCFG_EnumerateBcdObjectList_Call UN SERVICES \
  BASEBCDSTORE BCDSTORE BASEBCDOBJECT OBJECTLIST ERR
  Push ${SERVICES}
  Push ${BASEBCDSTORE}
  Push ${BCDSTORE}
  Push ${BASEBCDOBJECT}
  Push ${OBJECTLIST}
  Call `${UN}${BOOTCFG_PREFIX}EnumerateBcdObjectList`
  Pop ${ERR}
!macroend

; ${BOOTCFG_CreateGUID} guid
${BOOTCFG_FUNCDEF} CreateGUID
!macro BOOTCFG_CreateGUID_Call UN GUID
  Call `${UN}${BOOTCFG_PREFIX}CreateGUID`
  Pop ${GUID}
!macroend

; ${BOOTCFG_CreateObject} services basebcdstore bcdstore type guid obj err
${BOOTCFG_FUNCDEF} CreateObject
!macro BOOTCFG_CreateObject_Call UN SERVICES BASEBCDSTORE BCDSTORE \
  TYPE GUID OBJ ERR
  Push ${SERVICES}
  Push ${BASEBCDSTORE}
  Push ${BCDSTORE}
  Push ${TYPE}
  Push ${GUID}
  Call `${UN}${BOOTCFG_PREFIX}CreateObject`
  Pop ${ERR}
  Pop ${OBJ}
!macroend

; ${BOOTCFG_DeleteObject} services basebcdstore bcdstore guid result err
${BOOTCFG_FUNCDEF} DeleteObject
!macro BOOTCFG_DeleteObject_Call UN SERVICES BASEBCDSTORE BCDSTORE GUID \
  RESULT ERR
  Push ${SERVICES}
  Push ${BASEBCDSTORE}
  Push ${BCDSTORE}
  Push ${GUID}
  Call `${UN}${BOOTCFG_PREFIX}DeleteObject`
  Pop ${ERR}
  Pop ${RESULT}
!macroend

; ${BOOTCFG_SetStringElement} services basebcdobject bcdobject type string \
;   result err
${BOOTCFG_FUNCDEF} SetStringElement
!macro BOOTCFG_SetStringElement_Call UN SERVICES BASEBCDOBJECT BCDOBJECT \
  TYPE STRING RESULT ERR
  Push ${SERVICES}
  Push ${BASEBCDOBJECT}
  Push ${BCDOBJECT}
  Push ${TYPE}
  Push "${STRING}"
  Call `${UN}${BOOTCFG_PREFIX}SetStringElement`
  Pop ${ERR}
  Pop ${RESULT}
!macroend

; ${BOOTCFG_SetDescription} services basebcdobject bcdobject string \
;   result err
${BOOTCFG_FUNCDEF} SetDescription
!macro BOOTCFG_SetDescription_Call UN SERVICES BASEBCDOBJECT BCDOBJECT \
  STRING RESULT ERR
  Push ${SERVICES}
  Push ${BASEBCDOBJECT}
  Push ${BCDOBJECT}
  Push ${BOOTCFG_BCDE_DESCRIPTION}
  Push "${STRING}"
  Call `${UN}${BOOTCFG_PREFIX}SetStringElement`
  Pop ${ERR}
  Pop ${RESULT}
!macroend
!macro BOOTCFG_SetDescription UN
${BOOTCFG_FUNCINC} "${UN}" SetStringElement
!macroend

; ${BOOTCFG_SetPath} services basebcdobject bcdobject path \
;   result err
${BOOTCFG_FUNCDEF} SetPath
!macro BOOTCFG_SetPath_Call UN SERVICES BASEBCDOBJECT BCDOBJECT \
  PATH RESULT ERR
  Push ${SERVICES}
  Push ${BASEBCDOBJECT}
  Push ${BCDOBJECT}
  Push ${BOOTCFG_BCDE_PATH}
  Push "${PATH}"
  Call `${UN}${BOOTCFG_PREFIX}SetStringElement`
  Pop ${ERR}
  Pop ${RESULT}
!macroend
!macro BOOTCFG_SetPath UN
${BOOTCFG_FUNCINC} "${UN}" SetStringElement
!macroend

; ${BOOTCFG_SetPartition} services basebcdobject bcdobject path \
;   result err
${BOOTCFG_FUNCDEF} SetPartition
!macro BOOTCFG_SetPartition_Call UN SERVICES BASEBCDOBJECT BCDOBJECT \
  PARTITION RESULT ERR
  Push ${SERVICES}
  Push ${BASEBCDOBJECT}
  Push ${BCDOBJECT}
  Push "${PARTITION}"
  Call `${UN}${BOOTCFG_PREFIX}SetPartition`
  Pop ${ERR}
  Pop ${RESULT}
!macroend

; ${BOOTCFG_SetObjectList} services basebcdobject bcdobject \
;   type objlist result err
${BOOTCFG_FUNCDEF} SetObjectList
!macro BOOTCFG_SetObjectList_Call UN SERVICES BASEBCDOBJECT \
  BCDOBJECT TYPE OBJLIST RESULT ERR
  Push ${SERVICES}
  Push ${BASEBCDOBJECT}
  Push ${BCDOBJECT}
  Push ${TYPE}
  Push ${OBJLIST}
  Call `${UN}${BOOTCFG_PREFIX}SetObjectList`
  Pop ${ERR}
  Pop ${RESULT}
!macroend

; ${BOOTCFG_AppendBootObject} services basebcdobject \
;   guid result err
${BOOTCFG_FUNCDEF} AppendBootObject
!macro BOOTCFG_AppendBootObject_Call UN SERVICES BASEBCDOBJECT \
  GUID RESULT ERR
  Push ${SERVICES}
  Push ${BASEBCDOBJECT}
  Push ${RESULT}
  Push ${GUID}
  Call `${UN}${BOOTCFG_PREFIX}AppendBootObject`
  Pop ${ERR}
  Pop ${RESULT}
!macroend

; ${BOOTCFG_AppendBootEntry} services basebcdstore bcdstore \
;   guid result err
${BOOTCFG_FUNCDEF} AppendBootEntry
!macro BOOTCFG_AppendBootEntry_Call UN SERVICES BASEBCDSTORE \
  BCDSTORE BASEBCDOBJECT GUID RESULT ERR
  !insertmacro ${BOOTCFG_PREFIX}GetBcdObject_Call "${UN}" \
    ${SERVICES} ${BASEBCDSTORE} ${BCDSTORE} ${BOOTCFG_BOOTMGR_GUID} \
    ${RESULT} ${ERR}
  ${If} ${ERR} == 0
    !insertmacro ${BOOTCFG_PREFIX}AppendBootObject_Call "${UN}" \
      ${SERVICES} ${BASEBCDOBJECT} ${GUID} ${RESULT} ${ERR}
  ${EndIf}
!macroend
!macro BOOTCFG_AppendBootEntry UN
  ${BOOTCFG_FUNCINC} "${UN}" GetBcdObject
  ${BOOTCFG_FUNCINC} "${UN}" AppendBootObject
!macroend

; ${BOOTCFG_AppendFwBootEntry} services basebcdstore bcdstore \
;   guid result err
${BOOTCFG_FUNCDEF} AppendFwBootEntry
!macro BOOTCFG_AppendFwBootEntry UN
  ${BOOTCFG_FUNCINC} "${UN}" GetBcdObject
  ${BOOTCFG_FUNCINC} "${UN}" AppendBootObject
!macroend
!macro BOOTCFG_AppendFwBootEntry_Call UN SERVICES BASEBCDSTORE \
  BCDSTORE BASEBCDOBJECT GUID RESULT ERR
  !insertmacro ${BOOTCFG_PREFIX}GetBcdObject_Call "${UN}" \
    ${SERVICES} ${BASEBCDSTORE} ${BCDSTORE} ${BOOTCFG_FWBOOTMGR_GUID} \
    ${RESULT} ${ERR}
  ${If} ${ERR} == 0
    !insertmacro ${BOOTCFG_PREFIX}AppendBootObject_Call "${UN}" \
      ${SERVICES} ${BASEBCDOBJECT} ${GUID} ${RESULT} ${ERR}
  ${EndIf}
!macroend

; ${BOOTCFG_RemoveBootEntry} services basebcdstore bcdstore \
;   guid result err
${BOOTCFG_FUNCDEF} RemoveBootEntry
!macro BOOTCFG_RemoveBootEntry_Call UN SERVICES BASEBCDSTORE \
  BCDSTORE BASEBCDOBJECT GUID RESULT ERR
  Push ${SERVICES}
  Push ${BASEBCDSTORE}
  Push ${BCDSTORE}
  Push ${BASEBCDOBJECT}
  Push ${GUID}
  Call `${UN}${BOOTCFG_PREFIX}RemoveBootEntry`
  Pop ${ERR}
  Pop ${RESULT}
!macroend

; ${BOOTCFG_SetActiveBootObject} services basebcdobject \
;   guid result err
${BOOTCFG_FUNCDEF} SetActiveBootObject
!macro BOOTCFG_SetActiveBootObject_Call UN SERVICES BASEBCDOBJECT \
  GUID RESULT ERR
  Push ${SERVICES}
  Push ${BASEBCDOBJECT}
  Push ${RESULT}
  Push ${GUID}
  Call `${UN}${BOOTCFG_PREFIX}SetActiveBootObject`
  Pop ${ERR}
  Pop ${RESULT}
!macroend

; ${BOOTCFG_SetActiveBootEntry} services basebcdstore bcdstore \
;   guid result err
${BOOTCFG_FUNCDEF} SetActiveBootEntry
!macro BOOTCFG_SetActiveBootEntry_Call UN SERVICES BASEBCDSTORE \
  BCDSTORE BASEBCDOBJECT GUID RESULT ERR
  !insertmacro ${BOOTCFG_PREFIX}GetBcdObject_Call "${UN}" \
    ${SERVICES} ${BASEBCDSTORE} ${BCDSTORE} ${BOOTCFG_BOOTMGR_GUID} \
    ${RESULT} ${ERR}
  ${If} ${ERR} == 0
    !insertmacro ${BOOTCFG_PREFIX}SetActiveBootObject_Call "${UN}" \
      ${SERVICES} ${BASEBCDOBJECT} ${GUID} ${RESULT} ${ERR}
  ${EndIf}
!macroend
!macro BOOTCFG_SetActiveBootEntry UN
  ${BOOTCFG_FUNCINC} "${UN}" GetBcdObject
  ${BOOTCFG_FUNCINC} "${UN}" SetActiveBootObject
!macroend

; ${BOOTCFG_SetActiveFwBootEntry} services basebcdstore bcdstore \
;   guid result err
${BOOTCFG_FUNCDEF} SetActiveFwBootEntry
!macro BOOTCFG_SetActiveFwBootEntry_Call UN SERVICES BASEBCDSTORE \
  BCDSTORE BASEBCDOBJECT GUID RESULT ERR
  !insertmacro ${BOOTCFG_PREFIX}GetBcdObject_Call "${UN}" \
    ${SERVICES} ${BASEBCDSTORE} ${BCDSTORE} ${BOOTCFG_FWBOOTMGR_GUID} \
    ${RESULT} ${ERR}
  ${If} ${ERR} == 0
    !insertmacro ${BOOTCFG_PREFIX}SetActiveBootObject_Call "${UN}" \
      ${SERVICES} ${BASEBCDOBJECT} ${GUID} ${RESULT} ${ERR}
  ${EndIf}
!macroend
!macro BOOTCFG_SetActiveFwBootEntry UN
  ${BOOTCFG_FUNCINC} "${UN}" GetBcdObject
  ${BOOTCFG_FUNCINC} "${UN}" SetActiveBootObject
!macroend

; ${BOOTCFG_ReleaseObject} obj
${BOOTCFG_FUNCDEF} ReleaseObject
!macro BOOTCFG_ReleaseObject_Call UN OBJ
  ${IUnknown::Release} ${OBJ} ""
!macroend
!macro BOOTCFG_ReleaseObject UN
  !define `${UN}${BOOTCFG_PREFIX}ReleaseObject_INCLUDED`
!macroend

!macro BOOTCFG_AllocEmptyVariant UN
; Allocate empty variant
; Caller is responsible to release returned object.
; Return value:
;   variant
${BOOTCFG_FUNCPROLOG} "${UN}" AllocEmptyVariant
  ; VARTYPE (unsigned short) vt, WORD wReserved1,
  ; WORD wReserved2, WORD wReserved3
  ; and the one consuming the most space
  ; struct __tagRECORD { PVOID pvRecord, IRecordInfo *pRecInfo)
  System::Call "*(&i2 ${VT_EMPTY}, &i2 0, &i2 0, &i2 0, p 0, p 0) p.s"
FunctionEnd
!macroend ; BOOTCFG_AllocEmptyVariant

!macro BOOTCFG_AllocI4Variant UN
; Allocate 4-byte signed integer (I4) variant and fill it with provided value
; Parameter:
;    value - integer
; Return value:
;    variant - I4 variant
${BOOTCFG_FUNCPROLOG} "${UN}" AllocI4Variant
  Push $0
  Exch
  Exch $1

  !insertmacro ${BOOTCFG_PREFIX}AllocEmptyVariant_Call "${UN}" $0
  ${If} $0 != 0
    System::Call "*$0(&i2 ${VT_I4}, &i2, &i2, &i2, &i4 r1)"
  ${EndIf}

  Pop $1
  Exch $0
FunctionEnd
!macroend ; BOOTCFG_AllocI4Variant

!macro BOOTCFG_AllocStringArrayVariant UN
; Allocate string array variant and fill it with provided array
; Parameter:
;    array - safe array of strings
; Return value:
;    variant - safe array variant
${BOOTCFG_FUNCPROLOG} "${UN}" AllocStringArrayVariant
  Push $0
  Exch
  Exch $1

  !insertmacro ${BOOTCFG_PREFIX}AllocEmptyVariant_Call "${UN}" $0
  ${If} $0 != 0
    System::Call "*$0(&i2 ${VT_ARRAY_BSTR}, &i2, &i2, &i2, p r1)"
  ${EndIf}

  Pop $1
  Exch $0
FunctionEnd
!macroend ; BOOTCFG_AllocStringVariant

!macro BOOTCFG_AllocStringVariant UN
; Allocate string variant and fill it with provided string
; Parameter:
;    string
; Return value:
;    variant - string variant
${BOOTCFG_FUNCPROLOG} "${UN}" AllocStringVariant
  Push $0
  Exch
  Exch $1

  !insertmacro ${BOOTCFG_PREFIX}AllocEmptyVariant_Call "${UN}" $0
  ${If} $0 != 0
    System::Call "oleaut32::SysAllocString(w r1) p.s"
    Pop $1
    ${If} $1 != 0
      System::Call "*$0(&i2 ${VT_BSTR}, &i2, &i2, &i2, p r1)"
    ${Else}
      System::Free $0
      StrCpy $0 0
    ${EndIf}
  ${EndIf}

  Pop $1
  Exch $0
FunctionEnd
!macroend ; BOOTCFG_AllocStringVariant

!macro BOOTCFG_FreeStringVariant UN
; Free string variant
; Parameter:
;    variant - string variant
${BOOTCFG_FUNCPROLOG} "${UN}" FreeStringVariant
  Exch $0

  ${If} $0 != 0
    Push $0
    System::Call "*$0(&i2, &i2, &i2, &i2, p .r0)"
    ${If} $0 != 0
      System::Call "oleaut32::SysFreeString(p r0)"
    ${EndIf}
    Pop $0
    System::Free $0
  ${EndIf}

  Pop $0
FunctionEnd
!macroend ; BOOTCFG_FreeStringVariant

!macro BOOTCFG_GetVariantType UN
; Get variant type
; Parameter:
;   variant
; Return value:
;   type of variant
${BOOTCFG_FUNCPROLOG} "${UN}" GetVariantType
  Push $0
  Exch
  Exch $1
  System::Call "*$1(&i2 .r0)"
  Pop $1
  Exch $0
FunctionEnd
!macroend ; BOOTCFG_GetVariantType

!macro BOOTCFG_GetBoolOfVariant UN
${BOOTCFG_FUNCINC} "${UN}" GetVariantType
; Get boolean value of variant
; Parameter:
;   variant
; Return values:
;   value - boolean value (0 = false) or error message
;   err - error code
${BOOTCFG_FUNCPROLOG} "${UN}" GetBoolOfVariant
  Push $0
  Exch
  Push $1
  Exch
  Exch $2

  ; variant=$2

  !insertmacro ${BOOTCFG_PREFIX}GetVariantType_Call "${UN}" $2 $0
  ${If} $0 != ${VT_BOOL}
    StrCpy $1 "unexpected variant type: $0"
    StrCpy $0 ${ERROR_INVALID_DATA}
  ${Else}
    System::Call "*$2(&i2, &i2, &i2, &i2, i .r1)"
    StrCpy $0 0
  ${EndIf}

  Pop $2
  Exch $1
  Exch
  Exch $0
FunctionEnd
!macroend ; BOOTCFG_GetBoolOfVariant

!macro BOOTCFG_GetI4OfVariant UN
${BOOTCFG_FUNCINC} "${UN}" GetVariantType
; Get double word (I4) value of variant
; Parameter:
;   variant
; Return values:
;   value - double word integer or error message
;   err - error code
${BOOTCFG_FUNCPROLOG} "${UN}" GetI4OfVariant
  Push $0
  Exch
  Push $1
  Exch
  Exch $2

  ; variant=$2

  !insertmacro ${BOOTCFG_PREFIX}GetVariantType_Call "${UN}" $2 $0
  ${If} $0 != ${VT_I4}
    StrCpy $1 "unexpected variant type: $0"
    StrCpy $0 ${ERROR_INVALID_DATA}
  ${Else}
    System::Call "*$2(&i2, &i2, &i2, &i2, i .r1)"
    StrCpy $0 0
  ${EndIf}

  Pop $2
  Exch $1
  Exch
  Exch $0
FunctionEnd
!macroend ; BOOTCFG_GetI4OfVariant

!macro BOOTCFG_GetObjectOfVariant UN
${BOOTCFG_FUNCINC} "${UN}" GetVariantType
; Get object of variant
; Parameter:
;   variant
; Return values:
;   obj - object or error message
;   err - error code
${BOOTCFG_FUNCPROLOG} "${UN}" GetObjectOfVariant
  Push $0
  Exch
  Push $1
  Exch
  Exch $2

  ; variant=$2

  !insertmacro ${BOOTCFG_PREFIX}GetVariantType_Call "${UN}" $2 $0
  ${If} $0 != ${VT_UNKNOWN}
    StrCpy $1 "unexpected variant type: $0"
    StrCpy $0 ${ERROR_INVALID_DATA}
  ${Else}
    System::Call "*$2(&i2, &i2, &i2, &i2, p .r1)"
    StrCpy $0 0
  ${EndIf}

  Pop $2
  Exch $1
  Exch
  Exch $0
FunctionEnd
!macroend ; BOOTCFG_GetObjectOfVariant

!macro BOOTCFG_GetStringOfVariant UN
${BOOTCFG_FUNCINC} "${UN}" GetVariantType
; Get string of variant
; Parameter:
;   variant
; Return values:
;   string - string or error message
;   err - error code
${BOOTCFG_FUNCPROLOG} "${UN}" GetStringOfVariant
  Push $0
  Exch
  Push $1
  Exch
  Exch $2

  ; variant=$2

  !insertmacro ${BOOTCFG_PREFIX}GetVariantType_Call "${UN}" $2 $0
  ${If} $0 != ${VT_BSTR}
    StrCpy $1 "unexpected variant type: $0"
    StrCpy $0 ${ERROR_INVALID_DATA}
  ${Else}
    System::Call "*$2(&i2, &i2, &i2, &i2, w .r1)"
    StrCpy $0 0
  ${EndIf}

  Pop $2
  Exch $1
  Exch
  Exch $0
FunctionEnd
!macroend ; BOOTCFG_GetStringOfVariant

!macro BOOTCFG_GetStringArrayOfVariant UN
${BOOTCFG_FUNCINC} "${UN}" GetVariantType
; Get string array of variant
; Parameter:
;   variant
; Return values:
;   array - string array or error message
;   err - error code
${BOOTCFG_FUNCPROLOG} "${UN}" GetStringArrayOfVariant
  Push $0
  Exch
  Push $1
  Exch
  Exch $2

  ; variant=$2

  !insertmacro ${BOOTCFG_PREFIX}GetVariantType_Call "${UN}" $2 $0
  ${If} $0 != ${VT_ARRAY_BSTR}
    StrCpy $1 "unexpected variant type: $0"
    StrCpy $0 ${ERROR_INVALID_DATA}
  ${Else}
    System::Call "*$2(&i2, &i2, &i2, &i2, p .r1)"
    StrCpy $0 0
  ${EndIf}

  Pop $2
  Exch $1
  Exch
  Exch $0
FunctionEnd
!macroend ; BOOTCFG_GetStringArrayOfVariant

!macro BOOTCFG_IsUEFI UN
; Check for UEFI firmware
; Return value:
;   1 - UEFI
;   0 - BIOS
${BOOTCFG_FUNCPROLOG} "${UN}" IsUEFI
  Push $0
  Push $1
  Push $2

  ; Initialize return value
  StrCpy $0 0
  ; GetFirmwareType is available in Windows 8 or a later version of Windows
  System::Call "kernel32::GetFirmwareType(*i 0 r2) i .r1"
  ${If} $1 != "error"
  ${AndIf} $1 != 0
    ${If} $2 == ${FIRMWARE_TYPE_UEFI}
      StrCpy $0 1
    ${EndIf}
  ${Else}
    ; Fall back to GetFirmwareEnvironmentVariable API function
    System::Call "kernel32::GetFirmwareEnvironmentVariable(t '', \
      t '{00000000-0000-0000-0000-000000000000}', p 0, i 0) i .r1 ? e"
    Pop $2
    ${If} $1 != "error"
    ${AndIf} $2 == ${ERROR_NO_ACCESS}
      StrCpy $0 1
    ${EndIf}
  ${EndIf}

  Pop $2
  Pop $1
  Exch $0
FunctionEnd
!macroend ; BOOTCFG_IsUEFI

!macro BOOTCFG_GetObject UN
; Get object of provided class name
; Caller is responsible to release returned object.
; Parameters:
;   services - IWbemServices object
;   name - class name
; Return values:
;   obj - object or error message
;   err - error code
${BOOTCFG_FUNCPROLOG} "${UN}" GetObject
  Push $0
  Exch 2
  Push $1
  Exch 2
  Exch $2
  Exch
  Exch $3

  ; name=$2, services=$3

  ; IWbemServices::GetObject(objectpath, flags, context, object, result)
  ${IWbemServices::GetObject} $3 "(r2,0,0,0r1,0).r0"
  ${If} $0 != 0
     StrCpy $1 "GetObject $2 failed"
  ${EndIf}

  Pop $3
  Pop $2
  Exch $1
  Exch
  Exch $0
FunctionEnd
!macroend ; BOOTCFG_GetObject

!macro BOOTCFG_GetObjectPropertyValue UN
${BOOTCFG_FUNCINC} "${UN}" AllocEmptyVariant
; Get property value of provided object
; Caller is responsible to free returned variant.
; Parameters:
;   object - IWbemClassObject
;   property - name of property
; Return values:
;   value - allocated property value as a variant
;   err - error code
${BOOTCFG_FUNCPROLOG} "${UN}" GetObjectPropertyValue
  Push $0
  Exch 2
  Push $1
  Exch 2
  Exch $2
  Exch
  Exch $3

  ; property=$2, object=$3

  !insertmacro ${BOOTCFG_PREFIX}AllocEmptyVariant_Call "${UN}" $1
  ; variant=$1

  ${If} $1 != 0
    ; IWbemClassObject::Get(property, flags, variant, vtType, flavor)
    ${IWbemClassObject::Get} $3 "(r2,0,r1,0,0).r0"
    ${If} $0 != 0
      System::Free $1
      StrCpy $1 "Unable to get property $2"
    ${EndIf}
  ${Else}
    StrCpy $1 "Failed to allocate variant for property $2"
    StrCpy $0 ${ERROR_NOT_ENOUGH_MEMORY}
  ${EndIf}

  Pop $3
  Pop $2
  Exch $1
  Exch
  Exch $0
FunctionEnd
!macroend ; BOOTCFG_GetObjectPropertyValue

!macro BOOTCFG_GetMethod UN
; Prepare method
; Parameters:
;   baseobject - base class object
;   method - name of method
; Return values:
;   inparams - inparams object respectively an error message
;   err - error code
${BOOTCFG_FUNCPROLOG} "${UN}" GetMethod
  Push $0
  Exch 2
  Push $1
  Exch 2
  Exch $2
  Exch
  Exch $3

  ; method=$2, baseobject=$3

  ; IWbemClassObject::GetMethod(method, flags, inparams, outparams)
  ${IWbemClassObject::GetMethod} $3 "(r2,0,0r1,0).r0"
  ${If} $0 != 0
    StrCpy $1 "GetMethod $3 failed"
  ${EndIf}

  Pop $3
  Pop $2
  Exch $1
  Exch
  Exch $0
FunctionEnd
!macroend ; BOOTCFG_GetMethod

!macro BOOTCFG_SpawnInstance UN
; Spawn instance
; Parameter:
;   inparams - inparams object
; Return values:
;   instance - instance object
;   err - error code
${BOOTCFG_FUNCPROLOG} "${UN}" SpawnInstance
  Push $0
  Exch
  Push $1
  Exch
  Exch $2

  ; inparams=$2

  ; IWbemClassObject::SpawnInstance(flags, classinstance)
  ; inparmasinstance=$1
  ${IWbemClassObject::SpawnInstance} $2 "(0,0r1).r0"

  Pop $2
  Exch $1
  Exch
  Exch $0
FunctionEnd
!macroend ; BOOTCFG_SpawnInstance

!macro BOOTCFG_PrepareMethod UN
${BOOTCFG_FUNCINC} "${UN}" GetMethod
${BOOTCFG_FUNCINC} "${UN}" SpawnInstance
; Prepare method
; Parameters:
;   baseobject - base class object
;   method - name of method
; Return values:
;   instance - instance object
;   err - error code
${BOOTCFG_FUNCPROLOG} "${UN}" PrepareMethod
  Push $0
  Exch 2
  Push $1
  Exch 2
  Exch $2
  Exch
  Exch $3

  ; method=$2, baseobject=$3

  !insertmacro ${BOOTCFG_PREFIX}GetMethod_Call "${UN}" $3 $2 $1 $0
  ${If} $0 == 0
    !insertmacro ${BOOTCFG_PREFIX}SpawnInstance_Call "${UN}" $1 $3 $0
    ; Release inparams
    !insertmacro ${BOOTCFG_PREFIX}ReleaseObject_Call "${UN}" $1
    ${If} $0 != 0
      StrCpy $1 "SpawnInstance $2 failed"
    ${Else}
      StrCpy $1 $3
    ${EndIf}
  ${EndIf}

  Pop $3
  Pop $2
  Exch $1
  Exch
  Exch $0
FunctionEnd
!macroend ; BOOTCFG_PrepareMethod

!macro BOOTCFG_Put UN
; Fill instance object of input parameters with key/value pair
; Parameters:
;   instance - instance object of input parameters
;   key - name of input parameter
;   value - input parameter variant value
; Return value:
;   err - error code
${BOOTCFG_FUNCPROLOG} "${UN}" Put
  Push $0
  Exch 3
  Exch $1
  Exch 2
  Exch $2
  Exch
  Exch $3

  ; instance=$1, param=$2, varvalue=$3
  ; IWbemClassObject::Put(param, flags, valuevariant, vtType)
  ${IWbemClassObject::Put} $1 "(r2,0,r3,0).r0"

  Pop $3
  Pop $2
  Pop $1
  Exch $0
FunctionEnd
!macroend ; BOOTCFG_Put

!macro BOOTCFG_ExecMethod UN
${BOOTCFG_FUNCINC} "${UN}" GetObjectPropertyValue
${BOOTCFG_FUNCINC} "${UN}" GetBoolOfVariant
; Execute method
; Parameters:
;   services - IWbemServices object
;   obj - object to perform method
;   instance - instance object of input parameters
;   method - name of method
; Return values:
;   result object
;   err - error code
${BOOTCFG_FUNCPROLOG} "${UN}" ExecMethod
  Push $0
  Exch 4
  Push $1
  Exch 4
  Exch $2
  Exch 3
  Exch $3
  Exch 2
  Exch $4
  Exch
  Exch $5
  Push $6

  ; obj=$2, instance=$3, method=$4, services=$5

  StrCpy $6 "__RELPATH"
  !insertmacro ${BOOTCFG_PREFIX}GetObjectPropertyValue_Call "${UN}" \
    $2 $6 $1 $0
  ${If} $0 == 0
    ; Extract path from variant structure
    System::Call "*$1(&i2 .r0)"
    IntOp $0 $0 - ${VT_BSTR}
    ${If} $0 != 0
      IntOp $0 $0 + ${VT_BSTR}
      StrCpy $1 "Get $6 for method $4 returned unexpected variant \
        type: $0"
      StrCpy $0 ${ERROR_INVALID_DATA}
    ${Else}
      System::Call "*$1(&i2, &i2, &i2, &i2, w .r6)"
    ${EndIf}
    ; Free variant
    System::Free $1
    ${If} $0 == 0
      ; IWbemServices::ExecMethod(objectpath, method, flags, context
      ;   inparamsinstance, outparams, result)
      ${IWbemServices::ExecMethod} $5 "(r6,r4,0,0,r3,0r1,0).r0"
      ${If} $0 != 0
        StrCpy $1 "ExecMethod $4 failed"
      ${Else}
        StrCpy $6 "ReturnValue"
        !insertmacro ${BOOTCFG_PREFIX}GetObjectPropertyValue_Call \
          "${UN}" $1 $6 $5 $0
        ${If} $0 == 0
          ; Push variant onto stack
          Push $5
          !insertmacro ${BOOTCFG_PREFIX}GetBoolOfVariant_Call "${UN}" \
            $5 $5 $0
          Exch $5
          ; Free variant
          System::Free $5
          Pop $5
          ${If} $0 != 0
            StrCpy $5 "Get $6 for method $4 returned $5"
          ${ElseIf} $5 == 0
            StrCpy $5 "Get $6 for method $4 returned unexpected \
              value: $5"
            StrCpy $0 ${ERROR_NO_MORE_ITEMS}
          ${EndIf}
        ${EndIf}
        ${If} $0 != 0
          ; If an error occurred then release outparams
          !insertmacro ${BOOTCFG_PREFIX}ReleaseObject_Call "${UN}" $1
          StrCpy $1 $5
        ${EndIf}
      ${EndIf}
    ${EndIf}
  ${EndIf}

  Pop $6
  Pop $5
  Pop $4
  Pop $3
  Pop $2
  Exch $1
  Exch
  Exch $0
FunctionEnd
!macroend ; BOOTCFG_ExecMethod

!macro BOOTCFG_CallMethod UN
${BOOTCFG_FUNCINC} "${UN}" GetMethod
${BOOTCFG_FUNCINC} "${UN}" SpawnInstance
${BOOTCFG_FUNCINC} "${UN}" Put
${BOOTCFG_FUNCINC} "${UN}" ExecMethod
; Call method of WMI object with provided input parameters and
; return output parameters in result object
; Caller is responsible to release returned result object.
; Parameters:
;   services - IWbemServices object
;   baseobject - base class object
;   obj - object to perform method
;   method - name of method
;   key - name of input parameter
;   value - input parameter variant value
; Return values:
;   result object
;   err - error code
${BOOTCFG_FUNCPROLOG} "${UN}" CallMethod
  Push $0
  Exch 6
  Push $1
  Exch 6
  Exch $2
  Exch 5
  Exch $3
  Exch 4
  Exch $4
  Exch 3
  Exch $5
  Exch 2
  Exch $6
  Exch
  Exch $7
  Push $8

  ; baseobject=$2, obj=$3, method=$4, key=$5, value=$6, services=$7

  ; GetMethod(baseobject, method)
  !insertmacro ${BOOTCFG_PREFIX}GetMethod_Call "${UN}" $2 $4 $8 $0
  ${If} $0 != 0
    StrCpy $1 $8
  ${Else}
    ; Copy inparams to $2
    StrCpy $2 $8
    ${If} $5 != ""
      ; Handle input parameters
      ; SpawnInstance(inparams)
      !insertmacro ${BOOTCFG_PREFIX}SpawnInstance_Call "${UN}" \
        $2 $8 $0
      ; instance=$8
      ${If} $0 != 0
        StrCpy $1 "SpawnInstance $4 failed"
      ${Else}
        ; Put(instance, key, value)
        !insertmacro ${BOOTCFG_PREFIX}Put_Call "${UN}" \
          $8 $5 $6 $0
        ${If} $0 != 0
          StrCpy $1 "Put $4 failed for $5"
        ${EndIf}
      ${EndIf}
    ${Else}
      StrCpy $8 0
    ${EndIf}
    ; Release inparams
    !insertmacro ${BOOTCFG_PREFIX}ReleaseObject_Call "${UN}" $2
    ${If} $0 == 0
      ; ExecMethod(services, obj, instance, method)
      !insertmacro ${BOOTCFG_PREFIX}ExecMethod_Call "${UN}" \
        $7 $3 $8 $4 $1 $0
    ${EndIf}
    ${If} $8 != 0
      ; If input parameters were provided then release inparams instance
      !insertmacro ${BOOTCFG_PREFIX}ReleaseObject_Call "${UN}" $8
    ${EndIf}
  ${EndIf}

  Pop $8
  Pop $7
  Pop $6
  Pop $5
  Pop $4
  Pop $3
  Pop $2
  Exch $1
  Exch
  Exch $0
FunctionEnd
!macroend ; BOOTCFG_CallMethod

!macro BOOTCFG_OpenBcdStore UN
${BOOTCFG_FUNCINC} "${UN}" AllocStringVariant
${BOOTCFG_FUNCINC} "${UN}" FreeStringVariant
${BOOTCFG_FUNCINC} "${UN}" CallMethod
${BOOTCFG_FUNCINC} "${UN}" GetObjectPropertyValue
${BOOTCFG_FUNCINC} "${UN}" GetObjectOfVariant
; Open Bcd store
; Parameters:
;   services - IWbemServices object
;   baseobject - BcdStore base class object
;   filename - complete path to BCD store file
; Return values:
;   obj - BcdStore object respectively an error message
;   err - error code
${BOOTCFG_FUNCPROLOG} "${UN}" OpenBcdStore
  Push $0
  Exch 3
  Push $1
  Exch 3
  Exch $2
  Exch 2
  Exch $3
  Exch
  Exch $4
  Push $5

  ; bcdstore=$2, filename=$3, services=$4

  StrCpy $5 "OpenStore"
  !insertmacro ${BOOTCFG_PREFIX}AllocStringVariant_Call "${UN}" $3 $3

  ${If} $3 != 0
    !insertmacro ${BOOTCFG_PREFIX}CallMethod_Call "${UN}" \
      $4 $2 $2 $5 "File" $3 $1 $0
    !insertmacro ${BOOTCFG_PREFIX}FreeStringVariant_Call "${UN}" $3
    ${If} $0 == 0
      StrCpy $3 "Store"
      !insertmacro ${BOOTCFG_PREFIX}GetObjectPropertyValue_Call "${UN}" \
        $1 $3 $2 $0
      !insertmacro ${BOOTCFG_PREFIX}ReleaseObject_Call "${UN}" $1
      ${If} $0 != 0
        StrCpy $1 $2
      ${Else}
        ; Get BcdStore object from variant structure
        !insertmacro ${BOOTCFG_PREFIX}GetObjectOfVariant_Call "${UN}" \
           $2 $1 $0
        System::Free $2
        ${If} $0 != 0
          StrCpy $1 "Get $3 for method $5 returned $1"
        ${EndIf}
      ${EndIf}
    ${EndIf}
  ${Else}
    StrCpy $1 "Failed to allocate variant for method $5"
    StrCpy $0 ${ERROR_NOT_ENOUGH_MEMORY}
  ${EndIf}

  Pop $5
  Pop $4
  Pop $3
  Pop $2
  Exch $1
  Exch
  Exch $0
FunctionEnd
!macroend ; BOOTCFG_OpenBcdStore

!macro BOOTCFG_GetBcdObject UN
${BOOTCFG_FUNCINC} "${UN}" AllocStringVariant
${BOOTCFG_FUNCINC} "${UN}" FreeStringVariant
${BOOTCFG_FUNCINC} "${UN}" CallMethod
${BOOTCFG_FUNCINC} "${UN}" GetObjectPropertyValue
${BOOTCFG_FUNCINC} "${UN}" GetObjectOfVariant
; Open default Bcd store
; Get BcdObject given id from BCD store
; Parameters:
;   services - IWbemServices
;   baseobject - BcdStore base class object
;   bcdstore - BcdStore object
;   id - BcdObject identifier
; Return values:
;   obj - BcdObject respectively an error message
;   err - error code
${BOOTCFG_FUNCPROLOG} "${UN}" GetBcdObject
  ; Handle parameters and save registers
  Push $0
  Exch 4
  Push $1
  Exch 4
  Exch $2
  Exch 3
  Exch $3
  Exch 2
  Exch $4
  Exch
  Exch $5
  Push $6

  ; baseobject=$2, bcdstore=$3, id=$4, services=$5

  StrCpy $6 "OpenObject"
  !insertmacro ${BOOTCFG_PREFIX}AllocStringVariant_Call "${UN}" $4 $4

  ${If} $4 != 0
    ; CallMethod(services, baseobject, object, method, param, value)
    !insertmacro ${BOOTCFG_PREFIX}CallMethod_Call "${UN}" \
      $5 $2 $3 $6 "Id" $4 $1 $0
    !insertmacro ${BOOTCFG_PREFIX}FreeStringVariant_Call "${UN}" $4
    ${If} $0 == 0
      StrCpy $2 "Object"
      !insertmacro ${BOOTCFG_PREFIX}GetObjectPropertyValue_Call "${UN}" \
        $1 $2 $3 $0
      ; Release outparams
      !insertmacro ${BOOTCFG_PREFIX}ReleaseObject_Call "${UN}" $1
      ${If} $0 != 0
        StrCpy $1 $3
      ${Else}
        ; Get BcdObject from variant structure
        !insertmacro ${BOOTCFG_PREFIX}GetObjectOfVariant_Call "${UN}" \
          $3 $1 $0
        System::Free $3
        ${If} $0 != 0
          StrCpy $1 "Get $2 for method $6 returned $1"
        ${EndIf}
      ${EndIf}
    ${EndIf}
  ${Else}
    StrCpy $1 "Failed to allocate variant for method $6"
    StrCpy $0 ${ERROR_NOT_ENOUGH_MEMORY}
  ${EndIf}

  Pop $6
  Pop $5
  Pop $4
  Pop $3
  Pop $2
  Exch $1
  Exch
  Exch $0
FunctionEnd
!macroend ; BOOTCFG_GetBcdObject

!macro BOOTCFG_GetBcdElement UN
${BOOTCFG_FUNCINC} "${UN}" AllocI4Variant
${BOOTCFG_FUNCINC} "${UN}" CallMethod
${BOOTCFG_FUNCINC} "${UN}" GetObjectPropertyValue
${BOOTCFG_FUNCINC} "${UN}" GetObjectOfVariant
; Get element from BCDObject of given element type
; Parameters:
;   services - IWbemServices object
;   baseobject - BcdObject base class object
;   bcdobject - BcdObject
;   elemtype - element type
; Return values:
;   element - element object respectively an error message
;   err - error code
${BOOTCFG_FUNCPROLOG} "${UN}" GetBcdElement
  Push $0
  Exch 4
  Push $1
  Exch 4
  Exch $2
  Exch 3
  Exch $3
  Exch 2
  Exch $4
  Exch
  Exch $5
  Push $6

  ; baseobject=$2, bcdobject=$3, elemtype=$4, services=$5

  StrCpy $6 "GetElement"
  !insertmacro ${BOOTCFG_PREFIX}AllocI4Variant_Call "${UN}" $4 $4

  ${If} $4 != 0
    !insertmacro ${BOOTCFG_PREFIX}CallMethod_Call "${UN}" \
      $5 $2 $3 $6 "Type" $4 $1 $0
    System::Free $4
    ${If} $0 == 0
      StrCpy $2 "Element"
      !insertmacro ${BOOTCFG_PREFIX}GetObjectPropertyValue_Call "${UN}" \
        $1 $2 $3 $0
      ; Release outparams
      !insertmacro ${BOOTCFG_PREFIX}ReleaseObject_Call "${UN}" $1
      ${If} $0 != 0
        StrCpy $1 $3
      ${Else}
        ; Get BcdObject from variant structure
        !insertmacro ${BOOTCFG_PREFIX}GetObjectOfVariant_Call "${UN}" \
          $3 $1 $0
        System::Free $3
        ${If} $0 != 0
          StrCpy $1 "Get $2 for method $6 returned $1"
        ${EndIf}
      ${EndIf}
    ${EndIf}
  ${Else}
    StrCpy $1 "Failed to allocate variant for method $6"
    StrCpy $0 ${ERROR_NOT_ENOUGH_MEMORY}
  ${EndIf}

  Pop $6
  Pop $5
  Pop $4
  Pop $3
  Pop $2
  Exch $1
  Exch
  Exch $0
FunctionEnd
!macroend ; BOOTCFG_GetBcdElement

!macro BOOTCFG_GetElementFromBcd UN
${BOOTCFG_FUNCINC} "${UN}" GetBcdObject
${BOOTCFG_FUNCINC} "${UN}" GetBcdElement
; Get provided element type of BcdObject of given id from BCD store
; Parameters:
;   services - IWbemServices object
;   basebcdstore - BcdStore base class object
;   bcdstore - BcdStore object
;   basebcdobject - BcdObject base class object
;   id - BcdObject identifier
;   elemtype - element type
; Return values:
;   object - BcdObject
;   element - element object respectively an error message
;   err - error code
${BOOTCFG_FUNCPROLOG} "${UN}" GetElementFromBcd
  Push $0
  Exch 6
  Push $1
  Exch 6
  Push $2
  Exch 6
  Exch $3
  Exch 5
  Exch $4
  Exch 4
  Exch $5
  Exch 3
  Exch $6
  Exch 2
  Exch $7
  Exch
  Exch $8

  ; bcdstore=$3, basebcdobject=$4, id=$5, elemtyp=$6, services=$7,
  ; basebcdstore=$8

  !insertmacro ${BOOTCFG_PREFIX}GetBcdObject_Call "${UN}" \
    $7 $8 $3 $5 $2 $0
  ${If} $0 != 0
    StrCpy $1 $2
    StrCpy $2 ""
  ${Else}
    !insertmacro ${BOOTCFG_PREFIX}GetBcdElement_Call "${UN}" \
      $7 $4 $2 $6 $1 $0
    ${If} $0 != 0
      ; Release bcdobject
      !insertmacro ${BOOTCFG_PREFIX}ReleaseObject_Call "${UN}" $2
      StrCpy $2 ""
    ${EndIf}
  ${EndIf}

  Pop $8
  Pop $7
  Pop $6
  Pop $5
  Pop $4
  Pop $3
  Exch $2
  Exch 2
  Exch
  Exch $1
  Exch
  Exch $0
FunctionEnd
!macroend ; BOOTCFG_GetElementFromBcd

!macro BOOTCFG_GetESPVolume UN
${BOOTCFG_FUNCINC} "${UN}" GetElementFromBcd
${BOOTCFG_FUNCINC} "${UN}" GetObjectPropertyValue
${BOOTCFG_FUNCINC} "${UN}" GetObjectOfVariant
${BOOTCFG_FUNCINC} "${UN}" GetI4OfVariant
${BOOTCFG_FUNCINC} "${UN}" GetStringOfVariant
; Get volume identifier of EFI system partition
; Parameters:
;   services - IWbemServices object
;   basebcdstore - BcdStore base class object
;   bcdstore - BcdStore object
;   basebcdobject - BcdObject base class object
; Return values:
;   volume - volume identifier of EFI system partition or error message
;   err - error code
${BOOTCFG_FUNCPROLOG} "${UN}" GetESPVolume
  System::Store 'S'
  Pop $5 ; basebcdobject=$5
  Pop $4 ; bcdstore=$4
  Pop $3 ; basebcdstore=$3
  Pop $2 ; services=$2

  !insertmacro ${BOOTCFG_PREFIX}GetElementFromBcd_Call "${UN}" \
    $2 $3 $4 $5 ${BOOTCFG_BOOTMGR_GUID} ${BOOTCFG_BCDE_DEVICE} $6 $7 $0
  ; bcdobject=$6, element=$7
  ${If} $0 != 0
    StrCpy $1 $7
  ${Else}
    ; Release bcdobject
    !insertmacro ${BOOTCFG_PREFIX}ReleaseObject_Call "${UN}" $6
    ; Save element
    Push $7
    !insertmacro ${BOOTCFG_PREFIX}GetObjectPropertyValue_Call "${UN}" $7 "Device" $6 $0
    ${If} $0 != 0
      StrCpy $1 $6
    ${Else}
      ; Save device object
      Push $6
      !insertmacro ${BOOTCFG_PREFIX}GetObjectOfVariant_Call "${UN}" $6 $7 $0
      ${If} $0 != 0
        StrCpy $1 $7
      ${Else}
        !insertmacro ${BOOTCFG_PREFIX}GetObjectPropertyValue_Call "${UN}" $7 "DeviceType" $6 $0
        ${If} $0 != 0
          StrCpy $1 $6
        ${Else}
          ; Save device type variant
          Push $6
          !insertmacro ${BOOTCFG_PREFIX}GetI4OfVariant_Call "${UN}" $6 $1 $0
          ${If} $0 == 0
            ${If} $1 != ${BOOTCFG_PARTIONDEVICE_TYPE}
              StrCpy $1 "unexpected device type: $1"
            ${Else}
              !insertmacro ${BOOTCFG_PREFIX}GetObjectPropertyValue_Call "${UN}" $7 "Path" $6 $0
              ${If} $0 != 0
                StrCpy $1 $6
              ${Else}
                !insertmacro ${BOOTCFG_PREFIX}GetStringOfVariant_Call "${UN}" $6 $1 $0
                System::Free $6
              ${EndIf}
            ${EndIf}
          ${EndIf}
          ; Restore and free device type variant
          Pop $6
          System::Free $6
        ${EndIf}
      ${EndIf}
      ; Restore and free device object
      Pop $6
      System::Free $6
    ${EndIf}
    ; Restore and release element
    Pop $7
    !insertmacro ${BOOTCFG_PREFIX}ReleaseObject_Call "${UN}" $7
  ${EndIf}

  Push $1
  Push $0
  System::Store 'L'
FunctionEnd
!macroend ; BOOOTCFG_GetESPVolume

!macro BOOTCFG_GetFileSystemUUID UN
; Get universally unique identifer (UUID) of filesystem
; Parameter:
;   drive - MS-DOS device name of drive, e. g. C:
; Return value:
;   UUID - unique identifer of filesystem
${BOOTCFG_FUNCPROLOG} "${UN}" GetFileSystemUUID
  System::Store 'S'
  Pop $0

  StrCpy $0 "\\.\$0"
  System::Call "kernel32::CreateFile(t r0, i ${GENERIC_READ}, i ${FILE_SHARE_READ}|${FILE_SHARE_WRITE}, p 0, i ${OPEN_EXISTING}, i 0, p 0) p.r1"
  StrCpy $0 ""
  ${If} $1 P<> ${INVALID_HANDLE_VALUE}
    System::Call "kernel32::GetVolumeInformationByHandleW(p r1, p 0, i 0, *i .r2, p 0, p 0, w .r3, i ${NSIS_MAX_STRLEN}) i.r4"
    ${If} $4 != 0
      ${If} $3 == "NTFS"
        ; Get complete NTFS volume serial number (8 bytes)
        System::Alloc ${BOOTCFG_VOLUME_BLOCK_RECORD_SIZE}
        Pop $2
        ${If} $2 P<> 0
          ; The NTFS file system driver is sensitive about the
          ; buffer size. ReadFile is going to fail with
          ; ERROR_INVALID_PARAMETER (87 = 0x57) in case of reading
          ; from the NTFS volume if the buffer size is not a multiple
          ; of 512 bytes.
          System::Call "kernel32::ReadFile(p r1, p r2, i ${BOOTCFG_VOLUME_BLOCK_RECORD_SIZE}, p 0, p 0) i.r3"
          ${If} $3 != 0
            System::Call "*$2(&v${BOOTCFG_VOLUME_SERIAL_NUMBER_OFFSET}, l.r3)"
            System::Int64Op $3 & 0xffffffff
            System::Int64Op $3 >> 32
            Pop $3
            IntFmt $0 "%0x" $3
            Pop $3
            IntFmt $0 "$0%0x" $3
          ${EndIf}
          System::Free $2
        ${EndIf}
      ${Else}
        IntOp $0 $2 & 0xffff
        IntFmt $0 "%04x" $0
        IntOp $2 $2 >> 16
        IntFmt $0 "%04x-$0" $2
      ${EndIf}
    ${EndIf}
    System::Call "kernel32::CloseHandle(p r1)"
  ${EndIf}

  Push $0
  System::Store 'L'
FunctionEnd
!macroend ; BOOTCFG_GetFileSystemUUID

!macro BOOTCFG_GetSystemPartition UN
${BOOTCFG_FUNCINC} "${UN}" GetMethod
${BOOTCFG_FUNCINC} "${UN}" ExecMethod
${BOOTCFG_FUNCINC} "${UN}" GetObjectPropertyValue
${BOOTCFG_FUNCINC} "${UN}" GetStringOfVariant
; Get system partition
; Parameters:
;   services - IWbemServices object
;   basebcdstore - BcdStore base class object
; Return values:
;   partition - path of partition
;   err - error code
${BOOTCFG_FUNCPROLOG} "${UN}" GetSystemPartition
  Push $0
  Exch 2
  Push $1
  Exch 2
  Exch $2
  Exch
  Exch $3
  Push $4
  Push $5

  ; basebcdstore=$2, services=$3
  StrCpy $5 "GetSystemPartition"
  !insertmacro ${BOOTCFG_PREFIX}GetMethod_Call "${UN}" $2 $5 $4 $0
  ; instance=$4
  ${If} $0 != 0
    StrCpy $1 $4
  ${Else}
    !insertmacro ${BOOTCFG_PREFIX}ExecMethod_Call "${UN}" $3 $2 $4 $5 $1 $0
    ; Release instance
    !insertmacro ${BOOTCFG_PREFIX}ReleaseObject_Call "${UN}" $4
    ${If} $0 == 0
      StrCpy $5 "Partition"
      StrCpy $4 $1
      ; result=$4
      !insertmacro ${BOOTCFG_PREFIX}GetObjectPropertyValue_Call "${UN}" \
        $4 $5 $1 $0
      ; Release result
      !insertmacro ${BOOTCFG_PREFIX}ReleaseObject_Call "${UN}" $4
      ${If} $0 == 0
        StrCpy $4 $1
        ; variant=$4
        !insertmacro ${BOOTCFG_PREFIX}GetStringOfVariant_Call "${UN}" \
          $4 $1 $0
        ; Free variant
        System::Free $4
      ${EndIf}
    ${EndIf}
  ${EndIf}

  Pop $5
  Pop $4
  Pop $3
  Pop $2
  Exch $1
  Exch
  Exch $0
FunctionEnd
!macroend ; BOOOTCFG_GetSystemPartition

!macro BOOTCFG_GetVolumeUUID UN
; Get universally unique identifer (UUID) of volume
; Parameter:
;   volume - volume name
; Return values:
;   UUID - unique identifer of volume
;   err - error code
${BOOTCFG_FUNCPROLOG} "${UN}" GetVolumeUUID
  Exch $0
  Push $1
  Push $2
  Push $3

  System::Call "kernel32::CreateFile(t r0, i 0, i ${FILE_SHARE_READ}, p 0, i ${OPEN_EXISTING}, i 0, p 0) p.r2 ?e"
  ${If} $2 P<> ${INVALID_HANDLE_VALUE}
    StrCpy $0 0
    Exch $0
  ${EndIf}
  Pop $0
  ${If} $0 == 0
    System::Alloc ${PARTITION_INFORMATION_EX_SIZEOF}
    Pop $3
    ${If} $3 P<> 0
      System::Call "kernel32::DeviceIoControl(p r2, i ${IOCTL_DISK_GET_PARTITION_INFO_EX}, p 0, i 0, p r3, i ${PARTITION_INFORMATION_EX_SIZEOF}, *i .r1, p 0) i.r0 ?e"
      ${If} $0 != 0
        StrCpy $0 0
        Exch $0
      ${EndIf}
      Pop $0
      ${If} $0 == 0
        System::Call "*$3(&i4 .r1)"
        ${If} $1 == ${PARTITION_STYLE_GPT}
          System::Call "*$3(&i4, &i4, &i8, &i8, &i4, &i4, g, g .r1)"
        ${EndIf}
      ${EndIf}
      System::Free $3
    ${EndIf}
    System::Call "kernel32::CloseHandle(p r2)"
  ${EndIf}

  Pop $3
  Pop $2
  Exch $1
  Exch
  Exch $0
FunctionEnd
!macroend ; BOOOTCFG_GetVolumeUUID

!macro BOOTCFG_ObjectListGetSafeArray UN
${BOOTCFG_FUNCINC} "${UN}" GetStringArrayOfVariant
; Get safe array from object list
; Parameter:
;   objectlist - object list
; Return values:
;   safearray - safe array respectively an error message
;   err - error code
${BOOTCFG_FUNCPROLOG} "${UN}" ObjectListGetSafeArray
  Push $0
  Exch
  Push $1
  Exch
  Exch $2
  Push $3
  Push $4

  ; objectlist=$2

  ; Initialize return value
  StrCpy $0 0
  StrCpy $4 "Ids"
  !insertmacro ${BOOTCFG_PREFIX}GetObjectPropertyValue_Call "${UN}" \
    $2 $4 $3 $0
  ${If} $0 != 0
    StrCpy $1 $3
  ${Else}
    !insertmacro ${BOOTCFG_PREFIX}GetStringArrayOfVariant_Call "${UN}" \
      $3 $1 $0
    System::Free $3
    ${If} $0 != 0
      StrCpy $1 "Get $4 returned $1"
    ${EndIf}
  ${EndIf}

  Pop $4
  Pop $3
  Pop $2
  Exch $1
  Exch
  Exch $0
FunctionEnd
!macroend ; BOOTCFG_ObjectListGetSafeArray

!macro BOOTCFG_GetSizeOfSafeArray UN
; Get size of safe array
; Parameter:
;   safearray - reference to SAFEARRAY
; Return value:
;   size of safe array
${BOOTCFG_FUNCPROLOG} "${UN}" GetSizeOfSafeArray
  Push $0
  Exch
  Exch $1
  Push $2
  Push $3
  Push $4

  ; safearray=$1

  System::Call "oleaut32::SafeArrayGetDim(p r1) i.r2"
  System::Call "oleaut32::SafeArrayGetUBound(p r1, i r2, *i 0 r3) i.r0"
  ${If} $0 != 0
    StrCpy $0 0
  ${Else}
    System::Call "oleaut32::SafeArrayGetLBound(p r1, i r2, *i 0 r4) i.r0"
    ${If} $0 != 0
      StrCpy $0 0
    ${Else}
      IntOp $0 $3 - $4
      IntOp $0 $0 + 1
    ${EndIf}
  ${EndIf}

  Pop $4
  Pop $3
  Pop $2
  Pop $1
  Exch $0
FunctionEnd
!macroend ; BOOTCFG_GetSizeOfSafeArray

!macro BOOTCFG_GetElementDescription UN
${BOOTCFG_FUNCINC} "${UN}" GetObjectPropertyValue
; Get element description from BCDObject
; Parameter:
;   element - element object
; Return value:
;   description - description or "" if it could not be obtained
${BOOTCFG_FUNCPROLOG} "${UN}" GetElementDescription
  Push $0
  Exch
  Exch $1
  Push $2

  ; element=$1
  !insertmacro ${BOOTCFG_PREFIX}GetObjectPropertyValue_Call "${UN}" \
    $1 "String" $2 $0
  ${If} $0 != 0
    StrCpy $0 ""
  ${Else}
    System::Call "*$2(&i2 .r1)"
    IntOp $1 $1 - ${VT_BSTR}
    ${If} $1 != 0
      StrCpy $0 ""
    ${Else}
      System::Call "*$2(&i2, &i2, &i2, &i2, w .r0)"
    ${EndIf}
    System::Free $2
  ${EndIf}

  Pop $2
  Pop $1
  Exch $0
FunctionEnd
!macroend ; BOOTCFG_GetElementDescription

!macro BOOTCFG_GetBcdObjectDescription UN
${BOOTCFG_FUNCINC} "${UN}" GetElementFromBcd
${BOOTCFG_FUNCINC} "${UN}" GetElementDescription
; Get description of BcdObject of given id from BCD store
; Parameters:
;   services - IWbemServices object
;   basebcdstore - BcdStore base class object
;   bcdstore - BcdStore object
;   basebcdobject - BcdObject base class object
;   id - BcdObject identifier
; Return value:
;   description
${BOOTCFG_FUNCPROLOG} "${UN}" GetBcdObjectDescription
  ; Handle parameters and save registers
  Push $0
  Exch 5
  Exch $1
  Exch 4
  Exch $2
  Exch 3
  Exch $3
  Exch 2
  Exch $4
  Exch
  Exch $5

  ; services=$1, basebcdstore=$2, bcdstore=$3, basebcdobject=$4, id=$5

  ; GetElementFromBcd(services, basebcdstore, bcdstore,
  ;   basebcdobject, id, elemtype)
  !insertmacro ${BOOTCFG_PREFIX}GetElementFromBcd_Call "${UN}" \
    $1 $2 $3 $4 $5 ${BOOTCFG_BCDE_DESCRIPTION} $2 $1 $0

  ${If} $0 != 0
   StrCpy $0 ""
  ${Else}
    ; Release bcdobject
    !insertmacro ${BOOTCFG_PREFIX}ReleaseObject_Call "${UN}" $2
    ; GetElementDescription(element)
    !insertmacro ${BOOTCFG_PREFIX}GetElementDescription_Call "${UN}" $1 $0
    ; Release element
    !insertmacro ${BOOTCFG_PREFIX}ReleaseObject_Call "${UN}" $1
  ${EndIf}

  Pop $5
  Pop $4
  Pop $3
  Pop $2
  Pop $1
  Exch $0
FunctionEnd
!macroend ; GetBcdObjectDescription

!macro BOOTCFG_EnumerateBcdObjectList UN
${BOOTCFG_FUNCINC} "${UN}" ObjectListGetSafeArray
${BOOTCFG_FUNCINC} "${UN}" GetBcdObjectDescription
${BOOTCFG_FUNCINC} "${UN}" GetSizeOfSafeArray
; Enumerate BcdObject list
; Parameters:
;   services - IWbemServices object
;   basebcdstore - BcdStore base class object
;   bcdstore - BcdStore object
;   basebcdobject - BcdObject base class object
;   objectlist - object list
; Return value:
;   error message
${BOOTCFG_FUNCPROLOG} "${UN}" EnumerateBcdObjectList
  Push $0
  Exch 5
  Exch $1
  Exch 4
  Exch $2
  Exch 3
  Exch $3
  Exch 2
  Exch $4
  Exch
  Exch $5
  Push $6
  Push $7
  Push $8
  Push $9

  ; services=$1, basebcdstore=$2, bcdstore=$3, basebcdobject=$4,
  ; objectlist=$5

  !insertmacro ${BOOTCFG_PREFIX}ObjectListGetSafeArray_Call "${UN}" \
    $5 $7 $0
  ${If} $0 == 0
    ; Enumerate safe array
    !insertmacro ${BOOTCFG_PREFIX}GetSizeOfSafeArray_Call "${UN}" \
      $7 $6
    System::Call "oleaut32::SafeArrayAccessData(p r7, *p 0 r8) i.r0"
    ${If} $0 != 0
      StrCpy $0 "SafeArrayAccessData failed"
    ${Else}
      ${For} $0 1 $6
        System::Call "*$8(w .r5)"
        !insertmacro ${BOOTCFG_PREFIX}GetBcdObjectDescription_Call \
          "${UN}" $1 $2 $3 $4 $5 $9
!ifndef BOOTCFG_ENUMERATION_CALLBACK
        DetailPrint "$5 [$9]"
!else
        Push $9
        Push $5
        Call ${BOOTCFG_ENUMERATION_CALLBACK}
!endif
        IntOp $8 $8 + ${NSIS_PTR_SIZE}
      ${Next}
      System::Call "oleaut32::SafeArrayUnaccessData(p r7) i.r0"
      ${If} $0 != 0
        StrCpy $0 "SafeArrayUnaccessData failed"
      ${Else}
        StrCpy $0 ""
      ${EndIf}
    ${EndIf}
  ${EndIf}

  Pop $9
  Pop $8
  Pop $7
  Pop $6
  Pop $5
  Pop $4
  Pop $3
  Pop $2
  Pop $1
  Exch $0
FunctionEnd
!macroend ; BOOTCFG_EnumerateBcdObjectList

!macro BOOTCFG_CreateGUID UN
; Create a globally unique identifier (guid)
; Return value:
;   guid
${BOOTCFG_FUNCPROLOG} "${UN}" CreateGUID
  Push $0
  Push $1
  Push $2

  ; Initialize return value
  StrCpy $0 ""
  System::Call "*(g) p.r2"
  ${If} $2 != 0
    System::Call "rpcrt4::UuidCreate(p r2) i.r1"
    ${If} $1 == 0
      System::Call "*$2(g .r0)"
    ${EndIf}
    System::Free $2
  ${EndIf}

  Pop $2
  Pop $1
  Exch $0
FunctionEnd
!macroend ; BOOTCFG_CreateGUID

!macro BOOTCFG_CreateObject UN
${BOOTCFG_FUNCINC} "${UN}" PrepareMethod
${BOOTCFG_FUNCINC} "${UN}" AllocI4Variant
${BOOTCFG_FUNCINC} "${UN}" AllocStringVariant
${BOOTCFG_FUNCINC} "${UN}" Put
${BOOTCFG_FUNCINC} "${UN}" FreeStringVariant
${BOOTCFG_FUNCINC} "${UN}" ExecMethod
${BOOTCFG_FUNCINC} "${UN}" GetObjectPropertyValue
${BOOTCFG_FUNCINC} "${UN}" GetObjectOfVariant
; Create BCD object
; Parameters:
;   services - IWbemServices object
;   basebcdstore - BcdStore base class object
;   bcdstore - BcdStore object
;   type - application type of BCD object
;   guid - GUID of boot entry
; Return values:
;   obj - BCD object or error message
;   err - error code
${BOOTCFG_FUNCPROLOG} "${UN}" CreateObject
  Push $0
  Exch 5
  Push $1
  Exch 5
  Exch $2
  Exch 4
  Exch $3
  Exch 3
  Exch $4
  Exch 2
  Exch $5
  Exch
  Exch $6
  Push $7
  Push $8

  ; basebcdstore=$2, bcdstore=$3, type=$4, guid=$5, services=$6
  StrCpy $7 "CreateObject"
  !insertmacro ${BOOTCFG_PREFIX}PrepareMethod_Call "${UN}" $2 $7 $1 $0
  ; instance=$1
  ${If} $0 == 0
    !insertmacro ${BOOTCFG_PREFIX}AllocI4Variant_Call "${UN}" $4 $8
    ${If} $8 != 0
      !insertmacro ${BOOTCFG_PREFIX}Put_Call "${UN}" $1 "Type" $8 $0
      ; Free variant
      System::Free $8
      ${If} $0 == 0
        !insertmacro ${BOOTCFG_PREFIX}AllocStringVariant_Call "${UN}" $5 $8
        ${If} $8 != 0
          !insertmacro ${BOOTCFG_PREFIX}Put_Call "${UN}" $1 "Id" $8 $0
          !insertmacro ${BOOTCFG_PREFIX}FreeStringVariant_Call "${UN}" $8
        ${Else}
          StrCpy $0 ${ERROR_NOT_ENOUGH_MEMORY}
        ${EndIf}
      ${EndIf}
    ${Else}
      StrCpy $0 ${ERROR_NOT_ENOUGH_MEMORY}
    ${EndIf}
    ${If} $0 != 0
      ; Release instance
      !insertmacro ${BOOTCFG_PREFIX}ReleaseObject_Call "${UN}" $1
      ${If} $0 != ${ERROR_NOT_ENOUGH_MEMORY}
        StrCpy $1 "Failed to allocate memory for method $7"
      ${Else}
        StrCpy $1 "Put failed for method $7"
      ${EndIf}
    ${Else}
      StrCpy $4 $1
      ; instance=$4
      !insertmacro ${BOOTCFG_PREFIX}ExecMethod_Call "${UN}" \
        $6 $3 $4 $7 $1 $0
      ; Release instance
      !insertmacro ${BOOTCFG_PREFIX}ReleaseObject_Call "${UN}" $4
      ${If} $0 == 0
        StrCpy $7 "Object"
        StrCpy $4 $1
        ; result=$4
        !insertmacro ${BOOTCFG_PREFIX}GetObjectPropertyValue_Call "${UN}" \
          $4 $7 $1 $0
        ; Release result
        !insertmacro ${BOOTCFG_PREFIX}ReleaseObject_Call "${UN}" $4
        ${If} $0 == 0
          StrCpy $4 $1
          ; variant=$4
          ; Get BCD object from variant structure
          !insertmacro ${BOOTCFG_PREFIX}GetObjectOfVariant_Call "${UN}" \
            $4 $1 $0
          ; Free variant
          System::Free $4
        ${EndIf}
      ${EndIf}
    ${EndIf}
  ${EndIf}

  Pop $8
  Pop $7
  Pop $6
  Pop $5
  Pop $4
  Pop $3
  Pop $2
  Exch $1
  Exch
  Exch $0
FunctionEnd
!macroend ; BOOTCFG_CreateObject

!macro BOOTCFG_DeleteObject UN
${BOOTCFG_FUNCINC} "${UN}" PrepareMethod
${BOOTCFG_FUNCINC} "${UN}" AllocStringVariant
${BOOTCFG_FUNCINC} "${UN}" Put
${BOOTCFG_FUNCINC} "${UN}" FreeStringVariant
${BOOTCFG_FUNCINC} "${UN}" ExecMethod
; Delete BCD object
; Parameters:
;   services - IWbemServices object
;   basebcdstore - BcdStore base class object
;   bcdstore - BcdStore object
;   guid - GUID of boot entry
; Return values:
;   result - result object or error message
;   err - error code
${BOOTCFG_FUNCPROLOG} "${UN}" DeleteObject
  Push $0
  Exch 4
  Push $1
  Exch 4
  Exch $2
  Exch 3
  Exch $3
  Exch 2
  Exch $4
  Exch
  Exch $5
  Push $6
  Push $7

  ; basebcdstore=$2, bcdstore=$3, guid=$4, services=$5
  StrCpy $6 "DeleteObject"
  !insertmacro ${BOOTCFG_PREFIX}PrepareMethod_Call "${UN}" $2 $6 $1 $0
  ; instance=$1
  ${If} $0 == 0
    !insertmacro ${BOOTCFG_PREFIX}AllocStringVariant_Call "${UN}" $4 $7
    ${If} $7 != 0
      !insertmacro ${BOOTCFG_PREFIX}Put_Call "${UN}" $1 "Id" $7 $0
      !insertmacro ${BOOTCFG_PREFIX}FreeStringVariant_Call "${UN}" $7
    ${Else}
      StrCpy $0 ${ERROR_NOT_ENOUGH_MEMORY}
    ${EndIf}
    ${If} $0 != 0
      ; Release instance
      !insertmacro ${BOOTCFG_PREFIX}ReleaseObject_Call "${UN}" $1
      ${If} $0 != ${ERROR_NOT_ENOUGH_MEMORY}
        StrCpy $1 "Failed to allocate memory for method $6"
      ${Else}
        StrCpy $1 "Put failed for method $6"
      ${EndIf}
    ${Else}
      StrCpy $4 $1
      ; instance=$4
      !insertmacro ${BOOTCFG_PREFIX}ExecMethod_Call "${UN}" \
        $5 $3 $4 $6 $1 $0
      ; Release instance
      !insertmacro ${BOOTCFG_PREFIX}ReleaseObject_Call "${UN}" $4
    ${EndIf}
  ${EndIf}

  Pop $7
  Pop $6
  Pop $5
  Pop $4
  Pop $3
  Pop $2
  Exch $1
  Exch
  Exch $0
FunctionEnd
!macroend ; BOOTCFG_DeleteObject

!macro BOOTCFG_SetStringElement UN
${BOOTCFG_FUNCINC} "${UN}" PrepareMethod
${BOOTCFG_FUNCINC} "${UN}" AllocI4Variant
${BOOTCFG_FUNCINC} "${UN}" AllocStringVariant
${BOOTCFG_FUNCINC} "${UN}" Put
${BOOTCFG_FUNCINC} "${UN}" FreeStringVariant
${BOOTCFG_FUNCINC} "${UN}" ExecMethod
; Set string element
; Parameters:
;   services - IWbemServices object
;   basebcdobject - BcdObject base class
;   bcdobject - BcdObject
;   type - element type
;   string
; Return values:
;   result - result object or error message
;   err - error code
${BOOTCFG_FUNCPROLOG} "${UN}" SetStringElement
  Push $0
  Exch 5
  Push $1
  Exch 5
  Exch $2
  Exch 4
  Exch $3
  Exch 3
  Exch $4
  Exch 2
  Exch $5
  Exch
  Exch $6
  Push $7
  Push $8

  ; basebcdobject=$2, bcdobject=$3, type=$4, string=$5, services=$6
  StrCpy $7 "SetStringElement"
  !insertmacro ${BOOTCFG_PREFIX}PrepareMethod_Call "${UN}" $2 $7 $1 $0
  ; instance=$1
  ${If} $0 == 0
    !insertmacro ${BOOTCFG_PREFIX}AllocI4Variant_Call "${UN}" $4 $8
    ${If} $8 != 0
      !insertmacro ${BOOTCFG_PREFIX}Put_Call "${UN}" $1 "Type" $8 $0
      ; Free variant
      System::Free $8
      ${If} $0 == 0
        !insertmacro ${BOOTCFG_PREFIX}AllocStringVariant_Call "${UN}" $5 $8
        ${If} $8 != 0
          !insertmacro ${BOOTCFG_PREFIX}Put_Call "${UN}" $1 "String" $8 $0
          !insertmacro ${BOOTCFG_PREFIX}FreeStringVariant_Call "${UN}" $8
        ${Else}
          StrCpy $0 ${ERROR_NOT_ENOUGH_MEMORY}
        ${EndIf}
      ${EndIf}
    ${Else}
      StrCpy $0 ${ERROR_NOT_ENOUGH_MEMORY}
    ${EndIf}
    ${If} $0 != 0
      ; Release instance
      !insertmacro ${BOOTCFG_PREFIX}ReleaseObject_Call "${UN}" $1
      ${If} $0 != ${ERROR_NOT_ENOUGH_MEMORY}
        StrCpy $1 "Failed to allocate memory for $7"
      ${Else}
        StrCpy $1 "Put failed for method $7"
      ${EndIf}
    ${Else}
      StrCpy $4 $1
      ; instance=$4
      !insertmacro ${BOOTCFG_PREFIX}ExecMethod_Call "${UN}" \
        $6 $3 $4 $7 $1 $0
      ; Release instance
      !insertmacro ${BOOTCFG_PREFIX}ReleaseObject_Call "${UN}" $4
    ${EndIf}
  ${EndIf}

  Pop $8
  Pop $7
  Pop $6
  Pop $5
  Pop $4
  Pop $3
  Pop $2
  Exch $1
  Exch
  Exch $0
FunctionEnd
!macroend ; BOOTCFG_SetStringElement

!macro BOOTCFG_SetPartition UN
${BOOTCFG_FUNCINC} "${UN}" PrepareMethod
${BOOTCFG_FUNCINC} "${UN}" AllocI4Variant
${BOOTCFG_FUNCINC} "${UN}" AllocStringVariant
${BOOTCFG_FUNCINC} "${UN}" Put
${BOOTCFG_FUNCINC} "${UN}" FreeStringVariant
${BOOTCFG_FUNCINC} "${UN}" ExecMethod
; Set partition
; Parameters:
;   services - IWbemServices object
;   basebcdobject - BcdObject base class
;   bcdobject - BcdObject
;   partition - partition identifier
; Return values:
;   result - result object or error message
;   err - error code
${BOOTCFG_FUNCPROLOG} "${UN}" SetPartition
  Push $0
  Exch 4
  Push $1
  Exch 4
  Exch $2
  Exch 3
  Exch $3
  Exch 2
  Exch $4
  Exch
  Exch $5
  Push $6
  Push $7

  ; basebcdobject=$2, bcdobject=$3, partition=$4, services=$5
  StrCpy $6 "SetPartitionDeviceElement"
  !insertmacro ${BOOTCFG_PREFIX}PrepareMethod_Call "${UN}" $2 $6 $1 $0
  ; instance=$1
  ${If} $0 == 0
    !insertmacro ${BOOTCFG_PREFIX}AllocI4Variant_Call "${UN}" \
      ${BOOTCFG_BCDE_DEVICE} $7
    ${If} $7 != 0
      !insertmacro ${BOOTCFG_PREFIX}Put_Call "${UN}" $1 "Type" $7 $0
      ; Free variant
      System::Free $7
      ${If} $0 == 0
        !insertmacro ${BOOTCFG_PREFIX}AllocI4Variant_Call "${UN}" \
          ${BOOTCFG_PARTIONDEVICE_TYPE} $7
        ${If} $7 != 0
          !insertmacro ${BOOTCFG_PREFIX}Put_Call "${UN}" \
            $1 "DeviceType" $7 $0
          ; Free variant
          System::Free $7
          ${If} $0 == 0
            !insertmacro ${BOOTCFG_PREFIX}AllocStringVariant_Call "${UN}" \
              $4 $7
            ${If} $7 != 0
              !insertmacro ${BOOTCFG_PREFIX}Put_Call "${UN}" \
                $1 "Path" $7 $0
              !insertmacro ${BOOTCFG_PREFIX}FreeStringVariant_Call \
                "${UN}" $7
              ${If} $0 == 0
                !insertmacro ${BOOTCFG_PREFIX}AllocStringVariant_Call \
                  "${UN}" "" $7
                ${If} $7 != 0
                  !insertmacro ${BOOTCFG_PREFIX}Put_Call "${UN}" \
                    $1 "AdditionalOptions" $7 $0
                  !insertmacro ${BOOTCFG_PREFIX}FreeStringVariant_Call \
                    "${UN}" $7
                ${Else}
                  StrCpy $0 ${ERROR_NOT_ENOUGH_MEMORY}
                ${EndIf}
              ${EndIf}
            ${Else}
              StrCpy $0 ${ERROR_NOT_ENOUGH_MEMORY}
            ${EndIf}
          ${EndIf}
        ${Else}
          StrCpy $0 ${ERROR_NOT_ENOUGH_MEMORY}
        ${EndIf}
      ${EndIf}
    ${Else}
      StrCpy $0 ${ERROR_NOT_ENOUGH_MEMORY}
    ${EndIf}
    ${If} $0 != 0
      ; Release instance
      !insertmacro ${BOOTCFG_PREFIX}ReleaseObject_Call "${UN}" $1
      ${If} $0 != ${ERROR_NOT_ENOUGH_MEMORY}
        StrCpy $1 "Failed to allocate memory for $6"
      ${Else}
        StrCpy $1 "Put failed for method $6"
      ${EndIf}
    ${Else}
      StrCpy $4 $1
      ; instance=$4
      !insertmacro ${BOOTCFG_PREFIX}ExecMethod_Call "${UN}" \
        $5 $3 $4 $6 $1 $0
      ; Release instance
      !insertmacro ${BOOTCFG_PREFIX}ReleaseObject_Call "${UN}" $4
    ${EndIf}
  ${EndIf}

  Pop $7
  Pop $6
  Pop $5
  Pop $4
  Pop $3
  Pop $2
  Exch $1
  Exch
  Exch $0
FunctionEnd
!macroend ; BOOTCFG_SetPartition

!macro BOOTCFG_SetObjectList UN
${BOOTCFG_FUNCINC} "${UN}" PrepareMethod
${BOOTCFG_FUNCINC} "${UN}" AllocI4Variant
${BOOTCFG_FUNCINC} "${UN}" AllocStringArrayVariant
${BOOTCFG_FUNCINC} "${UN}" Put
${BOOTCFG_FUNCINC} "${UN}" ExecMethod
; Set object list
; Parameters:
;   services - IWbemServices object
;   basebcdobject - BcdObject base class object
;   bcdobject - BcdObject
;   type - type of object list
;   objectlist - object list
; Return values:
;   result object
;   err - error code
${BOOTCFG_FUNCPROLOG} "${UN}" SetObjectList
  ; Handle parameters and save registers
  Push $0
  Exch 5
  Push $1
  Exch 5
  Exch $2
  Exch 4
  Exch $3
  Exch 3
  Exch $4
  Exch 2
  Exch $5
  Exch
  Exch $6
  Push $7
  Push $8

  ; basebcdobject=$2, bcdobject=$3, type=$4, objectlist=$5, services=$6

  StrCpy $7 "SetObjectListElement"
  !insertmacro ${BOOTCFG_PREFIX}PrepareMethod_Call "${UN}" \
    $2 $7 $8 $0
  ; instance=$8
  ${If} $0 != 0
    StrCpy $1 $8
  ${Else}
    !insertmacro ${BOOTCFG_PREFIX}AllocI4Variant_Call "${UN}" \
      $4 $4
    ${If} $4 != 0
      !insertmacro ${BOOTCFG_PREFIX}Put_Call "${UN}" \
        $8 "Type" $4 $0
      System::Free $4
      ${If} $0 == 0
        ; Allocate array variant and fill it with safe array
       !insertmacro ${BOOTCFG_PREFIX}AllocStringArrayVariant_Call \
         "${UN}" $5 $4
        ${If} $4 != 0
          !insertmacro ${BOOTCFG_PREFIX}Put_Call "${UN}" \
            $8 "Ids" $4 $0
          System::Free $4
        ${Else}
          StrCpy $0 ${ERROR_NOT_ENOUGH_MEMORY}
        ${EndIf}
      ${EndIf}
    ${Else}
      StrCpy $0 ${ERROR_NOT_ENOUGH_MEMORY}
    ${EndIf}
    ${If} $0 != 0
      StrCpy $1 "Put failed for $7"
    ${Else}
      !insertmacro ${BOOTCFG_PREFIX}ExecMethod_Call "${UN}" \
        $6 $3 $8 $7 $1 $0
    ${EndIf}
    ; Release instance
    !insertmacro ${BOOTCFG_PREFIX}ReleaseObject_Call "${UN}" $8
  ${EndIf}

  Pop $8
  Pop $7
  Pop $6
  Pop $5
  Pop $4
  Pop $3
  Pop $2
  Exch $1
  Exch
  Exch $0
FunctionEnd
!macroend ; BOOTCFG_SetObjectList

!macro BOOTCFG_AppendBootObject UN
${BOOTCFG_FUNCINC} "${UN}" GetBcdElement
${BOOTCFG_FUNCINC} "${UN}" ObjectListGetSafeArray
${BOOTCFG_FUNCINC} "${UN}" GetSizeOfSafeArray
${BOOTCFG_FUNCINC} "${UN}" SetObjectList
; Append boot object to boot manager
; Parameters:
;   services - IWbemServices object
;   basebcdobject - BcdObject base class object
;   bcdobject - boot manager BcdObject
;   guid - GUID of boot entry
; Return values:
;   result object
;   err - error code
${BOOTCFG_FUNCPROLOG} "${UN}" AppendBootObject
  System::Store 'S'
  Pop $5 ; guid=$5
  Pop $4 ; bcdobject=$4
  Pop $3 ; basebcdobject=$3
  Pop $2 ; services=$2

  ${If} $4 != 0
    !insertmacro ${BOOTCFG_PREFIX}GetBcdElement_Call "${UN}" \
      $2 $3 $4 ${BOOTCFG_DISPLAY_ORDER} $7 $0
    ${If} $0 != 0
      StrCpy $1 $7
    ${Else}
      !insertmacro ${BOOTCFG_PREFIX}ObjectListGetSafeArray_Call "${UN}" \
        $7 $6 $0
      ; Release object list element
      !insertmacro ${BOOTCFG_PREFIX}ReleaseObject_Call "${UN}" $7
      ${If} $0 != 0
        System::Call "oleaut32::SafeArrayCreateVector(i ${VT_BSTR}, \
          i 0, i 1) p.r6?e"
        Pop $0
        ${If} $6 P<> 0
          StrCpy $1 0
          StrCpy $0 0
        ${Else}
          StrCpy $1 "SafeArrayCreateVector failed"
        ${EndIf}
      ${Else}
        !insertmacro ${BOOTCFG_PREFIX}GetSizeOfSafeArray_Call "${UN}" \
          $6 $1
        System::Call "oleaut32::SafeArrayAccessData(p r6, *p.r7) i.r0"
        ${If} $0 == 0
          ; Search for GUID in array
          ${For} $0 1 $1
            System::Call "*$7(w.r8)"
            ${If} $8 == $5
              ${ExitFor}
            ${EndIf}
            IntOp $7 $7 + ${NSIS_PTR_SIZE}
          ${Next}
          System::Call "oleaut32::SafeArrayUnaccessData(p r6) i.r7"
          ${If} $7 == 0
            ${If} $0 == $1
              ; Array includes the GUID and it is the last element
              System::Call "oleaut32::SafeArrayDestroy(p r6)"
              StrCpy $6 0
              StrCpy $0 0
            ${Else}
              ${If} $0 > $1
                ; Array does not include the GUID, hence extend the array
                System::Call "*(i r0, i 0) p.r7"
                ${If} $7 P<> 0
                  System::Call "oleaut32::SafeArrayRedim(p r6r6, p r7) i.r0"
                  ${If} $0 != 0
                    StrCpy $1 "SafeArrayReDim failed"
                  ${EndIf}
                  System::Free $7
                ${Else}
                  StrCpy $1 "Memory allocation failed for SafeArrayRedim"
                  StrCpy $0 ${ERROR_NOT_ENOUGH_MEMORY}
                ${EndIf}
              ${Else}
                ; Array includes the GUID, hence move this entry to the end
                System::Call "oleaut32::SafeArrayAccessData(p r6, *p.r7) i.r8"
                ${If} $8 == 0
                  ; Zero-based index for SafeArrayPutElement
                  IntOp $0 $0 - 1
                  IntOp $1 $1 - 1
                  ${While} $0 < $1
                    IntOp $7 $7 + ${NSIS_PTR_SIZE}
                    System::Call "*$7(w.r8)"
                    System::Call "oleaut32::SysAllocString(w r8) p.r8"
                    ${If} $8 P<> 0
                      System::Call "oleaut32::SafeArrayPutElement(p r6, *i r0, p r8) p.s"
                      System::Call "oleaut32::SysFreeString(p r8)"
                      Pop $8
                      ${If} $8 != 0
                        StrCpy $1 "SafeArrayPutElement failed"
                        StrCpy $0 $8
                        ${ExitWhile}
                      ${EndIf}
                    ${Else}
                      StrCpy $1 "SysAllocString failed"
                      StrCpy $0 ${ERROR_NOT_ENOUGH_MEMORY}
                      ${ExitWhile}
                    ${EndIf}
                    IntOp $0 $0 + 1
                  ${EndWhile}
                  ${If} $0 == $1
                    StrCpy $0 0
                  ${EndIf}
                  System::Call "oleaut32::SafeArrayUnaccessData(p r6)"
                ${Else}
                  StrCpy $1 "SafeArrayAccessData failed"
                  StrCpy $0 $8
                ${EndIf}
              ${EndIf}
            ${EndIf}
          ${Else}
            StrCpy $1 "SafeArrayUnaccessData failed"
            StrCpy $0 $7
          ${EndIf}
        ${Else}
          StrCpy $1 "SafeArrayAccessData failed"
        ${EndIf}
      ${EndIf}
      ${If} $0 == 0
      ${AndIf} $6 P<> 0
        ; Allocate BSTR variant and fill it with GUID
        System::Call "oleaut32::SysAllocString(w r5) p.r5"
        ${If} $5 != 0
          System::Call "oleaut32::SafeArrayPutElement(p r6, *i r1, p r5) i.r0"
          System::Call "oleaut32::SysFreeString(p r5)"
          ${If} $0 != 0
            StrCpy $1 "SafeArrayPutElement failed"
            System::Call "oleaut32::SafeArrayDestroy(p r6)"
          ${Else}
            !insertmacro ${BOOTCFG_PREFIX}SetObjectList_Call "${UN}" \
              $2 $3 $4 ${BOOTCFG_DISPLAY_ORDER} $6 $1 $0
            System::Call "oleaut32::SafeArrayDestroy(p r6)"
          ${EndIf}
        ${Else}
          StrCpy $1 "SysAllocString failed"
          StrCpy $0 ${ERROR_NOT_ENOUGH_MEMORY}
        ${EndIf}
      ${EndIf}
    ${EndIf}
    ; Release bcdobject
    !insertmacro ${BOOTCFG_PREFIX}ReleaseObject_Call "${UN}" $4
  ${EndIf}

  Push $1
  Push $0
  System::Store 'L'
FunctionEnd
!macroend ; BOOTCFG_AppendBootObject

!macro BOOTCFG_RemoveBootEntry UN
${BOOTCFG_FUNCINC} "${UN}" GetElementFromBcd
${BOOTCFG_FUNCINC} "${UN}" ObjectListGetSafeArray
${BOOTCFG_FUNCINC} "${UN}" GetSizeOfSafeArray
${BOOTCFG_FUNCINC} "${UN}" SetObjectList
; Remove boot entry from boot manager
; Parameters:
;   services - IWbemServices object
;   basebcdstore - BcdStore base class object
;   bcdstore - BcdStore object
;   basebcdobject - BcdObject base class object
;   guid - GUID of boot entry
; Return values:
;   result object
;   err - error code
${BOOTCFG_FUNCPROLOG} "${UN}" RemoveBootEntry
  System::Store 'S'
  Pop $6 ; guid=$6
  Pop $5 ; basebcdobject=$5
  Pop $4 ; bcdstore=$4
  Pop $3 ; basebcdstore=$3
  Pop $2 ; services=$2

  !insertmacro ${BOOTCFG_PREFIX}GetElementFromBcd_Call "${UN}" \
    $2 $3 $4 $5 ${BOOTCFG_BOOTMGR_GUID} ${BOOTCFG_DISPLAY_ORDER} $3 $8 $0
  ; element=$8, bcdobject=$3
  ${If} $0 != 0
    StrCpy $1 $8
  ${Else}
    !insertmacro ${BOOTCFG_PREFIX}ObjectListGetSafeArray_Call "${UN}" \
      $8 $7 $0
    ; Release object list element
    !insertmacro ${BOOTCFG_PREFIX}ReleaseObject_Call "${UN}" $8
    ${If} $0 != 0
      System::Call "oleaut32::SafeArrayCreateVector(i ${VT_BSTR}, \
        i 0, i 1) p.r7"
      StrCpy $1 0
      StrCpy $0 0
    ${Else}
      !insertmacro ${BOOTCFG_PREFIX}GetSizeOfSafeArray_Call "${UN}" \
        $7 $1
      System::Call "oleaut32::SafeArrayAccessData(p r7, *p 0 r8) i.r0"
      ${If} $0 != 0
        StrCpy $1 "SafeArrayAccessData failed"
      ${Else}
        ${If} $1 > 0
          IntOp $1 $1 - 1
          ${For} $0 0 $1
            System::Call "*$8(w .r9)"
            ${If} $9 == $6
              IntOp $0 $0 + 1
              ${Break}
            ${EndIf}
            IntOp $8 $8 + ${NSIS_PTR_SIZE}
          ${Next}
          ${For} $0 $0 $1
            IntOp $9 $8 + ${NSIS_PTR_SIZE}
            System::Call "*$9(p .s)"
            Exch $9
            System::Call "*$8(p r9)"
            Pop $8
          ${Next}
        ${EndIf}
        System::Call "oleaut32::SafeArrayUnaccessData(p r7) i.r0"
        ${If} $0 != 0
          StrCpy $1 "SafeArrayUnaccessData failed"
        ${Else}
          System::Call "*(i $1, i 0) p.r8"
          ${If} $8 P<> 0
            System::Call "oleaut32::SafeArrayRedim(p r7r7, p r8) i.r0"
            ${If} $0 != 0
              StrCpy $1 "SafeArrayReDim failed"
            ${EndIf}
            System::Free $8
            !insertmacro ${BOOTCFG_PREFIX}GetSizeOfSafeArray_Call "${UN}" \
              $7 $1
          ${Else}
            StrCpy $1 "Memory allocation failed for SafeArrayRedim"
            StrCpy $0 ${ERROR_NOT_ENOUGH_MEMORY}
          ${EndIf}
        ${EndIf}
      ${EndIf}
      !insertmacro ${BOOTCFG_PREFIX}SetObjectList_Call "${UN}" \
        $2 $5 $3 ${BOOTCFG_DISPLAY_ORDER} $7 $1 $0
      System::Call "oleaut32::SafeArrayDestroy(p r7)"
    ${EndIf}
    ; Release bcdobject
    !insertmacro ${BOOTCFG_PREFIX}ReleaseObject_Call "${UN}" $3
  ${EndIf}

  Push $1
  Push $0
  System::Store 'L'
FunctionEnd
!macroend ; BOOTCFG_RemoveBootEntry

!macro BOOTCFG_SetActiveBootObject UN
${BOOTCFG_FUNCINC} "${UN}" GetBcdObject
${BOOTCFG_FUNCINC} "${UN}" SetObjectList
; Set active boot entry
; Parameters:
;   services - IWbemServices object
;   basebcdobject - BcdObject base class object
;   bcdobject - Boot manager BcdObject
;   guid - GUID of boot entry
; Return values:
;   result object
;   err - error code
${BOOTCFG_FUNCPROLOG} "${UN}" SetActiveBootObject
  System::Store 'S'
  Pop $5 ; guid=$5
  Pop $4 ; bcdobject=$4
  Pop $3 ; basebcdobject=$3
  Pop $2 ; services=$2

  ${If} $4 != 0
    System::Call "oleaut32::SafeArrayCreateVector(i ${VT_BSTR}, i 0, i 1) \
      p.r6"
    ${If} $6 != 0
      ; Allocate BSTR variant and fill it with GUID
      System::Call "oleaut32::SysAllocString(w r5) p.r7"
      ${If} $7 != 0
        StrCpy $1 0
        System::Call "oleaut32::SafeArrayPutElement(p r6, *i r1, p r7) i.r0"
        System::Call "oleaut32::SysFreeString(p r7)"
        ${If} $0 != 0
          StrCpy $1 "SafeArrayPutElement failed"
        ${Else}
          ; bootorderlist=$6
          !insertmacro ${BOOTCFG_PREFIX}SetObjectList_Call "${UN}" \
            $2 $3 $4 ${BOOTCFG_BOOTSEQUENCE} $6 $1 $0
        ${EndIf}
      ${Else}
        StrCpy $1 "SysAllocString failed"
        StrCpy $0 ${ERROR_NOT_ENOUGH_MEMORY}
      ${EndIf}
    ${EndIf}
    ; Release bcdobject
    !insertmacro ${BOOTCFG_PREFIX}ReleaseObject_Call "${UN}" $4
    System::Call "oleaut32::SafeArrayDestroy(p r6)"
  ${EndIf}

  Push $1
  Push $0
  System::Store 'L'
FunctionEnd
!macroend ; BOOTCFG_SetActiveBootObject

!macro BOOTCFG_ConnectWMI UN
${SECURITY_FUNCINC} "${UN}" AcquirePrivileges
${SECURITY_FUNCINC} "${UN}" COMSetDefaultLevel
${SECURITY_FUNCINC} "${UN}" PrivilegeValueToStack
; Connect to the root/wmi namespace
; Return values:
;   locator - IWbemLocator object
;   services - IWbemServices object respectively an error mesage
;   err - error code
${BOOTCFG_FUNCPROLOG} "${UN}" ConnectWMI
  Push $0
  Push $1
  Push $2
  Push $3

  ; Set default COM security level
  !insertmacro ${SECURITY_PREFIX}COMSetDefaultLevel_Call "${UN}" $0
  ${If} $0 != 0
     StrCpy $1 "CoInitializeSecurity failed"
  ${Else}
    ; Acquire backup and restore privileges
    StrCpy $0 0
    !insertmacro ${SECURITY_PREFIX}PrivilegeValueToStack_Call "${UN}" \
      ${SE_BACKUP_NAME} $0
    !insertmacro ${SECURITY_PREFIX}PrivilegeValueToStack_Call "${UN}" \
      ${SE_RESTORE_NAME} $0
    Push $0
    !insertmacro ${SECURITY_PREFIX}AcquirePrivileges_Call "${UN}" $0
    ${If} $0 != 0
      StrCpy $1 "Failed to acquire required privileges"
    ${Else}
      ; Connect to root/WMI namespace
      ; Create IWbemLocator interface
      System::Call "ole32::CoCreateInstance( \
        g '${CLSID_WbemLocator}', p 0, \
        i ${CLSCTX_INPROC_SERVER}, \
        g '${IID_IWbemLocator}', *p .r2) i.r0"
      ; locator=$2
      ${If} $0 != 0
        StrCpy $1 "CoCreateInstance failed"
      ${Else}
        ; IWbemLocator::ConnectServer(objectpath, user, password, locale,
        ;   securityflags, authority, context, services)
	${IWbemLocator::ConnectServer} $2 "('root/wmi',0,0,0,0,0,0,.r3).r0"
        ; services=$3
        ${If} $0 != 0
          ; Release locator
          !insertmacro ${BOOTCFG_PREFIX}ReleaseObject_Call "${UN}" $2
          StrCpy $3 ""
          StrCpy $1 "ConnectServer failed"
        ${Else}
          StrCpy $1 $3
        ${EndIf}
      ${EndIf}
    ${EndIf}
  ${EndIf}

  Pop $3
  Exch $2
  Exch 2
  Exch
  Exch $1
  Exch
  Exch $0
FunctionEnd
!macroend ; BOOTCFG_ConnectWMI

!endif ; BOOTCFG_INCLUDED
