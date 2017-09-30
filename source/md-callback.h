#ifndef MD_CALLBACK_H
#define MD_CALLBACK_H

#include "market_plugins.h"
#include "ThostFtdcMdApi.h"
#include <EventTransport.h>
#include <EventCodec.h>
#include <IAF_TimestampConfig.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdexcept>
#include <sstream>
#include <WinDef.h>
#include <conio.h>
#include <iostream>

class MdCallback : public CThostFtdcMdSpi
{
public:

	MdCallback(AP_EventDecoder* pEventDecoder, HANDLE& loginHandle);

	virtual ~MdCallback(void);

	void OnFrontConnected();

	///���ͻ����뽻�׺�̨ͨ�����ӶϿ�ʱ���÷��������á���������������API���Զ��������ӣ��ͻ��˿ɲ�������
	///@param nReason ����ԭ��
	///        0x1001 �����ʧ��
	///        0x1002 ����дʧ��
	///        0x2001 ����������ʱ
	///        0x2002 ��������ʧ��
	///        0x2003 �յ�������
	void OnFrontDisconnected(int nReason);

	///������ʱ���档����ʱ��δ�յ�����ʱ���÷��������á�
	///@param nTimeLapse �����ϴν��ձ��ĵ�ʱ��
	void OnHeartBeatWarning(int nTimeLapse);

	///��¼������Ӧ
	void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///�ǳ�������Ӧ
	void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///����Ӧ��
	void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///��������Ӧ��
	void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///ȡ����������Ӧ��
	void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///�������֪ͨ
	void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* current);

private:
	
	void addString(struct AP_NormalisedEvent* ev, AP_NormalisedEventKey key, const char* value);

	void addInt(char* buf, int size, struct AP_NormalisedEvent* ev, AP_NormalisedEventKey key, int value);

	void addDouble(char* buf, int size, struct AP_NormalisedEvent* ev, AP_NormalisedEventKey key, double value);

private:

	AP_EventDecoder* m_pEventDecoder;
	HANDLE m_hLogin;

};

#endif
