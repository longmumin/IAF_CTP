#include "md-callback.h"
#include "engine_client_cpp.hpp"

#define CHAR_BUF_SIZE 128

MdCallback::MdCallback(AP_EventDecoder* pEventDecoder, HANDLE& loginHandle)
{	
	m_pEventDecoder = pEventDecoder;
	m_hLogin = loginHandle;
}

MdCallback::~MdCallback(void)
{
}

void MdCallback::addString(struct AP_NormalisedEvent* ev, AP_NormalisedEventKey key, const char* value)
{
	if (value == NULL){
		ev->functions->addQuick(ev, key, "");
	}else{
		AP_char8* utf8 = com::apama::convertToUTF8(value);
		ev->functions->addQuick(ev, key, utf8);
	}
}

void MdCallback::addInt(char* buf, int size, struct AP_NormalisedEvent* ev, AP_NormalisedEventKey key, int value) 
{
	sprintf_s(buf, size, "%d", value);
	ev->functions->addQuick(ev, key, buf);
}

void MdCallback::addDouble(char* buf, int size, struct AP_NormalisedEvent* ev, AP_NormalisedEventKey key, double value) 
{
	if (value > 999999999999.9999 || value < -999999999999.9999) {
		AP_LogWarn("ignoring double [%s] %12.4f", key, value);
	} else {
		sprintf_s(buf, size, "%12.4f", value);
		ev->functions->addQuick(ev, key, buf);
	}
}

void MdCallback::OnFrontConnected()
{	
	AP_LogInfo("OnFrontConnected");
	SetEvent(m_hLogin);	
	{
		AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
		addString(ev, "Type", "OnFrontConnected");
		AP_TimestampSet* tss = AP_TimestampSet_ctor();
		m_pEventDecoder->functions->sendTransportEvent(m_pEventDecoder, ev, tss);
	}
}


///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
///@param nReason 错误原因
///        0x1001 网络读失败
///        0x1002 网络写失败
///        0x2001 接收心跳超时
///        0x2002 发送心跳失败
///        0x2003 收到错误报文
void MdCallback::OnFrontDisconnected(int nReason) 
{	
	AP_LogInfo("OnFrontDisconnected-->>Reason: %d", nReason);
	{
		AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
		char* buf = new char[CHAR_BUF_SIZE];
		addString(ev, "Type", "OnFrontDisconnected");
		addInt(buf, CHAR_BUF_SIZE, ev, "reason", nReason);
		delete[] buf;
		buf = NULL;
		AP_TimestampSet* tss = AP_TimestampSet_ctor();
		m_pEventDecoder->functions->sendTransportEvent(m_pEventDecoder, ev, tss);
	}
}		

///心跳超时警告。当长时间未收到报文时，该方法被调用。
///@param nTimeLapse 距离上次接收报文的时间
void MdCallback::OnHeartBeatWarning(int nTimeLapse)
{
	AP_LogInfo("OnHeartBeatWarning-->>TimeLapse: %d", nTimeLapse);
	{
		AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
		char* buf = new char[CHAR_BUF_SIZE];
		addString(ev, "Type", "OnHeartBeatWarning");
		addInt(buf, CHAR_BUF_SIZE, ev, "timeLapse", nTimeLapse);
		delete[] buf;
		buf = NULL;
		AP_TimestampSet* tss = AP_TimestampSet_ctor();
		m_pEventDecoder->functions->sendTransportEvent(m_pEventDecoder, ev, tss);
	}
}

