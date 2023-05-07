cl /Feayane /IC:\mpir /O2 /std:c++17 C:\ayane\*.cc C:\mpir\release.lib
if %errorlevel% neq 0 goto :eof

timer /nologo
ayane %*
timer /nologo /s
