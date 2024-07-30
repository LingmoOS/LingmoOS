<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  version="1.0">

  <xsl:template match="email">
    <xsl:call-template name="inline.monoseq">
      <xsl:with-param name="content">
        <xsl:text>(</xsl:text>
        <xsl:call-template name="replaceCharsInString">
          <xsl:with-param name="stringIn" select="."/>
          <xsl:with-param name="charsIn" select="'@'"/>
          <xsl:with-param name="charsOut" select="' AT '"/>
        </xsl:call-template>
        <xsl:text>)</xsl:text>
      </xsl:with-param>
    </xsl:call-template>
  </xsl:template>
  <xsl:template name="replaceCharsInString">
    <xsl:param name="stringIn"/>
    <xsl:param name="charsIn"/>
    <xsl:param name="charsOut"/>
    <xsl:choose>
      <xsl:when test="contains($stringIn,$charsIn)">
        <xsl:value-of select="concat(substring-before($stringIn,$charsIn),$charsOut)"/>
        <xsl:call-template name="replaceCharsInString">
          <xsl:with-param name="stringIn" select="substring-after($stringIn,$charsIn)"/>
          <xsl:with-param name="charsIn" select="$charsIn"/>
          <xsl:with-param name="charsOut" select="$charsOut"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$stringIn"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="header.navigation">
    <xsl:param name="prev" select="/foo"/>
    <xsl:param name="next" select="/foo"/>
    <xsl:variable name="home" select="/*[1]"/>
    <xsl:variable name="up" select="parent::*"/>
    <xsl:if test="$suppress.navigation = '0'">
      <div id="header">
        <div id="header_content">
          <div id="header_left">
            <div id="header_right">
              <img src="{$kde.common}top-kde.jpg" width="36" height="34" />
              <!-- The space is for spacing between the logo and title text -->
              <xsl:text> </xsl:text>
              <xsl:apply-templates
                select="." mode="title.markup"/>
            </div>
          </div>
        </div>
      </div>

      <!-- output navigation links -->
      <div class="navCenter">
        <table class="navigation">
          <tr>
            <td class="prevCell">
              <xsl:if test="count($prev)>0">
                <a accesskey="p">
                  <xsl:attribute name="href">
                    <xsl:call-template name="href.target">
                      <xsl:with-param name="object" select="$prev"/>
                    </xsl:call-template>
                  </xsl:attribute>
                  <xsl:call-template name="gentext.nav.prev"/>
                </a>
              </xsl:if>
            </td>
            <td class="upCell">
              <xsl:choose>
                <xsl:when test="count($up) > 0 and $up != $home">
                  <xsl:apply-templates select="$up" mode="title.markup"/>
                </xsl:when>
                <xsl:otherwise>&#160;</xsl:otherwise>
              </xsl:choose>
            </td>
            <td class="nextCell">
              <xsl:if test="count($next)>0">
                <a accesskey="n">
                  <xsl:attribute name="href">
                    <xsl:call-template name="href.target">
                      <xsl:with-param name="object" select="$next"/>
                    </xsl:call-template>
                  </xsl:attribute>
                  <xsl:call-template name="gentext.nav.next"/>
                </a>
              </xsl:if>
            </td>
          </tr>
        </table>
      </div>
    </xsl:if>
  </xsl:template>

<!-- ==================================================================== -->

<xsl:template name="footer.navigation">
  <xsl:param name="prev" select="/foo"/>
  <xsl:param name="next" select="/foo"/>
  <xsl:variable name="home" select="/*[1]"/>
  <xsl:variable name="up" select="parent::*"/>

  <xsl:if test="$suppress.navigation = '0'">
    <div id="footer">
      <!-- output navigation links -->
      <div class="navCenter">
        <table class="navigation">
          <tr>
            <td class="prevCell">
              <xsl:if test="count($prev)>0">
                <a accesskey="p">
                  <xsl:attribute name="href">
                    <xsl:call-template name="href.target">
                      <xsl:with-param name="object" select="$prev"/>
                    </xsl:call-template>
                  </xsl:attribute>
                  <xsl:call-template name="gentext.nav.prev"/>
                </a>
              </xsl:if>
            </td>
            <td class="upCell">
              <xsl:choose>
                <xsl:when test="$home != .">
                  <a accesskey="h">
                    <xsl:attribute name="href">
                      <xsl:call-template name="href.target">
                        <xsl:with-param name="object" select="$home"/>
                      </xsl:call-template>
                    </xsl:attribute>
                    <xsl:call-template name="gentext.nav.home"/>
                  </a>
                </xsl:when>
                <xsl:otherwise>&#160;</xsl:otherwise>
              </xsl:choose>
            </td>
            <td class="nextCell">
              <xsl:if test="count($next)>0">
                <a accesskey="n">
                  <xsl:attribute name="href">
                    <xsl:call-template name="href.target">
                      <xsl:with-param name="object" select="$next"/>
                    </xsl:call-template>
                  </xsl:attribute>
                  <xsl:call-template name="gentext.nav.next"/>
                </a>
              </xsl:if>
            </td>
          </tr>
          <tr>
            <td class="prevCell">
              <xsl:apply-templates select="$prev" mode="title.markup"/>
              <xsl:text>&#160;</xsl:text>
            </td>
            <td class="upCell">
              <xsl:choose>
                <xsl:when test="count($up) > 0 and $up != $home">
                  <xsl:apply-templates select="$up" mode="title.markup"/>
                </xsl:when>
                <xsl:otherwise>&#160;</xsl:otherwise>
              </xsl:choose>
            </td>
            <td class="nextCell">
              <xsl:text>&#160;</xsl:text>
              <xsl:apply-templates select="$next" mode="title.markup"/>
            </td>
          </tr>
        </table>
      </div>
      <div id="footer_text">
        <xsl:call-template name="gentext.footer-doc-comment"/>
        <br/>
        <xsl:call-template name="gentext.footer-doc-feedback"/>
        <a href="mailto:{$footer.email}" class="footer_email">
          <xsl:call-template name="gentext.footer-doc-teamname"/>
        </a>
      </div>
    </div>
  </xsl:if>
</xsl:template>

<xsl:template name="gentext.footer-doc-comment"> 
  <xsl:call-template name="gentext"> 
    <xsl:with-param name="key" select="'footer-doc-comment'"/> 
  </xsl:call-template> 
</xsl:template> 
 
<xsl:template name="gentext.footer-doc-feedback"> 
  <xsl:call-template name="gentext"> 
    <xsl:with-param name="key" select="'footer-doc-feedback'"/> 
  </xsl:call-template> 
</xsl:template> 
 
<xsl:template name="gentext.footer-doc-teamname"> 
  <xsl:call-template name="gentext"> 
    <xsl:with-param name="key" select="'footer-doc-teamname'"/> 
  </xsl:call-template> 
</xsl:template> 


</xsl:stylesheet>
<!-- vim: set sw=2: -->
