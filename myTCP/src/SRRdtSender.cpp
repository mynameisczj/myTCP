#include "stdafx.h"
#include "Global.h"
#include "SRRdtSender.h"

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

SRRdtSender::SRRdtSender() :expectSequenceNumberSend(0), windowSize(Configuration::SR_WINDOW_SIZE)
{
}


SRRdtSender::~SRRdtSender()
{
}



bool SRRdtSender::getWaitingState() {
	return packetBuffer.size() < windowSize ? false : true;
}




bool SRRdtSender::send(const Message& message) {
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

	this->packetBuffer.emplace(make_pair(pkt.seqnum,pkt)); //�����ļ��뷢�ͻ�����
	pns->sendToNetworkLayer(RECEIVER, pkt);								//����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�
	pUtils->printPacket("���ͷ����ͱ���", pkt);
	this->expectSequenceNumberSend = (this->expectSequenceNumberSend + 1) % Configuration::MOD;

	pns->startTimer(SENDER, Configuration::TIME_OUT, pkt.seqnum);			//�������ͷ���ʱ��
	return true;
}

void SRRdtSender::receive(const Packet& ackPkt) {
	//���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	//���У�����ȷ������ȷ�����=���ͷ��ѷ��Ͳ��ȴ�ȷ�ϵ����ݰ����
	if (!packetBuffer.empty() && checkSum == ackPkt.checksum && checkInWindow(ackPkt.acknum, packetBuffer.begin()->first, windowSize)) {
		while (!packetBuffer.empty() && packetBuffer.begin()->first != (ackPkt.acknum + 1) % Configuration::MOD) {
			pns->stopTimer(SENDER, packetBuffer.begin()->first);		//�رն�ʱ��
			this->packetBuffer.erase(packetBuffer.begin());
		}
		pUtils->printPacket("���ͷ���ȷ�յ�ȷ��", ackPkt);

	}
	else {}
}

void SRRdtSender::timeoutHandler(int seqNum) {

	pns->stopTimer(SENDER, seqNum);										//���ȹرն�ʱ��
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//�����������ͷ���ʱ��
	
	if (packetBuffer.find(seqNum) == packetBuffer.end()) return; //�����seqNum���ڷ��ͻ������У�ֱ�ӷ���
	pns->sendToNetworkLayer(RECEIVER, packetBuffer[seqNum]);			//���·������ݰ�
	pUtils->printPacket("���ͷ��ط�����", packetBuffer[seqNum]);
}
