// skeleton version of a tiny webserver, minimal error handling
// by U. Willers - 07.11.2013
#include <sys/types.h>				// size_t, pid_t
#include <sys/socket.h>				// accept, bind, listen
#include <netinet/in.h>				// sockaddr_in, htons
#include <stdio.h>					// printf
#include <strings.h>				// bzero, strncmp
#include <stdlib.h>					// exit
#include <errno.h>					// strerror
#include <fcntl.h>					// open files
#include <string.h>					// string functions
#include <strings.h>				// string type

#define BUFSIZE 8096				// size of request buffer

// Prototypes, maybe outsource to webserver.h
void handle_request( int s1 );
int setup_webserver( void );

int main( void )
{
	int server_socket, client_socket, addrlen;
	struct sockaddr_in client_addr;
	printf( "Starting webserver...\n" );
	server_socket = setup_webserver();

	// create socket and listen on it
	while( 42 )
	{
		addrlen = sizeof( client_addr );
		// wait for client requests
		client_socket = accept( server_socket, (struct sockaddr*) &client_addr, &addrlen );
		// process request
		printf( "Handling Client Request...\n");
		handle_request( client_socket );	
	}
}

int setup_webserver( void )
{
	int s, ret;
	struct sockaddr_in s_addr;							// create TCP/IP socket
	
	s = socket( AF_INET, SOCK_STREAM, 0 );				// listen on port 80 (http) at localhost for client requests
	bzero( &s_addr, sizeof( s_addr ) );					// clear data structure
	
	s_addr.sin_family = AF_INET;						// IP address
	s_addr.sin_port = htons( 80 );						// port is 80
	s_addr.sin_addr.s_addr = INADDR_ANY;				// addr. is 127.0.0.1
														// bind port and addr. to socket
	ret = bind( s, (struct sockaddr*) &s_addr, sizeof( s_addr ) );
	
	if( ret < 0 )										// error?
	{													// Yes, convert into error message
		printf( "Bind Error: %s\n", strerror( errno ) );
		exit( 1 );										// terminate program and return error code
	}
	listen( s, 5 );										// wait for requests, max. 5 requests in queue
	return( s );										// return server socket descriptor
}

// Entry: cs is client socket descriptor
void handle_request( int cs )
{
	char request[BUFSIZE]; 								// buffer for first line of client request
	int i = 0;
	char c;												// buffer for one input character
	char fileName[100];
	int fileDescriptor;
	struct stat fileStat;
	char data[BUFSIZE], fulldata[BUFSIZE];				// data from file
	int n;												// read the data from file 
	
	// read client request and terminate it with 0
	printf( "Getting request: " );
	
	while( read( cs, &c, 1 ) == 1 && c != '\n' && c != '\r' && i < sizeof( request ) -1 )
		request[i++] = c;
	
	request[i] = '\0';									// terminate string
	
	// Check if it is a GET request
	if(strncmp(request, "GET /", 5) == 0)				// request is a GET
	{ 			
		if(strstr(request, "..") != NULL)				// request contains relative path
		{												// this function returns a pointer to the first occurrence of str2 in str1
			printf( "Avoid relative path! Closing client socket..." );
			close(cs);
			return;
		}
		
		// Map "GET /" to "GET /index.html"
		if(request[5] == ' ' || request[5] == '\0')
		{
			strcpy(request, "GET /index.html");	
		}
		
		// Open the html file
		fileDescriptor = open("index.html", O_RDONLY);
		
		// Check if file was opened correctly
		if(fileDescriptor != -1)
		{
			
			fstat(fileDescriptor, &fileStat);					// get info from fileDesc. and put into struct
			n = read(fileDescriptor, &data, sizeof(data));		//read the file
			
			if(n > 0)
			{
				char headers[] = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\nContent-length:";
				sprintf(headers, "%s %ld\r\n\r\n", headers,  fileStat.st_size);
				snprintf(fulldata, sizeof(fulldata), "%s %s", headers,  data);
				
				n = write(cs, fulldata, sizeof(fulldata));
			}
			else
			{
				perror( "File is empty!" );	
			}
			close(fileDescriptor);
		}
		else
			perror( "Error on opening file!" );	
	}
	
	printf( "%s\n", request ); 				// print request
	close( cs );
	
}

