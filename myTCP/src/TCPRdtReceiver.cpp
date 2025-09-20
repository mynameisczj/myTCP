#include "stdafx.h"
#include "Global.h"
#include "TCPRdtReceiver.h"

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
TCPRdtReceiver::TCPRdtReceiver() :expectSequenceNumberRcvd(0), windowSize(Configuration::GBN_RECEIVER_WINDOW_SIZE)
{
	lastAckPkt.acknum = -1; //��ʼ״̬�£��ϴη��͵�ȷ�ϰ���ȷ�����Ϊ-1��ʹ�õ���һ�����ܵ����ݰ�����ʱ��ȷ�ϱ��ĵ�ȷ�Ϻ�Ϊ-1
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;	//���Ը��ֶ�
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
		lastAckPkt.payload[i] = '.';
	}
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
}


TCPRdtReceiver::~TCPRdtReceiver()
{
}

void TCPRdtReceiver::receive(const Packet& packet) {
	//���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(packet);

	//���У�����ȷ��ͬʱ�յ����ĵ���ŵ��ڽ��շ��ڴ��յ��ı������һ��
	if (checkSum == packet.checksum && this->expectSequenceNumberRcvd == packet.seqnum) {
		pUtils->printPacket("���շ���ȷ�յ����ͷ��ı���", packet);
		receivedSeqNums.emplace(packet.seqnum, packet); //���յ��ı�����ż����ѽ��ձ�����ż���
		//ȡ��Message�����ϵݽ���Ӧ�ò�
		Message msg;
		while (receivedSeqNums.find(this->expectSequenceNumberRcvd) != receivedSeqNums.end()) {
			memcpy(msg.data, receivedSeqNums[this->expectSequenceNumberRcvd].payload, sizeof(receivedSeqNums[this->expectSequenceNumberRcvd].payload));

			pns->delivertoAppLayer(RECEIVER, msg);	//����ģ�����绷����delivertoAppLayer���ݽ���Ӧ�ò�

			lastAckPkt.acknum = this->expectSequenceNumberRcvd; //ȷ����ŵ����յ��ı������
			lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
			pUtils->printPacket("���շ�����ȷ�ϱ���", lastAckPkt);
			pns->sendToNetworkLayer(SENDER, lastAckPkt);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�

			receivedSeqNums.erase(this->expectSequenceNumberRcvd); //���ѽ��ձ�����ż�����ɾ���ñ������
			this->expectSequenceNumberRcvd = (this->expectSequenceNumberRcvd + 1) % Configuration::MOD; //���������0~(MOD-1)֮��ѭ��
		}
	}
	else {
		if (checkSum != packet.checksum) {
			pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı���,����У�����", packet);
		}
		else {
			pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı���,������Ų���", packet);
			if (checkInWindow(packet.seqnum, this->expectSequenceNumberRcvd, windowSize)) {
				receivedSeqNums.emplace(make_pair(packet.seqnum, packet));
			}
		}
		pUtils->printPacket("���շ����·����ϴε�ȷ�ϱ���", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢���ϴε�ȷ�ϱ���

	}
}