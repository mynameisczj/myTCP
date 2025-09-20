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
	if (this->getWaitingState()) { //窗口已满，不可发送
		return false;
	}
	Packet pkt;
	//填充报文
	
	pkt.acknum = -1; //忽略该字段
	pkt.seqnum = this->expectSequenceNumberSend;
	pkt.checksum = 0;
	memcpy(pkt.payload, message.data, sizeof(message.data));
	pkt.checksum = pUtils->calculateCheckSum(pkt);

	this->packetBuffer.emplace(make_pair(pkt.seqnum,pkt)); //将报文加入发送缓冲区
	pns->sendToNetworkLayer(RECEIVER, pkt);								//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
	pUtils->printPacket("发送方发送报文", pkt);
	this->expectSequenceNumberSend = (this->expectSequenceNumberSend + 1) % Configuration::MOD;

	pns->startTimer(SENDER, Configuration::TIME_OUT, pkt.seqnum);			//启动发送方定时器
	return true;
}

void SRRdtSender::receive(const Packet& ackPkt) {
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	//如果校验和正确，并且确认序号=发送方已发送并等待确认的数据包序号
	if (!packetBuffer.empty() && checkSum == ackPkt.checksum && checkInWindow(ackPkt.acknum, packetBuffer.begin()->first, windowSize)) {
		while (!packetBuffer.empty() && packetBuffer.begin()->first != (ackPkt.acknum + 1) % Configuration::MOD) {
			pns->stopTimer(SENDER, packetBuffer.begin()->first);		//关闭定时器
			this->packetBuffer.erase(packetBuffer.begin());
		}
		pUtils->printPacket("发送方正确收到确认", ackPkt);

	}
	else {}
}

void SRRdtSender::timeoutHandler(int seqNum) {

	pns->stopTimer(SENDER, seqNum);										//首先关闭定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//重新启动发送方定时器
	
	if (packetBuffer.find(seqNum) == packetBuffer.end()) return; //如果该seqNum不在发送缓冲区中，直接返回
	pns->sendToNetworkLayer(RECEIVER, packetBuffer[seqNum]);			//重新发送数据包
	pUtils->printPacket("发送方重发报文", packetBuffer[seqNum]);
}
