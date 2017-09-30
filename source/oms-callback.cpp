#include "oms-callback.h"
#include "oms-ctp-transport.h"
#include "engine_client_cpp.hpp"

#define CHAR_BUF_SIZE 128

OMSCallback::OMSCallback(OMS_Transport* pTransport, HANDLE& loginHandle)
{	
	m_pTransport = pTransport;
	m_hLogin = loginHandle;
}

OMSCallback::~OMSCallback()
{
}

void OMSCallback::addChar(char* buf, int size, struct AP_NormalisedEvent* ev, AP_NormalisedEventKey key, char value)
{
	sprintf_s(buf, size, "%c", value);
	ev->functions->addQuick(ev, key, buf);
}

void OMSCallback::addString(struct AP_NormalisedEvent* ev, AP_NormalisedEventKey key, const char* value)
{
	if (value == NULL){
		ev->functions->addQuick(ev, key, "");
	} else {
		AP_char8* utf8 = com::apama::convertToUTF8(value);
		ev->functions->addQuick(ev, key, utf8);
	}
}

void OMSCallback::addInt(char* buf, int size, struct AP_NormalisedEvent* ev, AP_NormalisedEventKey key, int value) 
{
	sprintf_s(buf, size, "%d", value);
	ev->functions->addQuick(ev, key, buf);
}

void OMSCallback::addDouble(char* buf, int size, struct AP_NormalisedEvent* ev, AP_NormalisedEventKey key, double value) 
{
	if (value > 999999999999.9999 || value < -999999999999.9999) {
		AP_LogWarn("Ignore double [%s] %12.4f", key, value);
	} else {
		sprintf_s(buf, size, "%12.4f", value);
		ev->functions->addQuick(ev, key, buf);
	}
}

void OMSCallback::OnFrontConnected()
{	
	AP_LogInfo("OnFrontConnected");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	m_pTransport->decode(ev, "OnFrontConnected");
	SetEvent(m_hLogin);
}

void OMSCallback::OnFrontDisconnected(int nReason)
{
	AP_LogInfo("OnFrontDisconnected");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addInt(buf, CHAR_BUF_SIZE, ev, "reason", nReason);
	delete[] buf;
	buf = NULL;
	m_pTransport->decode(ev, "OnFrontDisconnected");
}

void OMSCallback::OnHeartBeatWarning(int nTimeLapse)
{
	AP_LogInfo("OnHeartBeatWarning");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addInt(buf, CHAR_BUF_SIZE, ev, "timeLapse", nTimeLapse);
	delete[] buf;
	buf = NULL;
	m_pTransport->decode(ev, "OnHeartBeatWarning");
}

void OMSCallback::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	AP_LogInfo("OnRspUserLogin");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
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
	m_pTransport->decode(ev, "OnRspUserLogin");
	delete[] buf;
	buf = NULL;

}

void OMSCallback::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	AP_LogInfo("OnRspUserLogout");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pUserLogout != NULL){	
		addString(ev, "brokerId", pUserLogout->BrokerID);
		addString(ev, "userId", pUserLogout->UserID);
	}
	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	m_pTransport->decode(ev, "OnRspUserLogout");
	delete[] buf;
	buf = NULL;
}

void OMSCallback::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	AP_LogInfo("OnRspOrderInsert");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pInputOrder != NULL){	
		addString(ev, "brokerId", pInputOrder->BrokerID);
		addString(ev, "investorId", pInputOrder->InvestorID);
		addString(ev, "instrumentId", pInputOrder->InstrumentID);
		addString(ev, "orderRef", pInputOrder->OrderRef);
		addString(ev, "userId", pInputOrder->UserID);
		addChar(buf, CHAR_BUF_SIZE, ev, "orderPriceType", pInputOrder->OrderPriceType);
		addChar(buf, CHAR_BUF_SIZE, ev, "direction", pInputOrder->Direction);
		addString(ev, "combOffsetFlag", pInputOrder->CombOffsetFlag);
		addString(ev, "combHedgeFlag", pInputOrder->CombHedgeFlag);
		addDouble(buf, CHAR_BUF_SIZE, ev, "limitPrice", pInputOrder->LimitPrice);
		addInt(buf, CHAR_BUF_SIZE, ev, "volumeTotalOriginal", pInputOrder->VolumeTotalOriginal);
		addChar(buf, CHAR_BUF_SIZE, ev, "timeCondition", pInputOrder->TimeCondition);
		addString(ev, "gtdDate", pInputOrder->GTDDate);
		addChar(buf, CHAR_BUF_SIZE, ev, "volumeCondition", pInputOrder->VolumeCondition);
		addInt(buf, CHAR_BUF_SIZE, ev, "minVolume", pInputOrder->MinVolume);
		addChar(buf, CHAR_BUF_SIZE, ev, "contingentCondition", pInputOrder->ContingentCondition);
		addDouble(buf, CHAR_BUF_SIZE, ev, "stopPrice", pInputOrder->StopPrice);
		addChar(buf, CHAR_BUF_SIZE, ev, "forceCloseReason", pInputOrder->ForceCloseReason);
		addInt(buf, CHAR_BUF_SIZE, ev, "isAutoSuspend", pInputOrder->IsAutoSuspend);
		addString(ev, "businessUnit", pInputOrder->BusinessUnit);
		addInt(buf, CHAR_BUF_SIZE, ev, "userForceClose", pInputOrder->UserForceClose);	
	}
	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	m_pTransport->decode(ev, "OnRspOrderInsert");
	delete[] buf;
	buf = NULL;
}

void OMSCallback::OnRspParkedOrderInsert(CThostFtdcParkedOrderField *pParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	AP_LogInfo("OnRspParkedOrderInsert");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pParkedOrder != NULL){	
		addString(ev, "brokerId", pParkedOrder->BrokerID);
		addString(ev, "investorId", pParkedOrder->InvestorID);
		addString(ev, "instrumentId", pParkedOrder->InstrumentID);
		addString(ev, "orderRef", pParkedOrder->OrderRef);
		addString(ev, "userId", pParkedOrder->UserID);
		addChar(buf, CHAR_BUF_SIZE, ev, "orderPriceType", pParkedOrder->OrderPriceType);
		addChar(buf, CHAR_BUF_SIZE, ev, "direction", pParkedOrder->Direction);
		addString(ev, "combOffsetFlag", pParkedOrder->CombOffsetFlag);
		addString(ev, "combHedgeFlag", pParkedOrder->CombHedgeFlag);
		addDouble(buf, CHAR_BUF_SIZE, ev, "limitPrice", pParkedOrder->LimitPrice);
		addInt(buf, CHAR_BUF_SIZE, ev, "volumeTotalOriginal", pParkedOrder->VolumeTotalOriginal);
		addChar(buf, CHAR_BUF_SIZE, ev, "timeCondition", pParkedOrder->TimeCondition);
		addString(ev, "gtdDate", pParkedOrder->GTDDate);
		addChar(buf, CHAR_BUF_SIZE, ev, "volumeCondition", pParkedOrder->VolumeCondition);
		addInt(buf, CHAR_BUF_SIZE, ev, "minVolume", pParkedOrder->MinVolume);
		addChar(buf, CHAR_BUF_SIZE, ev, "contingentCondition", pParkedOrder->ContingentCondition);
		addDouble(buf, CHAR_BUF_SIZE, ev, "stopPrice", pParkedOrder->StopPrice);
		addChar(buf, CHAR_BUF_SIZE, ev, "forceCloseReason", pParkedOrder->ForceCloseReason);
		addInt(buf, CHAR_BUF_SIZE, ev, "isAutoSuspend", pParkedOrder->IsAutoSuspend);
		addString(ev, "businessUnit", pParkedOrder->BusinessUnit);
		addInt(buf, CHAR_BUF_SIZE, ev, "userForceClose", pParkedOrder->UserForceClose);	
		addString(ev, "exchangeId", pParkedOrder->ExchangeID);
		addString(ev, "parkedOrderId", pParkedOrder->ParkedOrderID);
		addChar(buf, CHAR_BUF_SIZE, ev, "userType", pParkedOrder->UserType);
		addChar(buf, CHAR_BUF_SIZE, ev, "status", pParkedOrder->Status);
	}
	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	m_pTransport->decode(ev, "OnRspParkedOrderInsert");
	delete[] buf;
	buf = NULL;
}

void OMSCallback::OnRspParkedOrderAction(CThostFtdcParkedOrderActionField *pParkedOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	AP_LogInfo("OnRspParkedOrderAction");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pParkedOrderAction != NULL){	
		addString(ev, "brokerId", pParkedOrderAction->BrokerID);
		addString(ev, "investorId", pParkedOrderAction->InvestorID);
		addInt(buf, CHAR_BUF_SIZE, ev, "orderActionRef", pParkedOrderAction->OrderActionRef);
		addString(ev, "orderRef", pParkedOrderAction->OrderRef);
		addInt(buf, CHAR_BUF_SIZE, ev, "frontId", pParkedOrderAction->FrontID);
		addInt(buf, CHAR_BUF_SIZE, ev, "sessionId", pParkedOrderAction->SessionID);
		addString(ev, "exchangeId", pParkedOrderAction->ExchangeID);
		addString(ev, "orderSysId", pParkedOrderAction->OrderSysID);
		addChar(buf, CHAR_BUF_SIZE, ev, "actionFlag", pParkedOrderAction->ActionFlag);
		addDouble(buf, CHAR_BUF_SIZE, ev, "limitPrice", pParkedOrderAction->LimitPrice);
		addInt(buf, CHAR_BUF_SIZE, ev, "volumeChange", pParkedOrderAction->VolumeChange);
		addString(ev, "userId", pParkedOrderAction->UserID);
		addString(ev, "instrumentId", pParkedOrderAction->InstrumentID);
		addString(ev, "parkedOrderActionId", pParkedOrderAction->ParkedOrderActionID);
		addChar(buf, CHAR_BUF_SIZE, ev, "userType", pParkedOrderAction->UserType);
		addChar(buf, CHAR_BUF_SIZE, ev, "status", pParkedOrderAction->Status);
	}
	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	m_pTransport->decode(ev, "OnRspParkedOrderAction");
	delete[] buf;
	buf = NULL;
}

