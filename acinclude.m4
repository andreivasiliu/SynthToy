
dnl MSK_CHECK_OPTIONAL_MODULE(LOWERCASE-NAME, PRETTY-NAME, VAR, PKG-MODULES)
AC_DEFUN([MSK_CHECK_OPTIONAL_MODULE],
  [AC_ARG_WITH($1, AC_HELP_STRING(--with-$1, enable support for $2 [[[default=check]]]),
   [], with_$1=check)
   eval __with_mod_name=[$with_]$1
   AS_IF(
     test "x$__with_mod_name" == "xyes",
     [PKG_CHECK_MODULES]($3, $4),
     test "x$__with_mod_name" == "xcheck",
     [PKG_CHECK_MODULES]($3, $4, [], have_$1=no),
     have_$1=no)
   AS_IF(test "x[$have_]$1" != "xno",
     [AC_DEFINE](HAVE_$3, [1], [Define if you have] $2)
     have_$1=yes)
  ])

