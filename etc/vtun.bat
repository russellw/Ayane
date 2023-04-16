if "%VCINSTALLDIR%"=="" call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
if [%~1]==[] goto :eof
cl /Feayane /IC:\mpir /MTd /O2 /Zi /std:c++17 C:\ayane\src\*.cc C:\mpir\debug.lib dbghelp.lib
"C:\Program Files (x86)\Intel\oneAPI\vtune\latest\bin64\vtune" -collect hotspots -user-data-dir \temp ayane.exe %*
