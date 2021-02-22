#include <stdio.h>
#include <stdlib.h>
#include "sqlite3.h"
#include "insert.h"

int Sqlite(void)
{
	sqlite3 *db = NULL;
	char *zErrMsg = 0;
	int rc;
	char *sql;

	rc = sqlite3_open("IEC104_data.db",&db);//打开数据库，如果不存在就创建一个
	if(rc){
		fprintf(stderr,"can't open database:%s \n",sqlite3_errmsg(db));
		sqlite3_close(db);
		return 0;
	}
	else 
		printf("You have open a sqliet3 database named IEC104_data.db successful!\n");
	//创建表，如果该表存在则不创建，并给出提示信息，存储在zErrMsg中
	sql = "CREATE TABLE Table_YXDev1(\
			ID INTEGER PRIMARY KEY,\
			YaoXinName TEXT,\
			Prio INTEGER,\
			Time VARCHAR(12),\
			YX_DATA REAL \
			);";

	sqlite3_exec(db,sql,0,0,&zErrMsg);
	printf("create table name is (Table_YXDev1)\n");
	//插入数据
	sql = "INSERT INTO \"Table_YXDev1\" VALUES(null,'YX1',1,'20190719',1);";
	sqlite3_exec(db,sql,0,0,&zErrMsg);

	sql = "INSERT INTO \"Table_YXDev1\" VALUES(null,'YX2',1,'20190710',22.9);";
	sqlite3_exec(db,sql,0,0,&zErrMsg);

	sql = "INSERT INTO \"Table_YXDev1\" VALUES(null,'YX3',1,'20190711',26.9);";
	sqlite3_exec(db,sql,0,0,&zErrMsg);

	int nrow=0,ncolumn=0;
	char **azResult;//二维数组存放结果

	//查询数据
	sql = "SELECT * FROM Table_YXDev1";
	sqlite3_get_table(db,sql,&azResult,&nrow,&ncolumn,&zErrMsg);

	int i=0;

	printf("row:%d,column:%d\n",nrow,ncolumn);
	printf("\nthe result of querying is :\n");

	for(i=0;i<(nrow+1)*ncolumn;i++){
		printf("azResult[%d] = %s\n",i,azResult[i]);
	}

	sqlite3_free_table(azResult);

	//条件查询数据
	sql = "SELECT * FROM Table_YXDev1 where YX_DATA BETWEEN 2 AND 30 ";
	sqlite3_get_table(db,sql,&azResult,&nrow,&ncolumn,&zErrMsg);

	
	printf("row:%d,column:%d\n",nrow,ncolumn);
	printf("\nthe result of tiaojianchanxun is :\n");

	for(i=0;i<(nrow+1)*ncolumn;i++){
		printf("azResult[%d] = %s\n",i,azResult[i]);
	}

	sqlite3_free_table(azResult);

	

	printf("ERORR_MSG is %s\n",zErrMsg);
	
	sqlite3_close(db);
	return 0;
	
}

