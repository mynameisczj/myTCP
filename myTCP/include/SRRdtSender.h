#ifndef SR_RDT_SENDER_H
#define SR_RDT_SENDER_H
#include "RdtSender.h"
#include <map>
#include <deque>

class SRRdtSender :public RdtSender
{
private:
	int expectSequenceNumberSend;	// 下一个发送序号 

	std::deque<int> packetBuffer; //发送缓冲区，按发送顺序存储Packet.seqNum
	std::map<int, Packet> sentPackets; //已发送但未确认的报文集合，key为Packet.seqNum，value为Packet
	uint32_t windowSize;			//窗口大小
	int base;						//窗口基序号

public:

	bool getWaitingState();
	bool send(const Message& message);						//发送应用层下来的Message，由NetworkServiceSimulator调用,如果发送方成功地将Message发送到网络层，返回true;如果因为发送方处于等待正确确认状态而拒绝发送Message，则返回false
	void receive(const Packet& ackPkt);						//接受确认Ack，将被NetworkServiceSimulator调用	
	void timeoutHandler(int seqNum);					//Timeout handler，将被NetworkServiceSimulator调用

public:
	SRRdtSender();
	virtual ~SRRdtSender();
private:
	void printDeque(const std::deque<int>& dq);
};

#endif

