#!/bin/bash
#put this bash outside the folder
#kill the running dcu progress
LOG_FILE=/mnt/internal_storage/supervisor_sh.log
maxLogSize=$((1024*10))

while [ 1 ]
do
	pidlist=`ps -ef |grep "WQ-T9001.bin" |grep -v "grep"|awk '{print $2}'`
	#echo "id list:$pidlist"
	if [ "$pidlist" = "" ];then
		echo $(date "+%Y-%m-%d %H:%M:%S") "--no app 'WQ-T9001' pid alive, start to restart srv2.service" >> ${LOG_FILE}
		systemctl restart srv2.service
		echo $(date "+%Y-%m-%d %H:%M:%S") "--no app 'WQ-T9001' pid alive, start to restart srv2.service"
	fi
	sleep 10
	logfilesize=`ls -l $LOG_FILE | awk '{ print $5 }'`
	# echo "logfilesize is:$logfilesize " 
	if [ $logfilesize -gt $maxLogSize ]
	then
		echo "$logfilesize > $maxLogSize"
		mv /mnt/internal_storage/supervisor_sh.log /mnt/internal_storage/supervisor_sh_old.log
	fi
done

