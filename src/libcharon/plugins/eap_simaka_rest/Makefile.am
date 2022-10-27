AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon \
	-I$(top_srcdir)/src/libsimaka \
	-DIPSEC_CONFDIR=\"${sysconfdir}\"

AM_CFLAGS = \
	$(PLUGIN_CFLAGS) $(json_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-eap-simaka-rest.la
else
plugin_LTLIBRARIES = libstrongswan-eap-simaka-rest.la
libstrongswan_eap_simaka_rest_la_LIBADD = $(top_builddir)/src/libsimaka/libsimaka.la $(json_LIBS)
endif

libstrongswan_eap_simaka_rest_la_SOURCES = \
	eap_simaka_rest_client.h eap_simaka_rest_client.c \
	eap_simaka_rest_plugin.h eap_simaka_rest_plugin.c \
	eap_simaka_rest_provider.h eap_simaka_rest_provider.c

libstrongswan_eap_simaka_rest_la_LDFLAGS = -module -avoid-version -no-undefined
