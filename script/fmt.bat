rem Standard tools for Python
black %~dp0..
if %errorlevel% neq 0 goto :eof

isort %~dp0..
if %errorlevel% neq 0 goto :eof

rem Custom tools for Python
python %~dp0python_comments.py
if %errorlevel% neq 0 goto :eof

python %~dp0python_sort.py
if %errorlevel% neq 0 goto :eof

python %~dp0python_sortf.py
if %errorlevel% neq 0 goto :eof

python %~dp0keywords.py
if %errorlevel% neq 0 goto :eof

rem Standard tools for C++
"C:\Program Files\LLVM\bin\clang-format" -i -style=file %~dp0..\src\*.cc %~dp0..\src\*.h
if %errorlevel% neq 0 goto :eof

rem Custom tools for C++
python %~dp0fmt_c.py %~dp0..\src
if %errorlevel% neq 0 goto :eof

rem Show results
git diff
