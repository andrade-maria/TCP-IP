// skeleton version of a tiny webserver, minimal error handling
// by U. Willers - 07.11.2013
#include <sys/types.h>				// size_t, pid_t
#include <sys/socket.h>				// accept, bind, listen
#include <netinet/in.h>				// sockaddr_in, htons
#include <stdio.h>				// printf
#include <strings.h>				// bzero, strncmp
#include <stdlib.h>				// exit
#include <errno.h>				// strerror
#include <fcntl.h>				// open files
#include <string.h>				// string functions
#include <strings.h>				// string type
#include <unistd.h>				// fork()
#include <sys/wait.h>				// waitpid()	
#include <stdbool.h>				// bool values

#define BUFSIZE 8096				// size of request buffer

// Prototypes, maybe outsource to webserver.h
void handle_request( int s1 );
int setup_webserver( void );
void findExtension( char [], char *, char *);
int checkMIME( char []);
bool openFile( int , int , char []);
bool readFile( int , int ,int );
bool sendPage( int, int, int, char []);
void sendErrorPage( int, int );

struct{
	char *extension;
	char *fileType; 
} acceptedMIMEs[] = {
	{"gif","image/gif"},
	{"jpg","image/jpg"},
	{"jpeg","image/jpeg"},
	{"png","image/png"},
	{"htm","text/htm"},
	{"html","text/html"},
	{"css","text/css"},
	{" "," "},
};


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
		printf("Setup\n");
		// process request
		if(fork() == 0)
		{			
			printf( "Handling Client Request...\n");
			handle_request( client_socket );
		}

		waitpid(-1, 0, WNOHANG);
	}
}

int setup_webserver( void )
{
	
	int s, ret;	
	struct sockaddr_in s_addr;						// create TCP/IP socket
	
	s = socket( AF_INET, SOCK_STREAM, 0 );					// listen on port 80 (http) at localhost for client requests
	bzero( &s_addr, sizeof( s_addr ) );					// clear data structure
	
	s_addr.sin_family = AF_INET;						// IP address
	s_addr.sin_port = htons( 80 );						// port is 80
	s_addr.sin_addr.s_addr = INADDR_ANY;					// addr. is 127.0.0.1
										// bind port and addr. to socket
	ret = bind( s, (struct sockaddr*) &s_addr, sizeof( s_addr ) );
	
	if( ret < 0 )								// error?
	{									// Yes, convert into error message
		fprintf( stderr, "Bind Error: %s\n", strerror( errno ) );
		exit( 1 );							// terminate program and return error code
	}
	listen( s, 5 );								// wait for requests, max. 5 requests in queue
	return( s );								// return server socket descriptor
}

// Entry: cs is client socket descriptor
void handle_request( int cs )
{
	char request[BUFSIZE]; 							// buffer for first line of client request
	int i = 0;
	char c;									// buffer for one input character
	char fileName[BUFSIZE];
	char extensionName[BUFSIZE];				
	
	// read client request and terminate it with 0
	printf( "Getting request: " );
	
	while( read( cs, &c, 1 ) == 1 && c != '\n' && c != '\r' && i < sizeof( request ) -1 )
		request[i++] = c;
	
	request[i] = '\0';							// terminate string
	printf( "%s\n", request ); 						// print request
	
	// Check if it is a GET request
	if(strncmp(request, "GET /", 5) == 0)					// request is a GET
	{ 			
		// Avoid relative path
		if(strstr(request, "..") != NULL)				// request contains relative path
		{								// this function returns a pointer to the first occurrence of str2 in str1
			perror( "Avoid relative path! Closing client socket..." );
			close(cs);
			return;
		}
		
		// Map "GET /" to "GET /index.html"
		if(request[5] == ' ' || request[5] == '\0')
			strcpy(request, "GET /index.html");	
		
		
		// Find extension of request		
		findExtension(request, fileName, extensionName);
		printf("Extension: %s\n",extensionName);
		
		// Check if the request contain an acceptable extension
		int indexMime = checkMIME(extensionName);			// if not acceptable index = -1
		printf("Index: %d\n",indexMime);
		
		// If acceptable file, open it
		if (indexMime != -1){
			printf("File: %s\nExt: %s\n", fileName, extensionName);	
			bool success = openFile(cs, indexMime, fileName);
							
			if(success)				
				;
			else
				sendErrorPage(cs, indexMime);			
		}
		else{
			perror( "MIME type not allowed!" );
			sendErrorPage(cs, indexMime);
		}
	}
	else
	{
		perror( "Request Error\n It is not a GET request!");	
		sendErrorPage(cs, 7);						//indexMime = 6 is empty
	}
	
	
	close( cs );	
}

