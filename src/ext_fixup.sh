#!/usr/bin/env sh
#
# dyld is hard.  lets go shopping.
#

echo Rewriting libpython.dylib link paths for .bundles in: $1
for b in $1/*.bundle ; do
  AFTER='@loader_path/../libpython_bp.dylib'
  BEFORE=`otool -LX $b | grep libpython | sed -E -e 's/^[^/]*\//\//g' -e 's/dylib .*$/dylib/'`
  install_name_tool -change "${BEFORE}" "${AFTER}" $b
done

for b in $1/*/*.bundle ; do
  AFTER='@loader_path/../../libpython_bp.dylib'
  BEFORE=`otool -LX $b | grep libpython | sed -E -e 's/^[^/]*\//\//g' -e 's/dylib .*$/dylib/'`
  install_name_tool -change "${BEFORE}" "${AFTER}" $b
done

for b in $1/*/*/*.bundle ; do
  AFTER='@loader_path/../../libpython_bp.dylib'
  BEFORE=`otool -LX $b | grep libpython | sed -E -e 's/^[^/]*\//\//g' -e 's/dylib .*$/dylib/'`
  install_name_tool -change "${BEFORE}" "${AFTER}" $b
done

