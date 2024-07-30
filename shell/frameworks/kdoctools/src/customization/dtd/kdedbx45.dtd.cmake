<!--
    Document Type Definition for DocBook XML as used in the KDE documentation
    
    SPDX-FileCopyrightText: 2002 Frederik Fouvry 
     
    SPDX-License-Identifier: GPL-2.0-or-later

    Send suggestions, comments, etc. to the KDE docbook list 
    <kde-docbook@kde.org>.


    USAGE

    Refer to this DTD as

      "-//KDE//DTD DocBook XML V4.5-Based Variant V1.1//EN"

    For instance:

      <!DOCTYPE book PUBLIC "-//KDE//DTD DocBook XML V4.5-Based Variant V1.1//EN"
                     "customization/dtd/kdedbx45.dtd">

    and use catalog.xml or docbook.cat as the catalog file.
    Note: the system identifier is interpreted relative to the place
    where it is defined, unless otherwise defined by the application you use.
    If you have permanent internet access, you can use any URLs (and
    you don't have to install docbook).

    After a version number change, plenty of problems may occur because
    of non-compatibilities (or DTDs not found, etc).  That doesn't mean your
    document is invalid; it just means that the reference DTD has changed 
    (new documentation and documentation updates).  Check out the previous
    version and point to the appropriate catalog or DTD and things should
    work again (=still ;-).
-->


<!-- ============================================================= -->
<!--	      Modifications to the original DocBook DTD		   -->
<!-- ============================================================= -->
<!-- Declarations in this part should only *restrict* the DocBook 
     DTD                                                           -->

<!-- copied from docbookx.dtd: these values are for 
     %xml.features; = "INCLUDE"
     Files that define elements need them (currently only 
     modifications.elements) -->
<!ENTITY % ho "">
<!ENTITY % ubiq.inclusion "">
<!ENTITY % beginpage.exclusion "">

<!-- Modifications in pool
     %rdbpool; is called in the DocBook DTD
     NEEDS TO BE CHECKED ON CHANGES
 -->
<!ENTITY % rdbpool	    PUBLIC
  "-//KDE//ELEMENTS DocBook XML Pool Redeclarations V1.1//EN"	     
  "rdbpool.elements"                                                 >
<!ENTITY % dbpool.redecl.module "INCLUDE"			     >

<!-- Modifications in hierarchy
     %rdbhier2; is called in the DocBook DTD
     NEEDS TO BE CHECKED ON CHANGES
 -->
<!ENTITY % rdbhier2	    PUBLIC
  "-//KDE//ELEMENTS DocBook XML Hierarchy Redeclarations 2 V1.0//EN"     
  "rdbhier2.elements"                                                >
<!ENTITY % dbhier.redecl2.module "INCLUDE"			     >

<!-- For Java-style refentries	   
     Based on DocBook 4.0.                                         -->
<!--ENTITY % jrefentry      PUBLIC
  "-//Norman Walsh//DTD JRefEntry V1.0//EN"                          >
%jrefentry; -->

<!-- Temporary solution -->
<!ENTITY copy  "(c)"> <!--=copyright sign-->
<!ENTITY trade "[tm]"> <!--=trade mark sign-->
<!ENTITY reg   "(R)"> <!--/circledR =registered sign-->


<!-- ============================================================= -->
<!--			 Original DocBook DTD			   -->
<!-- ============================================================= -->

<!ENTITY % DocBookDTD	    PUBLIC
  "-//OASIS//DTD DocBook XML V4.5//EN"
  "@DocBookXML4_DTD_DIR@/docbookx.dtd"         >
%DocBookDTD;


<!-- ============================================================= -->
<!--          Modifications to the original DocBook DTD            -->
<!-- ============================================================= -->
<!-- ONLY to supply declarations that were switched off in the 
     first "Modifications" section -->

<!ENTITY % kde.modifications PUBLIC
  "-//KDE//ELEMENTS DocBook XML Modifications V1.1//EN"
  "modifications.elements"                                           >
%kde.modifications;


<!-- ============================================================= -->
<!--		    Extensions to the DocBook DTD		   -->
<!-- ============================================================= -->
<!-- Is empty, except for what could go in dbgenent.mod            -->

<!ENTITY % kde.general.entities	PUBLIC
  "-//KDE//ENTITIES DocBook XML General Entity Declarations V1.2//EN"    
  "../entities/general.entities"                                     >
%kde.general.entities;

<!ENTITY % kde.i18n.entities PUBLIC
  "-//KDE//ENTITIES DocBook XML Localisation Entity Declarations V1.0//EN"
  "../entities/l10n.entities"                                        >
%kde.i18n.entities;
