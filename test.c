#include <stdio.h>
double bcd2longitude1(unsigned char* bcd)
{
	int num = 0;
	if (((*(bcd) & 0xF0) >>4) >= 8) 
	{
		num = (((*(bcd) & 0xF0) >>4) - 8) * 100;
		//值为负
	}
	else {
		num = (((*(bcd) & 0xF0) >>4) ) * 100;
	}
	num += (*bcd & 0x0F) * 10 + ((*(bcd+1) & 0xF0) >>4);
	int point = ((*(bcd+1) & 0x0F) *10000);
	point += ((*(bcd+2) & 0xF0) >>4) *1000;
	point += ((*(bcd+2) & 0x0F) *100);
	point += ((*(bcd+3) & 0xF0) >>4) *10;
	point += (*(bcd+3) & 0x0F);


	double du = (double)num + ((double)point)/1000/60;
    fprintf(stderr," %f \n", du );

	return (double)du;
	
}


main()
{
	unsigned char s[] = {0x03,0x95,   0x73,0x72, 0x11,0x61, 0x96,0x78};
	double lat = bcd2longitude1(s);
	double lng = bcd2longitude1(s+4);
	printf("jing wei du %f \n",   lat);

	printf("jing wei du %f \n",   lng);

	
}