void OMSCallback::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	AP_LogInfo("OnRspOrderAction");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pInputOrderAction != NULL) {	
		addString(ev, "brokerId", pInputOrderAction->BrokerID);
		addString(ev, "investorId", pInputOrderAction->InvestorID);
		addInt(buf, CHAR_BUF_SIZE, ev, "orderActionRef", pInputOrderAction->OrderActionRef);
		addString(ev, "orderRef", pInputOrderAction->OrderRef);
		addInt(buf, CHAR_BUF_SIZE, ev, "frontId", pInputOrderAction->FrontID);
		addInt(buf, CHAR_BUF_SIZE, ev, "sessionId", pInputOrderAction->SessionID);
		addString(ev, "exchangeId", pInputOrderAction->ExchangeID);
		addString(ev, "orderSysId", pInputOrderAction->OrderSysID);
		addChar(buf, CHAR_BUF_SIZE, ev, "actionFlag", pInputOrderAction->ActionFlag);
		addDouble(buf, CHAR_BUF_SIZE, ev, "limitPrice", pInputOrderAction->LimitPrice);
		addInt(buf, CHAR_BUF_SIZE, ev, "volumeChange", pInputOrderAction->VolumeChange);
		addString(ev, "userId", pInputOrderAction->UserID);		
		addString(ev, "instrumentId", pInputOrderAction->InstrumentID);	
	}
	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	m_pTransport->decode(ev, "OnRspOrderAction");
	delete[] buf;
	buf = NULL;	
}

void OMSCallback::OnRspQueryMaxOrderVolume(CThostFtdcQueryMaxOrderVolumeField *pQueryMaxOrderVolume, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	AP_LogInfo("OnRspQueryMaxOrderVolume");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pQueryMaxOrderVolume != NULL) {	
		addString(ev, "brokerId", pQueryMaxOrderVolume->BrokerID);
		addString(ev, "investorId", pQueryMaxOrderVolume->InvestorID);
		addString(ev, "instrumentId", pQueryMaxOrderVolume->InstrumentID);
		addChar(buf, CHAR_BUF_SIZE, ev, "direction", pQueryMaxOrderVolume->Direction);
		addChar(buf, CHAR_BUF_SIZE, ev, "offsetFlag", pQueryMaxOrderVolume->OffsetFlag);
		addChar(buf, CHAR_BUF_SIZE, ev, "hedgeFlag", pQueryMaxOrderVolume->HedgeFlag);
		addInt(buf, CHAR_BUF_SIZE, ev, "maxVolume", pQueryMaxOrderVolume->MaxVolume);
	}
	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	m_pTransport->decode(ev, "OnRspQueryMaxOrderVolume");
	delete[] buf;
	buf = NULL;	
}

void OMSCallback::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	AP_LogInfo("OnRspSettlementInfoConfirm");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pSettlementInfoConfirm != NULL) {	
		addString(ev, "brokerId", pSettlementInfoConfirm->BrokerID);
		addString(ev, "investorId", pSettlementInfoConfirm->InvestorID);
		addString(ev, "confirmDate", pSettlementInfoConfirm->ConfirmDate);
		addString(ev, "confirmTime", pSettlementInfoConfirm->ConfirmTime);
	}
	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	m_pTransport->decode(ev, "OnRspSettlementInfoConfirm");
	delete[] buf;
	buf = NULL;	
}

void OMSCallback::OnRspRemoveParkedOrder(CThostFtdcRemoveParkedOrderField *pRemoveParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	AP_LogInfo("OnRspRemoveParkedOrder");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pRemoveParkedOrder != NULL) {	
		addString(ev, "brokerId", pRemoveParkedOrder->BrokerID);
		addString(ev, "investorId", pRemoveParkedOrder->InvestorID);
		addString(ev, "parkedOrderId", pRemoveParkedOrder->ParkedOrderID);
	}
	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	m_pTransport->decode(ev, "OnRspRemoveParkedOrder");
	delete[] buf;
	buf = NULL;	
}

void OMSCallback::OnRspRemoveParkedOrderAction(CThostFtdcRemoveParkedOrderActionField *pRemoveParkedOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	AP_LogInfo("OnRspRemoveParkedOrderAction");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pRemoveParkedOrderAction != NULL) {	
		addString(ev, "brokerId", pRemoveParkedOrderAction->BrokerID);
		addString(ev, "investorId", pRemoveParkedOrderAction->InvestorID);
		addString(ev, "parkedOrderId", pRemoveParkedOrderAction->ParkedOrderActionID);
	}
	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	m_pTransport->decode(ev, "OnRspRemoveParkedOrderAction");
	delete[] buf;
	buf = NULL;	
}

void OMSCallback::OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	AP_LogInfo("OnRspQryOrder");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pOrder != NULL) {	
		addString(ev, "brokerId", pOrder->BrokerID);
		addString(ev, "investorId", pOrder->InvestorID);
		addString(ev, "instrumentId", pOrder->InstrumentID);
		addString(ev, "orderRef", pOrder->OrderRef);
		addString(ev, "userId", pOrder->UserID);
		addChar(buf, CHAR_BUF_SIZE, ev, "orderPriceType", pOrder->OrderPriceType);
		addChar(buf, CHAR_BUF_SIZE, ev, "direction", pOrder->Direction);
		addString(ev, "combOffsetFlag", pOrder->CombOffsetFlag);
		addString(ev, "combHedgeFlag", pOrder->CombHedgeFlag);
		addDouble(buf, CHAR_BUF_SIZE, ev, "limitPrice", pOrder->LimitPrice);
		addInt(buf, CHAR_BUF_SIZE, ev, "volumeTotalOriginal", pOrder->VolumeTotalOriginal);
		addChar(buf, CHAR_BUF_SIZE, ev, "timeCondition", pOrder->TimeCondition);
		addString(ev, "gtdDate", pOrder->GTDDate);
		addChar(buf, CHAR_BUF_SIZE, ev, "volumeCondition", pOrder->VolumeCondition);
		addInt(buf, CHAR_BUF_SIZE, ev, "minVolume", pOrder->MinVolume);
		addChar(buf, CHAR_BUF_SIZE, ev, "contingentCondition", pOrder->ContingentCondition);
		addDouble(buf, CHAR_BUF_SIZE, ev, "stopPrice", pOrder->StopPrice);
		addChar(buf, CHAR_BUF_SIZE, ev, "forceCloseReason", pOrder->ForceCloseReason);
		addInt(buf, CHAR_BUF_SIZE, ev, "isAutoSuspend", pOrder->IsAutoSuspend);
		addString(ev, "businessUnit", pOrder->BusinessUnit);
		addString(ev, "orderLocalId", pOrder->OrderLocalID);
		addString(ev, "exchangeId", pOrder->ExchangeID);
		addString(ev, "participantId", pOrder->ParticipantID);
		addString(ev, "clientId", pOrder->ClientID);
		addString(ev, "exchangeInstId", pOrder->ExchangeInstID);
		addString(ev, "traderId", pOrder->TraderID);
		addInt(buf, CHAR_BUF_SIZE, ev, "installId", pOrder->InstallID);
		addChar(buf, CHAR_BUF_SIZE, ev, "orderSubmitStatus", pOrder->OrderSubmitStatus);
		addInt(buf, CHAR_BUF_SIZE, ev, "notifySequence", pOrder->NotifySequence);
		addString(ev, "tradingDay", pOrder->TradingDay);
		addInt(buf, CHAR_BUF_SIZE, ev, "settlementId", pOrder->SettlementID);
		addString(ev, "orderSysId", pOrder->OrderSysID);
		addChar(buf, CHAR_BUF_SIZE, ev, "orderSource", pOrder->OrderSource);
		addChar(buf, CHAR_BUF_SIZE, ev, "orderStatus", pOrder->OrderStatus);
		addChar(buf, CHAR_BUF_SIZE, ev, "orderType", pOrder->OrderType);
		addInt(buf, CHAR_BUF_SIZE, ev, "volumeTraded", pOrder->VolumeTraded);
		addInt(buf, CHAR_BUF_SIZE, ev, "volumeTotal", pOrder->VolumeTotal);
		addString(ev, "insertDate", pOrder->InsertDate);
		addString(ev, "insertTime", pOrder->InsertTime);
		addString(ev, "activeTime", pOrder->ActiveTime);
		addString(ev, "suspendTime", pOrder->SuspendTime);
		addString(ev, "updateTime", pOrder->UpdateTime);
		addString(ev, "cancelTime", pOrder->CancelTime);
		addString(ev, "activeTraderId", pOrder->ActiveTraderID);
		addString(ev, "clearingPartId", pOrder->ClearingPartID);
		addInt(buf, CHAR_BUF_SIZE, ev, "sequenceNo", pOrder->SequenceNo);
		addInt(buf, CHAR_BUF_SIZE, ev, "frontId", pOrder->FrontID);
		addInt(buf, CHAR_BUF_SIZE, ev, "sessionId", pOrder->SessionID);
		addString(ev, "userProductInfo", pOrder->UserProductInfo);
		addString(ev, "statusMsg", pOrder->StatusMsg);
		addInt(buf, CHAR_BUF_SIZE, ev, "userForceClose", pOrder->UserForceClose);
		addString(ev, "activeUserId", pOrder->ActiveUserID);
		addInt(buf, CHAR_BUF_SIZE, ev, "brokerOrderSeq", pOrder->BrokerOrderSeq);
		addString(ev, "relativeOrderSysId", pOrder->RelativeOrderSysID);
	}
	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	m_pTransport->decode(ev, "OnRspQryOrder");
	delete[] buf;
	buf = NULL;	
}

