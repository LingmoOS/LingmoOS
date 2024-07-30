<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
        version="1.0">

<xsl:import href="kde-include-common.xsl"/>

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

<xsl:template name="user.header.navigation">
   <!-- xsl:attribute name="id">content</xsl:attribute -->
   <xsl:text disable-output-escaping="yes">&lt;div id="content"&gt;</xsl:text>
</xsl:template>

<xsl:template name="user.header.content">
   <xsl:text disable-output-escaping="yes">&lt;div id="contentBody"&gt;</xsl:text>
</xsl:template>

<xsl:template name="user.footer.content">
   <xsl:text disable-output-escaping="yes">&lt;/div&gt;</xsl:text>
</xsl:template>

<xsl:template name="user.footer.navigation">
   <xsl:text disable-output-escaping="yes">&lt;/div&gt;</xsl:text>
</xsl:template>

<xsl:template match="ulink[@type='commondoc']">
   <xsl:variable name="kde.commondoc.url"><xsl:value-of select="concat($kde.common,@url)"/></xsl:variable>
   <xsl:call-template name="ulink">
     <xsl:with-param name="url" select="$kde.commondoc.url"/>
   </xsl:call-template>
</xsl:template>

</xsl:stylesheet>
