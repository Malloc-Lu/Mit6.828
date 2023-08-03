#include "ns.h"
#define INPUT_BUFSIZE  1536

extern union Nsipc nsipcbuf;

void
input(envid_t ns_envid)
{
	binaryname = "ns_input";

	// LAB 6: Your code here:
	// 	- read a packet from the device driver
	//	- send it to the network server
	// Hint: When you IPC a page to the network server, it will be
	// reading from it for a while, so don't immediately receive
	// another packet in to the same physical page.
uint8_t inputbuf[INPUT_BUFSIZE];
int r, i;
	while(1){
		memset(inputbuf, 0, INPUT_BUFSIZE);

		while((r = sys_net_recv(inputbuf, sizeof(inputbuf))) == -1){
			sys_yield();
		}
		if(r < 0){
			panic("%s: inputbuf too small\n", binaryname);
		}
		nsipcbuf.pkt.jp_len = r;
		memcpy(nsipcbuf.pkt.jp_data, inputbuf, r);
		ipc_send(ns_envid, NSREQ_INPUT, &nsipcbuf, PTE_P | PTE_U);
		// send it to the network server
		sys_yield();

	}
}
