if "%VCINSTALLDIR%"=="" call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
r:
cd \
cl /DDEBUG /IC:\mpir /c /nologo /std:c++17 C:\ayane\*.cc >1
head 1
c:
