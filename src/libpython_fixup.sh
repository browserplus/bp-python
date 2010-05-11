#!/usr/bin/env sh

echo working on $1
AFTER='@loader_path/libpython_bp.dylib'
BEFORE=`otool -LX $1 | grep libpython | sed -E -e 's/^[^/]*\//\//g' -e 's/dylib .*$/dylib/'`
install_name_tool -id libpython_bp.dylib -change "${BEFORE}" "${AFTER}" $1
