set -e

CLANG=$(ls /usr/bin/clang-1* | grep -o '[0-9]\+' | sort | head -n 1)
GCC=$(ls /usr/bin/gcc-* | grep -o '[0-9]\+' | sort | head -n 1)

prefix=""

if [ "$1" = "cpp" ]; then
	prefix="++"
fi

# echo $CLANG $GCC

compiler="clang$prefix-$CLANG -target i386-unknown-pc-none"

echo $compiler
