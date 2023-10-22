
#include "DNS_client.h"	

#define PORT 53
#define DEST_ADD "8.8.8.8"
#define buffer_size 65536
	

int main(int argc, char *argv[]) {
	int sockfd,ID, packet_len;
	unsigned char buffer[buffer_size];
	char *hello = "Hello from client";
	unsigned char packet[buffer_size], * hostname, * hostname1, * hostname2, * hostname3;
	struct sockaddr_in	server_addr;

	if(argc < 2)
	{
		perror("Error! Enter domain name");
		exit(EXIT_FAILURE);
	}


	//printf("domain name = %s\n", argv[1]);
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0 ) {
		perror("Error during socket creation!!");
		exit(EXIT_FAILURE);
	}

	//get random number for ID
	ID = get_random_id(9999,1000);


	//construct the query
	struct header * dns_header = (struct header *)&packet;
	initialize_header(ID, dns_header);

	int array[3];
	char * string_array[3];
	convert_qname(argv[1], hostname, string_array, array);

	packet[sizeof(struct header)] = array[0];
	hostname1 = (unsigned char *)&packet[sizeof(struct header)+1];
	copy_to_hostname(hostname1,(unsigned char *)string_array[0]);
	//strcpy(packet+1,(unsigned char *)string_array[0]);

	packet[sizeof(struct header)+1+strlen(string_array[0])] = array[1];
	hostname2 = (unsigned char *)&packet[sizeof(struct header)+2+strlen(string_array[0])];
	copy_to_hostname(hostname2,(unsigned char *)string_array[1]);
	// //strcpy(packet+(sizeof(struct header)+2+strlen(string_array[0])),string_array[1]);

	packet[sizeof(struct header)+2+strlen(string_array[0])+strlen(string_array[1])] = array[2];
	hostname3 = (unsigned char *)&packet[sizeof(struct header)+3+strlen(string_array[0])+strlen(string_array[1])];
	copy_to_hostname(hostname3,(unsigned char *)string_array[2]);
	// //strcpy(packet+(sizeof(struct header)+3+strlen(string_array[0])+strlen(string_array[1])),string_array[2]);
	// //hostname = (unsigned char *)argv[1];

	//packet[sizeof(struct header)+3+strlen(string_array[0])+strlen(string_array[1])+strlen(string_array[2])] = 0;

	// packet[sizeof(struct header)+4+strlen(string_array[0])+strlen(string_array[1])+strlen(string_array[2])] = 0x0001;
	// packet[sizeof(struct header)+5+strlen(string_array[0])+strlen(string_array[1])+strlen(string_array[2])] = 0x0001;

	struct question * dns_question = (struct question *)&packet[sizeof(struct header)+4+strlen(string_array[0])+strlen(string_array[1])+strlen(string_array[2])];
	initialize_question(argv[1],dns_question);


	memset(&server_addr, 0, sizeof(server_addr));
		
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = inet_addr(DEST_ADD);
		
	int n;
	socklen_t len;
	printf("Contacting DNS server..\n");
	sendto(sockfd, (char *)packet, (sizeof(struct header)+4+strlen(string_array[0])+strlen(string_array[1])+strlen(string_array[2])+sizeof(struct question)) ,
		0, (const struct sockaddr *) &server_addr,
			sizeof(server_addr));
	printf("Sending DNS query..\n");
			
	n = recvfrom(sockfd, (char *)buffer, buffer_size,
				0, (struct sockaddr *) &server_addr,
				&len);
	printf("DNS response received \n");
	buffer[n] = '\0';
	printf("Processing DNS response..\n");

	struct header * response_header = (struct header *)(buffer);
	printf("header.id = %x\n",ntohs(response_header->ID));
	printf("header.flags = %x\n",ntohs(response_header->flags));
	printf("header.QDCount = %d\n", ntohs(response_header->QDCount));
	printf("header.ANCount = %d\n", ntohs(response_header->ANCount));
	printf("header.NSCount = %d\n", ntohs(response_header->NSCount));
	printf("header.ARCount = %d\n", ntohs(response_header->ARCount));

	const char * response_question_name = (char *)&buffer[sizeof(struct header)];
	printf("question.QName = %s\n", response_question_name);

	struct question * response_question = (struct question *)&buffer[sizeof(struct header)+ strlen(response_question_name)];
	printf("question.QTYPE = %x\n",(response_question->Qtype));
	printf("question.QCLASS = %x\n",(response_question->Qclass));

	char * answer_name = (char *)&buffer[sizeof(struct header)+ strlen(response_question_name)+sizeof(struct question)];
	// //printf("Name = %s\n", answer_name);
	int answer_len = sizeof(struct header)+ strlen(response_question_name)+sizeof(struct question)+strlen(answer_name);
	printf("\n");
	for(int s=0; s<ntohs(response_header->ANCount);s++)
	{
		//printf("hi\n");
		struct RR * answer = (struct RR *)&buffer[answer_len];
	printf("answer.Type = %x\n", ntohs(answer->Type));
	printf("answer.Class = %x\n",ntohs(answer->Class));
	printf("answer.TTL = %d\n",ntohl(answer->TTL));
	printf("answer.RDLength = %d\n", ntohs(answer->RDLength));
	//printf("answer.IP = %08\n", ntohl(answer->addr.s_addr));

	answer_len = answer_len+sizeof(struct RR)-2;

	if(ntohs(answer->Type) ==1)
	{
		uint8_t * IP = (uint8_t *)&buffer[answer_len];
		printf("IP = %d.%d.%d.%d\n", (IP[0]),(IP[1]),(IP[2]),(IP[3]));
	}
	else if(ntohs(answer->Type) ==5)
	{
		char * CNAME = (char *)&buffer[answer_len];
		printf("CNAME = %s\n", CNAME);
	}

	answer_len = answer_len + ntohs(answer->RDLength);
	//answer_len = answer_len + sizeof(IP);

	char * answer_name2 = (char *)&buffer[answer_len];

	answer_len = answer_len+strlen(answer_name2);
	printf("\n");

	}


	 return 0;
}

