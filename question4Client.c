#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

int main( void )
{
	int socket_number;
	struct sockaddr_in adressinfo;							//contains: sin_family, sin_port, sin_addr (is a struct)
	int result;
	char request[8096];										// save the whole request
	char c;  												// to send HTTP request
	int i = 0;
							
	socket_number = socket(AF_INET, SOCK_STREAM, 0); 		// create socket
	adressinfo.sin_family = AF_INET;
	adressinfo.sin_addr.s_addr = inet_addr("127.0.0.1"); 	// configure server IP and port
	adressinfo.sin_port = htons(80);
	result = connect(socket_number,(struct sockaddr*) &adressinfo, sizeof(adressinfo)); //connect
	
	if(result == 0)
	{	
		printf("connection established\n");
			
		printf("Type request: ");
		fgets(request, sizeof(request), stdin);				// to allow capture blank spaces
	
		write(socket_number, &request, sizeof(request));			
		printf("\nEnd of request\n");			
		
		printf("End of connection\n");
	}
	else
		perror("error: connection failed");
	
	close(socket_number); // close connection
}