void findExtension(char request[], char* fileName, char* extensionName){
	// GET FILENAME
	int i;
	char fn[BUFSIZE];
	
	for(i = 5; i < strlen(request); i++){
		if(request[i] == ' ')
			break;
		fn[i-5] = request[i];
	}
		
	
	strncpy(fileName, fn, strlen(fn));
	fileName[strlen(fn)] = '\0';
	
	
	//GET EXTENSION
	int j = 0;
	bool ext = false;
	char en[BUFSIZE];
	for(i = 0; i < strlen(fileName); i++){
		if (ext){
			if(fileName[i] == ' ')
				break;
			en[j] = fileName[i];
			j++;
		}
		if(fileName[i] == '.')
			ext = true;
	}
	
	strncpy(extensionName, en, strlen(en));
	extensionName[strlen(en)] = '\0';					// end of string
	
	return;
}

int checkMIME(char extensionName[]){
	int index = -1;
	for(int i = 0; i <= 7; i++)
		if(!strncmp(extensionName, acceptedMIMEs[i].extension, strlen(extensionName)))
			index = i;
	return index;
}

bool openFile(int cs, int indexMime, char file[]){
	printf("Opening File\n");
	int fileDescriptor;
	
	// Open the file
	fileDescriptor = open(file, O_RDONLY);
	// Check if file was opened correctly
	if(fileDescriptor != -1){
		return readFile(cs, indexMime, fileDescriptor);
	}
	else{
		fprintf( stderr, "Error on opening file: %s\n", strerror( errno ) );
		return false;
	}
	
}

bool readFile(int cs, int indexMime,int fileDescriptor){
	printf("Reading File\n");
	struct stat fileStat;
	char data[360000];							// data from file
	int n; 									// to read the file
	
	fstat(fileDescriptor, &fileStat);					// get info from fileDesc. and put into struct
	n = read(fileDescriptor, &data, sizeof(data));				//read the file
	if(n != -1){
		close(fileDescriptor);
		return sendPage(cs, fileStat.st_size, indexMime, data);
	}
	else
		fprintf( stderr, "File is empty: %s\n", strerror( errno ) );	
	close(fileDescriptor);
	return false;	
}

bool sendPage(int cs, int st_size, int indexMime, char data[])
{
	printf("Sending Page\n");
	int n; 									// to write data
	char fulldata[360000];							// fulldata includes headers and html, which can be huge
	char headers[360000] = "HTTP/1.1 200 OK\r\nContent-length:";
					
	sprintf(headers, "%s %ld\r\nContent-type: %s\r\n\r\n", headers,  
		st_size, acceptedMIMEs[indexMime].fileType);
	
	snprintf(fulldata, sizeof(fulldata), "%s\n%s", headers,  data);
	printf("Headers: \n%s", headers);
	printf("Data: \n%s", data);
	
	n = write(cs, fulldata, sizeof(fulldata));
	
	if(n != -1){
		printf("Writing was successful\n");
		return true;
	}
	else{
		fprintf( stderr, "Write Error: %s\n", strerror( errno ) );
		return false;
	}
}

void sendErrorPage(int cs, int index)
{
	char fileName[] = "errorPage.html";
	bool success = openFile(cs, index, fileName);
	
	if(success)
		;
	else
		perror("Something went wrong with your errorPage!");

	perror("Request: 404\n");	
	return;
}


