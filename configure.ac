AC_PREREQ([2.69])
AC_INIT([twitterfu], [1.0.1])
AM_INIT_AUTOMAKE([1.9])
AC_CONFIG_SRCDIR([src/twitterfu.cpp])
AC_CONFIG_HEADERS([src/config.h])

# Checks for programs.
AC_PROG_CXXCPP
AC_PROG_CXX
AC_LANG([C++])

# Checks for libraries.
AC_CHECK_LIB([twitcurl], [main])
AC_CHECK_LIB([sqlite3], [sqlite3_open])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE

# Additional compiler linker flags
AC_SUBST([AM_CXXFLAGS])
AC_SUBST([AM_LDFLAGS])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
