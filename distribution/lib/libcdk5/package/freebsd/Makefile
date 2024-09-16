# Created by: Andrey Zakhvatov
# $FreeBSD: head/devel/cdk/Makefile 439311 2017-04-24 15:17:34Z sunpoet $

PORTNAME=	cdk
DISTVERSION=	5.0-20230201
PORTEPOCH=	1
CATEGORIES=	devel
MASTER_SITES=	ftp://ftp.invisible-island.net/cdk/

MAINTAINER=	ports@FreeBSD.org
COMMENT=	Curses Development Kit for speedy development of full screen programs

LICENSE=	MIT
LICENSE_FILE=	${WRKSRC}/COPYING

USES=		gmake ncurses tar:tgz
GNU_CONFIGURE=	yes
PORTDOCS=	EXPANDING INSTALL NOTES README TODO
MAKE_JOBS_UNSAFE=	yes

CONFIGURE_ARGS+=--with-ncurses
ALL_TARGET=	default cdkshlib
INSTALL_TARGET=	install installCDKSHLibrary
USE_LDCONFIG=	yes

OPTIONS_DEFINE=	DOCS EXAMPLES

post-patch:
	@${REINPLACE_CMD} -e 's|gcc|$$(CC)| ; \
		 /^CDKREADME/s| COPYING||' ${WRKSRC}/Makefile.in

post-install:
	${STRIP_CMD} ${STAGEDIR}${PREFIX}/lib/libcdk*.so

post-install-EXAMPLES-on:
	${MKDIR} ${STAGEDIR}${EXAMPLESDIR}
	(cd ${WRKSRC} && \
		${COPYTREE_SHARE} "cli demos examples" \
		${STAGEDIR}${EXAMPLESDIR})

.include <bsd.port.mk>