void OMSCallback::OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	AP_LogInfo("OnRspQryTrade");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pTrade != NULL) {	
		addString(ev, "brokerId", pTrade->BrokerID);
		addString(ev, "investorId", pTrade->InvestorID);
		addString(ev, "instrumentId", pTrade->InstrumentID);
		addString(ev, "orderRef", pTrade->OrderRef);
		addString(ev, "userId", pTrade->UserID);
		addString(ev, "exchangeId", pTrade->ExchangeID);
		addString(ev, "tradeId", pTrade->TradeID);
		addChar(buf, CHAR_BUF_SIZE, ev, "direction", pTrade->Direction);
		addString(ev, "orderSysId", pTrade->OrderSysID);
		addString(ev, "participantId", pTrade->ParticipantID);
		addString(ev, "clientId", pTrade->ClientID);
		addChar(buf, CHAR_BUF_SIZE, ev, "tradingRole", pTrade->TradingRole);
		addString(ev, "exchangeInstId", pTrade->ExchangeInstID);
		addChar(buf, CHAR_BUF_SIZE, ev, "offsetFlag", pTrade->OffsetFlag);
		addChar(buf, CHAR_BUF_SIZE, ev, "hedgeFlag", pTrade->HedgeFlag);
		addDouble(buf, CHAR_BUF_SIZE, ev, "price", pTrade->Price);
		addInt(buf, CHAR_BUF_SIZE, ev, "volume", pTrade->Volume);
		addString(ev, "tradeDate", pTrade->TradeDate);
		addString(ev, "tradeTime", pTrade->TradeTime);
		addChar(buf, CHAR_BUF_SIZE, ev, "tradeType", pTrade->TradeType);
		addChar(buf, CHAR_BUF_SIZE, ev, "priceSource", pTrade->PriceSource);
		addString(ev, "traderId", pTrade->TraderID);
		addString(ev, "orderLocalId", pTrade->OrderLocalID);
		addString(ev, "clearingPartId", pTrade->ClearingPartID);
		addString(ev, "businessUnit", pTrade->BusinessUnit);
		addInt(buf, CHAR_BUF_SIZE, ev, "sequenceNo", pTrade->SequenceNo);
		addString(ev, "tradingDay", pTrade->TradingDay);
		addInt(buf, CHAR_BUF_SIZE, ev, "settlementId", pTrade->SettlementID);
		addInt(buf, CHAR_BUF_SIZE, ev, "brokerOrderSeq", pTrade->BrokerOrderSeq);
		addChar(buf, CHAR_BUF_SIZE, ev, "tradeSource", pTrade->TradeSource);
	}
	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	m_pTransport->decode(ev, "OnRspQryTrade");
	delete[] buf;
	buf = NULL;	
}

void OMSCallback::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{	
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pInvestorPosition != NULL) {	
		addString(ev, "instrumentId", pInvestorPosition->InstrumentID);
		addString(ev, "brokerId", pInvestorPosition->BrokerID);
		addString(ev, "investorId", pInvestorPosition->InvestorID);
		addChar(buf, CHAR_BUF_SIZE, ev, "posiDirection", pInvestorPosition->PosiDirection);
		addChar(buf, CHAR_BUF_SIZE, ev, "hedgeFlag", pInvestorPosition->HedgeFlag);
		addChar(buf, CHAR_BUF_SIZE, ev, "positionDate", pInvestorPosition->PositionDate);
		addInt(buf, CHAR_BUF_SIZE, ev, "ydPosition", pInvestorPosition->YdPosition);
		addInt(buf, CHAR_BUF_SIZE, ev, "position", pInvestorPosition->Position);
		addInt(buf, CHAR_BUF_SIZE, ev, "longFrozen", pInvestorPosition->LongFrozen);
		addInt(buf, CHAR_BUF_SIZE, ev, "shortFrozen", pInvestorPosition->ShortFrozen);
		addDouble(buf, CHAR_BUF_SIZE, ev, "longFrozenAmount", pInvestorPosition->LongFrozenAmount);
		addDouble(buf, CHAR_BUF_SIZE, ev, "shortFrozenAmount", pInvestorPosition->ShortFrozenAmount);
		addInt(buf, CHAR_BUF_SIZE, ev, "openVolume", pInvestorPosition->OpenVolume);
		addInt(buf, CHAR_BUF_SIZE, ev, "closeVolume", pInvestorPosition->CloseVolume);
		addDouble(buf, CHAR_BUF_SIZE, ev, "openAmount", pInvestorPosition->OpenAmount);
		addDouble(buf, CHAR_BUF_SIZE, ev, "closeAmount", pInvestorPosition->CloseAmount);
		addDouble(buf, CHAR_BUF_SIZE, ev, "positionCost", pInvestorPosition->PositionCost);
		addDouble(buf, CHAR_BUF_SIZE, ev, "preMargin", pInvestorPosition->PreMargin);
		addDouble(buf, CHAR_BUF_SIZE, ev, "useMargin", pInvestorPosition->UseMargin);
		addDouble(buf, CHAR_BUF_SIZE, ev, "frozenMargin", pInvestorPosition->FrozenMargin);
		addDouble(buf, CHAR_BUF_SIZE, ev, "frozenCash", pInvestorPosition->FrozenCash);
		addDouble(buf, CHAR_BUF_SIZE, ev, "frozenCommission", pInvestorPosition->FrozenCommission);
		addDouble(buf, CHAR_BUF_SIZE, ev, "cashIn", pInvestorPosition->CashIn);
		addDouble(buf, CHAR_BUF_SIZE, ev, "commission", pInvestorPosition->Commission);
		addDouble(buf, CHAR_BUF_SIZE, ev, "closeProfit", pInvestorPosition->CloseProfit);
		addDouble(buf, CHAR_BUF_SIZE, ev, "positionProfit", pInvestorPosition->PositionProfit);
		addDouble(buf, CHAR_BUF_SIZE, ev, "preSettlementPrice", pInvestorPosition->PreSettlementPrice);
		addDouble(buf, CHAR_BUF_SIZE, ev, "settlementPrice", pInvestorPosition->SettlementPrice);
		addString(ev, "tradingDay", pInvestorPosition->TradingDay);
		addInt(buf, CHAR_BUF_SIZE, ev, "settlementId", pInvestorPosition->SettlementID);
		addDouble(buf, CHAR_BUF_SIZE, ev, "openCost", pInvestorPosition->OpenCost);
		addDouble(buf, CHAR_BUF_SIZE, ev, "exchangeMargin", pInvestorPosition->ExchangeMargin);
		addInt(buf, CHAR_BUF_SIZE, ev, "combPosition", pInvestorPosition->CombPosition);
		addDouble(buf, CHAR_BUF_SIZE, ev, "combLongFrozen", pInvestorPosition->CombLongFrozen);
		addDouble(buf, CHAR_BUF_SIZE, ev, "combShortFrozen", pInvestorPosition->CombShortFrozen);
		addDouble(buf, CHAR_BUF_SIZE, ev, "closeProfitByDate", pInvestorPosition->CloseProfitByDate);
		addDouble(buf, CHAR_BUF_SIZE, ev, "closeProfitByTrade", pInvestorPosition->CloseProfitByTrade);
		addInt(buf, CHAR_BUF_SIZE, ev, "todayPosition", pInvestorPosition->TodayPosition);
		addDouble(buf, CHAR_BUF_SIZE, ev, "marginRateByMoney", pInvestorPosition->MarginRateByMoney);
		addDouble(buf, CHAR_BUF_SIZE, ev, "marginRateByVolume", pInvestorPosition->MarginRateByVolume);
	}
	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	m_pTransport->decode(ev, "OnRspQryInvestorPosition");
	delete[] buf;
	buf = NULL;	
}

