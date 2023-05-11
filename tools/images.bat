cd ..\images
..\tools\Images2Code.exe
del ..\FdsKey\Core\Inc\images\*.h
mv *.h ..\FdsKey\Core\Inc\images 
cd ..\FdsKey\Core\Inc\images 
..\..\..\..\tools\ImageParser.exe

cd ..\..\..\..\tools

cd ..\images_bootloader
..\tools\Images2Code.exe
del ..\FdsKey_bootloader\Core\Inc\images\*.h
mv *.h ..\FdsKey_bootloader\Core\Inc\images 
cd ..\FdsKey_bootloader\Core\Inc\images 
..\..\..\..\tools\ImageParser.exe

