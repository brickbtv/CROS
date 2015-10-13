#ifndef _NETWORK_DRIVER_H_
#define _NETWORK_DRIVER_H_

#include <stdint_shared.h>

typedef enum HW_NIC_FUNC{
	HW_NIC_FUNC_GETINFORMATION = 0,
	HW_NIC_FUNC_BUFFEROUTGOINGPACKET = 0, //1, TODO: fixed in next build
	HW_NIC_FUNC_RETRIEVEINCOMMINGPACKET = 2,
	HW_NIC_FUNC_QUERYBUFFERSTATE = 3, 
	HW_NIC_FUNC_QUERYGLOBALSTATS = 4
}HW_NIC_FUNC;

typedef enum HW_NIC_INTS{
	HW_NIC_INTS_LASTPACKETISSEND = 0
}HW_NIC_INTS;

typedef struct NetworkBuffersState{
	unsigned int out_buf_maxsize;
	unsigned int out_buf_usedsize;
	unsigned int inc_buf_maxsize;
	unsigned int inc_buf_usedsize;
}NetworkBuffersState;

typedef struct NetworkGlobalState{
	unsigned int total_sent;
	unsigned int total_recv;
	unsigned int total_out_packets_dropped;
	unsigned int total_inc_packets_dropped;
}NetworkGlobalState;

unsigned int hw_nic_getInformation();
void hw_nic_bufferOutgoingPacket(uint32_t network_address, void * addr, uint32_t size);
void hw_nic_retrieveIncomingPacket(void * addr, uint32_t size, uint32_t * network_address, uint32_t * recv_size);
NetworkBuffersState hw_nic_queryBufferState();
NetworkGlobalState hw_nic_queryGlobalState();

void hw_nic_handleInterrupt(uint32_t reason);

#endif