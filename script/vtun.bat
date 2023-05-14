if [%~1]==[] goto :eof

cl /Feayane /IC:\mpir /MP /MTd /Ox /WX /Zi /std:c++17 %~dp0..\src\*.cc C:\mpir\debug.lib dbghelp.lib
if %errorlevel% neq 0 goto :eof

"C:\Program Files (x86)\Intel\oneAPI\vtune\latest\bin64\vtune" -collect hotspots -user-data-dir %tmp% ayane.exe %*
