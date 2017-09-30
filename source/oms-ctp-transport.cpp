#include "oms-ctp-transport.h"
#include "engine_client_cpp.hpp"

#define STATUS_BUFSIZE 256
#define CHAR_BUF_SIZE 128

void addString(struct AP_NormalisedEvent* ev, AP_NormalisedEventKey key, const char* value)
{
	if (value == NULL){
		ev->functions->addQuick(ev, key, "");
	} else {
		AP_char8* utf8 = com::apama::convertToUTF8(value);
		ev->functions->addQuick(ev, key, utf8);
	}
}

void addInt(char* buf, int size, struct AP_NormalisedEvent* ev, AP_NormalisedEventKey key, int value) 
{
	sprintf_s(buf, size, "%d", value);
	ev->functions->addQuick(ev, key, buf);
}

const char* getString(AP_NormalisedEvent* ev, AP_NormalisedEventKey key) 
{
	const char* value = ev->functions->findValue(ev, key);
	return value == NULL ? "" : value;
}

const int getInt(AP_NormalisedEvent* ev, AP_NormalisedEventKey key) 
{
	const char* value = ev->functions->findValue(ev, key);
	return value == NULL ? -1 : atoi(value);
}

const bool getBool(AP_NormalisedEvent* ev, AP_NormalisedEventKey key) 
{
	const char* value = ev->functions->findValue(ev, key);
	return value == NULL ? false : (strcmp(value, "true") == 0);
}

const char getChar(AP_NormalisedEvent* ev, AP_NormalisedEventKey key) 
{
	const char* value = ev->functions->findValue(ev, key);
	return value == NULL ? 0 : *value;
}

const double getDouble(AP_NormalisedEvent* ev, AP_NormalisedEventKey key) 
{
	const char* value = ev->functions->findValue(ev, key);
	return value == NULL ? -1.0 : atof(value);
}

OMS_Transport::OMS_Transport(const char* _name, AP_EventTransportProperties* properties, AP_EventTransportError* err, IAF_TimestampConfig* timestampConfig)
{
	mRunStatus.status = (AP_char8*)malloc(STATUS_BUFSIZE);
	m_pErrMsg = new char[STATUS_BUFSIZE];
	mRunStatus.totalReceived = 0;
	mRunStatus.totalSent = 0;
	MUTEX_INIT(&mLock);
	m_pLastError = NULL;
	m_pTimestampConfig = NULL;
	m_pDecoder = NULL;

	if ((*err = updateProperties(properties, timestampConfig))){
		char* errMsg = _strdup(m_pLastError);
		AP_LogError(errMsg);
	}
}

OMS_Transport::~OMS_Transport() 
{
	MUTEX_DESTROY(&mLock);
	clearError();
	delete[]m_pErrMsg;
	m_pErrMsg = NULL;

	for(map<string, OMSCallback*>::iterator ctpIter = m_mapCallBacks.begin(); ctpIter != m_mapCallBacks.end(); ctpIter++)
	{
		delete ctpIter->second;
		ctpIter->second = NULL;
	}
}


void OMS_Transport::clearError() 
{
	if(m_pLastError != NULL){
		free(m_pLastError);
		m_pLastError = NULL;
	}
}


void OMS_Transport::setError(const AP_char8* msg) 
{
	clearError();
	m_pLastError = _strdup(msg);
}

AP_EventTransportError OMS_Transport::updateProperties(AP_EventTransportProperties* properties, IAF_TimestampConfig* timestampConfig) 
{
	AP_EventTransportProperty** newProperties;
	m_pTimestampConfig = timestampConfig;
	for(newProperties = properties->properties; *newProperties; newProperties++) 
	{
		AP_EventTransportProperty* property = *newProperties;
		if(strcmp(property->name, "url") == 0){
			m_szIP = string(property->value);
		}else if (strcmp(property->name, "broker") == 0){
			m_szBroker = string(property->value);
		}
	}

	if ("" == m_szIP || "" == m_szBroker){
		setError("IP is error.");
		return AP_EventTransport_BadProperties;
	}
	return AP_EventTransport_OK;
}

