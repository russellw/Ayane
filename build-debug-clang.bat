rem TODO: still need no-switch?
"C:\Program Files\LLVM\bin\clang-cl" /DDEBUG /Feayane /IC:\mpir /std:c++17 -Wimplicit-fallthrough -Wno-deprecated-declarations -Wno-switch -ferror-limit=1 C:\ayane\*.cc C:\mpir\debug.lib dbghelp.lib
