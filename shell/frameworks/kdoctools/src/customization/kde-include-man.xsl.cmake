<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
		version="1.0">

   <xsl:import href="@DOCBOOKXSL_DIR@/manpages/docbook.xsl"/>

   <xsl:param name="chunker.output.encoding" select="'UTF-8'"/>
   <xsl:output method="text" encoding="UTF-8" indent="no"/>

   <xsl:param name="l10n.xml" select="document('xsl/all-l10n.xml')"/>

</xsl:stylesheet>