AP_EventTransportError OMS_Transport::sendTransportEvent(AP_TransportEvent tevent, AP_TimestampSet* tss) 
{
	AP_NormalisedEvent *pEve = (AP_NormalisedEvent*) tevent;
	const char* reqType = pEve->functions->findValue(pEve, "Type");
	string szInvestorId = (string) pEve->functions->findValue(pEve, "investorId");
	bool rtn = false;
	if (reqType){
		if(strcmp(reqType, "ReqUserLogin") == 0){		
			if (m_mapTradeApis.find(szInvestorId) == m_mapTradeApis.end()) {
				rtn = ReqUserLogin(pEve);
				mRunStatus.totalSent++;
			}else{
				AP_LogWarn("User: %s is exist.", szInvestorId.c_str());
				rtn = false;
			}
		}else{
			if (m_mapTradeApis.find(szInvestorId) != m_mapTradeApis.end()) {
				CThostFtdcTraderApi* api = m_mapTradeApis[szInvestorId];
				if(strcmp(reqType, "ReqUserLogout") == 0){
					rtn = ReqUserLogout(api, pEve);
					m_mapCallBacks.erase(szInvestorId);
					m_mapTradeApis.erase(szInvestorId);
					mRunStatus.totalSent++;
				}else if (strcmp(reqType, "ReqOrderInsert") == 0){
					rtn = ReqOrderInsert(api, pEve);
					mRunStatus.totalSent++;

				}else if (strcmp(reqType, "ReqParkedOrderInsert") == 0){
					rtn = ReqParkedOrderInsert(api, pEve);
					mRunStatus.totalSent++;

				}else if (strcmp(reqType, "ReqParkedOrderAction") == 0){	
					rtn = ReqParkedOrderAction(api, pEve);
					mRunStatus.totalSent++;

				}else if (strcmp(reqType, "ReqOrderAction") == 0){
					rtn = ReqOrderAction(api, pEve);
					mRunStatus.totalSent++;

				}else if (strcmp(reqType, "ReqQueryMaxOrderVolume") == 0){
					rtn = ReqQueryMaxOrderVolume(api, pEve);
					mRunStatus.totalSent++;

				}else if (strcmp(reqType, "ReqQrySettlementInfoConfirm") == 0){	
					rtn = ReqQrySettlementInfoConfirm(api, pEve);
					mRunStatus.totalSent++;

				}else if (strcmp(reqType, "ReqQryOrder") == 0){
					rtn = ReqQryOrder(api, pEve);
					mRunStatus.totalSent++;

				}else if (strcmp(reqType, "ReqQryTrade") == 0){
					rtn = ReqQryTrade(api, pEve);
					mRunStatus.totalSent++;

				}else if (strcmp(reqType, "ReqQryInvestorPosition") == 0){
					rtn = ReqQryInvestorPosition(api, pEve);
					mRunStatus.totalSent++;

				}else if (strcmp(reqType, "ReqQryTradingAccount") == 0){
					rtn = ReqQryTradingAccount(api, pEve);
					mRunStatus.totalSent++;

				}else if (strcmp(reqType, "ReqQryInvestor") == 0){
					rtn = ReqQryInvestor(api, pEve);
					mRunStatus.totalSent++;

				}else if (strcmp(reqType, "ReqQryTradingCode") == 0){
					rtn = ReqQryTradingCode(api, pEve);
					mRunStatus.totalSent++;

				}else if (strcmp(reqType, "ReqQryInstrumentMarginRate") == 0){
					rtn = ReqQryInstrumentMarginRate(api, pEve);
					mRunStatus.totalSent++;

				}else if (strcmp(reqType, "ReqQryInstrumentCommissionRate") == 0){	
					rtn = ReqQryInstrumentCommissionRate(api, pEve);
					mRunStatus.totalSent++;

				}else if (strcmp(reqType, "ReqQryExchange") == 0){
					rtn = ReqQryExchange(api, pEve);
					mRunStatus.totalSent++;

				}else if (strcmp(reqType, "ReqQryInstrument") == 0){
					rtn = ReqQryInstrument(api, pEve);
					mRunStatus.totalSent++;

				}else if (strcmp(reqType, "ReqQryDepthMarketData") == 0){
					rtn = ReqQryDepthMarketData(api, pEve);
					mRunStatus.totalSent++;

				}else if (strcmp(reqType, "ReqQrySettlementInfo") == 0){
					rtn = ReqQrySettlementInfo(api, pEve);
					mRunStatus.totalSent++;

				}else if (strcmp(reqType, "ReqQryInvestorPositionDetail") == 0){
					rtn = ReqQryInvestorPositionDetail(api, pEve);
					mRunStatus.totalSent++;

				}else if (strcmp(reqType, "ReqSettlementInfoConfirm") == 0){
					rtn = ReqSettlementInfoConfirm(api, pEve);
					mRunStatus.totalSent++;

				}else if (strcmp(reqType, "ReqQryInvestorPositionCombineDetail") == 0){
					rtn = ReqQryInvestorPositionCombineDetail(api, pEve);
					mRunStatus.totalSent++;

				}else if (strcmp(reqType, "ReqQryEWarrantOffset") == 0){
					rtn = ReqQryEWarrantOffset(api, pEve);
					mRunStatus.totalSent++;

				}else if (strcmp(reqType, "ReqQryTradingNotice") == 0){
					rtn = ReqQryTradingNotice(api, pEve);
					mRunStatus.totalSent++;

				}else if (strcmp(reqType, "ReqQryBrokerTradingParams") == 0){
					rtn = ReqQryBrokerTradingParams(api, pEve);
					mRunStatus.totalSent++;

				}else if (strcmp(reqType, "ReqQryBrokerTradingAlgos") == 0){
					rtn = ReqQryBrokerTradingAlgos(api, pEve);
					mRunStatus.totalSent++;

				}else if (strcmp(reqType, "ReqRemoveParkedOrder") == 0){
					rtn = ReqRemoveParkedOrder(api, pEve);
					mRunStatus.totalSent++;

				}else if (strcmp(reqType, "ReqRemoveParkedOrderAction") == 0){
					rtn = ReqRemoveParkedOrderAction(api, pEve);
					mRunStatus.totalSent++;

				}else if (strcmp(reqType, "ReqQryParkedOrder") == 0){
					rtn = ReqQryParkedOrder(api, pEve);
					mRunStatus.totalSent++;

				}else if (strcmp(reqType, "ReqQryParkedOrderAction") == 0){
					rtn = ReqQryParkedOrderAction(api, pEve);
					mRunStatus.totalSent++;

				}else {						
					AP_LogError("Invalid Type: %s", reqType);
					rtn = false;
				}
			}else{
				AP_LogWarn("Invalid User: %s", szInvestorId.c_str());
				rtn = false;
			}
		}
	}else {
		AP_LogError("Missing Type.");
		rtn = false;
	}
	AP_NormalisedEvent_dtor(pEve);
	AP_TimestampSet_dtor(tss);
	return rtn ? AP_EventTransport_OK : AP_EventTransport_TransportFailure;
}

void OMS_Transport::decode(AP_NormalisedEvent* ev, AP_NormalisedEventValue type)
{
	MUTEX_LOCK(&mLock);	
	AP_TimestampSet* timeStamp = AP_TimestampSet_ctor();
	ev->functions->addQuick(ev, "Type", type);
	m_pDecoder->functions->sendTransportEvent(m_pDecoder, ev, timeStamp);
	MUTEX_UNLOCK(&mLock);
}

void OMS_Transport::addEventDecoder(const AP_char8* name, AP_EventDecoder* _decoder) 
{
	m_pDecoder = _decoder;
}

void OMS_Transport::removeEventDecoder(const AP_char8* name)
{
	m_pDecoder = NULL;
}

AP_EventTransportError OMS_Transport::flushUpstream() 
{
	return AP_EventTransport_OK;
}

AP_EventTransportError OMS_Transport::flushDownstream() 
{
	return AP_EventTransport_OK;
}

void OMS_Transport::ShowError(int ret, int reqId)
{
	AP_LogWarn("ReqId: %d, ErrorId: %d", reqId, ret);
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", reqId);
	addInt(buf, CHAR_BUF_SIZE, ev, "errorId", ret);
	addString(ev, "errorMsg", "Fail when send req.");
	delete[] buf;
	buf = NULL;
	decode(ev, "OnReqError");
}

bool OMS_Transport::ReqUserLogin(AP_NormalisedEvent* ev)
{
	AP_LogInfo("[ReqUserLogin] UserId: %s, Psw: ******", getString(ev, "userId"));
	try{
		HANDLE handle =  CreateEvent(NULL, FALSE, FALSE, NULL);
		CThostFtdcTraderApi* api = CThostFtdcTraderApi::CreateFtdcTraderApi();
		api->RegisterFront(const_cast<char*>(m_szIP.c_str()));
		string usr = string(getString(ev, "userId"));
		m_mapCallBacks[usr] = new OMSCallback(this, handle);
		api->RegisterSpi(m_mapCallBacks[usr]);
		api->SubscribePublicTopic(THOST_TERT_QUICK);
		api->SubscribePrivateTopic(THOST_TERT_QUICK);
		api->Init();
		m_mapTradeApis[usr] = api;
		WaitForSingleObject(handle, INFINITE);
		if(handle != NULL){
			CloseHandle(handle);
			handle = NULL;
		}
		AP_LogInfo("Success to register front: %s", getString(ev, "userId"));
		CThostFtdcReqUserLoginField req;
		memset(&req, 0, sizeof(req));
		int nRequest =  getInt(ev, "requestId");
		strncpy(req.BrokerID, m_szBroker.c_str(), sizeof(req.BrokerID));
		strncpy(req.UserID, getString(ev, "userId"), sizeof(req.UserID));
		strncpy(req.Password, getString(ev, "password"), sizeof(req.Password));		
		int iResult = api->ReqUserLogin(&req, nRequest);
		if (iResult != 0){
			ShowError(iResult, nRequest);
			AP_LogWarn("[ReqUserLogin] Fail to send request of login.");
			return false;
		}
	}
	catch (...){
		AP_LogError("[ReqUserLogin] exception when request.");
		return false;
	}
	return true;
}


