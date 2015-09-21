#include <stdio.h>
#include <time.h>

#define DEC_TO_BCD(x)  ((((((x))/10)<<4)|((x))%10))

int main( )
{
				char timestamp[7];
				time_t nowtime;
				struct tm *timeinfo;
				time( &nowtime );
				timeinfo = gmtime( &nowtime );

				timestamp[0] = DEC_TO_BCD( timeinfo->tm_year - 100);  
				timestamp[1] = DEC_TO_BCD( 1+timeinfo->tm_mon);  
				timestamp[2] = DEC_TO_BCD( timeinfo->tm_mday);  
				timestamp[3] = DEC_TO_BCD( timeinfo->tm_hour);  
				timestamp[4] = DEC_TO_BCD( timeinfo->tm_min);  
				timestamp[5] = DEC_TO_BCD( timeinfo->tm_sec);  


				for (int i = 0; i < 6; ++i)
				{
					        printf("Isdst:  %02x\n", timestamp[i]);  

				}

 return 0;
}