///登录请求响应
void MdCallback::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{	
	AP_LogInfo("OnRspUserLogin");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addString(ev, "Type", "OnRspUserLogin");
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pRspUserLogin != NULL){	
		addString(ev, "tradingDay", pRspUserLogin->TradingDay);
		addString(ev, "loginTime", pRspUserLogin->LoginTime);
		addString(ev, "brokerId", pRspUserLogin->BrokerID);
		addString(ev, "userId", pRspUserLogin->UserID);
		addString(ev, "systemName", pRspUserLogin->SystemName);
		addInt(buf, CHAR_BUF_SIZE, ev, "frontId", pRspUserLogin->FrontID);
		addInt(buf, CHAR_BUF_SIZE, ev, "sessionId", pRspUserLogin->SessionID);
		addString(ev, "maxOrderRef", pRspUserLogin->MaxOrderRef);
		addString(ev, "shfeTime", pRspUserLogin->SHFETime);
		addString(ev, "dceTime", pRspUserLogin->DCETime);
		addString(ev, "czceTime", pRspUserLogin->CZCETime);
		addString(ev, "ffexTime", pRspUserLogin->FFEXTime);
	}

	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	delete[] buf;
	buf = NULL;
	AP_TimestampSet* tss = AP_TimestampSet_ctor();
	m_pEventDecoder->functions->sendTransportEvent(m_pEventDecoder, ev, tss);
}

///登出请求响应
void MdCallback::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{	
	AP_LogInfo("OnRspUserLogout");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addString(ev, "Type", "OnRspUserLogout");
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pRspInfo != NULL){	
		addString(ev, "brokerId", pUserLogout->BrokerID);
		addString(ev, "userId", pUserLogout->UserID);
	}
	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);	
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	delete[] buf;
	buf = NULL;
	AP_TimestampSet* tss = AP_TimestampSet_ctor();
	m_pEventDecoder->functions->sendTransportEvent(m_pEventDecoder, ev, tss);
}

///错误应答
void MdCallback::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{	
	AP_LogInfo("OnRspError");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addString(ev, "Type", "OnRspError");
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);	
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	delete[] buf;
	buf = NULL;
	AP_TimestampSet* tss = AP_TimestampSet_ctor();
	m_pEventDecoder->functions->sendTransportEvent(m_pEventDecoder, ev, tss);
}

///订阅行情应答
void MdCallback::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{	
	AP_LogInfo("OnRspSubMarketData");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addString(ev, "Type", "OnRspSubMarketData");
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pSpecificInstrument != NULL){
		addString(ev, "instrumentId", pSpecificInstrument->InstrumentID);
	}
	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	delete[] buf;
	buf = NULL;
	AP_TimestampSet* tss = AP_TimestampSet_ctor();
	m_pEventDecoder->functions->sendTransportEvent(m_pEventDecoder, ev, tss);
}

///取消订阅行情应答
void MdCallback::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	AP_LogInfo("OnRspUnSubMarketData");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addString(ev, "Type", "OnRspUnSubMarketData");
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pSpecificInstrument != NULL){
		addString(ev, "instrumentId", pSpecificInstrument->InstrumentID);
	}	
	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);	
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	delete[] buf;
	buf = NULL;
	AP_TimestampSet* tss = AP_TimestampSet_ctor();
	m_pEventDecoder->functions->sendTransportEvent(m_pEventDecoder, ev, tss);
}