bool OMS_Transport::ReqUserLogout(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev)
{
	AP_LogInfo("[ReqUserLogout] %s", ev->functions->toString(ev));
	try{
		CThostFtdcUserLogoutField req;
		memset(&req, 0, sizeof(req));
		int nRequest =  getInt(ev, "requestId");
		strncpy(req.BrokerID, m_szBroker.c_str(), sizeof(req.BrokerID));
		strncpy(req.UserID, getString(ev, "userId"), sizeof(req.UserID));
		int iResult = api->ReqUserLogout(&req, nRequest);
		if (iResult != 0){
			ShowError(iResult, nRequest);
			AP_LogWarn("[ReqUserLogout] Fail to send request of logout.");
			return false;
		}

	}catch(...){
		AP_LogError("[ReqUserLogout] exception when request.");
		return false;
	}
	return true;
}

bool OMS_Transport::ReqOrderInsert(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev)
{
	AP_LogInfo("[ReqOrderInsert] %s", ev->functions->toString(ev));
	try{
		CThostFtdcInputOrderField req;
		memset(&req, 0, sizeof(req));
		int nRequest =  getInt(ev, "requestId");
		req.RequestID = nRequest;
		strncpy(req.BrokerID, m_szBroker.c_str(), sizeof(req.BrokerID));
		strncpy(req.InvestorID, getString(ev, "investorId"), sizeof(req.InvestorID));
		strncpy(req.InstrumentID, getString(ev, "instrumentId"), sizeof(req.InstrumentID));
		strncpy(req.OrderRef, getString(ev, "orderRef"), sizeof(req.OrderRef));
		strncpy(req.UserID, getString(ev, "userId"), sizeof(req.UserID));
		req.OrderPriceType =  getChar(ev, "orderPriceType");
		req.Direction = getChar(ev, "direction");
		strncpy(req.CombOffsetFlag, getString(ev, "combOffsetFlag"), sizeof(req.CombOffsetFlag));
		strncpy(req.CombHedgeFlag, getString(ev, "combHedgeFlag"), sizeof(req.CombHedgeFlag));
		req.LimitPrice = getDouble(ev, "limitPrice");
		req.VolumeTotalOriginal = getInt(ev, "volumeTotalOriginal");
		req.TimeCondition = getChar(ev, "timeCondition");
		strncpy(req.GTDDate, getString(ev, "gtdDate"), sizeof(req.GTDDate));
		req.VolumeCondition = getChar(ev, "volumeCondition");
		req.MinVolume = getInt(ev, "minVolume");
		req.ContingentCondition = getChar(ev, "contingentCondition");
		req.StopPrice = getDouble(ev, "stopPrice");
		req.ForceCloseReason = getChar(ev, "forceCloseReason");
		req.IsAutoSuspend = getInt(ev, "isAutoSuspend");
		strncpy(req.BusinessUnit, getString(ev, "businessUnit"), sizeof(req.BusinessUnit));
		req.UserForceClose = getInt(ev, "userForceClose");
		int iResult = api->ReqOrderInsert(&req, nRequest);
		if (iResult != 0){
			ShowError(iResult, nRequest);
			AP_LogWarn("[ReqOrderInsert] Fail to order insert.");
			return false;
		}

	}catch(...){
		AP_LogError("[ReqOrderInsert] exception when order insert.");
		return false;
	}
	return true;
}

bool OMS_Transport::ReqParkedOrderInsert(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev)
{
	AP_LogInfo("[ReqParkedOrderInsert] %s", ev->functions->toString(ev));
	try{
		CThostFtdcParkedOrderField req;
		memset(&req, 0, sizeof(req));
		int nRequest =  getInt(ev, "requestId");
		req.RequestID = nRequest;
		strncpy(req.BrokerID, m_szBroker.c_str(), sizeof(req.BrokerID));
		strncpy(req.InvestorID, getString(ev, "investorId"), sizeof(req.InvestorID));
		strncpy(req.InstrumentID, getString(ev, "instrumentId"), sizeof(req.InstrumentID));
		strncpy(req.OrderRef, getString(ev, "orderRef"), sizeof(req.OrderRef));
		strncpy(req.UserID, getString(ev, "userId"), sizeof(req.UserID));
		req.OrderPriceType =  getChar(ev, "orderPriceType");
		req.Direction = getChar(ev, "direction");
		strncpy(req.CombOffsetFlag, getString(ev, "combOffsetFlag"), sizeof(req.CombOffsetFlag));
		strncpy(req.CombHedgeFlag, getString(ev, "combHedgeFlag"), sizeof(req.CombHedgeFlag));
		req.LimitPrice = getDouble(ev, "limitPrice");
		req.VolumeTotalOriginal = getInt(ev, "volumeTotalOriginal");
		req.TimeCondition = getChar(ev, "timeCondition");
		strncpy(req.GTDDate, getString(ev, "gtdDate"), sizeof(req.GTDDate));
		req.VolumeCondition = getChar(ev, "volumeCondition");
		req.MinVolume = getInt(ev, "minVolume");
		req.ContingentCondition = getChar(ev, "contingentCondition");
		req.StopPrice = getDouble(ev, "stopPrice");
		req.ForceCloseReason = getChar(ev, "forceCloseReason");
		req.IsAutoSuspend = getInt(ev, "isAutoSuspend");
		strncpy(req.BusinessUnit, getString(ev, "businessUnit"), sizeof(req.BusinessUnit));
		req.UserForceClose = getInt(ev, "userForceClose");
		strncpy(req.ExchangeID, getString(ev, "exchangeId"), sizeof(req.ExchangeID));
		strncpy(req.ParkedOrderID, getString(ev, "parkedOrderId"), sizeof(req.ParkedOrderID));
		req.UserType = getChar(ev, "userType");
		req.Status = getChar(ev, "status");
		req.ErrorID = getInt(ev, "errorId");
		strncpy(req.ErrorMsg, getString(ev, "errorMsg"), sizeof(req.ErrorMsg));
		int iResult = api->ReqParkedOrderInsert(&req, nRequest);
		if (iResult != 0){
			ShowError(iResult, nRequest);
			AP_LogWarn("[ReqParkedOrderInsert] Fail to parked order insert.");
			return false;
		}

	}catch(...){
		AP_LogError("[ReqParkedOrderInsert] exception when parked order insert.");
		return false;
	}
	return true;
}

