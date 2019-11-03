#ifndef _HS_TRADE_CALLBACK_H_
#define _HS_TRADE_CALLBACK_H_

#include <string>

#include <t2sdk_interface.h>

#include "cJSON.h"

#include "hstrade_private.h"


typedef void(*on_recv_msg_pt)(void* clientd, int func_id, int issue_type, const std::string& msg);

class TradeCallback : public CCallbackInterface
{
public:
    TradeCallback();
    ~TradeCallback();

    // ��ΪCCallbackInterface�����մ��������IKnown��������Ҫʵ��һ����3������
    unsigned long  FUNCTION_CALL_MODE QueryInterface(const char *iid, IKnown **ppv);
    unsigned long  FUNCTION_CALL_MODE AddRef();
    unsigned long  FUNCTION_CALL_MODE Release();

    // �����¼�����ʱ�Ļص�������ʵ��ʹ��ʱ���Ը�����Ҫ��ѡ��ʵ�֣����ڲ���Ҫ���¼��ص���������ֱ��return
    // Reserved?Ϊ����������Ϊ�Ժ���չ��׼����ʵ��ʱ��ֱ��return��return 0��
    void FUNCTION_CALL_MODE OnConnect(CConnectionInterface *lpConnection);
    void FUNCTION_CALL_MODE OnSafeConnect(CConnectionInterface *lpConnection);
    void FUNCTION_CALL_MODE OnRegister(CConnectionInterface *lpConnection);
    void FUNCTION_CALL_MODE OnClose(CConnectionInterface *lpConnection);
    void FUNCTION_CALL_MODE OnSent(CConnectionInterface *lpConnection, int hSend, void *reserved1, void *reserved2, int nQueuingData);
    void FUNCTION_CALL_MODE Reserved1(void *a, void *b, void *c, void *d);
    void FUNCTION_CALL_MODE Reserved2(void *a, void *b, void *c, void *d);
    int  FUNCTION_CALL_MODE Reserved3();
    void FUNCTION_CALL_MODE Reserved4();
    void FUNCTION_CALL_MODE Reserved5();
    void FUNCTION_CALL_MODE Reserved6();
    void FUNCTION_CALL_MODE Reserved7();
    void FUNCTION_CALL_MODE OnReceivedBiz(CConnectionInterface *lpConnection, int hSend, const void *lpUnPackerOrStr, int nResult);
    void FUNCTION_CALL_MODE OnReceivedBizEx(CConnectionInterface *lpConnection, int hSend, LPRET_DATA lpRetData, const void *lpUnpackerOrStr, int nResult);
    void FUNCTION_CALL_MODE OnReceivedBizMsg(CConnectionInterface *lpConnection, int hSend, IBizMessage* lpMsg);
    //int FUNCTION_CALL_MODE EncodeEx(const char *pIn, char *pOut);

public:
    static int UnpackBizMessage(IF2UnPacker* lpUnPacker, void* ctxdata, int(*data_proc_pt)(IF2UnPacker* lpUnPacker, int ds_index, void* ctxdata));

    static int UnpackLoginData(IF2UnPacker* lpUnPacker, int ds_index, void* ctxdata);
    static int UnpackRspOrderData(IF2UnPacker* lpUnPacker, int ds_index, void* ctxdata);
    static int UnpackRspOrderActionData(IF2UnPacker* lpUnPacker, int ds_index, void* ctxdata);
    static int UnpackTradingAccountData(IF2UnPacker* lpUnPacker, int ds_index, void* ctxdata);
    static int UnpackPositionData(IF2UnPacker* lpUnPacker, int ds_index, void* ctxdata);
    static int UnpackQryOrderData(IF2UnPacker* lpUnPacker, int ds_index, void* ctxdata);
    static int UnpackQryTradeData(IF2UnPacker* lpUnPacker, int ds_index, void* ctxdata);
    static int UnpackRtnOrderData(IF2UnPacker* lpUnPacker, int ds_index, void* ctxdata);
    static int UnpackRtnTradeData(IF2UnPacker* lpUnPacker, int ds_index, void* ctxdata);

    static void GetErrorField(HSRspInfoField* rsp_info, IF2UnPacker* lpUnPacker);

    static int GenJsonData(cJSON* json_data, IF2UnPacker* lpUnPacker);
    static cJSON* GenJsonDatas(IF2UnPacker* lpUnPacker, int func_id, int issue_type);

public:
    void SetContextData(hstrade_t* hstd);

    void SetJsonMode(int data_proto);

    int  IsJsonMode();

    int NotifyJsonData(cJSON* json, int func_id, int issue_type);

    // 331100 ����
    void OnResponseUserLogin(IF2UnPacker* lpUnPacker);
    // 332255 �ʽ��˻���ѯ
    void OnResponseQryTradingAccount(IF2UnPacker* lpUnPacker);
    // 333104 �ֲֲ�ѯ
    void OnResponseQryPosition(IF2UnPacker* lpUnPacker);
    // 333101 ί�в�ѯ
    void OnResponseQryOrder(IF2UnPacker* lpUnPacker);
    // 333102 �ɽ���ѯ
    void OnResponseQryTrade(IF2UnPacker* lpUnPacker);
    // 330300 ֤ȯ������Ϣ��ѯ
    void OnResponseQrySecurityInfo(IF2UnPacker* lpUnPacker);
    // 400 �����ѯ
    void OnResponseQryMD(IF2UnPacker* lpUnPacker);
    // 620001 ���Ļر�Ӧ��
    void OnResponseSubscribe(IF2UnPacker* lpUnPacker);

    // ί��Ӧ��
    void OnResponseOrderInsert(IF2UnPacker* lpUnPacker);
    void OnResponseOrderCancel(IF2UnPacker* lpUnPacker);

    // 620003 �ر�����
    void OnRtnOrder(IF2UnPacker* lpUnPacker);
    void OnRtnTrade(IF2UnPacker* lpUnPacker);

private:
    hstrade_t*  m_hstd;
    int         m_data_proto;

    int         m_return_code;
    const char* m_return_msg;
};



#endif//_HS_TRADE_CALLBACK_H_
