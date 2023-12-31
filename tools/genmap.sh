nm -Cl -n iso/boot/kernel.elf | awk '{ if ($2 != "a") print; }' > kernel.map
