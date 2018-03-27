#include "networking.h"
#include "sdk/nic/network.h"
#include "sdk/os/debug.h"
#include "stdlib/string_shared.h"
#include "stdlib/stdio_shared.h"

static unsigned int free_port = 40000;

unsigned int net_free_port(void){
	return ++free_port;
}

STR_NET_SOCKET net_client(unsigned int port_dst, unsigned int ip_dst, unsigned int proto){
	STR_NET_SOCKET socket;
	socket.ip_src = sdk_nic_getAddress();
	socket.ip_dst = ip_dst;
	socket.port_src = net_free_port();
	socket.port_dst = port_dst;
	socket.protocol = proto;
	
	return socket;
}

STR_NET_SOCKET net_server(unsigned int port_src, unsigned int port_dst, unsigned int ip_src, unsigned int ip_dst, unsigned int proto){
	STR_NET_SOCKET socket;
	socket.ip_dst = ip_src;
	socket.ip_src = sdk_nic_getAddress();
	socket.port_src = port_src;
	socket.port_dst = port_dst;
	socket.protocol = proto;
	
	return socket;
}

void int_to_ip(int addr, char * out){
	sprintf(out, "%d.%d.%d.%d", (addr & 0xFF), (addr >> 8 & 0xFF), (addr >> 16 & 0xFF), (addr >> 24 & 0xFF));
}

void net_print_packet(STR_NET_PACKET * packet, char * mark){
	char ip_src_str[20];
	char ip_dst_str[20];
	int_to_ip(packet->ip_src, ip_src_str);
	int_to_ip(packet->ip_dst, ip_dst_str);


	sdk_debug_logf("Packet %s: ip_s=%s,ip_d=%s,port_s=%d,port_d=%d,proto=%d", 
						mark,
						ip_src_str,
						ip_dst_str, 
						packet->port_src, 
						packet->port_dst, 
						packet->protocol);
}

void net_send(STR_NET_SOCKET *socket, unsigned char * data, unsigned int lenght){
	STR_NET_PACKET packet;
	packet.magic_byte = 0x13;
	packet.magic_version = 0x01;
	packet.ip_src = socket->ip_src;
	packet.ip_dst = socket->ip_dst;
	packet.port_src = socket->port_src;
	packet.port_dst = socket->port_dst;
	packet.protocol = socket->protocol;

	unsigned char buffer[2048];
	memset(buffer, 0, 2048 * sizeof(unsigned char));
	memcpy(buffer, &packet, sizeof(STR_NET_PACKET));
	memcpy(&buffer[sizeof(STR_NET_PACKET)], data, lenght);

	net_print_packet(&packet, "OUT");
	sdk_nic_send(socket->ip_dst, buffer, lenght + sizeof(STR_NET_PACKET));
}

void net_recv(STR_NET_SOCKET *socket, unsigned char * bytes_in, unsigned char * data_out, unsigned int lenght){
	STR_NET_PACKET packet;
	memcpy(&packet, bytes_in, sizeof(STR_NET_PACKET));
	memcpy(data_out, &bytes_in[sizeof(STR_NET_PACKET)], lenght - sizeof(STR_NET_PACKET));
	net_print_packet(&packet, "IN");
}