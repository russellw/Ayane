if "%VCINSTALLDIR%"=="" call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
cl /Feayane /IC:\mpir /O2 /std:c++17 C:\ayane\*.cc C:\mpir\release.lib
