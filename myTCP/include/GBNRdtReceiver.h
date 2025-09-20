#ifndef GBN_RDT_RECEIVER_H
#define GBN_RDT_RECEIVER_H
#include "RdtReceiver.h"
#include <map>
class GBNRdtReceiver :public RdtReceiver
{
private:
	int expectSequenceNumberRcvd;	// �ڴ��յ�����һ���������
	std::map<int,Packet> receivedSeqNums; //�ѽ��ձ�����ż���
	uint32_t windowSize;			//���ڴ�С
	Packet lastAckPkt;			//�ϴη��͵�ȷ�ϱ���

public:
	GBNRdtReceiver();
	virtual ~GBNRdtReceiver();

public:

	void receive(const Packet& packet);	//���ձ��ģ�����NetworkService����
};

#endif

