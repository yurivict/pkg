libcore_la_SOURCES=	p_string.c \
			p_array.c \
			p_hash.c
libcore_static_la_SOURCES=	$(libcore_la_SOURCES)
libcore_la_CFLAGS=	-shared
libcore_static_la_LDFLAGS=	-all-static
libcore_static_la_CFLAGS=	-static

noinst_LTLIBRARIES=	libcore.la libcore_static.la
noinst_HEADERS=		p_string.h p_array.h p_hash.h
