#!/bin/bash

rm -rf temp
mkdir -p temp
mkdir -p temp/DEBIAN
touch temp/DEBIAN/{control,postinst,postrm}
chmod -R 755 temp/DEBIAN/

echo "Package: appdemo" > temp/DEBIAN/control
echo "Version: 1.1.001" >> temp/DEBIAN/control
echo "Architecture: arm64" >> temp/DEBIAN/control
echo "Section: utils" >> temp/DEBIAN/control
echo "Maintainer: huawei <huawei@huawei.com>" >> temp/DEBIAN/control
echo "Installed-Size: 2000" >> temp/DEBIAN/control
echo "Priority: optional" >> temp/DEBIAN/control
echo "Description: this is app SWQ" >> temp/DEBIAN/control

echo "#!/bin/bash" > temp/DEBIAN/postinst
echo "mkdir -p /app_demo_1" >> temp/DEBIAN/postinst

echo "#!/bin/bash" > temp/DEBIAN/postrm
echo "rm -rf /app_demo_1" >> temp/DEBIAN/postrm

mkdir -p temp/lib/systemd/system/
mkdir -p temp/usr/local/bin/
mkdir -p temp/usr/local/lib/
mkdir -p temp/usr/bin/
mkdir -p temp/lib/
mkdir -p temp/mnt/internal_storage/
mkdir -p temp/mnt/internal_storage/dcu/
mkdir -p temp/mnt/internal_storage/dcu_backup/
mkdir -p temp/mnt/internal_storage/dcu_update/

#cp helloworld temp/usr/bin/
#cp helloworld temp/usr/bin/
# cp WQ-T9001.bin temp/usr/bin/WQ-T9001.bin
# cp IEC104_data.db temp/usr/bin/
cp WQ-T9001.bin temp/mnt/internal_storage/dcu/
cp IEC104_data.db temp/mnt/internal_storage/dcu/
cp IEC104_data.db temp/mnt/internal_storage/dcu_update/
cp remote_update.sh temp/mnt/internal_storage
cp sqlite3/bin/sqlite3 temp/usr/bin/
cp -r sqlite3/lib/* temp/lib/
#cp libdatabase.so temp/lib/libdatabase1.so
cp srv2.service temp/lib/systemd/system/
#cp app1srv2.service temp/lib/systemd/system/
 
# chmod a+x temp/usr/bin/WQ-T9001.bin
chmod a+x temp/mnt/internal_storage/dcu/WQ-T9001.bin
chmod a+x temp/mnt/internal_storage/remote_update.sh
#chmod a+x temp/usr/bin/app1srv2
chmod 0644 ./temp/lib -R
chmod -x ./temp/lib/libsqlite3.so
chmod -x ./temp/lib/libsqlite3.so.0
chmod 0755 ./temp/usr -R

dpkg -b temp/ appdemo.deb
if [ $? = 0 ]; then
echo "create deb file success!"
create-ova -t app -d appdemo.deb -n appdemo -s srv2 40 50 -version_short SV01.002 -ar arm64 -ot debian -ov buster -vendor huawei
echo "create ova file success!"
cp appdemo.ova ../
echo "cp appdemo.ova ../"
exit 0
else
echo "create deb file failed!"
exit 0
fi
