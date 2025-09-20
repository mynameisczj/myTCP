// SR.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "Global.h"
#include "RdtSender.h"
#include "RdtReceiver.h"
#include "TCPRdtSender.h"
#include "TCPRdtReceiver.h"


int main(int argc, char* argv[])
{
	FILE* stream;
	freopen_s(&stream, "./Log/TCPlog.txt", "w", stdout); // "w" ��ʾд�루���ǣ�
	freopen_s(&stream, "./Log/TCPlog.txt", "a", stderr); // "a" ��ʾ׷��

	RdtSender* ps = new TCPRdtSender();
	RdtReceiver* pr = new TCPRdtReceiver();
	//pns->setRunMode(0);  //VERBOSģʽ
	pns->setRunMode(1);  //����ģʽ
	pns->init();
	pns->setRtdSender(ps);
	pns->setRtdReceiver(pr);
	pns->setInputFile("D:/homeworkspace/network/myTCP/myTCP/myTCP/input.txt");
	pns->setOutputFile("D:/homeworkspace/network/myTCP/myTCP/myTCP/output.txt");

	pns->start();

	delete ps;
	delete pr;
	delete pUtils;									//ָ��Ψһ�Ĺ�����ʵ����ֻ��main��������ǰdelete
	delete pns;										//ָ��Ψһ��ģ�����绷����ʵ����ֻ��main��������ǰdelete

	return 0;
}
