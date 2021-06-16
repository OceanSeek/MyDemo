#!/bin/bash
#put this bash outside the folder
#kill the running dcu progress
pidlist=`ps -ef |grep "WQ-T9001.bin" |grep -v "grep"|awk '{print $2}'`
echo "id list:$pidlist"
if [ "$pidlist" = "" ]
then
    echo "no app pid alive" $(date "+%Y-%m-%d %H:%M:%S")>> /mnt/internal_storage/updateInfo.txt
else
    systemctl stop srv2.service
    echo "kill srv2.service" $(date "+%Y-%m-%d %H:%M:%S")>> /mnt/internal_storage/updateInfo.txt
fi

#whether the the folder 'dcu_backup' is exist ,if exist then delete it
if [ -e /mnt/internal_storage/dcu_backup ]
then
    rm -rf /mnt/internal_storage/dcu_backup
	echo "delete dcu_back_up" $(date "+%Y-%m-%d %H:%M:%S")>> /mnt/internal_storage/updateInfo.txt
fi

#change current folder name 'dcu' to 'dcu_backup'
if [ -e dcu ]
then
	mv /mnt/internal_storage/dcu /mnt/internal_storage/dcu_backup
	echo "rename dcu to dcu_backup" $(date "+%Y-%m-%d %H:%M:%S")>> /mnt/internal_storage/updateInfo.txt
fi

#put this upgrade file into folder 'dcu_update' ,rename 'dcu_update' to 'dcu'
if [ -e /mnt/internal_storage/dcu_update ]
then
    cp -r /mnt/internal_storage/dcu_update /mnt/internal_storage/dcu
	echo "rename dcu_update to dcu" $(date "+%Y-%m-%d %H:%M:%S")>> /mnt/internal_storage/updateInfo.txt
else
    #if folder 'dcu_update' not exist, copy folder 'dcu_backup' to 'dcu',then run the progress in dcu
	cp -r /mnt/internal_storage/dcu_backup /mnt/internal_storage/dcu
	cd /mnt/internal_storage/dcu
    systemctl start srv2.service
	echo "start old dcu success" $(date "+%Y-%m-%d %H:%M:%S")>> /mnt/internal_storage/updateInfo.txt
	exit 0      
fi

#start the update progress
cd /mnt/internal_storage/dcu
systemctl start srv2.service
cd ..
echo "start new dcu success" $(date "+%Y-%m-%d %H:%M:%S")>> /mnt/internal_storage/updateInfo.txt

#wheter the new program start normally,if it start normal, then generate a new flag 'update successful'
#if upgrade failed, delete current folder 'dcu', cover the folder 'dcu' by 'dcu_backup'
updatepid=`ps -ef |grep "WQ-T9001.bin" |grep -v "grep"|awk '{print $2}'`
if [ "$updatepid" = "" ]
then
	rm -rf /mnt/internal_storage/dcu
	cp -r /mnt/internal_storage/dcu_backup /mnt/internal_storage/dcu
	cd /mnt/internal_storage/dcu
    systemctl start srv2.service
    echo "can not find WQ-T9001.bin" $(date "+%Y-%m-%d %H:%M:%S")>> /mnt/internal_storage/updateInfo.txt
	exit 0
else
    echo "update_success" $(date "+%Y-%m-%d %H:%M:%S")>> /mnt/internal_storage/updateInfo.txt
fi
