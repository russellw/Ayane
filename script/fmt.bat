black %~dp0..
if %errorlevel% neq 0 goto :eof

isort %~dp0..
if %errorlevel% neq 0 goto :eof

"C:\Program Files\LLVM\bin\clang-format" -i -style=file %~dp0..\src\*.cc %~dp0..\src\*.h
if %errorlevel% neq 0 goto :eof

python %~dp0fmt-c.py %~dp0..\src
if %errorlevel% neq 0 goto :eof

git diff
