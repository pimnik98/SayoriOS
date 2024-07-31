echo "Autoexec BAT"

echo "Hello," %USERNAME%
echo "Computer:" %HOSTNAME%
echo "System Path:" %SYSTEMROOT%
echo "Temp Path:" %TEMP%
echo "Build Info:" %BUILDUSER% "|" %VERNAME% %SUBVERSIONNAME% "|" %VERSION% "|" %BUILDDATA%
if %BUILDUSER% == "pimnik98" goto pimnik98
if %BUILDUSER% == "ndraey" goto ndraey

:ndraey
echo "Welcome to ndraey"

:pimnik98
echo "Welcome to pimnik98"