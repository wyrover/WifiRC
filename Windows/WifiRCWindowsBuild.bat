call "WifiRCWindowsDeconstruct.bat"

cd ..

copy "%cd%\Windows\WifiRC.vcxproj" "%cd%\WifiRC.vcxproj"
copy "%cd%\Windows\WifiRC.vcxproj.filters" "%cd%\WifiRC.vcxproj.filters"
copy "%cd%\Windows\WifiRC.vcxproj.user" "%cd%\WifiRC.vcxproj.user"

copy "%cd%\Windows\osFunctions.cpp" "%cd%\osFunctions.cpp"
copy "%cd%\Windows\osFunctions.h" "%cd%\osFunctions.h"
copy "%cd%\Windows\Serial.h" "%cd%\Serial.h"
copy "%cd%\Windows\spinLock.cpp" "%cd%\spinLock.cpp"
copy "%cd%\Windows\spinLock.h" "%cd%\spinLock.h"
cd Windows