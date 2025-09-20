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

	this->packetBuffer.push_back(pkt.seqnum); //将报文加入发送缓冲区
	sentPackets.emplace(pkt.seqnum, pkt); //将报文加入已发送但未确认的报文集合

	pns->sendToNetworkLayer(RECEIVER, pkt);								//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
	pUtils->printPacket("发送方发送报文", pkt);
	this->expectSequenceNumberSend = (this->expectSequenceNumberSend + 1) % Configuration::MOD;
	
	base = this->packetBuffer.front(); //窗口基序号为缓冲区第一个报文的序号
	pns->startTimer(SENDER, Configuration::TIME_OUT, pkt.seqnum);			//启动发送方定时器
	return true;
}

void SRRdtSender::receive(const Packet& ackPkt) {
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	printDeque(this->packetBuffer);
	//如果校验和正确，并且确认序号=发送方已发送并等待确认的数据包序号
	if (!packetBuffer.empty() && checkSum == ackPkt.checksum && checkInWindow(ackPkt.acknum, this->base, windowSize)) {
		//this->expectSequenceNumberSend = (this->expectSequenceNumberSend + 1) % Configuration::MOD; //发送序号在0~(MOD-1)之间循环	
		/*std::cout << this->expectSequenceNumberSend << std::endl;*/
		while (!packetBuffer.empty() && packetBuffer.front() != (ackPkt.acknum + 1) % Configuration::MOD) {
			pns->stopTimer(SENDER, packetBuffer.front());		//关闭定时器
			sentPackets.erase(packetBuffer.front()); //从已发送但未确认的报文集合中删除该报文
			this->packetBuffer.pop_front();
		}
		pUtils->printPacket("发送方正确收到确认", ackPkt);
	}
	printDeque(this->packetBuffer);
	if (!packetBuffer.empty()) {
		this->base = this->packetBuffer.front(); //窗口基序号为缓冲区第一个报文的序号
	}
}

void SRRdtSender::timeoutHandler(int seqNum) {

	pns->stopTimer(SENDER, seqNum);										//首先关闭定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//重新启动发送方定时器
	if (sentPackets.find(seqNum) == sentPackets.end()) return; //如果该报文已被确认，则不需要重传
	pns->sendToNetworkLayer(RECEIVER, sentPackets[seqNum]);			//重新发送数据包
	pUtils->printPacket("发送方重发报文", sentPackets[seqNum]);
}
