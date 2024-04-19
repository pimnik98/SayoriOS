set -e

if [ -z $PREFIX ]; then
	PREFIX=/usr
fi

CLANG=$(ls $PREFIX/bin/clang-1* $PREFIX/lib/llvm16/bin/clang-1* 2> /dev/null | grep -o '[0-9]\+' | sort | head -n 1)
GCC=$(ls $PREFIX/bin/gcc-* 2> /dev/null | grep -o '[0-9]\+' | sort | head -n 1)

x64=false

while [ -n "$1" ]; do
	case "$1" in
		--64) x64=true;;
		--32) x64=false;;
	esac

	shift
done

flags=""

if [ -n "$CLANG" ]; then
	if [ $x64 = false ]; then
		flags="-target i386-unknown-pc-none"
	else
		flags="-target x86_64-unknown-pc-none"
	fi
	
	compiler="clang-$CLANG $flags"
elif [ -n "$GCC" ]; then
	if [ $x64 = false ]; then
		flags="-m64"
	else
		flags="-m32"
	fi

		compiler="gcc-$GCC $flags"
else
	echo "No C compilers found!" >&2
	exit 1
fi

echo $compiler
