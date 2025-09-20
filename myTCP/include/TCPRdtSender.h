#ifndef TCP_RDT_SENDER_H
#define TCP_RDT_SENDER_H
#include "RdtSender.h"
#include <deque>
class TCPRdtSender :public RdtSender
{
private:
	int expectSequenceNumberSend;	// 下一个发送序号 
	std::deque<Packet> packetBuffer;		//发送缓冲区
	uint32_t windowSize;			//窗口大小
	uint32_t base;					//窗口基序号
	//快速重传相关
	int lastAckNum = -1;			//上次收到的确认号
	int duplicateAckCount = 0;		//重复确认计数

public:

	bool getWaitingState();
	bool send(const Message& message);						//发送应用层下来的Message，由NetworkServiceSimulator调用,如果发送方成功地将Message发送到网络层，返回true;如果因为发送方处于等待正确确认状态而拒绝发送Message，则返回false
	void receive(const Packet& ackPkt);						//接受确认Ack，将被NetworkServiceSimulator调用	
	void timeoutHandler(int seqNum);					//Timeout handler，将被NetworkServiceSimulator调用

public:
	TCPRdtSender();
	virtual ~TCPRdtSender();
private:
	void printDeque(const std::deque<Packet>& dq);
};

#endif

