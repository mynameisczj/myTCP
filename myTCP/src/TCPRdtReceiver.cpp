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
	lastAckPkt.acknum = -1; //初始状态下，上次发送的确认包的确认序号为-1，使得当第一个接受的数据包出错时该确认报文的确认号为-1
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;	//忽略该字段
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
		lastAckPkt.payload[i] = '.';
	}
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
}


TCPRdtReceiver::~TCPRdtReceiver()
{
}

void TCPRdtReceiver::receive(const Packet& packet) {
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(packet);

	//如果校验和正确，同时收到报文的序号等于接收方期待收到的报文序号一致
	if (checkSum == packet.checksum && this->expectSequenceNumberRcvd == packet.seqnum) {
		pUtils->printPacket("接收方正确收到发送方的报文", packet);
		receivedSeqNums.emplace(packet.seqnum, packet); //将收到的报文序号加入已接收报文序号集合
		//取出Message，向上递交给应用层
		Message msg;
		while (receivedSeqNums.find(this->expectSequenceNumberRcvd) != receivedSeqNums.end()) {
			memcpy(msg.data, receivedSeqNums[this->expectSequenceNumberRcvd].payload, sizeof(receivedSeqNums[this->expectSequenceNumberRcvd].payload));

			pns->delivertoAppLayer(RECEIVER, msg);	//调用模拟网络环境的delivertoAppLayer，递交给应用层

			lastAckPkt.acknum = this->expectSequenceNumberRcvd; //确认序号等于收到的报文序号
			lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
			pUtils->printPacket("接收方发送确认报文", lastAckPkt);
			pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方

			receivedSeqNums.erase(this->expectSequenceNumberRcvd); //从已接收报文序号集合中删除该报文序号
			this->expectSequenceNumberRcvd = (this->expectSequenceNumberRcvd + 1) % Configuration::MOD; //接收序号在0~(MOD-1)之间循环
		}
	}
	else {
		if (checkSum != packet.checksum) {
			pUtils->printPacket("接收方没有正确收到发送方的报文,数据校验错误", packet);
		}
		else {
			pUtils->printPacket("接收方没有正确收到发送方的报文,报文序号不对", packet);
			if (checkInWindow(packet.seqnum, this->expectSequenceNumberRcvd, windowSize)) {
				receivedSeqNums.emplace(make_pair(packet.seqnum, packet));
			}
		}
		pUtils->printPacket("接收方重新发送上次的确认报文", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送上次的确认报文

	}
}