bool OMS_Transport::ReqParkedOrderAction(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev)
{
	AP_LogInfo("[ReqParkedOrderAction] %s", ev->functions->toString(ev));
	try{
		CThostFtdcParkedOrderActionField req;
		memset(&req, 0, sizeof(req));
		int nRequest =  getInt(ev, "requestId");
		req.RequestID = nRequest;
		strncpy(req.BrokerID, m_szBroker.c_str(), sizeof(req.BrokerID));
		strncpy(req.InvestorID, getString(ev, "investorId"), sizeof(req.InvestorID));
		req.OrderActionRef = getInt(ev, "orderActionRef");
		strncpy(req.OrderRef, getString(ev, "orderRef"), sizeof(req.OrderRef));
		req.FrontID = getInt(ev, "frontId");
		req.SessionID = getInt(ev, "sessionId");
		strncpy(req.ExchangeID, getString(ev, "exchangeId"), sizeof(req.ExchangeID));
		strncpy(req.OrderSysID, getString(ev, "orderSysId"), sizeof(req.OrderSysID));
		req.ActionFlag =  getChar(ev, "actionFlag");
		req.LimitPrice = getDouble(ev, "limitPrice");
		req.VolumeChange = getInt(ev, "volumeChange");
		strncpy(req.UserID, getString(ev, "userId"), sizeof(req.UserID));
		strncpy(req.InstrumentID, getString(ev, "instrumentId"), sizeof(req.InstrumentID));
		strncpy(req.ParkedOrderActionID, getString(ev, "parkedOrderActionId"), sizeof(req.ParkedOrderActionID));
		req.UserType = getChar(ev, "userType");
		req.Status = getChar(ev, "status");
		req.ErrorID = getInt(ev, "errorId");
		strncpy(req.ErrorMsg, getString(ev, "errorMsg"), sizeof(req.ErrorMsg));
		int iResult = api->ReqParkedOrderAction(&req, nRequest);
		if (iResult != 0){
			ShowError(iResult, nRequest);
			AP_LogWarn("[ReqParkedOrderAction] Fail to parked order action.");
			return false;
		}

	}catch(...){
		AP_LogError("[ReqParkedOrderAction] exception when parked order action.");
		return false;
	}
	return true;
}

bool OMS_Transport::ReqOrderAction(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev)
{
	AP_LogInfo("[ReqOrderAction] %s", ev->functions->toString(ev));
	try{
		CThostFtdcInputOrderActionField req;
		memset(&req, 0, sizeof(req));
		int nRequest =  getInt(ev, "requestId");
		req.RequestID = nRequest;
		strncpy(req.BrokerID, m_szBroker.c_str(), sizeof(req.BrokerID));
		strncpy(req.InvestorID, getString(ev, "investorId"), sizeof(req.InvestorID));
		req.OrderActionRef = getInt(ev, "orderActionRef");
		strncpy(req.OrderRef, getString(ev, "orderRef"), sizeof(req.OrderRef));
		req.FrontID = getInt(ev, "frontId");
		req.SessionID = getInt(ev, "sessionId");
		strncpy(req.ExchangeID, getString(ev, "exchangeId"), sizeof(req.ExchangeID));
		strncpy(req.OrderSysID, getString(ev, "orderSysId"), sizeof(req.OrderSysID));
		req.ActionFlag =  getChar(ev, "actionFlag");
		req.LimitPrice = getDouble(ev, "limitPrice");
		req.VolumeChange = getInt(ev, "volumeChange");
		strncpy(req.UserID, getString(ev, "userId"), sizeof(req.UserID));
		strncpy(req.InstrumentID, getString(ev, "instrumentId"), sizeof(req.InstrumentID));
		int iResult = api->ReqOrderAction(&req, nRequest);
		if (iResult != 0){
			ShowError(iResult, nRequest);
			AP_LogWarn("[ReqOrderAction] Fail to order action.");
			return false;
		}

	}catch(...){
		AP_LogError("[ReqOrderAction] exception when order action.");
		return false;
	}
	return true;
}

bool OMS_Transport::ReqQueryMaxOrderVolume(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev)
{
	AP_LogInfo("[ReqQueryMaxOrderVolume] %s", ev->functions->toString(ev));
	try{
		CThostFtdcQueryMaxOrderVolumeField req;
		memset(&req, 0, sizeof(req));
		int nRequest =  getInt(ev, "requestId");
		strncpy(req.BrokerID, m_szBroker.c_str(), sizeof(req.BrokerID));
		strncpy(req.InvestorID, getString(ev, "investorId"), sizeof(req.InvestorID));
		strncpy(req.InstrumentID, getString(ev, "instrumentId"), sizeof(req.InstrumentID));
		req.Direction =  getChar(ev, "direction");
		req.OffsetFlag =  getChar(ev, "offsetFlag");
		req.HedgeFlag =  getChar(ev, "hedgeFlag");
		req.MaxVolume = getInt(ev, "maxVolume");	
		int iResult = api->ReqQueryMaxOrderVolume(&req, nRequest);
		if (iResult != 0){
			ShowError(iResult, nRequest);
			AP_LogWarn("[ReqQueryMaxOrderVolume] Fail to query maxOrderVolume.");
			return false;
		}

	}catch(...){
		AP_LogError("[ReqQueryMaxOrderVolume] exception when query maxOrderVolume.");
		return false;
	}
	return true;
}

bool OMS_Transport::ReqQrySettlementInfoConfirm(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev)
{
	AP_LogInfo("[ReqQrySettlementInfoConfirm] %s", ev->functions->toString(ev));
	try{
		CThostFtdcQrySettlementInfoConfirmField req;
		memset(&req, 0, sizeof(req));
		int nRequest =  getInt(ev, "requestId");
		strncpy(req.BrokerID, m_szBroker.c_str(), sizeof(req.BrokerID));
		strncpy(req.InvestorID, getString(ev, "investorId"), sizeof(req.InvestorID));
		int iResult = api->ReqQrySettlementInfoConfirm(&req, nRequest);
		if (iResult != 0){
			ShowError(iResult, nRequest);
			AP_LogWarn("[ReqQrySettlementInfoConfirm] Fail to query settlementInfoConfirm.");
			return false;
		}

	}catch(...){
		AP_LogError("[ReqQrySettlementInfoConfirm] exception when query settlementInfoConfirm.");
		return false;
	}
	return true;
}

bool OMS_Transport::ReqQryOrder(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev)
{
	AP_LogInfo("[ReqQryOrder] %s", ev->functions->toString(ev));
	try{
		CThostFtdcQryOrderField req;
		memset(&req, 0, sizeof(req));
		int nRequest =  getInt(ev, "requestId");
		strncpy(req.BrokerID, m_szBroker.c_str(), sizeof(req.BrokerID));
		strncpy(req.InvestorID, getString(ev, "investorId"), sizeof(req.InvestorID));
		strncpy(req.InstrumentID, getString(ev, "instrumentId"), sizeof(req.InstrumentID));
		strncpy(req.ExchangeID, getString(ev, "exchangeId"), sizeof(req.ExchangeID));
		strncpy(req.OrderSysID, getString(ev, "orderSysId"), sizeof(req.OrderSysID));
		strncpy(req.InsertTimeStart, getString(ev, "insertTimeStart"), sizeof(req.InsertTimeStart));
		strncpy(req.InsertTimeEnd, getString(ev, "insertTimeEnd"), sizeof(req.InsertTimeEnd));
		int iResult = api->ReqQryOrder(&req, nRequest);
		if (iResult != 0){
			ShowError(iResult, nRequest);
			AP_LogWarn("[ReqQryOrder] Fail to query order.");
			return false;
		}

	}catch(...){
		AP_LogError("[ReqQryOrder] exception when query order.");
		return false;
	}
	return true;
}