void OMSCallback::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	AP_LogInfo("OnRspQryTradingAccount");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pTradingAccount != NULL) {	
		addString(ev, "brokerId", pTradingAccount->BrokerID);
		addString(ev, "accountId", pTradingAccount->AccountID);
		addDouble(buf, CHAR_BUF_SIZE, ev, "preMortgage", pTradingAccount->PreMortgage);
		addDouble(buf, CHAR_BUF_SIZE, ev, "preCredit", pTradingAccount->PreCredit);
		addDouble(buf, CHAR_BUF_SIZE, ev, "preDeposit", pTradingAccount->PreDeposit);
		addDouble(buf, CHAR_BUF_SIZE, ev, "preBalance", pTradingAccount->PreBalance);
		addDouble(buf, CHAR_BUF_SIZE, ev, "preMargin", pTradingAccount->PreMargin);
		addDouble(buf, CHAR_BUF_SIZE, ev, "interestBase", pTradingAccount->InterestBase);
		addDouble(buf, CHAR_BUF_SIZE, ev, "interest", pTradingAccount->Interest);
		addDouble(buf, CHAR_BUF_SIZE, ev, "deposit", pTradingAccount->Deposit);
		addDouble(buf, CHAR_BUF_SIZE, ev, "withdraw", pTradingAccount->Withdraw);
		addDouble(buf, CHAR_BUF_SIZE, ev, "frozenMargin", pTradingAccount->FrozenMargin);
		addDouble(buf, CHAR_BUF_SIZE, ev, "frozenCash", pTradingAccount->FrozenCash);
		addDouble(buf, CHAR_BUF_SIZE, ev, "frozenCommission", pTradingAccount->FrozenCommission);
		addDouble(buf, CHAR_BUF_SIZE, ev, "currMargin", pTradingAccount->CurrMargin);
		addDouble(buf, CHAR_BUF_SIZE, ev, "cashIn", pTradingAccount->CashIn);
		addDouble(buf, CHAR_BUF_SIZE, ev, "commission", pTradingAccount->Commission);
		addDouble(buf, CHAR_BUF_SIZE, ev, "closeProfit", pTradingAccount->CloseProfit);
		addDouble(buf, CHAR_BUF_SIZE, ev, "positionProfit", pTradingAccount->PositionProfit);
		addDouble(buf, CHAR_BUF_SIZE, ev, "balance", pTradingAccount->Balance);
		addDouble(buf, CHAR_BUF_SIZE, ev, "available", pTradingAccount->Available);
		addDouble(buf, CHAR_BUF_SIZE, ev, "withdrawQuota", pTradingAccount->WithdrawQuota);
		addDouble(buf, CHAR_BUF_SIZE, ev, "reserve", pTradingAccount->Reserve);
		addString(ev, "tradingDay", pTradingAccount->TradingDay);
		addInt(buf, CHAR_BUF_SIZE, ev, "settlementId", pTradingAccount->SettlementID);
		addDouble(buf, CHAR_BUF_SIZE, ev, "credit", pTradingAccount->Credit);
		addDouble(buf, CHAR_BUF_SIZE, ev, "mortgage", pTradingAccount->Mortgage);
		addDouble(buf, CHAR_BUF_SIZE, ev, "exchangeMargin", pTradingAccount->ExchangeMargin);
		addDouble(buf, CHAR_BUF_SIZE, ev, "deliveryMargin", pTradingAccount->DeliveryMargin);
		addDouble(buf, CHAR_BUF_SIZE, ev, "exchangeDeliveryMargin", pTradingAccount->ExchangeDeliveryMargin);
	}
	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	m_pTransport->decode(ev, "OnRspQryTradingAccount");
	delete[] buf;
	buf = NULL;	
}

void OMSCallback::OnRspQryInvestor(CThostFtdcInvestorField *pInvestor, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	AP_LogInfo("OnRspQryInvestor");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pInvestor != NULL) {	
		addString(ev, "investorId", pInvestor->InvestorID);
		addString(ev, "brokerId", pInvestor->BrokerID);
		addString(ev, "investorGroupId", pInvestor->InvestorGroupID);
		addString(ev, "investorName", pInvestor->InvestorName);
		addChar(buf, CHAR_BUF_SIZE, ev, "identifiedCardType", pInvestor->IdentifiedCardType);
		addString(ev, "identifiedCardNo", pInvestor->IdentifiedCardNo);
		addInt(buf, CHAR_BUF_SIZE, ev, "isActive", pInvestor->IsActive);
		addString(ev, "telephone", pInvestor->Telephone);
		addString(ev, "address", pInvestor->Address);
		addString(ev, "openDate", pInvestor->OpenDate);
		addString(ev, "mobile", pInvestor->Mobile);
		addString(ev, "commModelId", pInvestor->CommModelID);
	}
	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	m_pTransport->decode(ev, "OnRspQryInvestor");
	delete[] buf;
	buf = NULL;	
}

void OMSCallback::OnRspQryTradingCode(CThostFtdcTradingCodeField *pTradingCode, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	AP_LogInfo("OnRspQryTradingCode");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pTradingCode != NULL) {	
		addString(ev, "investorId", pTradingCode->InvestorID);
		addString(ev, "brokerId", pTradingCode->BrokerID);
		addString(ev, "exchangeId", pTradingCode->ExchangeID);
		addString(ev, "clientId", pTradingCode->ClientID);
		addInt(buf, CHAR_BUF_SIZE, ev, "isActive", pTradingCode->IsActive);
		addChar(buf, CHAR_BUF_SIZE, ev, "clientIdType", pTradingCode->ClientIDType);
	}
	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	m_pTransport->decode(ev, "OnRspQryTradingCode");
	delete[] buf;
	buf = NULL;	
}

void OMSCallback::OnRspQryInstrumentMarginRate(CThostFtdcInstrumentMarginRateField *pInstrumentMarginRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	AP_LogInfo("OnRspQryInstrumentMarginRate");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pInstrumentMarginRate != NULL) {	
		addString(ev, "instrumentId", pInstrumentMarginRate->InstrumentID);
		addChar(buf, CHAR_BUF_SIZE, ev, "investorRange", pInstrumentMarginRate->InvestorRange);
		addString(ev, "brokerId", pInstrumentMarginRate->BrokerID);
		addString(ev, "investorId", pInstrumentMarginRate->InvestorID);
		addChar(buf, CHAR_BUF_SIZE, ev, "hedgeFlag", pInstrumentMarginRate->HedgeFlag);
		addDouble(buf, CHAR_BUF_SIZE, ev, "longMarginRatioByMoney", pInstrumentMarginRate->LongMarginRatioByMoney);
		addDouble(buf, CHAR_BUF_SIZE, ev, "longMarginRatioByVolume", pInstrumentMarginRate->LongMarginRatioByVolume);
		addDouble(buf, CHAR_BUF_SIZE, ev, "shortMarginRatioByMoney", pInstrumentMarginRate->ShortMarginRatioByMoney);
		addDouble(buf, CHAR_BUF_SIZE, ev, "shortMarginRatioByVolume", pInstrumentMarginRate->ShortMarginRatioByVolume);
	}
	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	m_pTransport->decode(ev, "OnRspQryInstrumentMarginRate");
	delete[] buf;
	buf = NULL;	
}

void OMSCallback::OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	AP_LogInfo("OnRspQryInstrumentCommissionRate");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pInstrumentCommissionRate != NULL) {	
		addString(ev, "instrumentId", pInstrumentCommissionRate->InstrumentID);
		addChar(buf, CHAR_BUF_SIZE, ev, "investorRange", pInstrumentCommissionRate->InvestorRange);
		addString(ev, "brokerId", pInstrumentCommissionRate->BrokerID);
		addString(ev, "investorId", pInstrumentCommissionRate->InvestorID);
		addDouble(buf, CHAR_BUF_SIZE, ev, "openRatioByMoney", pInstrumentCommissionRate->OpenRatioByMoney);
		addDouble(buf, CHAR_BUF_SIZE, ev, "openRatioByVolume", pInstrumentCommissionRate->OpenRatioByVolume);
		addDouble(buf, CHAR_BUF_SIZE, ev, "closeRatioByMoney", pInstrumentCommissionRate->CloseRatioByMoney);
		addDouble(buf, CHAR_BUF_SIZE, ev, "closeRatioByVolume", pInstrumentCommissionRate->CloseRatioByVolume);
		addDouble(buf, CHAR_BUF_SIZE, ev, "closeTodayRatioByMoney", pInstrumentCommissionRate->CloseTodayRatioByMoney);
		addDouble(buf, CHAR_BUF_SIZE, ev, "closeTodayRatioByVolume", pInstrumentCommissionRate->CloseTodayRatioByVolume);
	}
	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	m_pTransport->decode(ev, "OnRspQryInstrumentCommissionRate");
	delete[] buf;
	buf = NULL;	
}

void OMSCallback::OnRspQryExchange(CThostFtdcExchangeField *pExchange, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	AP_LogInfo("OnRspQryExchange");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pExchange != NULL) {	
		addString(ev, "exchangeId", pExchange->ExchangeID);
		addString(ev, "exchangeName", pExchange->ExchangeName);
		addInt(buf, CHAR_BUF_SIZE, ev, "exchangeProperty", pExchange->ExchangeProperty);
	}
	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	m_pTransport->decode(ev, "OnRspQryExchange");
	delete[] buf;
	buf = NULL;	
}

void OMSCallback::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	AP_LogInfo("OnRspQryInstrument");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pInstrument != NULL) {	
		addString(ev, "instrumentId", pInstrument->InstrumentID);
		addString(ev, "exchangeId", pInstrument->ExchangeID);
		addString(ev, "instrumentName", pInstrument->InstrumentName);
		addString(ev, "exchangeInstId", pInstrument->ExchangeInstID);
		addString(ev, "productId", pInstrument->ProductID);
		addChar(buf, CHAR_BUF_SIZE, ev, "productClass", pInstrument->ProductClass);
		addInt(buf, CHAR_BUF_SIZE, ev, "deliveryYear", pInstrument->DeliveryYear);
		addInt(buf, CHAR_BUF_SIZE, ev, "deliveryMonth", pInstrument->DeliveryMonth);
		addInt(buf, CHAR_BUF_SIZE, ev, "maxMarketOrderVolume", pInstrument->MaxMarketOrderVolume);
		addInt(buf, CHAR_BUF_SIZE, ev, "minMarketOrderVolume", pInstrument->MinMarketOrderVolume);
		addInt(buf, CHAR_BUF_SIZE, ev, "maxLimitOrderVolume", pInstrument->MaxLimitOrderVolume);
		addInt(buf, CHAR_BUF_SIZE, ev, "minLimitOrderVolume", pInstrument->MinLimitOrderVolume);
		addInt(buf, CHAR_BUF_SIZE, ev, "volumeMultiple", pInstrument->VolumeMultiple);
		addDouble(buf, CHAR_BUF_SIZE, ev, "priceTick", pInstrument->PriceTick);
		addString(ev, "createDate", pInstrument->CreateDate);
		addString(ev, "openDate", pInstrument->OpenDate);
		addString(ev, "expireDate", pInstrument->ExpireDate);
		addString(ev, "startDelivDate", pInstrument->StartDelivDate);
		addString(ev, "endDelivDate", pInstrument->EndDelivDate);
		addChar(buf, CHAR_BUF_SIZE, ev, "instLifePhase", pInstrument->InstLifePhase);
		addInt(buf, CHAR_BUF_SIZE, ev, "isTrading", pInstrument->IsTrading);
		addChar(buf, CHAR_BUF_SIZE, ev, "positionType", pInstrument->PositionType);
		addChar(buf, CHAR_BUF_SIZE, ev, "positionDateType", pInstrument->PositionDateType);
		addDouble(buf, CHAR_BUF_SIZE, ev, "longMarginRatio", pInstrument->LongMarginRatio);
		addDouble(buf, CHAR_BUF_SIZE, ev, "shortMarginRatio", pInstrument->ShortMarginRatio);
	}
	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	m_pTransport->decode(ev, "OnRspQryInstrument");
	delete[] buf;
	buf = NULL;	
}

