#!/bin/bash
#此脚本放在dcu文件夹的外面
#杀掉正在运行的dcu进程
pidlist=`ps -ef |grep "WQ-T9001.bin" |grep -v "grep"|awk '{print $2}'`
echo "id list:$pidlist"
if [ "$pidlist" = "" ]
then
    echo "no app pid alive" $(date "+%Y-%m-%d %H:%M:%S")>> /mnt/internal_storage/updateInfo.txt
else
    systemctl stop srv2.service
    echo "kill srv2.service" $(date "+%Y-%m-%d %H:%M:%S")>> /mnt/internal_storage/updateInfo.txt
fi

#查看dcu_backup文件夹是否存在，如果存在则删除
if [ -e /mnt/internal_storage/dcu_backup ]
then
    rm -rf /mnt/internal_storage/dcu_backup
	echo "delete dcu_back_up" $(date "+%Y-%m-%d %H:%M:%S")>> /mnt/internal_storage/updateInfo.txt
fi

#将当前dcu文件夹改名成dcu_backup
if [ -e dcu ]
then
	mv /mnt/internal_storage/dcu /mnt/internal_storage/dcu_backup
	echo "rename dcu to dcu_backup" $(date "+%Y-%m-%d %H:%M:%S")>> /mnt/internal_storage/updateInfo.txt
fi

#升级文件放在dcu_update中，将dcu_update改名成dcu
if [ -e /mnt/internal_storage/dcu_update ]
then
    cp -r /mnt/internal_storage/dcu_update /mnt/internal_storage/dcu
	echo "rename dcu_update to dcu" $(date "+%Y-%m-%d %H:%M:%S")>> /mnt/internal_storage/updateInfo.txt
else
    #如果dcu_update文件夹不存在，则将dcu_backup文件夹拷贝到dcu，启动里面的dcu程序
	cp -r /mnt/internal_storage/dcu_backup /mnt/internal_storage/dcu
	cd /mnt/internal_storage/dcu
    systemctl start srv2.service
	# ./WQ-T9001.bin &   #加"&"，让dcu后台运行
	echo "start old dcu success" $(date "+%Y-%m-%d %H:%M:%S")>> /mnt/internal_storage/updateInfo.txt
	exit 0      #退出
fi

#启动升级后的dcu
cd /mnt/internal_storage/dcu
# ./WQ-T9001.bin &    #加"&"，让dcu后台运行
systemctl start srv2.service
cd ..
echo "start new dcu success" $(date "+%Y-%m-%d %H:%M:%S")>> /mnt/internal_storage/updateInfo.txt

#检查升级后的dcu是否正常启动，如果正常启动则生成一个标识升级成功的文件，退出
#如果没有检测到升级后的dcu进程，则删除当前dcu文件夹，拷贝备份程序为当前dcu
updatepid=`ps -ef |grep "WQ-T9001.bin" |grep -v "grep"|awk '{print $2}'`
if [ "$updatepid" = "" ]
then
    #没有检测到升级后的dcu进程
	rm -rf /mnt/internal_storage/dcu
	cp -r /mnt/internal_storage/dcu_backup /mnt/internal_storage/dcu
	cd /mnt/internal_storage/dcu
	# ./WQ-T9001.bin &   #加"&"，让dcu后台运行
    systemctl start srv2.service
    echo "can not find WQ-T9001.bin" $(date "+%Y-%m-%d %H:%M:%S")>> /mnt/internal_storage/updateInfo.txt
	exit 0
else
    #升级成功，创建update_success空文件
    #dcu启动后检测时候有此文件，如果有，则发送升级成功消息，并删除此文件；如果没有，则发送升级失败消息
    echo "update_success" $(date "+%Y-%m-%d %H:%M:%S")>> /mnt/internal_storage/updateInfo.txt
fi
