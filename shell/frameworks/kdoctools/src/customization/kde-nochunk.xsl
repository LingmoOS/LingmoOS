<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
		version="1.0">

<xsl:import href="kde-chunk-common.xsl"/>
<!-- <xsl:include href="kde-print-navig.xsl"/> -->
<xsl:include href="kde-ttlpg.xsl"/>
<xsl:include href="kde-style.xsl"/>

<xsl:variable name="KDOCTOOLS_XSLT_VERSION">1.13</xsl:variable> 

<xsl:param name="using.chunker">0</xsl:param>
<xsl:param name="chunk.first.sections" select="0"/>
<xsl:param name="chunk.sections" select="0"/>
<xsl:param name="chunk.section.depth" select="0"/>

<xsl:param name="use.id.as.filename">0</xsl:param>
<xsl:param name="generate.section.toc">0</xsl:param>
<xsl:param name="generate.component.toc">0</xsl:param>
<xsl:param name="use.extensions">0</xsl:param>
<xsl:param name="admon.graphics">0</xsl:param>
<xsl:param name="kde.common">../common/</xsl:param>
<xsl:param name="html.stylesheet" select="concat($kde.common,'kde-web.css')"/>
<xsl:param name="admon.graphics.path"><xsl:value-of select="kde.common"/></xsl:param>


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