void OMSCallback::OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	AP_LogInfo("OnRspQryDepthMarketData");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pDepthMarketData != NULL) {	
		addString(ev, "tradingDay", pDepthMarketData->TradingDay);
		addString(ev, "instrumentId", pDepthMarketData->InstrumentID);
		addString(ev, "exchangeId", pDepthMarketData->ExchangeID);
		addString(ev, "exchangeInstId", pDepthMarketData->ExchangeInstID);
		addDouble(buf, CHAR_BUF_SIZE, ev, "lastPrice", pDepthMarketData->LastPrice);
		addDouble(buf, CHAR_BUF_SIZE, ev, "preSettlementPrice", pDepthMarketData->PreSettlementPrice);
		addDouble(buf, CHAR_BUF_SIZE, ev, "preClosePrice", pDepthMarketData->PreClosePrice);
		addDouble(buf, CHAR_BUF_SIZE, ev, "preOpenInterest", pDepthMarketData->PreOpenInterest);
		addDouble(buf, CHAR_BUF_SIZE, ev, "openPrice", pDepthMarketData->OpenPrice);
		addDouble(buf, CHAR_BUF_SIZE, ev, "highestPrice", pDepthMarketData->HighestPrice);
		addDouble(buf, CHAR_BUF_SIZE, ev, "lowestPrice", pDepthMarketData->LowestPrice);
		addInt(buf, CHAR_BUF_SIZE, ev, "volume", pDepthMarketData->Volume);
		addDouble(buf, CHAR_BUF_SIZE, ev, "turnover", pDepthMarketData->Turnover);
		addDouble(buf, CHAR_BUF_SIZE, ev, "openInterest", pDepthMarketData->OpenInterest);
		addDouble(buf, CHAR_BUF_SIZE, ev, "closePrice", pDepthMarketData->ClosePrice);
		addDouble(buf, CHAR_BUF_SIZE, ev, "settlementPrice", pDepthMarketData->SettlementPrice);
		addDouble(buf, CHAR_BUF_SIZE, ev, "upperLimitPrice", pDepthMarketData->UpperLimitPrice);
		addDouble(buf, CHAR_BUF_SIZE, ev, "lowerLimitPrice", pDepthMarketData->LowerLimitPrice);
		addDouble(buf, CHAR_BUF_SIZE, ev, "preDelta", pDepthMarketData->PreDelta);
		addDouble(buf, CHAR_BUF_SIZE, ev, "currDelta", pDepthMarketData->CurrDelta);
		addString(ev, "updateTime", pDepthMarketData->UpdateTime);
		addInt(buf, CHAR_BUF_SIZE, ev, "updateMillisec", pDepthMarketData->UpdateMillisec);
		addDouble(buf, CHAR_BUF_SIZE, ev, "bidPrice1", pDepthMarketData->BidPrice1);
		addInt(buf, CHAR_BUF_SIZE, ev, "bidVolume1", pDepthMarketData->BidVolume1);
		addDouble(buf, CHAR_BUF_SIZE, ev, "askPrice1", pDepthMarketData->AskPrice1);
		addInt(buf, CHAR_BUF_SIZE, ev, "askVolume1", pDepthMarketData->AskVolume1);
		addDouble(buf, CHAR_BUF_SIZE, ev, "bidPrice2", pDepthMarketData->BidPrice2);
		addInt(buf, CHAR_BUF_SIZE, ev, "bidVolume2", pDepthMarketData->BidVolume2);
		addDouble(buf, CHAR_BUF_SIZE, ev, "askPrice2", pDepthMarketData->AskPrice2);
		addInt(buf, CHAR_BUF_SIZE, ev, "askVolume2", pDepthMarketData->AskVolume2);
		addDouble(buf, CHAR_BUF_SIZE, ev, "bidPrice3", pDepthMarketData->BidPrice3);
		addInt(buf, CHAR_BUF_SIZE, ev, "bidVolume3", pDepthMarketData->BidVolume3);
		addDouble(buf, CHAR_BUF_SIZE, ev, "askPrice3", pDepthMarketData->AskPrice3);
		addInt(buf, CHAR_BUF_SIZE, ev, "askVolume3", pDepthMarketData->AskVolume3);
		addDouble(buf, CHAR_BUF_SIZE, ev, "bidPrice4", pDepthMarketData->BidPrice4);
		addInt(buf, CHAR_BUF_SIZE, ev, "bidVolume4", pDepthMarketData->BidVolume4);
		addDouble(buf, CHAR_BUF_SIZE, ev, "askPrice4", pDepthMarketData->AskPrice4);
		addInt(buf, CHAR_BUF_SIZE, ev, "askVolume4", pDepthMarketData->AskVolume4);
		addDouble(buf, CHAR_BUF_SIZE, ev, "bidPrice5", pDepthMarketData->BidPrice5);
		addInt(buf, CHAR_BUF_SIZE, ev, "bidVolume5", pDepthMarketData->BidVolume5);
		addDouble(buf, CHAR_BUF_SIZE, ev, "askPrice5", pDepthMarketData->AskPrice5);
		addInt(buf, CHAR_BUF_SIZE, ev, "askVolume5", pDepthMarketData->AskVolume5);
		addDouble(buf, CHAR_BUF_SIZE, ev, "averagePrice", pDepthMarketData->AveragePrice);
	}
	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	m_pTransport->decode(ev, "OnRspQryDepthMarketData");
	delete[] buf;
	buf = NULL;	
}

void OMSCallback::OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	AP_LogInfo("OnRspQrySettlementInfo");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pSettlementInfo != NULL) {	
		addString(ev, "tradingDay", pSettlementInfo->TradingDay);
		addInt(buf, CHAR_BUF_SIZE, ev, "settlementId", pSettlementInfo->SettlementID);
		addString(ev, "brokerId", pSettlementInfo->BrokerID);
		addString(ev, "investorId", pSettlementInfo->InvestorID);
		addInt(buf, CHAR_BUF_SIZE, ev, "sequenceNo", pSettlementInfo->SequenceNo);
		addString(ev, "content", pSettlementInfo->Content);
	}
	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	m_pTransport->decode(ev, "OnRspQrySettlementInfo");
	delete[] buf;
	buf = NULL;	
}

void OMSCallback::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	AP_LogInfo("OnRspQryInvestorPositionDetail");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pInvestorPositionDetail != NULL) {	
		addString(ev, "instrumentId", pInvestorPositionDetail->InstrumentID);
		addString(ev, "brokerId", pInvestorPositionDetail->BrokerID);
		addString(ev, "investorId", pInvestorPositionDetail->InvestorID);
		addChar(buf, CHAR_BUF_SIZE, ev, "hedgeFlag", pInvestorPositionDetail->HedgeFlag);
		addChar(buf, CHAR_BUF_SIZE, ev, "direction", pInvestorPositionDetail->Direction);
		addString(ev, "openDate", pInvestorPositionDetail->OpenDate);
		addString(ev, "tradeId", pInvestorPositionDetail->TradeID);
		addInt(buf, CHAR_BUF_SIZE, ev, "volume", pInvestorPositionDetail->Volume);
		addDouble(buf, CHAR_BUF_SIZE, ev, "openPrice", pInvestorPositionDetail->OpenPrice);
		addString(ev, "tradingDay", pInvestorPositionDetail->TradingDay);
		addInt(buf, CHAR_BUF_SIZE, ev, "settlementId", pInvestorPositionDetail->SettlementID);
		addChar(buf, CHAR_BUF_SIZE, ev, "tradeType", pInvestorPositionDetail->TradeType);
		addString(ev, "combInstrumentId", pInvestorPositionDetail->CombInstrumentID);
		addString(ev, "exchangeId", pInvestorPositionDetail->ExchangeID);
		addDouble(buf, CHAR_BUF_SIZE, ev, "closeProfitByDate", pInvestorPositionDetail->CloseProfitByDate);
		addDouble(buf, CHAR_BUF_SIZE, ev, "closeProfitByTrade", pInvestorPositionDetail->CloseProfitByTrade);
		addDouble(buf, CHAR_BUF_SIZE, ev, "positionProfitByDate", pInvestorPositionDetail->PositionProfitByDate);
		addDouble(buf, CHAR_BUF_SIZE, ev, "positionProfitByTrade", pInvestorPositionDetail->PositionProfitByTrade);
		addDouble(buf, CHAR_BUF_SIZE, ev, "margin", pInvestorPositionDetail->Margin);
		addDouble(buf, CHAR_BUF_SIZE, ev, "exchMargin", pInvestorPositionDetail->ExchMargin);
		addDouble(buf, CHAR_BUF_SIZE, ev, "marginRateByMoney", pInvestorPositionDetail->MarginRateByMoney);
		addDouble(buf, CHAR_BUF_SIZE, ev, "marginRateByVolume", pInvestorPositionDetail->MarginRateByVolume);
		addDouble(buf, CHAR_BUF_SIZE, ev, "lastSettlementPrice", pInvestorPositionDetail->LastSettlementPrice);
		addDouble(buf, CHAR_BUF_SIZE, ev, "settlementPrice", pInvestorPositionDetail->SettlementPrice);
		addDouble(buf, CHAR_BUF_SIZE, ev, "closeVolume", pInvestorPositionDetail->CloseVolume);
		addDouble(buf, CHAR_BUF_SIZE, ev, "closeAmount", pInvestorPositionDetail->CloseAmount);
	}
	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	m_pTransport->decode(ev, "OnRspQryInvestorPositionDetail");
	delete[] buf;
	buf = NULL;	
}

