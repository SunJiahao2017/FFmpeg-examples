#include <stdio.h>
int single2Double(char *pData, int nSize)
{
    unsigned short szBuf[4096];
    
   unsigned short *pst = (unsigned short*)pData;
    memset(szBuf, 0, sizeof(szBuf));
    memcpy(szBuf, pData, nSize);
    for (int i = 0; i < nSize; i++)
    {
        if (i % 2 == 0) 
        { 
            pst[i] = szBuf[i / 2];
        }
        else 
        {
             pst[i] = pst[i - 1]; 
        } 
   } 
   return nSize * 2;
}