bool OMS_Transport::ReqQryTrade(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev)
{
	AP_LogInfo("[ReqQryTrade] %s", ev->functions->toString(ev));
	try{
		CThostFtdcQryTradeField req;
		memset(&req, 0, sizeof(req));
		int nRequest =  getInt(ev, "requestId");
		strncpy(req.BrokerID, m_szBroker.c_str(), sizeof(req.BrokerID));
		strncpy(req.InvestorID, getString(ev, "investorId"), sizeof(req.InvestorID));
		strncpy(req.InstrumentID, getString(ev, "instrumentId"), sizeof(req.InstrumentID));
		strncpy(req.ExchangeID, getString(ev, "exchangeId"), sizeof(req.ExchangeID));
		strncpy(req.TradeID, getString(ev, "tradeId"), sizeof(req.TradeID));
		strncpy(req.TradeTimeStart, getString(ev, "tradeTimeStart"), sizeof(req.TradeTimeStart));
		strncpy(req.TradeTimeEnd, getString(ev, "tradeTimeEnd"), sizeof(req.TradeTimeEnd));
		int iResult = api->ReqQryTrade(&req, nRequest);
		if (iResult != 0){
			ShowError(iResult, nRequest);
			AP_LogWarn("[ReqQryTrade] Fail to query trade.");
			return false;
		}

	}catch(...){
		AP_LogError("[ReqQryTrade] exception when query trade.");
		return false;
	}
	return true;
}

bool OMS_Transport::ReqQryInvestorPosition(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev)
{
	AP_LogInfo("[ReqQryInvestorPosition] %s", ev->functions->toString(ev));
	try{
		CThostFtdcQryInvestorPositionField req;
		memset(&req, 0, sizeof(req));
		int nRequest =  getInt(ev, "requestId");
		strncpy(req.BrokerID, m_szBroker.c_str(), sizeof(req.BrokerID));
		strncpy(req.InvestorID, getString(ev, "investorId"), sizeof(req.InvestorID));
		strncpy(req.InstrumentID, getString(ev, "instrumentId"), sizeof(req.InstrumentID));
		int iResult = api->ReqQryInvestorPosition(&req, nRequest);
		if (iResult != 0){
			ShowError(iResult, nRequest);
			AP_LogWarn("[ReqQryInvestorPosition] Fail to query investorPosition.");
			return false;
		}

	}catch(...){
		AP_LogError("[ReqQryInvestorPosition] exception when query investorPosition.");
		return false;
	}
	return true;
}

bool OMS_Transport::ReqQryTradingAccount(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev)
{
	AP_LogInfo("[ReqQryTradingAccount] %s", ev->functions->toString(ev));
	try{
		CThostFtdcQryTradingAccountField req;
		memset(&req, 0, sizeof(req));
		int nRequest =  getInt(ev, "requestId");
		strncpy(req.BrokerID, m_szBroker.c_str(), sizeof(req.BrokerID));
		strncpy(req.InvestorID, getString(ev, "investorId"), sizeof(req.InvestorID));
		int iResult = api->ReqQryTradingAccount(&req, nRequest);
		if (iResult != 0){
			ShowError(iResult, nRequest);
			AP_LogWarn("[ReqQryTradingAccount] Fail to query tradingAccount.");
			return false;
		}

	}catch(...){
		AP_LogError("[ReqQryTradingAccount] exception when query tradingAccount.");
		return false;
	}
	return true;
}

bool OMS_Transport::ReqQryInvestor(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev)
{
	AP_LogInfo("[ReqQryInvestor] %s", ev->functions->toString(ev));
	try{
		CThostFtdcQryInvestorField req;
		memset(&req, 0, sizeof(req));
		int nRequest =  getInt(ev, "requestId");
		strncpy(req.BrokerID, m_szBroker.c_str(), sizeof(req.BrokerID));
		strncpy(req.InvestorID, getString(ev, "investorId"), sizeof(req.InvestorID));
		int iResult = api->ReqQryInvestor(&req, nRequest);
		if (iResult != 0){
			ShowError(iResult, nRequest);
			AP_LogWarn("[ReqQryInvestor] Fail to query investor.");
			return false;
		}

	}catch(...){
		AP_LogError("[ReqQryInvestor] exception when query investor.");
		return false;
	}
	return true;
}

bool OMS_Transport::ReqQryTradingCode(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev)
{
	AP_LogInfo("[ReqQryTradingCode] %s", ev->functions->toString(ev));
	try{
		CThostFtdcQryTradingCodeField req;
		memset(&req, 0, sizeof(req));
		int nRequest =  getInt(ev, "requestId");
		strncpy(req.BrokerID, m_szBroker.c_str(), sizeof(req.BrokerID));
		strncpy(req.InvestorID, getString(ev, "investorId"), sizeof(req.InvestorID));
		strncpy(req.ExchangeID, getString(ev, "exchangeId"), sizeof(req.ExchangeID));
		strncpy(req.ClientID, getString(ev, "clientId"), sizeof(req.ClientID));
		req.ClientIDType =  getChar(ev, "clientIdType");
		int iResult = api->ReqQryTradingCode(&req, nRequest);
		if (iResult != 0){
			ShowError(iResult, nRequest);
			AP_LogWarn("[ReqQryTradingCode] Fail to query tradingCode.");
			return false;
		}

	}catch(...){
		AP_LogError("[ReqQryTradingCode] exception when query tradingCode.");
		return false;
	}
	return true;
}

bool OMS_Transport::ReqQryInstrumentMarginRate(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev)
{
	AP_LogInfo("[ReqQryInstrumentMarginRate] %s", ev->functions->toString(ev));
	try{
		CThostFtdcQryInstrumentMarginRateField req;
		memset(&req, 0, sizeof(req));
		int nRequest =  getInt(ev, "requestId");
		strncpy(req.BrokerID, m_szBroker.c_str(), sizeof(req.BrokerID));
		strncpy(req.InvestorID, getString(ev, "investorId"), sizeof(req.InvestorID));
		strncpy(req.InstrumentID, getString(ev, "instrumentId"), sizeof(req.InstrumentID));
		req.HedgeFlag =  getChar(ev, "hedgeFlag");
		int iResult = api->ReqQryInstrumentMarginRate(&req, nRequest);
		if (iResult != 0){
			ShowError(iResult, nRequest);
			AP_LogWarn("[ReqQryInstrumentMarginRate] Fail to query instrumentMarginRate.");
			return false;
		}

	}catch(...){
		AP_LogError("[ReqQryInstrumentMarginRate] exception when query instrumentMarginRate.");
		return false;
	}
	return true;
}

bool OMS_Transport::ReqQryInstrumentCommissionRate(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev)
{
	AP_LogInfo("[ReqQryInstrumentCommissionRate] %s", ev->functions->toString(ev));
	try{
		CThostFtdcQryInstrumentCommissionRateField req;
		memset(&req, 0, sizeof(req));
		int nRequest =  getInt(ev, "requestId");
		strncpy(req.BrokerID, m_szBroker.c_str(), sizeof(req.BrokerID));
		strncpy(req.InvestorID, getString(ev, "investorId"), sizeof(req.InvestorID));
		strncpy(req.InstrumentID, getString(ev, "instrumentId"), sizeof(req.InstrumentID));
		int iResult = api->ReqQryInstrumentCommissionRate(&req, nRequest);
		if (iResult != 0){
			ShowError(iResult, nRequest);
			AP_LogWarn("[ReqQryInstrumentCommissionRate] Fail to query instrumentCommissionRate.");
			return false;
		}

	}catch(...){
		AP_LogError("[ReqQryInstrumentCommissionRate] exception when query instrumentCommissionRate.");
		return false;
	}
	return true;
}

