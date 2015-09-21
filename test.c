
unsigned char * bcd2latitude(unsigned char* bcd)
{
	unsigned char longitude_latitude[10];
	int num = 0;
	if (((*(bcd) & 0xF0) >>4) >= 8) 
	{
		num = (((*(bcd) & 0xF0) >>4) - 8) * 100;
		//值为负
	}
	else {
		num = (((*(bcd) & 0xF0) >>4) ) * 100;
	}
	printf("%d\n",num);
	num += (*bcd & 0x0F) * 10 + ((*(bcd+1) & 0xF0) >>4);
	printf("%d\n",num);

	int point = ((*(bcd+1) & 0x0F) *10000);
	point += ((*(bcd+2) & 0xF0) >>4) *1000;
	point += ((*(bcd+2) & 0x0F) *100);
	point += ((*(bcd+3) & 0xF0) >>4) *10;
	point += (*(bcd+3) & 0x0F);

	sprintf(longitude_latitude,"%d.%05d",num, point);
	return longitude_latitude;
}


main()
{
	unsigned char s[] = {0x03,0x20,   0x18,0x90, 0x11,0x84, 0x77,0x99};
	printf("jing wei du %s \n",   bcd2latitude(s));
	printf("jing wei du %s \n",   bcd2latitude(s+4));

	
}