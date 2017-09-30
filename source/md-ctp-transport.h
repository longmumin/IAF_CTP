#ifndef MD_TRANSPORT_H
#define MD_TRANSPORT_H
#include "md-callback.h"
#include <string>

using namespace std;

class MD_Transport
{
public:
	MD_Transport(const char* _name, AP_EventTransportProperties* properties, AP_EventTransportError* err, IAF_TimestampConfig* timestampConfig);
	virtual ~MD_Transport();
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

private:
	bool RegisterCTP();
	bool ReqUserLogin(AP_NormalisedEvent* ev);
	bool ReqUserLogout(AP_NormalisedEvent* ev);
	bool SubscribeMarketData(AP_NormalisedEvent* ev);
	bool UnSubscribeMarketData(AP_NormalisedEvent* ev);

private:

	IAF_TimestampConfig* m_pTimestampConfig;
	AP_EventTransportStatus	mRunStatus; 
	CThostFtdcMdApi* m_pMdApi;
	AP_char8* m_pLastError; 
	AP_EventDecoder* m_pDecoder; 
	MdCallback* m_pMdCallback;
	HANDLE mLoginHandle;
	string m_szIP;
	string m_szBroker;
	int m_nErrcode;
	char* m_pErrMsg;
};

#endif