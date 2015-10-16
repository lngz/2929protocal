//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// 
// server.c
// This is a udp server of gateway. Receive GPS information and Send Command to 
// handring.
//
// Compile: g++ event_test.c -o event_test -levent  -L/usr/local/lib -Wl,-rpath=/usr/local/lib
// Execture: ./event_test 
// 
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#define UDP_PORT 7212 
#include <event.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>


int g_count = 0;
struct timeval stTv;



FILE * fp = NULL;
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// TIMER FUNCTION
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void timer_function(int x, short int y, void *pargs)
{
	printf("Get %d Message in a 5 sec \n",g_count);
	g_count = 0 ;
	event_add((struct event*)pargs,&stTv);
}

void dump_proc(char * aReqBuffer,int nByte)
{
	int i ;
	int count = 0;
	for(i = 0; i < nByte; i++)
	{
		fprintf(stderr, "0x%x ",*(aReqBuffer + i));
		if (count == 9)
		{
			count = 0;
			fprintf(stderr,"\n");
		}else{
			count ++;	
		}
		

	}
	fprintf(stderr, "\n",*(aReqBuffer + i));

}



//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// Event Function
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void func_for_eve1(int x, short int y, void *pargs)
{
	unsigned int unFromAddrLen;
	int nByte = 0;
	char aReqBuffer[512];
	struct sockaddr_in stFromAddr;

	unFromAddrLen = sizeof(stFromAddr);

	if ((nByte = recvfrom(x, aReqBuffer, sizeof(aReqBuffer), 0,
		(struct sockaddr *)&stFromAddr, &unFromAddrLen)) == -1)
	{
		printf("error occured while receivingn");
	}
	dump_proc (aReqBuffer,nByte) ;
	if (g_count == 1){
		sleep(10);
	}
	if (( sendto(x, aReqBuffer, nByte, 0,
		(struct sockaddr *)&stFromAddr, unFromAddrLen)) == -1)
	{
		printf("error occured while receivingn");
	}
	//printf("Function called buffer is %sn",aReqBuffer);
	g_count++;

}

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// MAIN
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
int main(int argc, char **argv)
{
	struct event_base *base ;
	struct event g_Timer;
	

	struct event g_eve;
	int udpsock_fd;
	struct sockaddr_in stAddr;

	fp = fopen ("./debuglog","w");

	base = event_init();


	if ((udpsock_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		printf("ERROR - unable to create socket:n");
		exit(-1);
	}

	//Start : Set flags in non-blocking mode
	int nReqFlags = fcntl(udpsock_fd, F_GETFL, 0);
	if (nReqFlags< 0)
	{
		printf("ERROR - cannot set socket options");
	}

	if (fcntl(udpsock_fd, F_SETFL, nReqFlags | O_NONBLOCK) < 0)
	{
		printf("ERROR - cannot set socket options");
	}
	// End: Set flags in non-blocking mode
	memset(&stAddr, 0, sizeof(struct sockaddr_in));
	//stAddr.sin_addr.s_addr = inet_addr("192.168.64.1555552");
	stAddr.sin_addr.s_addr = INADDR_ANY; //listening on local ip
	stAddr.sin_port = htons(UDP_PORT);
	stAddr.sin_family = AF_INET;


	int nOptVal = 1;
	if (setsockopt(udpsock_fd, SOL_SOCKET, SO_REUSEADDR,
		(const void *)&nOptVal, sizeof(nOptVal)))
	{
		printf("ERROR - socketOptions: Error at Setsockopt");

	}

	if (bind(udpsock_fd, (struct sockaddr *)&stAddr, sizeof(stAddr)) != 0)
	{
		printf("Error: Unable to bind the default IP n");
		exit(-1);
	}

	event_set(&g_eve, udpsock_fd, EV_READ | EV_PERSIST, func_for_eve1, &g_eve);
	event_add(&g_eve, NULL);

	/////////////TIMER START/////////////////////////////////// 
	stTv.tv_sec = 5;
	stTv.tv_usec = 0;
	// event_set(&g_Timer, -1, EV_TIMEOUT , timer_function, &g_Timer);
	// event_add(&g_Timer, &stTv);
	////////////TIMER END/////////////////////////////////////

	event_base_dispatch(base);
	return 0;
}
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
