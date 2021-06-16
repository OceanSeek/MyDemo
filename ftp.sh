#!/bin/bash

FILE_DIR="/mnt/internal_storage/dcu/"
EXE_FILE="WQ-T9001.bin"
DB_FILE="IEC104_data.db"

exePath=$FILE_DIR$EXE_FILE
dbPath=$FILE_DIR$DB_FILE
curl ftp://weitao:111@192.168.7.134/WQ-T9001.bin -o $exePath
curl ftp://weitao:111@192.168.7.134/IEC104_data.db -o $dbPath
chmod 777 $exePath
cd $FILE_DIR
./WQ-T9001.bin
