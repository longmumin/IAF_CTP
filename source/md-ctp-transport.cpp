#include "md-ctp-transport.h"

#define STATUS_BUFSIZE 256
#define CHAR_BUF_SIZE 128

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

MD_Transport::MD_Transport(const char* _name, AP_EventTransportProperties* properties, AP_EventTransportError* err, IAF_TimestampConfig* timestampConfig)
{
	mRunStatus.status = (AP_char8*)malloc(STATUS_BUFSIZE);
	m_pErrMsg = new char[STATUS_BUFSIZE];
	mRunStatus.totalReceived = 0;
	mRunStatus.totalSent = 0;
	m_pLastError = NULL;
	m_pTimestampConfig = NULL;
	m_pDecoder = NULL;
	m_pMdCallback = NULL;
	mLoginHandle = NULL;
	m_pMdApi = NULL;
	if ((*err = updateProperties(properties, timestampConfig))){
		char* errMsg = _strdup(m_pLastError);
		AP_LogError(errMsg);
	}
}

MD_Transport::~MD_Transport() 
{	
	clearError();
	delete[]m_pErrMsg;
	m_pErrMsg = NULL;
	if(mLoginHandle != NULL){
		CloseHandle(mLoginHandle);
		mLoginHandle = NULL;
	}
	if(m_pMdCallback != NULL){
		delete m_pMdCallback;
		m_pMdCallback = NULL;
	}
}


void MD_Transport::clearError() 
{
	if(m_pLastError != NULL){
		free(m_pLastError);
		m_pLastError = NULL;
	}
}


void MD_Transport::setError(const AP_char8* msg) 
{
	clearError();
	m_pLastError = _strdup(msg);
}

AP_EventTransportError MD_Transport::updateProperties(AP_EventTransportProperties* properties, IAF_TimestampConfig* timestampConfig) 
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


AP_EventTransportError MD_Transport::sendTransportEvent(AP_TransportEvent tevent, AP_TimestampSet* tss) 
{
	AP_NormalisedEvent *pEve = (AP_NormalisedEvent*) tevent;
	const char* reqType = pEve->functions->findValue(pEve, "Type");
	bool rtn = false;
	if(reqType){
		if(strcmp(reqType, "ReqUserLogin") == 0){	//发送登录CTP交易平台请求
			rtn = ReqUserLogin(pEve);
			mRunStatus.totalSent++;

		}else if(strcmp(reqType, "ReqUserLogout") == 0){	//发送登出CTP交易平台请求
			rtn = ReqUserLogout(pEve);
			mRunStatus.totalSent++;

		}else if (strcmp(reqType, "SubscribeMarketData") == 0){	//发送订阅行情数据请求
			rtn = SubscribeMarketData(pEve);
			mRunStatus.totalSent++;

		}else if (strcmp(reqType, "UnSubscribeMarketData") == 0){		//发送取消订阅行情数据请求
			rtn = UnSubscribeMarketData(pEve);
			mRunStatus.totalSent++;

		}else {							//无效类型
			AP_LogError("Invalid Type: %s", reqType);
			rtn = false;
		}
	} else {
		AP_LogError("Missing Type.");
		rtn = false;
	}
	AP_NormalisedEvent_dtor(pEve);
	AP_TimestampSet_dtor(tss);
	return rtn ? AP_EventTransport_OK : AP_EventTransport_TransportFailure;
}


void MD_Transport::addEventDecoder(const AP_char8* name, AP_EventDecoder* _decoder) 
{
	m_pDecoder = _decoder;
}

void MD_Transport::removeEventDecoder(const AP_char8* name)
{
	m_pDecoder = NULL;
}

AP_EventTransportError MD_Transport::flushUpstream() 
{
	return AP_EventTransport_OK;
}

AP_EventTransportError MD_Transport::flushDownstream() 
{
	return AP_EventTransport_OK;
}