bool OMS_Transport::ReqQryExchange(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev)
{
	AP_LogInfo("[ReqQryExchange] %s", ev->functions->toString(ev));
	try{
		CThostFtdcQryExchangeField req;
		memset(&req, 0, sizeof(req));
		int nRequest =  getInt(ev, "requestId");
		strncpy(req.ExchangeID, getString(ev, "exchangeId"), sizeof(req.ExchangeID));
		int iResult = api->ReqQryExchange(&req, nRequest);
		if (iResult != 0){
			ShowError(iResult, nRequest);
			AP_LogWarn("[ReqQryExchange] Fail to query exchange.");
			return false;
		}

	}catch(...){
		AP_LogError("[ReqQryExchange] exception when query exchange.");
		return false;
	}
	return true;
}

bool OMS_Transport::ReqQryInstrument(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev)
{
	AP_LogInfo("[ReqQryInstrument] %s", ev->functions->toString(ev));
	try{
		CThostFtdcQryInstrumentField req;
		memset(&req, 0, sizeof(req));
		int nRequest =  getInt(ev, "requestId");
		strncpy(req.InstrumentID, getString(ev, "instrumentId"), sizeof(req.InstrumentID));
		strncpy(req.ExchangeID, getString(ev, "exchangeId"), sizeof(req.ExchangeID));
		strncpy(req.ExchangeInstID, getString(ev, "exchangeInstId"), sizeof(req.ExchangeInstID));
		strncpy(req.ProductID, getString(ev, "productId"), sizeof(req.ProductID));
		int iResult = api->ReqQryInstrument(&req, nRequest);
		if (iResult != 0){
			ShowError(iResult, nRequest);
			AP_LogWarn("[ReqQryInstrument] Fail to query instrument.");
			return false;
		}

	}catch(...){
		AP_LogError("[ReqQryInstrument] exception when query instrument.");
		return false;
	}
	return true;
}

bool OMS_Transport::ReqQryDepthMarketData(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev)
{
	AP_LogInfo("[ReqQryDepthMarketData] %s", ev->functions->toString(ev));
	try{
		CThostFtdcQryDepthMarketDataField req;
		memset(&req, 0, sizeof(req));
		int nRequest =  getInt(ev, "requestId");
		strncpy(req.InstrumentID, getString(ev, "instrumentId"), sizeof(req.InstrumentID));
		int iResult = api->ReqQryDepthMarketData(&req, nRequest);
		if (iResult != 0){
			ShowError(iResult, nRequest);
			AP_LogWarn("[ReqQryDepthMarketData] Fail to query depthMarketData.");
			return false;
		}

	}catch(...){
		AP_LogError("[ReqQryDepthMarketData] exception when query depthMarketData.");
		return false;
	}
	return true;
}

bool OMS_Transport::ReqQrySettlementInfo(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev)
{
	AP_LogInfo("[ReqQrySettlementInfo] %s", ev->functions->toString(ev));
	try{
		CThostFtdcQrySettlementInfoField req;
		memset(&req, 0, sizeof(req));
		int nRequest =  getInt(ev, "requestId");
		strncpy(req.BrokerID, m_szBroker.c_str(), sizeof(req.BrokerID));
		strncpy(req.InvestorID, getString(ev, "investorId"), sizeof(req.InvestorID));
		strncpy(req.TradingDay, getString(ev, "tradingDay"), sizeof(req.TradingDay));
		int iResult = api->ReqQrySettlementInfo(&req, nRequest);
		if (iResult != 0){
			ShowError(iResult, nRequest);
			AP_LogWarn("[ReqQrySettlementInfo] Fail to query settlementInfo.");
			return false;
		}

	}catch(...){
		AP_LogError("[ReqQrySettlementInfo] exception when query settlementInfo.");
		return false;
	}
	return true;
}

bool OMS_Transport::ReqQryInvestorPositionDetail(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev)
{
	AP_LogInfo("[ReqQryInvestorPositionDetail] %s", ev->functions->toString(ev));
	try{
		CThostFtdcQryInvestorPositionDetailField req;
		memset(&req, 0, sizeof(req));
		int nRequest =  getInt(ev, "requestId");
		strncpy(req.BrokerID, m_szBroker.c_str(), sizeof(req.BrokerID));
		strncpy(req.InvestorID, getString(ev, "investorId"), sizeof(req.InvestorID));
		strncpy(req.InstrumentID, getString(ev, "instrumentId"), sizeof(req.InstrumentID));
		int iResult = api->ReqQryInvestorPositionDetail(&req, nRequest);
		if (iResult != 0){
			ShowError(iResult, nRequest);
			AP_LogWarn("[ReqQryInvestorPositionDetail] Fail to query investorPositionDetail.");
			return false;
		}

	}catch(...){
		AP_LogError("[ReqQryInvestorPositionDetail] exception when query investorPositionDetail.");
		return false;
	}
	return true;
}

bool OMS_Transport::ReqSettlementInfoConfirm(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev)
{
	AP_LogInfo("[ReqSettlementInfoConfirm] %s", ev->functions->toString(ev));
	try{
		CThostFtdcSettlementInfoConfirmField req;
		memset(&req, 0, sizeof(req));
		int nRequest =  getInt(ev, "requestId");
		strncpy(req.BrokerID, m_szBroker.c_str(), sizeof(req.BrokerID));
		strncpy(req.InvestorID, getString(ev, "investorId"), sizeof(req.InvestorID));
		strncpy(req.ConfirmDate, getString(ev, "confirmDate"), sizeof(req.ConfirmDate));
		strncpy(req.ConfirmTime, getString(ev, "confirmTime"), sizeof(req.ConfirmTime));
		int iResult = api->ReqSettlementInfoConfirm(&req, nRequest);
		if (iResult != 0){
			ShowError(iResult, nRequest);
			AP_LogWarn("[ReqSettlementInfoConfirm] Fail to settlementInfoConfirm.");
			return false;
		}

	}catch(...){
		AP_LogError("[ReqSettlementInfoConfirm] exception when settlementInfoConfirm.");
		return false;
	}
	return true;
}

bool OMS_Transport::ReqQryInvestorPositionCombineDetail(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev)
{
	AP_LogInfo("[ReqQryInvestorPositionCombineDetail] %s", ev->functions->toString(ev));
	try{
		CThostFtdcQryInvestorPositionCombineDetailField req;
		memset(&req, 0, sizeof(req));
		int nRequest =  getInt(ev, "requestId");
		strncpy(req.BrokerID, m_szBroker.c_str(), sizeof(req.BrokerID));
		strncpy(req.InvestorID, getString(ev, "investorId"), sizeof(req.InvestorID));
		strncpy(req.CombInstrumentID, getString(ev, "combInstrumentId"), sizeof(req.CombInstrumentID));
		int iResult = api->ReqQryInvestorPositionCombineDetail(&req, nRequest);
		if (iResult != 0){
			ShowError(iResult, nRequest);
			AP_LogWarn("[ReqQryInvestorPositionCombineDetail] Fail to query investorPositionCombineDetail.");
			return false;
		}

	}catch(...){
		AP_LogError("[ReqQryInvestorPositionCombineDetail] exception when query investorPositionCombineDetail.");
		return false;
	}
	return true;
}

