cl /DDBG /Feayane /IC:\mpir /MP /MTd /WX /Zi /std:c++17 %~dp0..\src\*.cc C:\mpir\debug.lib dbghelp.lib
rem TODO: also check sanitizers https://nullprogram.com/blog/2023/04/29/
