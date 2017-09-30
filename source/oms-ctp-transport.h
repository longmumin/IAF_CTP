#ifndef OMS_TRANSPORT_H
#define OMS_TRANSPORT_H
#include "oms-callback.h"
#include "thostftdctraderapi.h"
#include <string>
#include <map>

using namespace std;


class OMS_Transport
{
public:
	OMS_Transport(const char* _name, AP_EventTransportProperties* properties, AP_EventTransportError* err, IAF_TimestampConfig* timestampConfig);
	virtual ~OMS_Transport();
	void clearError();
	void setError(const AP_char8* msg);
	AP_EventTransportError updateProperties(AP_EventTransportProperties* properties, IAF_TimestampConfig* timestampConfig);
	AP_EventTransportError sendTransportEvent(AP_TransportEvent tevent, AP_TimestampSet* tss);
	void addEventDecoder(const AP_char8* name, AP_EventDecoder* decoder);
	void removeEventDecoder(const AP_char8* name);
	AP_EventTransportError flushUpstream();
	AP_EventTransportError flushDownstream();
	AP_EventTransportError start();
	AP_EventTransportError stop();
	const AP_char8* getLastError();
	void getStatus(AP_EventTransportStatus* status);
	void decode(AP_NormalisedEvent* ev, AP_NormalisedEventValue type);

private:
	bool ReqUserLogin(AP_NormalisedEvent* ev);
	bool ReqUserLogout(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev);
	bool ReqOrderInsert(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev);
	bool ReqParkedOrderInsert(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev);
	bool ReqParkedOrderAction(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev);
	bool ReqOrderAction(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev);
	bool ReqQueryMaxOrderVolume(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev);
	bool ReqQrySettlementInfoConfirm(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev);
	bool ReqQryOrder(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev);
	bool ReqQryTrade(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev);
	bool ReqQryInvestorPosition(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev);
	bool ReqQryTradingAccount(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev);
	bool ReqQryInvestor(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev);
	bool ReqQryTradingCode(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev);
	bool ReqQryInstrumentMarginRate(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev);
	bool ReqQryInstrumentCommissionRate(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev);
	bool ReqQryExchange(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev);
	bool ReqQryInstrument(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev);
	bool ReqQryDepthMarketData(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev);
	bool ReqQrySettlementInfo(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev);
	bool ReqQryInvestorPositionDetail(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev);
	bool ReqSettlementInfoConfirm(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev);
	bool ReqQryInvestorPositionCombineDetail(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev);
	bool ReqQryEWarrantOffset(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev);
	bool ReqQryTradingNotice(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev);
	bool ReqQryBrokerTradingParams(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev);
	bool ReqQryBrokerTradingAlgos(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev);
	bool ReqRemoveParkedOrder(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev);
	bool ReqRemoveParkedOrderAction(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev);
	bool ReqQryParkedOrder(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev);
	bool ReqQryParkedOrderAction(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev);

private:
	void ShowError(int ret, int reqId);
private:
	
	IAF_TimestampConfig* m_pTimestampConfig;
	AP_EventTransportStatus	mRunStatus; 
	AP_char8* m_pLastError; 
	AP_EventDecoder* m_pDecoder; 
	map<string, CThostFtdcTraderApi*> m_mapTradeApis;
	map<string, OMSCallback*> m_mapCallBacks;	
	MUTEX_T mLock;
	string m_szIP;
	string m_szBroker;
	int m_nErrcode;
	char* m_pErrMsg;
};

#endif