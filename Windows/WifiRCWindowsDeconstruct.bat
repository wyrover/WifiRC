cd ..

copy /Y "%cd%\WifiRC.vcxproj" "%cd%\Windows\WifiRC.vcxproj"
copy /Y "%cd%\WifiRC.vcxproj.filters" "%cd%\Windows\WifiRC.vcxproj.filters"
copy /Y "%cd%\WifiRC.vcxproj.user" "%cd%\Windows\WifiRC.vcxproj.user"

del "WifiRC.vcxproj"
del "WifiRC.vcxproj.filters"
del "WifiRC.vcxproj.user"

del "osFunctions.cpp"
del "osFunctions.h"
del "Serial.h"
del "spinLock.cpp"
del "spinLock.h"

del "WifiRC"
del "WifiRCCompile.bash"

cd Windows