void OMSCallback::OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	AP_LogInfo("OnRspQrySettlementInfoConfirm");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pSettlementInfoConfirm != NULL) {	
		addString(ev, "brokerId", pSettlementInfoConfirm->BrokerID);
		addString(ev, "investorId", pSettlementInfoConfirm->InvestorID);
		addString(ev, "confirmDate", pSettlementInfoConfirm->ConfirmDate);
		addString(ev, "confirmTime", pSettlementInfoConfirm->ConfirmTime);
	}
	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	m_pTransport->decode(ev, "OnRspQrySettlementInfoConfirm");
	delete[] buf;
	buf = NULL;	
}

void OMSCallback::OnRspQryInvestorPositionCombineDetail(CThostFtdcInvestorPositionCombineDetailField *pInvestorPositionCombineDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	AP_LogInfo("OnRspQryInvestorPositionCombineDetail");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pInvestorPositionCombineDetail != NULL) {	
		addString(ev, "tradingDay", pInvestorPositionCombineDetail->TradingDay);
		addString(ev, "openDate", pInvestorPositionCombineDetail->OpenDate);
		addString(ev, "exchangeId", pInvestorPositionCombineDetail->ExchangeID);
		addInt(buf, CHAR_BUF_SIZE, ev, "settlementId", pInvestorPositionCombineDetail->SettlementID);
		addString(ev, "brokerId", pInvestorPositionCombineDetail->BrokerID);
		addString(ev, "investorId", pInvestorPositionCombineDetail->InvestorID);
		addString(ev, "comTradeId", pInvestorPositionCombineDetail->ComTradeID);
		addString(ev, "tradeId", pInvestorPositionCombineDetail->TradeID);
		addString(ev, "instrumentId", pInvestorPositionCombineDetail->InstrumentID);
		addChar(buf, CHAR_BUF_SIZE, ev, "hedgeFlag", pInvestorPositionCombineDetail->HedgeFlag);
		addChar(buf, CHAR_BUF_SIZE, ev, "direction", pInvestorPositionCombineDetail->Direction);
		addInt(buf, CHAR_BUF_SIZE, ev, "totalAmt", pInvestorPositionCombineDetail->TotalAmt);
		addDouble(buf, CHAR_BUF_SIZE, ev, "margin", pInvestorPositionCombineDetail->Margin);
		addDouble(buf, CHAR_BUF_SIZE, ev, "exchMargin", pInvestorPositionCombineDetail->ExchMargin);
		addDouble(buf, CHAR_BUF_SIZE, ev, "marginRateByMoney", pInvestorPositionCombineDetail->MarginRateByMoney);
		addDouble(buf, CHAR_BUF_SIZE, ev, "marginRateByVolume", pInvestorPositionCombineDetail->MarginRateByVolume);
		addInt(buf, CHAR_BUF_SIZE, ev, "legId", pInvestorPositionCombineDetail->LegID);
		addInt(buf, CHAR_BUF_SIZE, ev, "legMultiple", pInvestorPositionCombineDetail->LegMultiple);
		addString(ev, "combInstrumentId", pInvestorPositionCombineDetail->CombInstrumentID);
	}
	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	m_pTransport->decode(ev, "OnRspQryInvestorPositionCombineDetail");
	delete[] buf;
	buf = NULL;	
}

void OMSCallback::OnRspQryEWarrantOffset(CThostFtdcEWarrantOffsetField *pEWarrantOffset, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	AP_LogInfo("OnRspQryEWarrantOffset");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pEWarrantOffset != NULL) {	
		addString(ev, "tradingDay", pEWarrantOffset->TradingDay);
		addString(ev, "brokerId", pEWarrantOffset->BrokerID);
		addString(ev, "investorId", pEWarrantOffset->InvestorID);
		addString(ev, "exchangeId", pEWarrantOffset->ExchangeID);
		addString(ev, "instrumentId", pEWarrantOffset->InstrumentID);
		addChar(buf, CHAR_BUF_SIZE, ev, "direction", pEWarrantOffset->Direction);
		addChar(buf, CHAR_BUF_SIZE, ev, "hedgeFlag", pEWarrantOffset->HedgeFlag);
		addInt(buf, CHAR_BUF_SIZE, ev, "volume", pEWarrantOffset->Volume);
	}
	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	m_pTransport->decode(ev, "OnRspQryEWarrantOffset");
	delete[] buf;
	buf = NULL;	
}

void OMSCallback::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	AP_LogInfo("OnRspError");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pRspInfo != NULL) {	
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	m_pTransport->decode(ev, "OnRspError");
	delete[] buf;
	buf = NULL;	
}

void OMSCallback::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
	AP_LogInfo("OnRtnOrder");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	if (pOrder != NULL) {
		addInt(buf, CHAR_BUF_SIZE, ev, "requestId", pOrder->RequestID);
		addString(ev, "brokerId", pOrder->BrokerID);
		addString(ev, "investorId", pOrder->InvestorID);
		addString(ev, "instrumentId", pOrder->InstrumentID);
		addString(ev, "orderRef", pOrder->OrderRef);
		addString(ev, "userId", pOrder->UserID);
		addChar(buf, CHAR_BUF_SIZE, ev, "orderPriceType", pOrder->OrderPriceType);
		addChar(buf, CHAR_BUF_SIZE, ev, "direction", pOrder->Direction);
		addString(ev, "combOffsetFlag", pOrder->CombOffsetFlag);
		addString(ev, "combHedgeFlag", pOrder->CombHedgeFlag);
		addDouble(buf, CHAR_BUF_SIZE, ev, "limitPrice", pOrder->LimitPrice);
		addInt(buf, CHAR_BUF_SIZE, ev, "volumeTotalOriginal", pOrder->VolumeTotalOriginal);
		addChar(buf, CHAR_BUF_SIZE, ev, "timeCondition", pOrder->TimeCondition);
		addString(ev, "gtdDate", pOrder->GTDDate);
		addChar(buf, CHAR_BUF_SIZE, ev, "volumeCondition", pOrder->VolumeCondition);
		addInt(buf, CHAR_BUF_SIZE, ev, "minVolume", pOrder->MinVolume);
		addChar(buf, CHAR_BUF_SIZE, ev, "contingentCondition", pOrder->ContingentCondition);
		addDouble(buf, CHAR_BUF_SIZE, ev, "stopPrice", pOrder->StopPrice);
		addChar(buf, CHAR_BUF_SIZE, ev, "forceCloseReason", pOrder->ForceCloseReason);
		addInt(buf, CHAR_BUF_SIZE, ev, "isAutoSuspend", pOrder->IsAutoSuspend);
		addString(ev, "businessUnit", pOrder->BusinessUnit);
		addString(ev, "orderLocalId", pOrder->OrderLocalID);
		addString(ev, "exchangeId", pOrder->ExchangeID);
		addString(ev, "participantId", pOrder->ParticipantID);
		addString(ev, "clientId", pOrder->ClientID);
		addString(ev, "exchangeInstId", pOrder->ExchangeInstID);
		addString(ev, "traderId", pOrder->TraderID);
		addInt(buf, CHAR_BUF_SIZE, ev, "installId", pOrder->InstallID);
		addChar(buf, CHAR_BUF_SIZE, ev, "orderSubmitStatus", pOrder->OrderSubmitStatus);
		addInt(buf, CHAR_BUF_SIZE, ev, "notifySequence", pOrder->NotifySequence);
		addString(ev, "tradingDay", pOrder->TradingDay);
		addInt(buf, CHAR_BUF_SIZE, ev, "settlementId", pOrder->SettlementID);
		addString(ev, "orderSysId", pOrder->OrderSysID);
		addChar(buf, CHAR_BUF_SIZE, ev, "orderSource", pOrder->OrderSource);
		addChar(buf, CHAR_BUF_SIZE, ev, "orderStatus", pOrder->OrderStatus);
		addChar(buf, CHAR_BUF_SIZE, ev, "orderType", pOrder->OrderType);
		addInt(buf, CHAR_BUF_SIZE, ev, "volumeTraded", pOrder->VolumeTraded);
		addInt(buf, CHAR_BUF_SIZE, ev, "volumeTotal", pOrder->VolumeTotal);
		addString(ev, "insertDate", pOrder->InsertDate);
		addString(ev, "insertTime", pOrder->InsertTime);
		addString(ev, "activeTime", pOrder->ActiveTime);
		addString(ev, "suspendTime", pOrder->SuspendTime);
		addString(ev, "updateTime", pOrder->UpdateTime);
		addString(ev, "cancelTime", pOrder->CancelTime);
		addString(ev, "activeTraderId", pOrder->ActiveTraderID);
		addString(ev, "clearingPartId", pOrder->ClearingPartID);
		addInt(buf, CHAR_BUF_SIZE, ev, "sequenceNo", pOrder->SequenceNo);
		addInt(buf, CHAR_BUF_SIZE, ev, "frontId", pOrder->FrontID);
		addInt(buf, CHAR_BUF_SIZE, ev, "sessionId", pOrder->SessionID);
		addString(ev, "userProductInfo", pOrder->UserProductInfo);
		addString(ev, "statusMsg", pOrder->StatusMsg);
		addInt(buf, CHAR_BUF_SIZE, ev, "userForceClose", pOrder->UserForceClose);
		addString(ev, "activeUserId", pOrder->ActiveUserID);
		addInt(buf, CHAR_BUF_SIZE, ev, "brokerOrderSeq", pOrder->BrokerOrderSeq);
		addString(ev, "relativeOrderSysId", pOrder->RelativeOrderSysID);
	}
	m_pTransport->decode(ev, "OnRtnOrder");
	delete[] buf;
	buf = NULL;	
}

