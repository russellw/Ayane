@echo off

rem Backup
copy %~dp0*.py %tmp% >nul:
copy %~dp0..\test\*.py %tmp% >nul:
copy %~dp0..\src\*.cc %tmp% >nul:
copy %~dp0..\src\*.h %tmp% >nul:

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

python %~dp0keywords.py
if %errorlevel% neq 0 goto :eof

rem Restore standard line spacing
black %~dp0..
if %errorlevel% neq 0 goto :eof

rem Standard tools for C++
"C:\Program Files\LLVM\bin\clang-format" -i -style=file %~dp0..\src\*.cc %~dp0..\src\*.h
if %errorlevel% neq 0 goto :eof

rem Custom tools for C++
python %~dp0c_comments.py
if %errorlevel% neq 0 goto :eof

python %~dp0c_sort_case_labels.py
if %errorlevel% neq 0 goto :eof

python %~dp0c_sort_case_blocks.py
if %errorlevel% neq 0 goto :eof

python %~dp0c_sort.py
if %errorlevel% neq 0 goto :eof

rem Show results
git diff
