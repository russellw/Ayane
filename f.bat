black .
if errorlevel 1 goto :eof

isort .
if errorlevel 1 goto :eof

"C:\Program Files\LLVM\bin\clang-format" -i -style=file *.cc *.h
if errorlevel 1 goto :eof

python fmt-c.py .
if errorlevel 1 goto :eof

git diff