bool OMS_Transport::ReqQryEWarrantOffset(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev)
{
	AP_LogInfo("[ReqQryEWarrantOffset] %s", ev->functions->toString(ev));
	try{
		CThostFtdcQryEWarrantOffsetField req;
		memset(&req, 0, sizeof(req));
		int nRequest =  getInt(ev, "requestId");
		strncpy(req.BrokerID, m_szBroker.c_str(), sizeof(req.BrokerID));
		strncpy(req.InvestorID, getString(ev, "investorId"), sizeof(req.InvestorID));
		strncpy(req.ExchangeID, getString(ev, "exchangeId"), sizeof(req.ExchangeID));
		strncpy(req.InstrumentID, getString(ev, "instrumentId"), sizeof(req.InstrumentID));
		int iResult = api->ReqQryEWarrantOffset(&req, nRequest);
		if (iResult != 0){
			ShowError(iResult, nRequest);
			AP_LogWarn("[ReqQryEWarrantOffset] Fail to query EWarrantOffset.");
			return false;
		}

	}catch(...){
		AP_LogError("[ReqQryEWarrantOffset] exception when query EWarrantOffset.");
		return false;
	}
	return true;
}

bool OMS_Transport::ReqQryTradingNotice(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev)
{
	AP_LogInfo("[ReqQryTradingNotice] %s", ev->functions->toString(ev));
	try{
		CThostFtdcQryTradingNoticeField req;
		memset(&req, 0, sizeof(req));
		int nRequest =  getInt(ev, "requestId");
		strncpy(req.BrokerID, m_szBroker.c_str(), sizeof(req.BrokerID));
		strncpy(req.InvestorID, getString(ev, "investorId"), sizeof(req.InvestorID));
		int iResult = api->ReqQryTradingNotice(&req, nRequest);
		if (iResult != 0){
			ShowError(iResult, nRequest);
			AP_LogWarn("[ReqQryTradingNotice] Fail to query tradingNotice.");
			return false;
		}

	}catch(...){
		AP_LogError("[ReqQryTradingNotice] exception when query tradingNotice.");
		return false;
	}
	return true;
}

bool OMS_Transport::ReqQryBrokerTradingParams(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev)
{
	AP_LogInfo("[ReqQryBrokerTradingParams] %s", ev->functions->toString(ev));
	try{
		CThostFtdcQryBrokerTradingParamsField req;
		memset(&req, 0, sizeof(req));
		int nRequest =  getInt(ev, "requestId");
		strncpy(req.BrokerID, m_szBroker.c_str(), sizeof(req.BrokerID));
		strncpy(req.InvestorID, getString(ev, "investorId"), sizeof(req.InvestorID));
		int iResult = api->ReqQryBrokerTradingParams(&req, nRequest);
		if (iResult != 0){
			ShowError(iResult, nRequest);
			AP_LogWarn("[ReqQryBrokerTradingParams] Fail to query brokerTradingParams.");
			return false;
		}

	}catch(...){
		AP_LogError("[ReqQryBrokerTradingParams] exception when query brokerTradingParams.");
		return false;
	}
	return true;
}

bool OMS_Transport::ReqQryBrokerTradingAlgos(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev)
{
	AP_LogInfo("[ReqQryBrokerTradingAlgos] %s", ev->functions->toString(ev));
	try{
		CThostFtdcQryBrokerTradingAlgosField req;
		memset(&req, 0, sizeof(req));
		int nRequest =  getInt(ev, "requestId");
		strncpy(req.BrokerID, m_szBroker.c_str(), sizeof(req.BrokerID));
		strncpy(req.ExchangeID, getString(ev, "exchangeId"), sizeof(req.ExchangeID));
		strncpy(req.InstrumentID, getString(ev, "instrumentId"), sizeof(req.InstrumentID));
		int iResult = api->ReqQryBrokerTradingAlgos(&req, nRequest);
		if (iResult != 0){
			ShowError(iResult, nRequest);
			AP_LogWarn("[ReqQryBrokerTradingAlgos] Fail to query brokerTradingAlgos.");
			return false;
		}

	}catch(...){
		AP_LogError("[ReqQryBrokerTradingAlgos] exception when query brokerTradingAlgos.");
		return false;
	}
	return true;
}

bool OMS_Transport::ReqRemoveParkedOrder(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev)
{
	AP_LogInfo("[ReqRemoveParkedOrder] %s", ev->functions->toString(ev));
	try{
		CThostFtdcRemoveParkedOrderField req;
		memset(&req, 0, sizeof(req));
		int nRequest =  getInt(ev, "requestId");
		strncpy(req.BrokerID, m_szBroker.c_str(), sizeof(req.BrokerID));
		strncpy(req.InvestorID, getString(ev, "investorId"), sizeof(req.InvestorID));
		strncpy(req.ParkedOrderID, getString(ev, "parkedOrderId"), sizeof(req.ParkedOrderID));
		int iResult = api->ReqRemoveParkedOrder(&req, nRequest);
		if (iResult != 0){
			ShowError(iResult, nRequest);
			AP_LogWarn("[ReqRemoveParkedOrder] Fail to removeParkedOrder.");
			return false;
		}

	}catch(...){
		AP_LogError("[ReqRemoveParkedOrder] exception when removeParkedOrder.");
		return false;
	}
	return true;
}

bool OMS_Transport::ReqRemoveParkedOrderAction(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev)
{
	AP_LogInfo("[ReqRemoveParkedOrderAction] %s", ev->functions->toString(ev));
	try{
		CThostFtdcRemoveParkedOrderActionField req;
		memset(&req, 0, sizeof(req));
		int nRequest =  getInt(ev, "requestId");
		strncpy(req.BrokerID, m_szBroker.c_str(), sizeof(req.BrokerID));
		strncpy(req.InvestorID, getString(ev, "investorId"), sizeof(req.InvestorID));
		strncpy(req.ParkedOrderActionID, getString(ev, "parkedOrderActionId"), sizeof(req.ParkedOrderActionID));
		int iResult = api->ReqRemoveParkedOrderAction(&req, nRequest);
		if (iResult != 0){
			ShowError(iResult, nRequest);
			AP_LogWarn("[ReqRemoveParkedOrderAction] Fail to removeParkedOrderAction.");
			return false;
		}

	}catch(...){
		AP_LogError("[ReqRemoveParkedOrderAction] exception when removeParkedOrderAction.");
		return false;
	}
	return true;
}