void OMSCallback::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
	AP_LogInfo("OnRtnTrade");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	if (pTrade != NULL) {	
		addString(ev, "brokerId", pTrade->BrokerID);
		addString(ev, "investorId", pTrade->InvestorID);
		addString(ev, "instrumentId", pTrade->InstrumentID);
		addString(ev, "orderRef", pTrade->OrderRef);
		addString(ev, "userId", pTrade->UserID);
		addString(ev, "exchangeId", pTrade->ExchangeID);
		addString(ev, "tradeId", pTrade->TradeID);
		addChar(buf, CHAR_BUF_SIZE, ev, "direction", pTrade->Direction);
		addString(ev, "orderSysId", pTrade->OrderSysID);
		addString(ev, "participantId", pTrade->ParticipantID);
		addString(ev, "clientId", pTrade->ClientID);
		addChar(buf, CHAR_BUF_SIZE, ev, "tradingRole", pTrade->TradingRole);
		addString(ev, "exchangeInstId", pTrade->ExchangeInstID);
		addChar(buf, CHAR_BUF_SIZE, ev, "offsetFlag", pTrade->OffsetFlag);
		addChar(buf, CHAR_BUF_SIZE, ev, "hedgeFlag", pTrade->HedgeFlag);
		addDouble(buf, CHAR_BUF_SIZE, ev, "price", pTrade->Price);
		addInt(buf, CHAR_BUF_SIZE, ev, "volume", pTrade->Volume);
		addString(ev, "tradeDate", pTrade->TradeDate);
		addString(ev, "tradeTime", pTrade->TradeTime);
		addChar(buf, CHAR_BUF_SIZE, ev, "tradeType", pTrade->TradeType);
		addChar(buf, CHAR_BUF_SIZE, ev, "priceSource", pTrade->PriceSource);
		addString(ev, "traderId", pTrade->TraderID);
		addString(ev, "orderLocalId", pTrade->OrderLocalID);
		addString(ev, "clearingPartId", pTrade->ClearingPartID);
		addString(ev, "businessUnit", pTrade->BusinessUnit);
		addInt(buf, CHAR_BUF_SIZE, ev, "sequenceNo", pTrade->SequenceNo);
		addString(ev, "tradingDay", pTrade->TradingDay);
		addInt(buf, CHAR_BUF_SIZE, ev, "settlementId", pTrade->SettlementID);
		addInt(buf, CHAR_BUF_SIZE, ev, "brokerOrderSeq", pTrade->BrokerOrderSeq);
		addChar(buf, CHAR_BUF_SIZE, ev, "tradeSource", pTrade->TradeSource);
	}
	m_pTransport->decode(ev, "OnRtnTrade");
	delete[] buf;
	buf = NULL;	
}

void OMSCallback::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo)
{
	AP_LogInfo("OnErrRtnOrderInsert");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	if (pInputOrder != NULL) {
		addInt(buf, CHAR_BUF_SIZE, ev, "requestId", pInputOrder->RequestID);
		addString(ev, "brokerId", pInputOrder->BrokerID);
		addString(ev, "investorId", pInputOrder->InvestorID);
		addString(ev, "instrumentId", pInputOrder->InstrumentID);
		addString(ev, "orderRef", pInputOrder->OrderRef);
		addString(ev, "userId", pInputOrder->UserID);
		addChar(buf, CHAR_BUF_SIZE, ev, "orderPriceType", pInputOrder->OrderPriceType);
		addChar(buf, CHAR_BUF_SIZE, ev, "direction", pInputOrder->Direction);
		addString(ev, "combOffsetFlag", pInputOrder->CombOffsetFlag);
		addString(ev, "combHedgeFlag", pInputOrder->CombHedgeFlag);
		addDouble(buf, CHAR_BUF_SIZE, ev, "limitPrice", pInputOrder->LimitPrice);
		addInt(buf, CHAR_BUF_SIZE, ev, "volumeTotalOriginal", pInputOrder->VolumeTotalOriginal);
		addChar(buf, CHAR_BUF_SIZE, ev, "timeCondition", pInputOrder->TimeCondition);
		addString(ev, "gtdDate", pInputOrder->GTDDate);
		addChar(buf, CHAR_BUF_SIZE, ev, "volumeCondition", pInputOrder->VolumeCondition);
		addInt(buf, CHAR_BUF_SIZE, ev, "minVolume", pInputOrder->MinVolume);
		addChar(buf, CHAR_BUF_SIZE, ev, "contingentCondition", pInputOrder->ContingentCondition);
		addDouble(buf, CHAR_BUF_SIZE, ev, "stopPrice", pInputOrder->StopPrice);
		addChar(buf, CHAR_BUF_SIZE, ev, "forceCloseReason", pInputOrder->ForceCloseReason);
		addInt(buf, CHAR_BUF_SIZE, ev, "isAutoSuspend", pInputOrder->IsAutoSuspend);
		addString(ev, "businessUnit", pInputOrder->BusinessUnit);
		addInt(buf, CHAR_BUF_SIZE, ev, "userForceClose", pInputOrder->UserForceClose);
	}
	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);
	}
	m_pTransport->decode(ev, "OnErrRtnOrderInsert");
	delete[] buf;
	buf = NULL;	
}

void OMSCallback::OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo)
{
	AP_LogInfo("OnErrRtnOrderInsert");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	if (pOrderAction != NULL) {
		addInt(buf, CHAR_BUF_SIZE, ev, "requestId", pOrderAction->RequestID);
		addString(ev, "brokerId", pOrderAction->BrokerID);
		addString(ev, "investorId", pOrderAction->InvestorID);
		addInt(buf, CHAR_BUF_SIZE, ev, "orderActionRef", pOrderAction->OrderActionRef);
		addString(ev, "orderRef", pOrderAction->OrderRef);
		addInt(buf, CHAR_BUF_SIZE, ev, "frontId", pOrderAction->FrontID);
		addInt(buf, CHAR_BUF_SIZE, ev, "sessionId", pOrderAction->SessionID);
		addString(ev, "exchangeId", pOrderAction->ExchangeID);
		addString(ev, "orderSysId", pOrderAction->OrderSysID);
		addChar(buf, CHAR_BUF_SIZE, ev, "actionFlag", pOrderAction->ActionFlag);
		addDouble(buf, CHAR_BUF_SIZE, ev, "limitPrice", pOrderAction->LimitPrice);
		addInt(buf, CHAR_BUF_SIZE, ev, "volumeChange", pOrderAction->VolumeChange);
		addString(ev, "actionDate", pOrderAction->ActionDate);
		addString(ev, "actionTime", pOrderAction->ActionTime);
		addString(ev, "traderId", pOrderAction->TraderID);
		addInt(buf, CHAR_BUF_SIZE, ev, "installId", pOrderAction->InstallID);
		addString(ev, "orderLocalId", pOrderAction->OrderLocalID);	
		addString(ev, "actionLocalId", pOrderAction->ActionLocalID);
		addString(ev, "participantId", pOrderAction->ParticipantID);
		addString(ev, "clientId", pOrderAction->ClientID);	
		addString(ev, "businessUnit", pOrderAction->BusinessUnit);
		addChar(buf, CHAR_BUF_SIZE, ev, "orderActionStatus", pOrderAction->OrderActionStatus);
		addString(ev, "userId", pOrderAction->UserID);
		addString(ev, "statusMsg", pOrderAction->StatusMsg);	
		addString(ev, "instrumentId", pOrderAction->InstrumentID);
	}
	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);
	}
	m_pTransport->decode(ev, "OnErrRtnOrderInsert");
	delete[] buf;
	buf = NULL;	
}

void OMSCallback::OnRtnInstrumentStatus(CThostFtdcInstrumentStatusField *pInstrumentStatus)
{
	AP_LogInfo("OnRtnInstrumentStatus");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	if (pInstrumentStatus != NULL) {
		addString(ev, "exchangeId", pInstrumentStatus->ExchangeID);
		addString(ev, "exchangeInstId", pInstrumentStatus->ExchangeInstID);
		addString(ev, "settlementGroupId", pInstrumentStatus->SettlementGroupID);
		addString(ev, "instrumentId", pInstrumentStatus->InstrumentID);
		addChar(buf, CHAR_BUF_SIZE, ev, "instrumentStatus", pInstrumentStatus->InstrumentStatus);
		addInt(buf, CHAR_BUF_SIZE, ev, "tradingSegmentSN", pInstrumentStatus->TradingSegmentSN);
		addString(ev, "enterTime", pInstrumentStatus->EnterTime);
		addChar(buf, CHAR_BUF_SIZE, ev, "enterReason", pInstrumentStatus->EnterReason);
	}
	m_pTransport->decode(ev, "OnRtnInstrumentStatus");
	delete[] buf;
	buf = NULL;	
}

