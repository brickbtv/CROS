#ifndef _SDK_OS_NETWORK_H_
#define _SDK_OS_NETWORK_H_

void sdk_nic_send(unsigned int addr, const char * msg, unsigned int size);
void sdk_nic_sendf(unsigned int addr, const char * msg, ...);
int sdk_nic_recv(char * msg, int max_size, int * addr);

#endif 