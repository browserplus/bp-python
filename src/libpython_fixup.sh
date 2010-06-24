#!/usr/bin/env sh

echo working on $1
AFTER='@loader_path/libpython2.6_bp.dylib'
BEFORE=`otool -LX $1 | grep libpython | sed -E -e 's/^[^/]*\//\//g' -e 's/dylib .*$/dylib/'`
install_name_tool -id libpython2.6_bp.dylib -change "${BEFORE}" "${AFTER}" $1
