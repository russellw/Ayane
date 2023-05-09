cl /DDBG /Feayane /IC:\mpir /c /nologo /std:c++17 %~dp0..\src\*.cc >1
head -n50 1