int get_random_id(upper, lower)
{
  time_t t;
  srand((unsigned) time(&t));
  int ret = (rand()%(upper-lower+1))+lower;
  return ret;
}

void initialize_header(int ID, struct header * head)
{
	//struct header * head = (struct header *)malloc(sizeof(struct header));
	head->ID= (unsigned short) htons(ID);
    head->flags = htons(0x0100);
    head->QDCount = htons(1); //we have only 1 question
    head->ANCount = htons(0);
    head->NSCount = htons(0);
    head->ARCount = htons(0);

    //return head;
}

void convert_qname (char * input, unsigned char * ret_name, char * string_array[], int * labels)
{

  //char * ret_name = calloc (strlen (hostname) + 2, sizeof (char));
  ret_name = (unsigned char *)malloc(strlen (input) + 2);

  int count=0;
  //int label_len_pos = 0;
  int j=0,k=0;
  int label_count = 0;
  char  temp[256];
  memset(temp,0,strlen(temp));

  // char hex_val[4];
  // int hex_val_int;
  for (int i = 0; i < strlen (input); i++)
    {
      if (input[i] == '.')
        {
       		labels[label_count] = count;
       		string_array[label_count] = (char *)malloc(strlen(temp));
       		strcpy(string_array[label_count],temp);
       		label_count++;
       		
        	i++;
          
          count = 0;
          k=0;
          memset(temp,0,strlen(temp));
       }

     ret_name[j] =  input[i];
     temp[k] = input[i];
     j++;
     k++;
     count++;
     
    }
    string_array[label_count] = (char *)malloc(strlen(temp));
    strcpy(string_array[label_count],temp);
    labels[label_count] = count;
   
}


void initialize_question (char * hostname, struct question * dns_quest)
{
	//struct question * dns_quest = (struct question *)malloc(sizeof(struct question));
	dns_quest->Qtype = htons(1);
	dns_quest->Qclass = htons(1);
	//dns_quest->QName = convert_qname(hostname);

	//return dns_quest;
}
void copy_to_hostname(unsigned char* hostname ,unsigned char* str)
{
	for(int i=0;i<strlen((char*)str);i++)
	{
		hostname[i] = str[i];
	}
}