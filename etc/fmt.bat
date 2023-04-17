black C:\ayane\etc
if errorlevel 1 goto :eof

isort C:\ayane\etc
if errorlevel 1 goto :eof

clang-format -i -style=file C:\ayane\src\*.h C:\ayane\src\*.cc
if errorlevel 1 goto :eof

python C:\ayane\etc\fmt-c.py C:\ayane\src
if errorlevel 1 goto :eof

git diff
