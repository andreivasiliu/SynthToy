#!/bin/sh

set -e -x

aclocal
autoconf
automake -ac
autoheader
libtoolize -c
