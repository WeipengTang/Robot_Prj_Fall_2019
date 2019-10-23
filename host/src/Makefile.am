## Process this file with automake to produce Makefile.in

## Created by Anjuta

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir). \
	$(pthread-stubs_CFLAGS). \
	$(pthread-stubs_CFLAGS)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\"

AM_CFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = host_control

host_control_SOURCES = \
	main.c \
	serial.c \
	serial.h \
	joystick.c \
	joystick.h \
	utilities.c \
	control_functions.h \
	control_functions.c \
	utilities.h \
	com_manage.c \
	com_manage.h \
	socket_client.c \
	socket_client.h

host_control_LDFLAGS = 

host_control_LDADD =  \
	$(pthread-stubs_LIBS)

