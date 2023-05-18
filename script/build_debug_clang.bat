"C:\Program Files\LLVM\bin\clang-cl" -DDBG -Feayane -IC:\mpir -J -MTd -WX -Wimplicit-fallthrough -Wno-deprecated-declarations -Zi -std:c++17 %~dp0..\src\*.cc C:\mpir\debug.lib dbghelp.lib
