black C:\ayane\scripts
if errorlevel 1 goto :eof

clang-format -i -style=file C:\ayane\src\*.h C:\ayane\src\*.cc
if errorlevel 1 goto :eof

python C:\ayane\scripts\fmt-c.py C:\ayane\src
if errorlevel 1 goto :eof

git diff
