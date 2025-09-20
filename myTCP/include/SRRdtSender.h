#ifndef SR_RDT_SENDER_H
#define SR_RDT_SENDER_H
#include "RdtSender.h"
#include <map>
#include <deque>

class SRRdtSender :public RdtSender
{
private:
	int expectSequenceNumberSend;	// ��һ��������� 

	std::deque<int> packetBuffer; //���ͻ�������������˳��洢Packet.seqNum
	std::map<int, Packet> sentPackets; //�ѷ��͵�δȷ�ϵı��ļ��ϣ�keyΪPacket.seqNum��valueΪPacket
	uint32_t windowSize;			//���ڴ�С
	int base;						//���ڻ����

public:

	bool getWaitingState();
	bool send(const Message& message);						//����Ӧ�ò�������Message����NetworkServiceSimulator����,������ͷ��ɹ��ؽ�Message���͵�����㣬����true;�����Ϊ���ͷ����ڵȴ���ȷȷ��״̬���ܾ�����Message���򷵻�false
	void receive(const Packet& ackPkt);						//����ȷ��Ack������NetworkServiceSimulator����	
	void timeoutHandler(int seqNum);					//Timeout handler������NetworkServiceSimulator����

public:
	SRRdtSender();
	virtual ~SRRdtSender();
private:
	void printDeque(const std::deque<int>& dq);
};

#endif

