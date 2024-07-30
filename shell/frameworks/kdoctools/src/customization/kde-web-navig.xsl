<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
            	version="1.0">


<xsl:template name="header.navigation">
  <xsl:param name="prev" select="/foo"/>
  <xsl:param name="next" select="/foo"/>
  <xsl:variable name="home" select="/*[1]"/>
  <xsl:variable name="up" select="parent::*"/>
  <xsl:if test="$suppress.navigation = '0'">
	      <table border="0" cellpadding="0" cellspacing="0" width="100%">
		<tr class="header">
		  <td colspan="2">&#160;</td>
		</tr>
		<tr id="logo">
		  <td valign="top">
		    <img src="{$kde.common}part_of_the_kde_family_horizontal_190.png" alt="Part of the KDE family"
		      width="190" height="68" border="0" />
		  </td>
		  <td valign="middle" align="center" id="location">
		    <h1><!-- Why was this used in fact: could the text not just be copied?-->
		      <xsl:apply-templates select="." mode="title.markup"/>
		    </h1>
		  </td>
		</tr>
	      </table>

	      <table width="100%" class="header">
		<tbody>
		  <tr>
		    <td align="left" class="navLeft" width="33%">
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
		    
		    <td align="center" class="navCenter" width="34%">
		      <xsl:choose>
			<xsl:when test="count($up) > 0 and $up != $home">
			  <xsl:apply-templates select="$up" mode="title.markup"/>
			  </xsl:when>
			  <xsl:otherwise>&#160;</xsl:otherwise>
			</xsl:choose>
		    </td>
		    <td align="right" class="navRight" width="33%">&#160;
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
		</tbody>
	      </table>

	    </xsl:if>
	  </xsl:template>

<!-- ==================================================================== -->

<xsl:template name="footer.navigation">
  <xsl:param name="prev" select="/foo"/>
  <xsl:param name="next" select="/foo"/>
  <xsl:variable name="home" select="/*[1]"/>
  <xsl:variable name="up" select="parent::*"/>

  <xsl:if test="$suppress.navigation = '0'">

    <!-- Bottom Navigation -->
       <table width="100%" class="bottom-nav">
	<tr>
          <td width="33%" align="left" valign="top" class="navLeft">
            <xsl:if test="count($prev)>0">
              <a>
                <xsl:attribute name="href">
                  <xsl:call-template name="href.target">
                    <xsl:with-param name="object" select="$prev"/>
                  </xsl:call-template>
                </xsl:attribute>
                <xsl:call-template name="gentext.nav.prev"/>
              </a>
            </xsl:if>
          </td>
          <td width="34%" align="center" valign="top" class="navCenter">
            <xsl:choose>
              <xsl:when test="$home != .">
                <a>
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

          <td width="33%" align="right" valign="top" class="navRight">
            <xsl:if test="count($next)>0">
              <a>
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
          <td width="33%" align="left" class="navLeft">
            <xsl:apply-templates select="$prev" mode="title.markup"/>
            <xsl:text>&#160;</xsl:text>
          </td>
          <td width="34%" align="center" class="navCenter">
            <xsl:choose>
              <xsl:when test="count($up)>0">
                <a>
                  <xsl:attribute name="href">
                    <xsl:call-template name="href.target">
                      <xsl:with-param name="object" select="$up"/>
                    </xsl:call-template>
                  </xsl:attribute>
                  <xsl:call-template name="gentext.nav.up"/>
                </a>
              </xsl:when>
              <xsl:otherwise>&#160;</xsl:otherwise>
            </xsl:choose>
          </td>
          <td width="33%" align="right" class="navRight">
            <xsl:text>&#160;</xsl:text>
            <xsl:apply-templates select="$next" mode="title.markup"/>
          </td>
        </tr>
      </table>

  </xsl:if>
</xsl:template>


</xsl:stylesheet>
