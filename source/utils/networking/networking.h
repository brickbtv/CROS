#ifndef _NETWORKING_H_
#define _NETWORKING_H_

#define NET_PROTO_UNKNOWN 0
#define NET_PROTO_FILETRANSFER 1

typedef struct STR_NET_PACKET {
	unsigned char magic_byte;
	unsigned char magic_version; 
	unsigned int ip_dst;
	unsigned int ip_src;
	unsigned int port_src;
	unsigned int port_dst;
	unsigned int protocol;
} STR_NET_PACKET;

typedef struct STR_NET_SOCKET {
	unsigned int ip_dst;
	unsigned int ip_src;
	unsigned int port_src;
	unsigned int port_dst;
	unsigned int protocol;
} STR_NET_SOCKET;



STR_NET_SOCKET net_client(unsigned int port_dst, unsigned int ip_dst, unsigned int proto);
STR_NET_SOCKET net_server(unsigned int port_src, unsigned int port_dst, unsigned int ip_src, unsigned int ip_dst, unsigned int proto);

unsigned int net_free_port(void);

void net_send(STR_NET_SOCKET *socket, unsigned char * data, unsigned int lenght);
void net_recv(STR_NET_SOCKET *socket, unsigned char * bytes_in, unsigned char * data_out, unsigned int lenght);

#endif