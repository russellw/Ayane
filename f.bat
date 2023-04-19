black .
if errorlevel 1 goto :eof

isort .
if errorlevel 1 goto :eof

clang-format -i -style=file *.cc *.h
if errorlevel 1 goto :eof

python fmt-c.py .
if errorlevel 1 goto :eof

git diff