void OMSCallback::OnRtnTradingNotice(CThostFtdcTradingNoticeInfoField *pTradingNoticeInfo)
{
	AP_LogInfo("OnRtnTradingNotice");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	if (pTradingNoticeInfo != NULL) {
		addString(ev, "brokerId", pTradingNoticeInfo->BrokerID);
		addString(ev, "investorId", pTradingNoticeInfo->InvestorID);
		addString(ev, "sendTime", pTradingNoticeInfo->SendTime);
		addString(ev, "fieldContent", pTradingNoticeInfo->FieldContent);
		addInt(buf, CHAR_BUF_SIZE, ev, "sequenceSeries", pTradingNoticeInfo->SequenceSeries);
		addInt(buf, CHAR_BUF_SIZE, ev, "sequenceNo", pTradingNoticeInfo->SequenceNo);
	}
	m_pTransport->decode(ev, "OnRtnTradingNotice");
	delete[] buf;
	buf = NULL;	
}

void OMSCallback::OnRspQryParkedOrder(CThostFtdcParkedOrderField *pParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	AP_LogInfo("OnRspQryParkedOrder");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pParkedOrder != NULL) {	
		addString(ev, "brokerId", pParkedOrder->BrokerID);
		addString(ev, "investorId", pParkedOrder->InvestorID);
		addString(ev, "instrumentId", pParkedOrder->InstrumentID);
		addString(ev, "orderRef", pParkedOrder->OrderRef);
		addString(ev, "userId", pParkedOrder->UserID);
		addChar(buf, CHAR_BUF_SIZE, ev, "orderPriceType", pParkedOrder->OrderPriceType);
		addChar(buf, CHAR_BUF_SIZE, ev, "direction", pParkedOrder->Direction);
		addString(ev, "combOffsetFlag", pParkedOrder->CombOffsetFlag);
		addString(ev, "combHedgeFlag", pParkedOrder->CombHedgeFlag);
		addDouble(buf, CHAR_BUF_SIZE, ev, "limitPrice", pParkedOrder->LimitPrice);
		addInt(buf, CHAR_BUF_SIZE, ev, "volumeTotalOriginal", pParkedOrder->VolumeTotalOriginal);
		addChar(buf, CHAR_BUF_SIZE, ev, "timeCondition", pParkedOrder->TimeCondition);
		addString(ev, "gtdDate", pParkedOrder->GTDDate);
		addChar(buf, CHAR_BUF_SIZE, ev, "volumeCondition", pParkedOrder->VolumeCondition);
		addInt(buf, CHAR_BUF_SIZE, ev, "minVolume", pParkedOrder->MinVolume);
		addChar(buf, CHAR_BUF_SIZE, ev, "contingentCondition", pParkedOrder->ContingentCondition);
		addDouble(buf, CHAR_BUF_SIZE, ev, "stopPrice", pParkedOrder->StopPrice);
		addChar(buf, CHAR_BUF_SIZE, ev, "forceCloseReason", pParkedOrder->ForceCloseReason);
		addInt(buf, CHAR_BUF_SIZE, ev, "isAutoSuspend", pParkedOrder->IsAutoSuspend);
		addString(ev, "businessUnit", pParkedOrder->BusinessUnit);
		addInt(buf, CHAR_BUF_SIZE, ev, "userForceClose", pParkedOrder->UserForceClose);	
		addString(ev, "exchangeId", pParkedOrder->ExchangeID);
		addString(ev, "parkedOrderId", pParkedOrder->ParkedOrderID);
		addChar(buf, CHAR_BUF_SIZE, ev, "userType", pParkedOrder->UserType);
		addChar(buf, CHAR_BUF_SIZE, ev, "status", pParkedOrder->Status);
	}
	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	m_pTransport->decode(ev, "OnRspQryParkedOrder");
	delete[] buf;
	buf = NULL;	
}

void OMSCallback::OnRspQryParkedOrderAction(CThostFtdcParkedOrderActionField *pParkedOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	AP_LogInfo("OnRspQryParkedOrderAction");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pParkedOrderAction != NULL) {	
		addString(ev, "brokerId", pParkedOrderAction->BrokerID);
		addString(ev, "investorId", pParkedOrderAction->InvestorID);
		addInt(buf, CHAR_BUF_SIZE, ev, "orderActionRef", pParkedOrderAction->OrderActionRef);
		addString(ev, "orderRef", pParkedOrderAction->OrderRef);
		addInt(buf, CHAR_BUF_SIZE, ev, "frontId", pParkedOrderAction->FrontID);
		addInt(buf, CHAR_BUF_SIZE, ev, "sessionId", pParkedOrderAction->SessionID);
		addString(ev, "exchangeId", pParkedOrderAction->ExchangeID);
		addString(ev, "orderSysId", pParkedOrderAction->OrderSysID);
		addChar(buf, CHAR_BUF_SIZE, ev, "actionFlag", pParkedOrderAction->ActionFlag);
		addDouble(buf, CHAR_BUF_SIZE, ev, "limitPrice", pParkedOrderAction->LimitPrice);
		addInt(buf, CHAR_BUF_SIZE, ev, "volumeChange", pParkedOrderAction->VolumeChange);
		addString(ev, "userId", pParkedOrderAction->UserID);
		addString(ev, "instrumentId", pParkedOrderAction->InstrumentID);
		addString(ev, "parkedOrderActionId", pParkedOrderAction->ParkedOrderActionID);
		addChar(buf, CHAR_BUF_SIZE, ev, "userType", pParkedOrderAction->UserType);
		addChar(buf, CHAR_BUF_SIZE, ev, "status", pParkedOrderAction->Status);	
	}
	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	m_pTransport->decode(ev, "OnRspQryParkedOrderAction");
	delete[] buf;
	buf = NULL;	
}

void OMSCallback::OnRspQryTradingNotice(CThostFtdcTradingNoticeField *pTradingNotice, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	AP_LogInfo("OnRspQryTradingNotice");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pTradingNotice != NULL) {	
		addString(ev, "brokerId", pTradingNotice->BrokerID);
		addChar(buf, CHAR_BUF_SIZE, ev, "investorRange", pTradingNotice->InvestorRange);
		addString(ev, "investorId", pTradingNotice->InvestorID);
		addInt(buf, CHAR_BUF_SIZE, ev, "sequenceSeries", pTradingNotice->SequenceSeries);
		addString(ev, "userId", pTradingNotice->UserID);
		addString(ev, "sendTime", pTradingNotice->SendTime);
		addInt(buf, CHAR_BUF_SIZE, ev, "sequenceNo", pTradingNotice->SequenceNo);
		addString(ev, "fieldContent", pTradingNotice->FieldContent);
	}
	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	m_pTransport->decode(ev, "OnRspQryTradingNotice");
	delete[] buf;
	buf = NULL;	
}

void OMSCallback::OnRspQryBrokerTradingParams(CThostFtdcBrokerTradingParamsField *pBrokerTradingParams, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	AP_LogInfo("OnRspQryBrokerTradingParams");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pBrokerTradingParams != NULL) {	
		addChar(buf, CHAR_BUF_SIZE, ev, "algorithm", pBrokerTradingParams->Algorithm);
		addDouble(buf, CHAR_BUF_SIZE, ev, "availIncludeCloseProfit", pBrokerTradingParams->AvailIncludeCloseProfit);
		addString(ev, "brokerId", pBrokerTradingParams->BrokerID);
		addString(ev, "investorId", pBrokerTradingParams->InvestorID);
		addChar(buf, CHAR_BUF_SIZE, ev, "marginPriceType", pBrokerTradingParams->MarginPriceType);
	}
	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	m_pTransport->decode(ev, "OnRspQryBrokerTradingParams");
	delete[] buf;
	buf = NULL;	
}

void OMSCallback::OnRspQryBrokerTradingAlgos(CThostFtdcBrokerTradingAlgosField *pBrokerTradingAlgos, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	AP_LogInfo("OnRspQryBrokerTradingAlgos");
	AP_NormalisedEvent* ev = AP_NormalisedEvent_ctor();
	char* buf = new char[CHAR_BUF_SIZE];
	addInt(buf, CHAR_BUF_SIZE, ev, "requestId", nRequestID);
	if (pBrokerTradingAlgos != NULL) {	
		addString(ev, "brokerId", pBrokerTradingAlgos->BrokerID);
		addString(ev, "exchangeId", pBrokerTradingAlgos->ExchangeID);
		addChar(buf, CHAR_BUF_SIZE, ev, "findMarginRateAlgoId", pBrokerTradingAlgos->FindMarginRateAlgoID);
		addChar(buf, CHAR_BUF_SIZE, ev, "handlePositionAlgoId", pBrokerTradingAlgos->HandlePositionAlgoID);
		addChar(buf, CHAR_BUF_SIZE, ev, "handleTradingAccountAlgoId", pBrokerTradingAlgos->HandleTradingAccountAlgoID);
		addString(ev, "instrumentId", pBrokerTradingAlgos->InstrumentID);
	}
	if(pRspInfo != NULL){
		addInt(buf, CHAR_BUF_SIZE, ev, "errorId", pRspInfo->ErrorID);
		addString(ev, "errorMsg", pRspInfo->ErrorMsg);
	}
	addString(ev, "isLast", bIsLast ? "true": "false");
	m_pTransport->decode(ev, "OnRspQryBrokerTradingAlgos");
	delete[] buf;
	buf = NULL;	
}