///深度行情通知
void MdCallback::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* current)
{
	if (current == NULL)
		return;

	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addString(ev, "Type", "OnRtnDepthMarketData");
	addString(ev, "tradingDay", current->TradingDay);
	addString(ev, "instrumentId", current->InstrumentID);
	addString(ev, "exchangeId", current->ExchangeID);
	addString(ev, "exchangeInstId", current->ExchangeInstID);
	addDouble(buf, CHAR_BUF_SIZE, ev, "lastPrice", current->LastPrice);
	addDouble(buf, CHAR_BUF_SIZE, ev, "preSettlementPrice", current->PreSettlementPrice);
	addDouble(buf, CHAR_BUF_SIZE, ev, "preClosePrice", current->PreClosePrice);
	addDouble(buf, CHAR_BUF_SIZE, ev, "preOpenInterest", current->PreOpenInterest);
	addDouble(buf, CHAR_BUF_SIZE, ev, "openPrice", current->OpenPrice);
	addDouble(buf, CHAR_BUF_SIZE, ev, "highestPrice", current->HighestPrice);
	addDouble(buf, CHAR_BUF_SIZE, ev, "lowestPrice", current->LowestPrice);
	addInt(buf, CHAR_BUF_SIZE, ev, "volume", current->Volume);
	addDouble(buf, CHAR_BUF_SIZE, ev, "turnover", current->Turnover);
	addDouble(buf, CHAR_BUF_SIZE, ev, "openInterest", current->OpenInterest);
	addDouble(buf, CHAR_BUF_SIZE, ev, "closePrice", current->ClosePrice);
	addDouble(buf, CHAR_BUF_SIZE, ev, "settlementPrice", current->SettlementPrice);
	addDouble(buf, CHAR_BUF_SIZE, ev, "lowerLimitPrice", current->LowerLimitPrice);
	addDouble(buf, CHAR_BUF_SIZE, ev, "upperLimitPrice", current->UpperLimitPrice);
	addDouble(buf, CHAR_BUF_SIZE, ev, "preDelta", current->PreDelta);
	addDouble(buf, CHAR_BUF_SIZE, ev, "currDelta", current->CurrDelta);
	addString(ev, "updateTime", current->UpdateTime);
	addInt(buf, CHAR_BUF_SIZE, ev, "updateMillisec", current->UpdateMillisec);

	addDouble(buf, CHAR_BUF_SIZE, ev, "bidPrice1", current->BidPrice1);
	addInt(buf, CHAR_BUF_SIZE, ev, "bidVolume1", current->BidVolume1);
	addDouble(buf, CHAR_BUF_SIZE, ev, "askPrice1", current->AskPrice1);
	addInt(buf, CHAR_BUF_SIZE, ev, "askVolume1", current->AskVolume1);

	addDouble(buf, CHAR_BUF_SIZE, ev, "bidPrice2", current->BidPrice2);
	addInt(buf, CHAR_BUF_SIZE, ev, "bidVolume2", current->BidVolume2);
	addDouble(buf, CHAR_BUF_SIZE, ev, "askPrice2", current->AskPrice2);
	addInt(buf, CHAR_BUF_SIZE, ev, "askVolume2", current->AskVolume2);

	addDouble(buf, CHAR_BUF_SIZE, ev, "bidPrice3", current->BidPrice3);
	addInt(buf, CHAR_BUF_SIZE, ev, "bidVolume3", current->BidVolume3);
	addDouble(buf, CHAR_BUF_SIZE, ev, "askPrice3", current->AskPrice3);
	addInt(buf, CHAR_BUF_SIZE, ev, "askVolume3", current->AskVolume3);

	addDouble(buf, CHAR_BUF_SIZE, ev, "bidPrice4", current->BidPrice4);
	addInt(buf, CHAR_BUF_SIZE, ev, "bidVolume4", current->BidVolume4);
	addDouble(buf, CHAR_BUF_SIZE, ev, "askPrice4", current->AskPrice4);
	addInt(buf, CHAR_BUF_SIZE, ev, "askVolume4", current->AskVolume4);

	addDouble(buf, CHAR_BUF_SIZE, ev, "bidPrice5", current->BidPrice5);
	addInt(buf, CHAR_BUF_SIZE, ev, "bidVolume5", current->BidVolume5);
	addDouble(buf, CHAR_BUF_SIZE, ev, "askPrice5", current->AskPrice5);
	addInt(buf, CHAR_BUF_SIZE, ev, "askVolume5", current->AskVolume5);
	
	addDouble(buf, CHAR_BUF_SIZE, ev, "averagePrice",  current->AveragePrice);
	delete[] buf;
	buf = NULL;
	AP_TimestampSet* tss = AP_TimestampSet_ctor();
	m_pEventDecoder->functions->sendTransportEvent(m_pEventDecoder, ev, tss);
}