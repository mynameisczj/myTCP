#include "stdafx.h"
#include "Global.h"
#include "GBNRdtSender.h"

static bool checkInWindow(int seqNum, int base, int windowSize) {
	if (base + windowSize < Configuration::MOD) {
		if (seqNum >= base && seqNum < base + windowSize) return true;
		else return false;
	}
	else {
		if (seqNum >= base || seqNum < (base + windowSize) % Configuration::MOD) return true;
		else return false;
	}
}
void GBNRdtSender::printDeque(const std::deque<Packet>& dq) {
	if (dq.empty()) {
		std::cout << "deque is empty" << std::endl;
		return;
	}
	for (auto i : dq) {
		std::cout << i.seqnum << " ";
	}
	std::cout << std::endl;
}

GBNRdtSender::GBNRdtSender() :expectSequenceNumberSend(0),windowSize(Configuration::GBN_SENDER_WINDOW_SIZE), base(0)
{
}


GBNRdtSender::~GBNRdtSender()
{
}



bool GBNRdtSender::getWaitingState() {
	return packetBuffer.size()<windowSize?false:true;
}




bool GBNRdtSender::send(const Message& message) {
	if (this->getWaitingState()) { //�������������ɷ���
		return false;
	}
	Packet pkt;
	//��䱨��
	
	pkt.acknum = -1; //���Ը��ֶ�
	pkt.seqnum = this->expectSequenceNumberSend;
	pkt.checksum = 0;
	memcpy(pkt.payload, message.data, sizeof(message.data));
	pkt.checksum = pUtils->calculateCheckSum(pkt);

	this->packetBuffer.push_back(pkt); //�����ļ��뷢�ͻ�����
	pns->sendToNetworkLayer(RECEIVER, pkt);								//����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�
	pUtils->printPacket("���ͷ����ͱ���", pkt);
	this->expectSequenceNumberSend =(this->expectSequenceNumberSend+1)%Configuration::MOD; 
	
	base = this->packetBuffer.front().seqnum; //���ڻ����Ϊ��������һ�����ĵ����
	pns->startTimer(SENDER, Configuration::TIME_OUT, base);			//�������ͷ���ʱ��
	return true;
}

void GBNRdtSender::receive(const Packet& ackPkt) {
	//���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	printDeque(this->packetBuffer);
	//���У�����ȷ������ȷ�����=���ͷ��ѷ��Ͳ��ȴ�ȷ�ϵ����ݰ����
	if (!packetBuffer.empty()&& checkSum == ackPkt.checksum &&checkInWindow(ackPkt.acknum,this->base,windowSize)) {
		while(!packetBuffer.empty() && packetBuffer.front().seqnum != (ackPkt.acknum + 1) % Configuration::MOD) {
			this->packetBuffer.pop_front();
		}
		pns->stopTimer(SENDER, base);		//�رն�ʱ��
		//this->packetBuffer.pop_front();	//����ȷ�ϵı��Ĵӷ��ͻ�����ɾ��
		pUtils->printPacket("���ͷ���ȷ�յ�ȷ��", ackPkt);

		if (!this->packetBuffer.empty()) { //�����������Ϊ�գ�������ʱ��
			base = this->packetBuffer.front().seqnum; //���ڻ����Ϊ��������һ�����ĵ����
			pns->startTimer(SENDER, Configuration::TIME_OUT, base);			//�������ͷ���ʱ��
		}
	}
	printDeque(this->packetBuffer);
}

void GBNRdtSender::timeoutHandler(int seqNum) {
	//Ψһһ����ʱ��,���迼��seqNum
	pns->stopTimer(SENDER, base);										//���ȹرն�ʱ��
	pns->startTimer(SENDER, Configuration::TIME_OUT, base);			//�����������ͷ���ʱ��
	for(auto pkt:this->packetBuffer)
	{
		pns->sendToNetworkLayer(RECEIVER, pkt);			//���·������ݰ�
		pUtils->printPacket("���ͷ��ط�����", pkt);
	}
}
