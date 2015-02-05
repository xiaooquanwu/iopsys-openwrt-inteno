dnl init stuff needs to be first in file
AC_INIT([peripheral_manager], [0.1], [Kenneth Johansson <kenneth@southpole.se>])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AM_SILENT_RULES([yes])

AC_PROG_CC
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])


dnl check for uci , its manadatory
AC_SEARCH_LIBS([uci_load], [uci], [], [AC_MSG_ERROR([*** UCI library not found])])

dnl ubox
AC_SEARCH_LIBS([uloop_init], [ubox], [], [AC_MSG_ERROR([*** UBOX library not found])])

AC_OUTPUT

