#include "sqliteHelper.h"

#ifndef USE_IN_ANDROID
int select_callback(void *arg, int nr, char **values, char **names)
{
    int i;
    char str[128];
    for (i=0; i<nr; i++)
	{
        printf("\t%s\t", values[i]);
    }
	printf("\n"); //д��һ����¼����
    return 0;  //callback������������0
}

/*
�ص�ʽ��ѯ����ѯ����ڻص������С�
*/
int select_by_callback(sqlite3 *db, const char* sql)
{
	char *zErrMsg = 0;
	int nResult;
	nResult = sqlite3_exec(db, sql, select_callback, NULL, &zErrMsg);
	if (nResult != SQLITE_OK)
	{
		fprintf(stderr, "%s: %s\n", sql, sqlite3_errmsg(db));
		if(zErrMsg)  
            fprintf(stderr,"ErrMsg = %s \n", zErrMsg);
        //sqlite3_close(db);
        return -1;
	}
}


/*
�ǻص�ʽ��ѯ����ѯ����洢��ppResult��
ppResult�ڴ沼��Ϊ���ֶ����ƣ�����ΪnCol����������ÿ���ֶε�ֵ��nCol��nRow������nCol����nRow+1������
��������ʾ��ppResult��nColΪ2��nRowΪ4����ppResult��һά����Ԫ��Ϊ10����
id	data	
1	lines	
2	line	
3	line	
4	line	
*/
int select_by_table(sqlite3 *db, const char* sql)
{
	int i = 0 ;
    int j = 0 ;
	
	int nRow = 0;
	int nCol = 0;
	char** ppResult;//��ά�����Ž���������ʵ��һά����
	int nResult;
	char *zErrMsg = 0;
	
	nResult = sqlite3_get_table(db, sql, &ppResult, &nRow, &nCol, &zErrMsg);
	if (nResult != SQLITE_OK)
	{
		fprintf(stderr, "%s: %s\n", sql, sqlite3_errmsg(db));
		if(zErrMsg)  
            fprintf(stderr,"ErrMsg = %s \n", zErrMsg);
        sqlite3_free_table(ppResult);
        //sqlite3_close(db);
        return -1;
	}
	printf("nRow:%d,nCol:%d\n",nRow,nCol);
	if (nRow == 0)
	{
		printf("[select_by_table]no result");
		return -1;
	}
	
	for(j=0;j<nCol;j++)
	{
		printf("%s	",ppResult[j]);
	}
	printf("\n");
	
    for(i=0;i<nRow;i++)
    {
        for(j=0;j<nCol;j++)
        {
			printf("%s	",ppResult[(i+1)*nCol+j]);
        }
		printf("\n");
    }
	
	sqlite3_free_table(ppResult);
	return 0;
}

int select_by_table_with_name(sqlite3 *db, const char* sql, UserInfo *userInfo)
{
	int i = 0 ;
    int j = 0 ;
	
	int nRow = 0;
	int nCol = 0;
	char** ppResult;//��ά�����Ž���������ʵ��һά����
	int nResult;
	char *zErrMsg = 0;
	
	nResult = sqlite3_get_table(db, sql, &ppResult, &nRow, &nCol, &zErrMsg);
	if (nResult != SQLITE_OK)
	{
		fprintf(stderr, "%s: %s\n", sql, sqlite3_errmsg(db));
		if(zErrMsg)  
            fprintf(stderr,"ErrMsg = %s \n", zErrMsg);
        sqlite3_free_table(ppResult);
        //sqlite3_close(db);
        return -1;
	}
	printf("nRow:%d,nCol:%d\n",nRow,nCol);
	if (nRow == 0 || nRow > 1)
	{
		printf("[select_by_table]no result or more than one result");
		return -1;
	}
	
    for(j=0;j<nCol;j++)
    {
		printf("%s	",ppResult[nCol+j]);
    }
	printf("\n");

	strcpy(userInfo->name, ppResult[nCol+1]);
	userInfo->money = atoi(ppResult[nCol+2]);
	
	sqlite3_free_table(ppResult);
	return 0;
}
#endif
