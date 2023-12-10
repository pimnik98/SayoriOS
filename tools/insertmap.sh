OBJCOPY="${OBJCOPY:-objcopy}"
content=$(mktemp)

cat kernel.map > "$content"

"$OBJCOPY" --update-section .debug_symbols="$content" iso/boot/kernel.elf

rm "$content"
