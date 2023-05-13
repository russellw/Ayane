import os

for root, dirs, files in os.walk(arg):
    for file in files:
        if "-" in file:
            file1 = file.replace("-", "_")
            os.rename(file, file1)
