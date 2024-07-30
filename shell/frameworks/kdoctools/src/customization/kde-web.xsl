<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
		version="1.0">

<xsl:import href="kde-include-common.xsl"/>
<xsl:include href="kde-web-navig.xsl"/>
<xsl:include href="kde-ttlpg.xsl"/>
<xsl:include href="kde-style.xsl"/>

<xsl:variable name="KDOCTOOLS_XSLT_VERSION">1.13</xsl:variable> 

<xsl:template name="make-relative-filename">
  <xsl:param name="base.dir" select="'./'"/>
  <xsl:param name="base.name" select="''"/>

  <!-- XT makes chunks relative -->
  <xsl:choose>
    <xsl:when test="count(parent::*) = 0">
      <xsl:value-of select="concat($base.dir,$base.name)"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="$base.name"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template name="write.chunk">
  <xsl:param name="filename" select="''"/>
  <xsl:param name="method" select="'html'"/>
  <xsl:param name="encoding" select="'utf-8'"/>
  <xsl:param name="content" select="''"/>
  
  <FILENAME filename="{$filename}"> 
  <xsl:copy-of select="$content"/>
  </FILENAME>
</xsl:template>

<xsl:param name="use.id.as.filename">1</xsl:param>
<xsl:param name="generate.section.toc">0</xsl:param>
<xsl:param name="generate.component.toc">0</xsl:param>
<xsl:param name="use.extensions">0</xsl:param>
<xsl:param name="admon.graphics">0</xsl:param>
<xsl:param name="kde.common">../common/</xsl:param>
<xsl:param name="html.stylesheet" select="concat($kde.common,'kde-default.css')"/>
<xsl:param name="admon.graphics.path"><xsl:value-of select="kde.common"/></xsl:param>


<xsl:template name="dbhtml-filename">
<xsl:choose>
     <xsl:when test=". != /*">
      <xsl:value-of select="@id"/>
      <xsl:value-of select="$html.ext"/>
     </xsl:when>
     <xsl:otherwise>
	<xsl:text>index.html</xsl:text>
      </xsl:otherwise>
</xsl:choose>
</xsl:template>

<xsl:template name="dbhtml-dir">
</xsl:template>

<xsl:template name="user.head.content">
   <meta http-equiv="Content-Type" content="text/html; charset=utf-8"/> 
   <meta name="GENERATOR" content="KDE XSL Stylesheet V{$KDOCTOOLS_XSLT_VERSION} using libxslt"/>
</xsl:template>

<!-- try with olinks: it nearly works --><!--
  <xsl:template match="olink">
    <a>
      <xsl:attribute name="href">
	<xsl:choose>
	  <xsl:when test="@type = 'kde-installation'">
	    <xsl:choose>
	      <xsl:when test="@linkmode = 'kdems-man'">
		<xsl:value-of select="id(@linkmode)"/>
		<xsl:value-of select="@targetdocent"/>
		<xsl:text>(</xsl:text>
		<xsl:value-of select="@localinfo"/>
		<xsl:text>)</xsl:text>
	      </xsl:when>
	      <xsl:when test="@linkmode = 'kdems-help'">
		<xsl:value-of select="id(@linkmode)"/>
		<xsl:text>/</xsl:text>
		<xsl:value-of select="@targetdocent"/>
<xsl:variable name="targetdocent" select="@targetdocent"/>
<xsl:value-of select="$targetdocent"/>
          <xsl:if test="@targetdocent">
            <xsl:value-of select="unparsed-entity-uri(string($targetdocent))"/>
          </xsl:if>
                <xsl:for-each select="document('/home/fouvry/kdeutils/doc/kedit/index.docbook')">
		  <xsl:value-of select=".//*[@id=$localinfo]"/>
                </xsl:for-each>
		<xsl:text>#</xsl:text>
		<xsl:value-of select="@localinfo"/>
	      </xsl:when>
	    </xsl:choose>
	  </xsl:when>
	</xsl:choose>
      </xsl:attribute>
      <xsl:value-of select="."/>
    </a>
  </xsl:template>
-->

</xsl:stylesheet>