bool MD_Transport::ReqUserLogin(AP_NormalisedEvent* ev)
{	
	AP_LogInfo("[ReqUserLogin] UserId: %s, Psw: ******", getString(ev, "userId"));
	try{
		CThostFtdcReqUserLoginField req;
		memset(&req, 0, sizeof(req));
		strncpy(req.BrokerID, m_szBroker.c_str(), sizeof(req.BrokerID));
		strncpy(req.UserID, getString(ev, "userId"), sizeof(req.UserID));
		strncpy(req.Password, getString(ev, "password"), sizeof(req.Password));	
		int iResult = m_pMdApi->ReqUserLogin(&req, getInt(ev, "requestId"));
		if (iResult != 0){
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

bool MD_Transport::ReqUserLogout(AP_NormalisedEvent* ev)
{	
	AP_LogInfo("[ReqUserLogout] %s", ev->functions->toString(ev));
	try{
		CThostFtdcUserLogoutField req;
		memset(&req, 0, sizeof(req));
		strncpy(req.BrokerID, m_szBroker.c_str(), sizeof(req.BrokerID));
		strncpy(req.UserID, getString(ev, "userId"), sizeof(req.UserID));
		int iResult = m_pMdApi->ReqUserLogout(&req, getInt(ev, "requestId"));
		if (iResult != 0){
			AP_LogWarn("[ReqUserLogout] Fail to send request of logout.");
			return false;
		}

	}catch(...){
		AP_LogError("[ReqUserLogout] exception when request.");
		return false;
	}
	return true;
}

bool MD_Transport::SubscribeMarketData(AP_NormalisedEvent* ev)
{	
	AP_LogInfo("[SubscribeMarketData] %s", ev->functions->toString(ev));
	try{
		char* pInstrument = new char[CHAR_BUF_SIZE];
		char* ppInstrumentID[] = {pInstrument};
		strcpy_s(pInstrument, CHAR_BUF_SIZE, getString(ev, "instrumentId"));
		int iResult = m_pMdApi->SubscribeMarketData(ppInstrumentID, 1);
		if (iResult != 0){
			AP_LogWarn("[SubscribeMarketData] Fail to send request of subscribing %s.", pInstrument);
			delete[] pInstrument;
			pInstrument = NULL;
			return false;
		}
		delete[] pInstrument;
		pInstrument = NULL;

	}catch(...){
		AP_LogError("[SubscribeMarketData] exception when request.");
		return false;
	}
	return true;
}

bool MD_Transport::UnSubscribeMarketData(AP_NormalisedEvent* ev)
{	
	AP_LogInfo("[UnSubscribeMarketData] %s", ev->functions->toString(ev));
	try{
		char* pInstrument = new char[CHAR_BUF_SIZE];
		char* ppInstrumentID[] = {pInstrument};
		strcpy_s(pInstrument, CHAR_BUF_SIZE, getString(ev, "instrumentId"));
		int iResult = m_pMdApi->UnSubscribeMarketData(ppInstrumentID, 1);
		if (iResult != 0){
			AP_LogWarn("[UnSubscribeMarketData] Fail to send request of unSubscribing %s.", pInstrument);
			delete[] pInstrument;
			pInstrument = NULL;
			return false;
		}
		delete[] pInstrument;
		pInstrument = NULL;

	}catch(...){
		AP_LogError("[UnSubscribeMarketData] exception when request.");
		return false;
	}
	return true;
}

bool MD_Transport::RegisterCTP()
{
	try{	
		m_pMdApi = CThostFtdcMdApi::CreateFtdcMdApi();
		m_pMdApi->RegisterFront(const_cast<char*>(m_szIP.c_str()));
		m_pMdCallback = new MdCallback(m_pDecoder, mLoginHandle);
		m_pMdApi->RegisterSpi(m_pMdCallback);
		m_pMdApi->Init();
	}
	catch (...){
		AP_LogError("Error when Register Market CTP.");
		return false;
	}
	return true;
}

AP_EventTransportError MD_Transport::start() 
{	
	if(mLoginHandle == NULL)
		mLoginHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
	bool bRet = RegisterCTP();
	if(!bRet){	
		if(mLoginHandle != NULL){
			CloseHandle(mLoginHandle);
			mLoginHandle = NULL;
		}
		return AP_EventTransport_TransportFailure;
	}
	WaitForSingleObject(mLoginHandle, INFINITE);
	AP_LogInfo("Success to start md.");
	return AP_EventTransport_OK;
}


AP_EventTransportError MD_Transport::stop() 
{	
	try{
		if (m_pMdApi != NULL){
			m_pMdApi->Release();
			m_pMdApi = NULL;
		}
	}
	catch (...){
		AP_LogError("Fail to stop md.");
		return AP_EventTransport_InternalError;
	}
	return AP_EventTransport_OK;
}

const AP_char8* MD_Transport::getLastError() 
{
	return m_pLastError;
}


void MD_Transport::getStatus(AP_EventTransportStatus* statusp) 
{
	statusp->status = mRunStatus.status;
	statusp->totalReceived = mRunStatus.totalReceived;
	statusp->totalSent = mRunStatus.totalSent;
	statusp->statusDictionary->functions->addQuick(statusp->statusDictionary, "vesion", "1.0");
}

static AP_EventTransportError updateProperties(struct AP_EventTransport* transport, AP_EventTransportProperties* properties, IAF_TimestampConfig* timestampConfig) {
	MD_Transport* transportObject = (MD_Transport*)(transport->reserved);
	return transportObject->updateProperties(properties, timestampConfig);
}

static AP_EventTransportError sendTransportEvent(struct AP_EventTransport* transport, AP_TransportEvent tevent, AP_TimestampSet* tss) {
	MD_Transport* transportObject = (MD_Transport*)(transport->reserved);
	return transportObject->sendTransportEvent(tevent, tss);
}

static void addEventDecoder(struct AP_EventTransport* transport, const AP_char8* name, AP_EventDecoder* decoder) {
	MD_Transport* transportObject = (MD_Transport*)(transport->reserved);
	transportObject->addEventDecoder(name, decoder);
}

static void removeEventDecoder(struct AP_EventTransport* transport, const AP_char8* name) {
	MD_Transport* transportObject = (MD_Transport*)(transport->reserved);
	transportObject->removeEventDecoder(name);
}

static AP_EventTransportError flushUpstream(struct AP_EventTransport* transport) {
	MD_Transport* transportObject = (MD_Transport*)(transport->reserved);
	return transportObject->flushUpstream();
}

static AP_EventTransportError flushDownstream(struct AP_EventTransport* transport) {
	MD_Transport* transportObject = (MD_Transport*)(transport->reserved);
	return transportObject->flushDownstream();
}

static AP_EventTransportError start(struct AP_EventTransport* transport) {
	MD_Transport* transportObject = (MD_Transport*)(transport->reserved);
	return transportObject->start();
}

static AP_EventTransportError stop(struct AP_EventTransport* transport) {
	MD_Transport* transportObject = (MD_Transport*)(transport->reserved);
	return transportObject->stop();
}

static const AP_char8* getLastError(struct AP_EventTransport* transport) {
	MD_Transport* transportObject = (MD_Transport*)(transport->reserved);
	return transportObject->getLastError();
}

static void getStatus(struct AP_EventTransport* transport, AP_EventTransportStatus* status) {
	MD_Transport* transportObject = (MD_Transport*)(transport->reserved);
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
		MD_Transport* transportObject;
		transport = (AP_EventTransport*)malloc(sizeof(AP_EventTransport));

		try {
			transportObject = new MD_Transport(name, properties, err, timestampConfig);
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
		MD_Transport* transportObject = (MD_Transport*)(transport->reserved);
		delete transportObject;
		free(transport);
	}
}