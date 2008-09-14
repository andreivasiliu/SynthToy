#!/bin/sh

set -e -x

aclocal
libtoolize -c
autoconf
autoheader
automake -ac
