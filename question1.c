#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main( void )
{
	int socket_number;
	struct sockaddr_in adressinfo;	//contains: sin_family, sin_port, sin_addr (is a struct)
	int result;
	socket_number = socket(AF_INET, SOCK_STREAM, 0); // create socket
	adressinfo.sin_family = AF_INET;
	adressinfo.sin_addr.s_addr = inet_addr("139.13.193.16"); // configure server IP and port
	adressinfo.sin_port = htons(13);
	result = connect( socket_number,(struct sockaddr*) &adressinfo, sizeof(adressinfo)); //connect
	if(result == 0)
	{
		printf("\nconnection established\n");
	}
	else
	{
		perror("\nerror: connection failed\n");
	}
	close(socket_number); // close connection

}
