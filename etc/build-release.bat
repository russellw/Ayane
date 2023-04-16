if "%VCINSTALLDIR%"=="" call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
cl /Feayane /IC:\mpir /O2 /std:c++17 C:\ayane\src\*.cc C:\mpir\release.lib
