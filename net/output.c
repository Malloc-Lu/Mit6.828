#include "ns.h"

extern union Nsipc nsipcbuf;

void
output(envid_t ns_envid)
{
	binaryname = "ns_output";

	// LAB 6: Your code here:
	// 	- read a packet from the network server
	//	- send the packet to the device driver
int r;
int whom;
int32_t req;
int perm;
	while(1){
		if((req = ipc_recv(&whom, &nsipcbuf, &perm) == NSREQ_OUTPUT)){
			// sys_net_transmit(nsipcbuf.pkt.jp_data, nsipcbuf.pkt.jp_len);
			// return;	
			while((r = sys_net_transmit(nsipcbuf.pkt.jp_data, nsipcbuf.pkt.jp_len)) < 0){
				sys_yield();
			}
		}	
	}
}
