#ifndef SR_RDT_SENDER_H
#define SR_RDT_SENDER_H
#include "RdtSender.h"
#include <deque>
class SRRdtSender :public RdtSender
{
private:
	int expectSequenceNumberSend;	// ��һ��������� 
	//bool waitingState;				// �Ƿ��ڵȴ�Ack��״̬
	//Packet packetWaitingAck;		//�ѷ��Ͳ��ȴ�Ack�����ݰ�
	deque<Packet> packetBuffer;		//���ͻ�����
	uint32_t windowSize;			//���ڴ�С
	uint32_t base;					//���ڻ����

public:

	bool getWaitingState();
	bool send(const Message& message);						//����Ӧ�ò�������Message����NetworkServiceSimulator����,������ͷ��ɹ��ؽ�Message���͵�����㣬����true;�����Ϊ���ͷ����ڵȴ���ȷȷ��״̬���ܾ�����Message���򷵻�false
	void receive(const Packet& ackPkt);						//����ȷ��Ack������NetworkServiceSimulator����	
	void timeoutHandler(int seqNum);					//Timeout handler������NetworkServiceSimulator����

public:
	SRRdtSender();
	virtual ~SRRdtSender();
};

#endif

