#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <string.h>


struct header
{
	unsigned short ID;
	unsigned short flags;
	unsigned short QDCount;
	unsigned short ANCount;
	unsigned short NSCount;
	unsigned short ARCount;
};

struct question
{
  //char *QName;
  unsigned short Qtype;
  unsigned short Qclass;
};

struct size
{
	unsigned short len1;
	//unsigned short len2:1;
	//unsigned short len3:1;
};

struct RR
{
	//unsigned short compression;
	unsigned short Type;
	unsigned short Class;
	unsigned int TTL;
	unsigned short RDLength;

};
int get_random_id(int, int);
void initialize_header(int, struct header *);
void convert_qname (char *, unsigned char*, char **, int *);
void initialize_question(char *, struct question * );
void ChangetoDnsNameFormat(unsigned char* ,unsigned char* );
void copy_to_hostname(unsigned char* ,unsigned char*);

