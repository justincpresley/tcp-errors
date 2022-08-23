#include <netdb.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <syslog.h>
#include <ctype.h>

#include "../hdr/message.h"
#include "../hdr/stdtypes.h"
#include "../hdr/byte_help.h"
#include "../hdr/socket_msg_help.h"
#include "../hdr/section.h"

#define MAX_SIZE 8000

bool8 establish_channel(int sock, uint64 selfID, uint64* otherID, uint32 maxSize, uchar8* file, uint32 fileLength){
    MSG* send = form_msg(selfID, *otherID, CONTROL, SYN, 0, maxSize-1, file, fileLength);
    send_msg(sock, send);
    free_msg(send);
    MSG* recv = recv_msg(sock);
    verify_msg(recv);
    *otherID = recv->head->sendID;

    if(recv->head->sendID == 0){
        syslog(LOG_ALERT, "hipERROR: Channel could not be Created.");
        free_msg(recv);
        return false;
    }else{
        syslog(LOG_ALERT, "Channel is Created for a file %s", file);
        free_msg(recv);
        return true;
    }
}
void ready_channel(int sock, uint64 selfID, uint64 otherID, uint32 maxSize, uchar8* file, uint32 fileLength){
    MSG* send = form_msg(selfID, otherID, CONTROL, SYN, 0, maxSize-1, file, fileLength);
    send_msg(sock, send);
    free_msg(send);
    syslog(LOG_ALERT, "Channel is Ready and Active.");
}

bool8 gather_file(int sock, uint64 selfID, uint64 otherID, uint32 maxSize, char8* outfile){
    section_t* sec = create_section();
    uint32 numpcks = 0;
    bool8 keepReading = true;
    MSG* pck = NULL;
    FILE* fhandle = fopen(outfile,"wb");
    while(keepReading){
        pck = recv_msg(sock);
        if(pck == NULL){ syslog(LOG_ALERT, "hipERROR: Can not Read NULL PACKET."); return false; }
        if(verify_msg(pck) == false){
            add_to_section(sec, numpcks);
            if(pck->head->contentLength == 0){
                keepReading = false;
            }
        }else{
            if(pck->head->contentLength == 0){
                keepReading = false;
            }else{
                fwrite(pck->body->content,sizeof(uchar8),pck->head->contentLength,fhandle);
            }
        }
        free_msg(pck);
        numpcks = numpcks + 1;
    }

    uint64 temp;
    MSG* send = NULL;
    while(get_piece(sec, 0) != NULL){
        temp = first_low(sec);
        send = form_msg(selfID, otherID, CONTROL, RTS, temp * maxSize, (temp*maxSize) + (maxSize-1), NULL, 0);
        send_msg(sock, send);
        free_msg(send);
        MSG* pck = recv_msg(sock);
        if(verify_msg(pck)==true){
            delete_from_section(sec, temp);
        }
    }
    fclose(fhandle);

    syslog(LOG_ALERT, "Total Packets of %d recieved: %u", MAX_SIZE, numpcks);
    if(numpcks == 1){
        syslog(LOG_ALERT, "Desired File could not be found server-side.");
        unlink(outfile);
    }else{
        syslog(LOG_ALERT, "Desired File was found and written.");
    }
    return true;
}
void cease_channel(int sock, uint64 selfID, uint64 otherID){
    MSG* send = form_msg(selfID, otherID, CONTROL, FIN, 0, 0, NULL, 0);
    send_msg(sock, send);
    free_msg(send);
    MSG* recv = recv_msg(sock);
    verify_msg(recv);
    free_msg(recv);
    syslog(LOG_ALERT, "Channel closed.");
}
void cease_connection(int sock){
    MSG* send = form_msg(0, 0, CONTROL, FIN, 0, 0, NULL, 0);
    send_msg(sock, send);
    free_msg(send);
    syslog(LOG_ALERT, "Connection to the Server is Closed.");
    close(sock);
}

int main(int argc, char8** argv){
    openlog("hipFT-client", LOG_PERROR|LOG_PID, LOG_USER);
    char8* output_filename = "./files/output/output.txt";

	int port;
	int sock = -1;
	struct sockaddr_in address;
	struct hostent* host;

	//checking commandline parameter
	if(argc != 5){
		printf("usage: %s hostname port filename #downloads\n", argv[0]);
		return -1;
	}

    //checking #downloads parameter
    int numDownloads = atoi((char8*)argv[4]);
    if(numDownloads<0){
        printf("#downloads (%s) must be a postive integer (0 or above)\n", argv[3]);
        return -4;
    }

	//obtain port number
	if(sscanf(argv[2], "%d", &port) <= 0){
		fprintf(stderr, "%s: error: wrong parameter: port\n", argv[0]);
		return -2;
	}

	//create socket
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock <= 0){
		fprintf(stderr, "%s: error: cannot create socket\n", argv[0]);
		return -3;
	}

	//connect to server
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	host = gethostbyname(argv[1]);
	if(!host){
		fprintf(stderr, "%s: error: unknown host %s\n", argv[0], argv[1]);
		return -4;
	}
	memcpy(&address.sin_addr, host->h_addr_list[0], host->h_length);
	if (connect(sock, (struct sockaddr *)&address, sizeof(address))){
		fprintf(stderr, "%s: error: cannot connect to host %s\n", argv[0], argv[1]);
		return -5;
	}

    syslog(LOG_ALERT, "Connection to a Server is Established.");

    uint64 selfID = 0;
    uint64 otherID = 0;
    int cycle = 0;
    bool8 runForever = (numDownloads == 0) ? true : false;

    while(cycle < numDownloads || runForever){
        selfID = generate_random_uint(100, 5000000000);
        otherID = 0;
        establish_channel(sock, selfID, &otherID, MAX_SIZE, (uchar8*)argv[3], strlen(argv[3]));
        ready_channel(sock, selfID, otherID, MAX_SIZE, (uchar8*)argv[3], strlen(argv[3]));
        gather_file(sock, selfID, otherID, MAX_SIZE, output_filename);
        cease_channel(sock, selfID, otherID);
        cycle = cycle + 1;
    }

    //close and clean
    cease_connection(sock);
    closelog();
    fclose(stdin);
    fclose(stdout);
    fclose(stderr);
	return 0;
}