rm example || true
make example

valgrind --leak-check=full --track-origins=yes ./example

./example
