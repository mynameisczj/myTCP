#ifndef SR_RDT_RECEIVER_H
#define SR_RDT_RECEIVER_H
#include "RdtReceiver.h"
#include <map>
class SRRdtReceiver :public RdtReceiver
{
private:
	int expectSequenceNumberRcvd;	// �ڴ��յ�����һ���������
	std::map<int, Packet> receivedSeqNums; //�ѽ��ձ�����ż���
	uint32_t windowSize;			//���ڴ�С
	Packet lastAckPkt;			//�ϴη��͵�ȷ�ϱ���

public:
	SRRdtReceiver();
	virtual ~SRRdtReceiver();

public:

	void receive(const Packet& packet);	//���ձ��ģ�����NetworkService����
};

#endif