bool OMS_Transport::ReqQryParkedOrder(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev)
{
	AP_LogInfo("[ReqQryParkedOrder] %s", ev->functions->toString(ev));
	try{
		CThostFtdcQryParkedOrderField req;
		memset(&req, 0, sizeof(req));
		int nRequest =  getInt(ev, "requestId");
		strncpy(req.BrokerID, m_szBroker.c_str(), sizeof(req.BrokerID));
		strncpy(req.InvestorID, getString(ev, "investorId"), sizeof(req.InvestorID));
		strncpy(req.InstrumentID, getString(ev, "instrumentId"), sizeof(req.InstrumentID));
		strncpy(req.ExchangeID, getString(ev, "exchangeId"), sizeof(req.ExchangeID));
		int iResult = api->ReqQryParkedOrder(&req, nRequest);
		if (iResult != 0){
			ShowError(iResult, nRequest);
			AP_LogWarn("[ReqQryParkedOrder] Fail to query parkedOrder.");
			return false;
		}

	}catch(...){
		AP_LogError("[ReqQryParkedOrder] exception when query parkedOrder.");
		return false;
	}
	return true;
}

bool OMS_Transport::ReqQryParkedOrderAction(CThostFtdcTraderApi* api, AP_NormalisedEvent* ev)
{
	AP_LogInfo("[ReqQryParkedOrderAction] %s", ev->functions->toString(ev));
	try{
		CThostFtdcQryParkedOrderActionField req;
		memset(&req, 0, sizeof(req));
		int nRequest =  getInt(ev, "requestId");
		strncpy(req.BrokerID, m_szBroker.c_str(), sizeof(req.BrokerID));
		strncpy(req.InvestorID, getString(ev, "investorId"), sizeof(req.InvestorID));
		strncpy(req.InstrumentID, getString(ev, "instrumentId"), sizeof(req.InstrumentID));
		strncpy(req.ExchangeID, getString(ev, "exchangeId"), sizeof(req.ExchangeID));
		int iResult = api->ReqQryParkedOrderAction(&req, nRequest);
		if (iResult != 0){
			ShowError(iResult, nRequest);
			AP_LogWarn("[ReqQryParkedOrderAction] Fail to query parkedOrderAction.");
			return false;
		}

	}catch(...){
		AP_LogError("[ReqQryParkedOrderAction] exception when query parkedOrderAction.");
		return false;
	}
	return true;
}


AP_EventTransportError OMS_Transport::start() 
{	
	return AP_EventTransport_OK;
}


AP_EventTransportError OMS_Transport::stop() 
{	
	try{
		for (map<string, CThostFtdcTraderApi*>::iterator iter = m_mapTradeApis.begin(); iter != m_mapTradeApis.end(); iter++){	
			iter->second->Release();
			iter->second = NULL;
		}
	}
	catch (...){
		AP_LogError("Fail to stop oms.");
		return AP_EventTransport_InternalError;
	}
	return AP_EventTransport_OK;
}

const AP_char8* OMS_Transport::getLastError() 
{
	return m_pLastError;
}


void OMS_Transport::getStatus(AP_EventTransportStatus* statusp) 
{
	statusp->status = mRunStatus.status;
	statusp->totalReceived = mRunStatus.totalReceived;
	statusp->totalSent = mRunStatus.totalSent;
	statusp->statusDictionary->functions->addQuick(statusp->statusDictionary, "vesion", "1.0");
}

static AP_EventTransportError updateProperties(struct AP_EventTransport* transport, AP_EventTransportProperties* properties, IAF_TimestampConfig* timestampConfig) {
	OMS_Transport* transportObject = (OMS_Transport*)(transport->reserved);
	return transportObject->updateProperties(properties, timestampConfig);
}

static AP_EventTransportError sendTransportEvent(struct AP_EventTransport* transport, AP_TransportEvent tevent, AP_TimestampSet* tss) {
	OMS_Transport* transportObject = (OMS_Transport*)(transport->reserved);
	return transportObject->sendTransportEvent(tevent, tss);
}

static void addEventDecoder(struct AP_EventTransport* transport, const AP_char8* name, AP_EventDecoder* decoder) {
	OMS_Transport* transportObject = (OMS_Transport*)(transport->reserved);
	transportObject->addEventDecoder(name, decoder);
}

static void removeEventDecoder(struct AP_EventTransport* transport, const AP_char8* name) {
	OMS_Transport* transportObject = (OMS_Transport*)(transport->reserved);
	transportObject->removeEventDecoder(name);
}

static AP_EventTransportError flushUpstream(struct AP_EventTransport* transport) {
	OMS_Transport* transportObject = (OMS_Transport*)(transport->reserved);
	return transportObject->flushUpstream();
}

static AP_EventTransportError flushDownstream(struct AP_EventTransport* transport) {
	OMS_Transport* transportObject = (OMS_Transport*)(transport->reserved);
	return transportObject->flushDownstream();
}

static AP_EventTransportError start(struct AP_EventTransport* transport) {
	OMS_Transport* transportObject = (OMS_Transport*)(transport->reserved);
	return transportObject->start();
}

static AP_EventTransportError stop(struct AP_EventTransport* transport) {
	OMS_Transport* transportObject = (OMS_Transport*)(transport->reserved);
	return transportObject->stop();
}

static const AP_char8* getLastError(struct AP_EventTransport* transport) {
	OMS_Transport* transportObject = (OMS_Transport*)(transport->reserved);
	return transportObject->getLastError();
}

static void getStatus(struct AP_EventTransport* transport, AP_EventTransportStatus* status) {
	OMS_Transport* transportObject = (OMS_Transport*)(transport->reserved);
	return transportObject->getStatus(status);
}

static struct AP_EventTransport_Functions EventTransport_Functions = {
	updateProperties,
	sendTransportEvent,
	addEventDecoder,
	removeEventDecoder,
	flushUpstream,
	flushDownstream,
	start,
	stop,
	getLastError,
	getStatus
};

extern "C" {

	AP_EVENTTRANSPORT_API void AP_EVENTTRANSPORT_CALL AP_EVENTTRANSPORT_INFO_FUNCTION_NAME(AP_uint32* version, AP_uint32* capabilities) 
	{
		*version = AP_EVENTTRANSPORT_VERSION;
		*capabilities = 0;
	}

	AP_EVENTTRANSPORT_API AP_EventTransport* AP_EVENTTRANSPORT_CALL AP_EVENTTRANSPORT_CTOR_FUNCTION_NAME(AP_char8* name, AP_EventTransportProperties* properties, AP_EventTransportError* err, AP_char8** errMsg, IAF_TimestampConfig* timestampConfig) 
	{
		AP_EventTransport* transport;
		OMS_Transport* transportObject;
		transport = (AP_EventTransport*)malloc(sizeof(AP_EventTransport));

		try {
			transportObject = new OMS_Transport(name, properties, err, timestampConfig);
		} catch(std::exception &e) {
			*err=AP_EventTransport_BadProperties;
			*errMsg=_strdup(e.what());
			return NULL;
		}
		if((*err=transportObject->updateProperties(properties, timestampConfig))) {
			*errMsg=_strdup("Invalid properties");
		}

		transport->functions = &EventTransport_Functions;
		transport->reserved = transportObject;
		return transport;
	}

	AP_EVENTTRANSPORT_API void AP_EVENTTRANSPORT_CALL AP_EVENTTRANSPORT_DTOR_FUNCTION_NAME(AP_EventTransport* transport) 
	{
		OMS_Transport* transportObject = (OMS_Transport*)(transport->reserved);
		delete transportObject;
		free(transport);
	}
}