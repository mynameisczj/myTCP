#include "stdafx.h"
#include "Global.h"
#include "TCPRdtSender.h"

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
void TCPRdtSender::printDeque(const std::deque<Packet>& dq) {
	if (dq.empty()) {
		std::cout << "deque is empty" << std::endl;
		return;
	}
	for (auto i : dq) {
		std::cout << i.seqnum << " ";
	}
	std::cout << std::endl;
}

TCPRdtSender::TCPRdtSender() :expectSequenceNumberSend(0), windowSize(Configuration::GBN_SENDER_WINDOW_SIZE), base(0)
{
}


TCPRdtSender::~TCPRdtSender()
{
}



bool TCPRdtSender::getWaitingState() {
	return packetBuffer.size() < windowSize ? false : true;
}




bool TCPRdtSender::send(const Message& message) {
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

	this->packetBuffer.push_back(pkt); //将报文加入发送缓冲区
	pns->sendToNetworkLayer(RECEIVER, pkt);								//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
	pUtils->printPacket("发送方发送报文", pkt);
	this->expectSequenceNumberSend = (this->expectSequenceNumberSend + 1) % Configuration::MOD;

	base = this->packetBuffer.front().seqnum; //窗口基序号为缓冲区第一个报文的序号
	pns->startTimer(SENDER, Configuration::TIME_OUT, base);			//启动发送方定时器
	return true;
}

void TCPRdtSender::receive(const Packet& ackPkt) {
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	if (lastAckNum != ackPkt.acknum)
	{
		duplicateAckCount = 1;
		lastAckNum = ackPkt.acknum;
	}
	else duplicateAckCount++;
	if (duplicateAckCount >= 3) {
		/*cout << "快速重传：" << lastAckNum << std::endl;*/
		pns->sendToNetworkLayer(RECEIVER, packetBuffer.front());			//重新发送数据包
		pUtils->printPacket("快速重传", packetBuffer.front());
		duplicateAckCount = 0;
		return;
	}
	printDeque(this->packetBuffer);
	//如果校验和正确，并且确认序号=发送方已发送并等待确认的数据包序号
	if (!packetBuffer.empty() && checkSum == ackPkt.checksum && checkInWindow(ackPkt.acknum, this->base, windowSize)) {
		while (!packetBuffer.empty() && packetBuffer.front().seqnum != (ackPkt.acknum + 1) % Configuration::MOD) {
			this->packetBuffer.pop_front();
		}
		pns->stopTimer(SENDER, base);		//关闭定时器
		//this->packetBuffer.pop_front();	//将已确认的报文从发送缓冲区删除
		pUtils->printPacket("发送方正确收到确认", ackPkt);

		if (!this->packetBuffer.empty()) { //如果缓冲区不为空，启动定时器
			base = this->packetBuffer.front().seqnum; //窗口基序号为缓冲区第一个报文的序号
			pns->startTimer(SENDER, Configuration::TIME_OUT, base);			//启动发送方定时器
		}
	}
	printDeque(this->packetBuffer);
}

void TCPRdtSender::timeoutHandler(int seqNum) {
	//唯一一个定时器,无需考虑seqNum
	pns->stopTimer(SENDER, base);										//首先关闭定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT, base);			//重新启动发送方定时器
	pns->sendToNetworkLayer(RECEIVER, packetBuffer.front());			//重新发送数据包
	pUtils->printPacket("发送方重发报文", packetBuffer.front());
}
