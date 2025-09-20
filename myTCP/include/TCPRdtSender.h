#ifndef TCP_RDT_SENDER_H
#define TCP_RDT_SENDER_H
#include "RdtSender.h"
#include <deque>
class TCPRdtSender :public RdtSender
{
private:
	int expectSequenceNumberSend;	// ��һ��������� 
	std::deque<Packet> packetBuffer;		//���ͻ�����
	uint32_t windowSize;			//���ڴ�С
	uint32_t base;					//���ڻ����
	//�����ش����
	int lastAckNum = -1;			//�ϴ��յ���ȷ�Ϻ�
	int duplicateAckCount = 0;		//�ظ�ȷ�ϼ���

public:

	bool getWaitingState();
	bool send(const Message& message);						//����Ӧ�ò�������Message����NetworkServiceSimulator����,������ͷ��ɹ��ؽ�Message���͵�����㣬����true;�����Ϊ���ͷ����ڵȴ���ȷȷ��״̬���ܾ�����Message���򷵻�false
	void receive(const Packet& ackPkt);						//����ȷ��Ack������NetworkServiceSimulator����	
	void timeoutHandler(int seqNum);					//Timeout handler������NetworkServiceSimulator����

public:
	TCPRdtSender();
	virtual ~TCPRdtSender();
private:
	void printDeque(const std::deque<Packet>& dq);
};

#endif

