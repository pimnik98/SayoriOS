GRUB=$(ls $PREFIX/bin/grub-mkrescue 2> /dev/null)
GRUB2=$(ls $PREFIX/bin/grub2-mkrescue 2> /dev/null)

if [ -n "$GRUB2" ]; then
	echo $GRUB2
elif [ -n "$GRUB" ]; then
	echo $GRUB
fi