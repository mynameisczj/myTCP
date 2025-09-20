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
void SRRdtSender::printDeque(const std::deque<int>& dq) {
	if(dq.empty()) {
		std::cout << "deque is empty" << std::endl;
		return;
	}
	for (auto i : dq) {
		std::cout << i << " ";
	}
	std::cout << std::endl;
}

SRRdtSender::SRRdtSender() :expectSequenceNumberSend(0), windowSize(Configuration::SR_WINDOW_SIZE), base(0)
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

	this->packetBuffer.push_back(pkt.seqnum); //�����ļ��뷢�ͻ�����
	sentPackets.emplace(pkt.seqnum, pkt); //�����ļ����ѷ��͵�δȷ�ϵı��ļ���

	pns->sendToNetworkLayer(RECEIVER, pkt);								//����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�
	pUtils->printPacket("���ͷ����ͱ���", pkt);
	this->expectSequenceNumberSend = (this->expectSequenceNumberSend + 1) % Configuration::MOD;
	
	base = this->packetBuffer.front(); //���ڻ����Ϊ��������һ�����ĵ����
	pns->startTimer(SENDER, Configuration::TIME_OUT, pkt.seqnum);			//�������ͷ���ʱ��
	return true;
}

void SRRdtSender::receive(const Packet& ackPkt) {
	//���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	printDeque(this->packetBuffer);
	//���У�����ȷ������ȷ�����=���ͷ��ѷ��Ͳ��ȴ�ȷ�ϵ����ݰ����
	if (!packetBuffer.empty() && checkSum == ackPkt.checksum && checkInWindow(ackPkt.acknum, this->base, windowSize)) {
		//this->expectSequenceNumberSend = (this->expectSequenceNumberSend + 1) % Configuration::MOD; //���������0~(MOD-1)֮��ѭ��	
		/*std::cout << this->expectSequenceNumberSend << std::endl;*/
		while (!packetBuffer.empty() && packetBuffer.front() != (ackPkt.acknum + 1) % Configuration::MOD) {
			pns->stopTimer(SENDER, packetBuffer.front());		//�رն�ʱ��
			sentPackets.erase(packetBuffer.front()); //���ѷ��͵�δȷ�ϵı��ļ�����ɾ���ñ���
			this->packetBuffer.pop_front();
		}
		pUtils->printPacket("���ͷ���ȷ�յ�ȷ��", ackPkt);
	}
	printDeque(this->packetBuffer);
	if (!packetBuffer.empty()) {
		this->base = this->packetBuffer.front(); //���ڻ����Ϊ��������һ�����ĵ����
	}
}

void SRRdtSender::timeoutHandler(int seqNum) {

	pns->stopTimer(SENDER, seqNum);										//���ȹرն�ʱ��
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//�����������ͷ���ʱ��
	if (sentPackets.find(seqNum) == sentPackets.end()) return; //����ñ����ѱ�ȷ�ϣ�����Ҫ�ش�
	pns->sendToNetworkLayer(RECEIVER, sentPackets[seqNum]);			//���·������ݰ�
	pUtils->printPacket("���ͷ��ط�����", sentPackets[seqNum]);
}
