if [%~1]==[] goto :eof
cl /Feayane /IC:\mpir /MTd /O2 /Zi /std:c++17 C:\ayane\*.cc C:\mpir\debug.lib dbghelp.lib
"C:\Program Files (x86)\Intel\oneAPI\vtune\latest\bin64\vtune" -collect hotspots -user-data-dir %tmp% ayane.exe %*
