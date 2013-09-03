#include "FileOperation.h"



//载入 STRING->INT 结构文件 放入哈希表
bool LoadKeysCnts(const char* pchFileName,hash_map<string,int> &hmStrCnt)
{

	FILE *pFile = fopen(pchFileName, "rb");
	if (pFile == NULL)
	{		
		return false;
	}	

	char chBuffer[1024];
	char word[100];
	int nIndex;
	while (fgets(chBuffer, 1024, pFile) != NULL)
	{
		sscanf(chBuffer, "%s %d", word, &nIndex);		
		hmStrCnt[word] = nIndex;	
	}
	fclose(pFile);	
	return true;
}
