# Different stylesheets and how to use them

## kde-chunk.xsl

The standard KDE stylesheet, as used to generate the content for
KHelpCenter.  If you do not specify a stylesheet, this is the default.

This stylesheet includes `kde-navig.xsl`, which controls the
presentation of the top and bottom of the page graphics, and is reused
in other places.  You should probably not call `kde-navig.xsl` on it's own.

## kde-nochunk.xsl

This is very similar to the standard KDE stylesheet, but it creates
one single html file for the entire document.  This is one way to get
print output, using `html2pdf` or `html2ps`.  It's also usable to create
a "printable version" of a document to link to on a website.

## kde-style.xsl

All KDE "look and feel" customizations (other than the navigation
graphics in `kde-navig.xsl`) are here. This is where changes to how
things render are placed.  In most cases, the actual rendering is
controlled by the CSS files, and this file simply has the instructions
to ensure class attributes are written into the HTML as necessary, for
the CSS to display.

## kde-ttlpg.xsl

The Title Page layout for the KDE documentation.  This one covers
customizing the TOC, presentation of legal notices, and a slightly
different navigation graphic.

## kde-web.xsl
(This stylesheet includes `kde-web-navig.xsl`, which should not be used
directly on it's own)

Simplified version of the KDE stylesheet, aimed at websites.  Content
is constrained to a 680px wide table.  You could use this for example,
to generate web pages that will print nicely and display on older
browsers nicely.

## kde-chunk-online.xsl
(This stylesheet includes `kde-web-navig-online.xsl`, which should not
be used directly on it's own)

Slightly altered version of the KDE stylesheet, used to generate the
docs.kde.org website, among others.

## kde-man.xsl

A stylesheet for generating *roff output (for manpages) from DocBook
files. It's virtually the same as the original manpages/docbook.xsl file
from the DocBook XSL package, except that the output file's name is
hardcoded to 'manpage.troff', and the 'Writing manpage.troff' message
is suppressed.

