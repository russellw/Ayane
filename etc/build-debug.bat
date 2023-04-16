if "%VCINSTALLDIR%"=="" call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

rem Try compiling with clang first, to get a second opinion on warnings/errors
rem clang-cl /DDEBUG /IC:\mpir /std:c++17 -Werror -Wimplicit-fallthrough -Wno-assume -Wno-deprecated-declarations -Wno-switch -c -ferror-limit=1 C:\ayane\src\*.cc
rem if errorlevel 1 goto :eof

rem Then compile with Microsoft C++
cl /DDEBUG /Feayane /IC:\mpir /MP /MTd /Os /WX /Zi /std:c++17 C:\ayane\src\*.cc C:\mpir\debug.lib dbghelp.lib
