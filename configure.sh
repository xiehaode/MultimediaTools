pyinstaller -F WordToPdf/WordToPdf.py
mv WordToPdf/dist/WordToPdf.exe bin/MyConvert.exe
sh ./generate_version.sh