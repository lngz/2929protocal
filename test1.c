#include <stdio.h>
unsigned char tel[6] ;
 bcd2longitude1( char* bcd, unsigned char *tel)
{
        char *p = bcd;
        int count = 0;
        memset (tel,0xFF, 6);
        while(*p != NULL)
        {

                if (count == 0){
                        *tel = ((*p - '0')<<4)|0xf;
                        count++;
                }else if (count == 1) {
                        *tel = (*tel & 0xf0) + (*p - '0') ;

                        tel ++;
                        count =0;
                }
                p++;
        }

}



main()
{
	char s[] = "13520975760";
	int i;
	 bcd2longitude1(s, tel);
	for(i = 0; i< 6; i++)
	{
		printf("%02x ",tel[i]);

	}
	printf("jing wei du %s \n",   tel);



	
}