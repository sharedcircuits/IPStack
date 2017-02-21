#include "Arduino.h"
#include "usb_serial.h"
#include "ser_print.h"
#include "IPAddress.h"
#include "lwipk66.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/stats.h"

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)

extern uint32_t inpkts, outpkts;

void do_print(char *msg) {

}

void prregs() {
	PRREG(MPU_RGDAAC0);
	PRREG(SIM_SCGC2);
	PRREG(SIM_SOPT2);
	PRREG(ENET_PALR);
	PRREG(ENET_PAUR);
	PRREG(ENET_EIR);
	PRREG(ENET_EIMR);
	PRREG(ENET_ECR);
	PRREG(ENET_MSCR);
	PRREG(ENET_MRBR);
	PRREG(ENET_RCR);
	PRREG(ENET_TCR);
	PRREG(ENET_TACC);
	PRREG(ENET_RACC);
	PRREG(ENET_MMFR);
}

void print_stats() {
	// lwip stats_display() needed printf
#if LWIP_STATS
	char str[128];

	// my  LINK stats
	sprintf(str, "LINK in %d out %d drop %d memerr %d",
		lwip_stats.link.recv, lwip_stats.link.xmit, lwip_stats.link.drop, lwip_stats.link.memerr);
	Serial.println(str);
	sprintf(str, "TCP in %d out %d drop %d memerr %d",
		lwip_stats.tcp.recv, lwip_stats.tcp.xmit, lwip_stats.tcp.drop, lwip_stats.tcp.memerr);
	Serial.println(str);
	sprintf(str, "UDP in %d out %d drop %d memerr %d",
		lwip_stats.udp.recv, lwip_stats.udp.xmit, lwip_stats.udp.drop, lwip_stats.udp.memerr);
	Serial.println(str);
	sprintf(str, "ICMP in %d out %d",
		lwip_stats.icmp.recv, lwip_stats.icmp.xmit);
	Serial.println(str);
#if MEM_STATS
	sprintf(str, "HEAP avail %d used %d max %d err %d",
		lwip_stats.mem.avail, lwip_stats.mem.used, lwip_stats.mem.max, lwip_stats.mem.err);
	Serial.println(str);
#endif
#endif
}

void tcperr_callback(void * arg, err_t err)
{
	// set with tcp_err()
	Serial.print("TCP err "); Serial.println(err);
	*(int *)arg = err;
}

err_t recv_callback(void * arg, struct tcp_pcb * tpcb, struct pbuf * p, err_t err)
{
	static uint32_t t0 = 0, t, bytes = 0;

	t = micros();
	if (!t0) t0 = t;
	if (p == NULL) {
		// other end closed
		t = t - t0;
		Serial.println("remote closed");
		Serial.print(bytes); Serial.print(" ");
		Serial.print(t); Serial.print(" us   ");
		Serial.println(8.*bytes / t);

		tcp_close(tpcb);
		return 0;
	}
	tcp_recved(tpcb, p->tot_len);   // data processed
	bytes += p->tot_len;
	pbuf_free(p);
	return 0;
}

static struct tcp_pcb * pcba;   // accepted pcb

err_t accept_callback(void * arg, struct tcp_pcb * newpcb, err_t err) {
	Serial.println("accepted");
	tcp_accepted(newpcb);
	pcba = newpcb;    // let tcprx proceed
	return 0;
}

err_t connect_callback(void *arg, struct tcp_pcb *tpcb, err_t err) {
	Serial.print("connected "); Serial.println((tpcb)->snd_buf);
	*(int *)arg = 1;
	return 0;
}

void sendtcp() {
	struct tcp_pcb * pcb;
	int connected = 0;

	ip_addr_t server;
	inet_aton("192.168.66.160", &server);
	pcb = tcp_new();
	tcp_err(pcb, tcperr_callback);
	tcp_arg(pcb, &connected);
	Serial.println("Binding");
	Serial.println(tcp_bind(pcb, IP_ADDR_ANY, 3333));
	Serial.println("Done");
	Serial.println("Connect");
	tcp_connect(pcb, &server, 80, connect_callback);
}

void tcprx() {
	struct tcp_pcb * pcb;
	struct tcp_pcb * pcbl;   // listen

	Serial.println("server listening");
	pcb = tcp_new();
	Serial.println("new Con");
	Serial.println(tcp_bind(pcb, IP_ADDR_ANY, 5001));  // server port
	Serial.println("Port Bound");
	/*pcbl = tcp_listen(pcb);   // pcb deallocated
	Serial.println("tcp listen called");
	tcp_accept(pcbl, accept_callback);
	Serial.println("tcp accept");
	while (pcbl == NULL) ether_poll();   // waiting connection
	Serial.println("ether poll");
	tcp_err(pcba, tcperr_callback);
	Serial.println("tcp err");
	tcp_recv(pcba, recv_callback);  // all the action is now in callback
	Serial.println("tcp recv");
	tcp_close(pcbl);
	Serial.println("tcp close");*/
	// fall through to main ether_poll loop ....
}



void setup()
{
	Serial.begin(115200);
	while (!Serial);
	Serial.println("Ether Test Starting");
	ether_init("192.168.66.161", "255.255.255.0", "192.168.66.2");
	Serial.println("Done ether init");
	IPAddress myip(ether_get_ipaddr());
	Serial.print("my IP "); Serial.println(myip);
	//prregs();
	//tcprx();
	sendtcp();
	Serial.println("Ether Test Started");
}

void loop()
{

	static uint32_t ms = millis();
	ether_poll();    // check for new pkts, check timers
	if (millis() - ms > 5000) {   // active "delay"
		char str[128];
		sprintf(str, "%lu ms  in %lu  out %lu ", ms, inpkts, outpkts);
		Serial.println(str);
		print_stats();
		//	   Serial.println(thdstr);
		ms = millis();
	}

}
