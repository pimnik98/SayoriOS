set -e

CLANG=$(ls /usr/bin/clang-1* | grep -o '[0-9]\+' | sort | head -n 1)
GCC=$(ls /usr/bin/gcc-* | grep -o '[0-9]\+' | sort | head -n 1)

prefix=""

if [ "$1" = "cpp" ]; then
	prefix="++"
fi

# echo $CLANG $GCC

if [ -n "$CLANG" ]; then
	compiler="clang$prefix-$CLANG -target i386-unknown-pc-none"
elif [ -n "$GCC" ]; then
	if [ "$prefix" = "" ]; then
		compiler="gcc-$GCC -m32"
	else
		compiler="g++-$GCC -m32"
	fi
else
	echo "No C compilers found!" >&2
	exit 1
fi

echo $compiler
