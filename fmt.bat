black etc
if errorlevel 1 goto :eof

isort etc
if errorlevel 1 goto :eof

clang-format -i -style=file src\*.h src\*.cc
if errorlevel 1 goto :eof

python etc\fmt-c.py src
if errorlevel 1 goto :eof

git diff
