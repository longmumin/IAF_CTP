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

	///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
	///@param nReason 错误原因
	///        0x1001 网络读失败
	///        0x1002 网络写失败
	///        0x2001 接收心跳超时
	///        0x2002 发送心跳失败
	///        0x2003 收到错误报文
	void OnFrontDisconnected(int nReason);

	///心跳超时警告。当长时间未收到报文时，该方法被调用。
	///@param nTimeLapse 距离上次接收报文的时间
	void OnHeartBeatWarning(int nTimeLapse);

	///登录请求响应
	void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///登出请求响应
	void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///错误应答
	void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///订阅行情应答
	void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///取消订阅行情应答
	void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///深度行情通知
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
