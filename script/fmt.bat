black .
if %errorlevel% neq 0 goto :eof

isort .
if %errorlevel% neq 0 goto :eof

"C:\Program Files\LLVM\bin\clang-format" -i -style=file *.cc *.h
if %errorlevel% neq 0 goto :eof

python fmt-c.py .
if %errorlevel% neq 0 goto :eof

git diff
