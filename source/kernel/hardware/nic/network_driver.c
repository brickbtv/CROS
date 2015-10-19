#include "network_driver.H"

#include "hwi/hwi.h"

#include <stdint_shared.h>
#include "kernel/kernel.h"

unsigned int hw_nic_getInformation(){
	hw_HwiData data;
	hwi_call(HW_BUS_NIC, HW_NIC_FUNC_GETINFORMATION, &data);
	
	return data.regs[0];
}

void hw_nic_bufferOutgoingPacket(uint32_t network_address, void * addr, uint32_t size){
	hw_HwiData data;
		
	data.regs[0] = network_address;
	data.regs[1] = (uint32_t) addr;
	data.regs[2] = size;
	hwi_call(HW_BUS_NIC, HW_NIC_FUNC_BUFFEROUTGOINGPACKET, &data);
}
	
void hw_nic_retrieveIncomingPacket(char * msg, uint32_t size, uint32_t * network_address, uint32_t * recv_size){
	hw_HwiData data;
	
	data.regs[0] = (uint32_t)msg;
	data.regs[1] = 1024;
	
	int res = hwi_call(HW_BUS_NIC, HW_NIC_FUNC_RETRIEVEINCOMMINGPACKET, &data);
	if (res == 0x1){	// nothing to receive
		return;
	}
	*network_address = data.regs[0];
	*recv_size = data.regs[1];
}

NetworkBuffersState hw_nic_queryBufferState(){
	hw_HwiData data;
	hwi_call(HW_BUS_NIC, HW_NIC_FUNC_QUERYBUFFERSTATE, &data);
	
	NetworkBuffersState state;
	state.out_buf_maxsize = data.regs[0];
	state.out_buf_usedsize = data.regs[1];
	state.inc_buf_maxsize = data.regs[2];
	state.inc_buf_usedsize = data.regs[3];
	
	return state;
}

NetworkGlobalState hw_nic_queryGlobalState(){
	hw_HwiData data;
	hwi_call(HW_BUS_NIC, HW_NIC_FUNC_QUERYGLOBALSTATS, &data);
	
	NetworkGlobalState state; 
	state.total_sent = data.regs[0];
	state.total_recv = data.regs[1];
	state.total_out_packets_dropped = data.regs[2];
	state.total_inc_packets_dropped = data.regs[3];
	
	return state;
}

void hw_nic_handleInterrupt(uint32_t reason){
	if (reason == HW_NIC_INTS_LASTPACKETISSEND){
		// TODO: ?
		// NOTE: don't send info about this interuption to NIC0. never.
	} else {
		krn_debugBSODf("NIC0 interruption", "Unknown reason - %d", reason);
	}
}