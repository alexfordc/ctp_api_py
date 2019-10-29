#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <t2sdk_interface.h>

#include "hstrade.h"
#include "hstrade_callback.h"


#define HS_TRADE                0x01

#define HS_TRADE_VERSION        "0.0.0"


extern void ShowPacket(IF2UnPacker *lpUnPacker);


static int hstrade_is_async(hstrade_t* hstd)
{
    return hstd->is_async;
}

static IBizMessage* _hstrade_make_bizmsg(hstrade_t* hstd, int funcid)
{
    IBizMessage* lpBizMessage;
    lpBizMessage = NewBizMessage();
    lpBizMessage->AddRef();

    lpBizMessage->SetFunction(funcid);
    lpBizMessage->SetPacketType(REQUEST_PACKET);

#if 0
    lpBizMessage->SetBranchNo(hstd->apidata.branch_no);
    lpBizMessage->SetSystemNo(hstd->apidata.system_no);
    lpBizMessage->SetCompanyID(91000);
    lpBizMessage->SetSenderCompanyID(91000);
#endif

    return lpBizMessage;
}

static IF2Packer* _hstrade_make_if2pakcer(hstrade_t* hstd)
{
    hstrade_apidata_t* apidata = &hstd->apidata;

    // ��ȡ�汾Ϊ2���͵�pack�����
    IF2Packer* lpPacker = NewPacker(2);
    if (!lpPacker)
    {
        fprintf(stderr, "ȡ�����ʧ��!\n");
        return lpPacker;
    }

    lpPacker->AddRef();
    lpPacker->BeginPack();

    // �����ֶ���
    lpPacker->AddField("op_branch_no",  'I', 5);
    lpPacker->AddField("op_entrust_way",'C', 1);
    lpPacker->AddField("op_station",    'S', 255);
    lpPacker->AddField("branch_no",     'I', 5);
    lpPacker->AddField("client_id",     'S', 18);
    lpPacker->AddField("fund_account",  'S', 18);
    lpPacker->AddField("password",      'S', 10);
    lpPacker->AddField("password_type", 'C', 1);
    lpPacker->AddField("user_token",    'S', 512);

    // �����Ӧ���ֶ�ֵ
    lpPacker->AddInt(apidata->op_branch_no);      // op_branch_no	
    lpPacker->AddChar(apidata->entrust_way);     // op_entrust_way	
    lpPacker->AddStr(apidata->op_station);       // op_station
    lpPacker->AddInt(apidata->branch_no);        // branch_no
    lpPacker->AddStr(hstd->client_id);          // client_id
    lpPacker->AddStr(hstd->client_id);          // fund_account
    lpPacker->AddStr(hstd->password);           // password	
    lpPacker->AddChar(apidata->password_type);   // password_type	
    lpPacker->AddStr(apidata->user_token);       // user_token

    // �������
    lpPacker->EndPack();

    return lpPacker;
}

//////////////////////////////////////////////////////////////////////////
const char* HS_TRADE_STDCALL hstrade_version(int* pver)
{
    if (pver) {
        *pver = GetVersionInfo();
    }
    return HS_TRADE_VERSION;
}

hstrade_t* HS_TRADE_STDCALL hstrade_create(int is_async)
{
    hstrade_t* hstd;
    hstd = (hstrade_t*)malloc(sizeof(hstrade_t));
    memset(hstd, 0, sizeof(hstrade_t));

    hstd->is_async = is_async;

    hstd->config = NewConfig();
    hstd->config->AddRef();

    hstd->callback = new TradeCallback();
    hstd->callback->SetContextData(hstd);

    return hstd;
}

void HS_TRADE_STDCALL hstrade_realese(hstrade_t* hstd)
{
    if (hstd)
    {
        free(hstd);
    }
}

void  HS_TRADE_STDCALL hstrade_set_userdata(hstrade_t* hstd, void* userdata)
{
    hstd->userdata = userdata;
}

void* HS_TRADE_STDCALL hstrade_get_userdata(hstrade_t* hstd)
{
    return hstd->userdata;
}

void HS_TRADE_STDCALL hstrade_register_spi(hstrade_t* hstd, hstrade_spi_t* spi)
{
    hstd->spi = spi;
}

int HS_TRADE_STDCALL hstrade_init(hstrade_t* hstd)
{
    CConfigInterface* config;
    CConnectionInterface* conn;

    config = hstd->config;

    config->Load("t2sdk.ini");
//     const char *p_fund_account = config->GetString("ufx", "fund_account", "");
//     const char *p_password = config->GetString("ufx", "password", "");
//     strcpy(hstd->client_id, p_fund_account);
//     strcpy(hstd->password, p_password);
    hstd->apidata.entrust_way = '7';
    hstd->apidata.op_branch_no = 1;

    int iRet = 0;

    conn = NewConnection(config);
    conn->AddRef();
    iRet = conn->Create2BizMsg(hstd->callback);
    if (0 != iRet)
    {
        fprintf(stderr, "��ʼ��ʧ��.iRet=%d, msg:%s\n", iRet, conn->GetErrorMsg(iRet));
        return -1;
    }
    if (0 != (iRet = conn->Connect(3000)))
    {
        fprintf(stderr, "����.iRet=%d, msg:%s\n", iRet, conn->GetErrorMsg(iRet));
        return -1;
    }

    hstd->conn = conn;
    return 0;
}

int HS_TRADE_STDCALL hstrade_subscribe_topic(hstrade_t* hstd, int issue_type)
{
    // hstd->topic = topic;

    hstrade_apidata_t*      apidata;
    CConnectionInterface*   conn;

    apidata = &hstd->apidata;
    conn = hstd->conn;

    int iRet = 0, hSend = 0;
    IF2UnPacker* lpUnPacker = NULL;

    ///��ȡ�汾Ϊ2���͵�pack�����
    IF2Packer *lpPacker = NewPacker(2);
    if (!lpPacker)
    {
        printf("ȡ�����ʧ��!\r\n");
        return -1;
    }
    lpPacker->AddRef();

    IBizMessage* lpBizMessage = NewBizMessage();
    lpBizMessage->AddRef();

    lpBizMessage->SetFunction(UFX_FUNC_SUBSCRIBE);
    lpBizMessage->SetPacketType(REQUEST_PACKET);
    lpBizMessage->SetSequeceNo(12);     // FIXME

    ///��ʼ���
    lpPacker->BeginPack();
    lpPacker->AddField("branch_no", 'I', 5);
    lpPacker->AddField("fund_account", 'S', 18);
    lpPacker->AddField("op_branch_no", 'I', 5);
    lpPacker->AddField("op_entrust_way", 'C', 1);
    lpPacker->AddField("op_station", 'S', 255);
    lpPacker->AddField("client_id", 'S', 18);
    lpPacker->AddField("password", 'S', 10);
    lpPacker->AddField("user_token", 'S', 40);
    lpPacker->AddField("issue_type", 'I', 8);

    ///�����Ӧ���ֶ�ֵ
    lpPacker->AddInt(apidata->branch_no);
    lpPacker->AddStr(hstd->client_id);
    lpPacker->AddInt(apidata->op_branch_no);
    lpPacker->AddChar(apidata->entrust_way);
    lpPacker->AddStr(apidata->op_station);
    lpPacker->AddStr(hstd->client_id);
    lpPacker->AddStr(hstd->password);
    lpPacker->AddStr(apidata->user_token);
    lpPacker->AddInt(issue_type);

    ///�������
    lpPacker->EndPack();
    // lpBizMessage->SetKeyInfo(lpPacker->GetPackBuf(), lpPacker->GetPackLen());
    lpBizMessage->SetBuff(lpPacker->GetPackBuf(), lpPacker->GetPackLen());

    int rv = conn->SendBizMsg(lpBizMessage, 1);
    if (rv != 0)
    {
        // 
    }

    lpPacker->FreeMem(lpPacker->GetPackBuf());
    lpPacker->Release();
    lpBizMessage->Release();

    return rv;
}

const char* HS_TRADE_STDCALL hstrade_get_trading_day(hstrade_t* hstd)
{
    return hstd->trading_date;
}

int HS_TRADE_STDCALL hstrade_user_login(hstrade_t* hstd, HSReqUserLoginField* login_field)
{
    int iRet = 0, hSend = 0, iLen = 0;
    // cout<<"password"<<m_Password<<endl;
    // EncodeEx(m_Password,pout);
    // cout<<"pout: "<<pout<<endl;

    fprintf(stderr, "331100�ͻ���½������£�\n");

    IBizMessage* lpBizMessage;
    lpBizMessage = _hstrade_make_bizmsg(hstd, UFX_FUNC_LOGIN);

    IF2Packer* lpPacker;
    lpPacker = NewPacker(2);
    if (!lpPacker)
    {
        fprintf(stderr, "ȡ�����ʧ�ܣ�\r\n");
        return -1;
    }

    lpPacker->AddRef();
    lpPacker->BeginPack();

    lpPacker->AddField("op_branch_no", 'I', 5);//������֧����
    lpPacker->AddField("op_entrust_way", 'C', 1);//ί�з�ʽ 
    lpPacker->AddField("op_station", 'S', 255);//վ���ַ
    lpPacker->AddField("branch_no", 'I', 5);
    lpPacker->AddField("input_content", 'C');
    lpPacker->AddField("account_content", 'S', 30);
    lpPacker->AddField("content_type", 'S', 6);
    lpPacker->AddField("password", 'S', 10);
    lpPacker->AddField("password_type", 'C');
    //lpPacker->AddField("asset_prop",'C');

    strcpy(hstd->client_id, login_field->client_id);
    strcpy(hstd->password, login_field->password);

    // �����Ӧ���ֶ�ֵ
    lpPacker->AddInt(hstd->apidata.op_branch_no);
    lpPacker->AddChar(hstd->apidata.entrust_way);
    lpPacker->AddStr(hstd->apidata.op_station);
    lpPacker->AddInt(hstd->apidata.branch_no);
    lpPacker->AddChar('1');
    lpPacker->AddStr(hstd->client_id);
    lpPacker->AddStr("0");
    lpPacker->AddStr(hstd->password);
    lpPacker->AddChar(hstd->apidata.password_type);
    //lpPacker->AddChar('0');

    fprintf(stderr, "hstrade_user_login account:%s,password:%s\n", hstd->client_id, hstd->password);

    // �������
    lpPacker->EndPack();

    // ���ô������
    lpBizMessage->SetContent(lpPacker->GetPackBuf(), lpPacker->GetPackLen());

    // ��������
    hSend = hstd->conn->SendBizMsg(lpBizMessage, hstd->is_async);
    if (!hstrade_is_async(hstd))
    {
        // ͬ��ģʽ
        IBizMessage* lpBizMessageRecv = NULL;
        iRet = hstd->conn->RecvBizMsg(hSend, &lpBizMessageRecv, 5000);
        fprintf(stderr, "hstrade_user_login RecvBizMsg ret:%d\n", iRet);

        const void * lpBuffer = lpBizMessageRecv->GetContent(iLen);
        IF2UnPacker * lpUnPacker = NewUnPacker((void *)lpBuffer, iLen);
        ShowPacket(lpUnPacker);
    }

    if (lpPacker)
    {
        lpPacker->FreeMem(lpPacker->GetPackBuf());
        lpPacker->Release();
    }
    if (lpBizMessage)
    {
        lpBizMessage->Release();
    }
    return 0;
}

int HS_TRADE_STDCALL hstrade_order_insert(hstrade_t* hstd, HSReqOrderInsertField* order_req)
{
    hstrade_apidata_t*      apidata;
    CConnectionInterface*   conn;

    apidata = &hstd->apidata;
    conn = hstd->conn;

    int rv;
    int hSend = 0;

    IBizMessage* lpBizMessage;
    lpBizMessage = _hstrade_make_bizmsg(hstd, UFX_FUNC_PLACE_ORDER);

    ///��ȡ�汾Ϊ2���͵�pack�����
    IF2Packer *lpPacker = NewPacker(2);
    if (!lpPacker)
    {
        printf("ȡ�����ʧ��!\r\n");
        return -1;
    }
    lpPacker->AddRef();

    ///��ʼ���
    lpPacker->BeginPack();

    ///�����ֶ���
    lpPacker->AddField("op_branch_no", 'I', 5);      // ���� ���� ����
    lpPacker->AddField("op_entrust_way", 'C', 1);
    lpPacker->AddField("op_station", 'S', 255);
    lpPacker->AddField("branch_no", 'I', 5);
    lpPacker->AddField("client_id", 'S', 18);
    lpPacker->AddField("fund_account", 'S', 18);
    lpPacker->AddField("password", 'S', 50);
    lpPacker->AddField("password_type", 'C', 1);
    lpPacker->AddField("user_token", 'S', 512);
    lpPacker->AddField("exchange_type", 'S', 4);
    lpPacker->AddField("stock_account", 'S', 11);
    lpPacker->AddField("stock_code", 'S', 16);
    lpPacker->AddField("entrust_amount", 'F', 19, 2);
    lpPacker->AddField("entrust_price", 'F', 18, 3);
    lpPacker->AddField("entrust_bs", 'C', 1);
    lpPacker->AddField("entrust_prop", 'S', 3);
    lpPacker->AddField("batch_no", 'I', 8);

    ///�����Ӧ���ֶ�ֵ
    lpPacker->AddInt(apidata->op_branch_no);      // op_branch_no
    lpPacker->AddChar(apidata->entrust_way);     // op_entrust_way
    lpPacker->AddStr(apidata->op_station);       // op_station
    lpPacker->AddInt(apidata->branch_no);       // branch_no
    lpPacker->AddStr(hstd->client_id);          // client_id
    lpPacker->AddStr(hstd->client_id);          // fund_account
    lpPacker->AddStr(hstd->password);           // password
    lpPacker->AddChar(apidata->password_type);   // password_type
    lpPacker->AddStr(apidata->user_token);      // user_token
    lpPacker->AddStr(order_req->exchange_type);     // exchange_type
    lpPacker->AddStr("");      // stock_account
    lpPacker->AddStr(order_req->stock_code);        // stock_code
    lpPacker->AddDouble(order_req->entrust_amount); // entrust_amount
    lpPacker->AddDouble(order_req->entrust_price);  // entrust_price
    lpPacker->AddChar(order_req->entrust_bs);       // entrust_bs
    lpPacker->AddStr(order_req->entrust_prop);      // entrust_prop
    lpPacker->AddInt(order_req->batch_no);          // batch_no

    // �������
    lpPacker->EndPack();

    lpBizMessage->SetContent(lpPacker->GetPackBuf(), lpPacker->GetPackLen());

    hSend = conn->SendBizMsg(lpBizMessage, hstd->is_async);
    if (!hstrade_is_async(hstd))
    {
        // ͬ��ģʽ
        if (hSend < 0)
        {
            printf("���͹���333002ʧ��, �����: %d, ԭ��: %s!\r\n", hSend, conn->GetErrorMsg(hSend));
            hSend = -2;
            goto EXIT;
        }

        printf("���͹���333002�ɹ�, ���ؽ��վ��: %d!\r\n", hSend);

        IBizMessage* lpBizMessageRecv = NULL;

        hSend = conn->RecvBizMsg(hSend, &lpBizMessageRecv, 1000);
        if (hSend != 0)
        {
            printf("���չ���333002ʧ��, �����: %d, ԭ��: %s!\r\n", hSend, conn->GetErrorMsg(hSend));
            goto EXIT;
        }
        else
        {
            int iReturnCode = lpBizMessageRecv->GetReturnCode();
            if (iReturnCode != 0) //����
            {
                printf("���չ���333002ʧ��,errorNo:%d,errorInfo:%s\n", lpBizMessageRecv->GetReturnCode(), lpBizMessageRecv->GetErrorInfo());
            }
            else if (iReturnCode == 0) // ��ȷ
            {
                puts("ҵ������ɹ�");
                int iLen = 0;
                const void * lpBuffer = lpBizMessageRecv->GetContent(iLen);
                IF2UnPacker * lpUnPacker = NewUnPacker((void *)lpBuffer, iLen);
                ShowPacket(lpUnPacker);

                TradeCallback::UnpackBizMessage(lpUnPacker, hstd, TradeCallback::UnpackRspOrderData);
            }
        }
    }

EXIT:
    ///�ͷ�pack��������ڴ�
    if (lpPacker)
    {
        lpPacker->FreeMem(lpPacker->GetPackBuf());
        lpPacker->Release();
    }
    if (lpBizMessage)
    {
        lpBizMessage->Release();
    }

    return hSend;
    return 0;
}

int HS_TRADE_STDCALL hstrade_order_action(hstrade_t* hstd, HSReqOrderActionField* order_action)
{
    hstrade_apidata_t*      apidata;
    CConnectionInterface*   conn;

    apidata = &hstd->apidata;
    conn = hstd->conn;

    int rv;
    int hSend = 0;

    IBizMessage* lpBizMessage;
    lpBizMessage = _hstrade_make_bizmsg(hstd, UFX_FUNC_CANCEL_ORDER);

    ///��ȡ�汾Ϊ2���͵�pack�����
    IF2Packer *lpPacker = NewPacker(2);
    if (!lpPacker)
    {
        printf("ȡ�����ʧ��!\r\n");
        return -1;
    }
    lpPacker->AddRef();

    ///��ʼ���
    lpPacker->BeginPack();

    // �����ֶ���
    lpPacker->AddField("op_branch_no", 'I', 5);
    lpPacker->AddField("op_entrust_way", 'C', 1);
    lpPacker->AddField("op_station", 'S', 255);
    lpPacker->AddField("branch_no", 'I', 5);
    lpPacker->AddField("client_id", 'S', 18);
    lpPacker->AddField("fund_account", 'S', 18);
    lpPacker->AddField("password", 'S', 10);
    lpPacker->AddField("password_type", 'C', 1);
    lpPacker->AddField("user_token", 'S', 512);
    lpPacker->AddField("batch_flag", 'S', 4);   // FIXME
    lpPacker->AddField("entrust_no", 'S', 18);  // FIXME

    // �����Ӧ���ֶ�ֵ
    lpPacker->AddInt(apidata->op_branch_no);      // op_branch_no	
    lpPacker->AddChar(apidata->entrust_way);     // op_entrust_way	
    lpPacker->AddStr(apidata->op_station);       // op_station
    lpPacker->AddInt(apidata->branch_no);        // branch_no
    lpPacker->AddStr(hstd->client_id);          // client_id
    lpPacker->AddStr(hstd->client_id);          // fund_account
    lpPacker->AddStr(hstd->password);           // password	
    lpPacker->AddChar(apidata->password_type);   // password_type	
    lpPacker->AddStr(apidata->user_token);       // user_token
    lpPacker->AddStr("0");                      // action single order
    lpPacker->AddStr(order_action->entrust_no);   // entrust_no

    // �������
    lpPacker->EndPack();

    lpBizMessage->SetContent(lpPacker->GetPackBuf(), lpPacker->GetPackLen());

    hSend = conn->SendBizMsg(lpBizMessage, hstd->is_async);
    if (!hstrade_is_async(hstd))
    {
        // ͬ��ģʽ
        if (hSend < 0)
        {
            printf("���͹���333017ʧ��, �����: %d, ԭ��: %s!\r\n", hSend, conn->GetErrorMsg(hSend));
            hSend = -2;
            goto EXIT;
        }

        printf("���͹���333017�ɹ�, ���ؽ��վ��: %d!\r\n", hSend);

        IBizMessage* lpBizMessageRecv = NULL;

        hSend = conn->RecvBizMsg(hSend, &lpBizMessageRecv, 1000);
        if (hSend != 0)
        {
            printf("���չ���333017ʧ��, �����: %d, ԭ��: %s!\r\n", hSend, conn->GetErrorMsg(hSend));
            goto EXIT;
        }
        else
        {
            int iReturnCode = lpBizMessageRecv->GetReturnCode();
            if (iReturnCode != 0) //����
            {
                printf("���չ���333017ʧ��,errorNo:%d,errorInfo:%s\n", lpBizMessageRecv->GetReturnCode(), lpBizMessageRecv->GetErrorInfo());
            }
            else if (iReturnCode == 0) // ��ȷ
            {
                puts("ҵ������ɹ�");
                int iLen = 0;
                const void * lpBuffer = lpBizMessageRecv->GetContent(iLen);
                IF2UnPacker * lpUnPacker = NewUnPacker((void *)lpBuffer, iLen);
                ShowPacket(lpUnPacker);
            }
        }
    }

EXIT:
    ///�ͷ�pack��������ڴ�
    if (lpPacker)
    {
        lpPacker->FreeMem(lpPacker->GetPackBuf());
        lpPacker->Release();
    }
    if (lpBizMessage)
    {
        lpBizMessage->Release();
    }

    return hSend;
    return 0;
}

int HS_TRADE_STDCALL hstrade_qry_trading_account(hstrade_t* hstd, HSReqQueryField* qry_field)
{
    hstrade_apidata_t*      apidata;
    CConnectionInterface*   conn;
    int rv;

    apidata = &hstd->apidata;
    conn = hstd->conn;

    // ����ҵ����Ϣ����
    IBizMessage* lpBizMessage = _hstrade_make_bizmsg(hstd, UFX_FUNC_QRY_CASH);

    // ��ȡ�汾Ϊ2���͵�pack�����
    IF2Packer* lpPacker = _hstrade_make_if2pakcer(hstd);

    // ���ð�����
    lpBizMessage->SetContent(lpPacker->GetPackBuf(), lpPacker->GetPackLen());

    // ����ҵ������
    rv = conn->SendBizMsg(lpBizMessage, hstd->is_async);
    if (!hstrade_is_async(hstd))
    {
        // ͬ��ģʽ
        if (rv < 0)
        {
            printf("���͹���332255ʧ��, �����: %d, ԭ��: %s!\r\n", rv, conn->GetErrorMsg(rv));
            rv = -2;
            goto EXIT;
        }

        printf("���͹���332255�ɹ�, ���ؽ��վ��: %d!\r\n", rv);

        IBizMessage* lpBizMessageRecv = NULL;

        rv = conn->RecvBizMsg(rv, &lpBizMessageRecv, 1000);
        if (rv != 0)
        {
            printf("���չ���332255ʧ��, �����: %d, ԭ��: %s!\r\n", rv, conn->GetErrorMsg(rv));
            goto EXIT;
        }
        else
        {
            int iReturnCode = lpBizMessageRecv->GetReturnCode();
            if (iReturnCode != 0)
            {
                // ����
                printf("���չ���332255ʧ��,errorNo:%d,errorInfo:%s\n", lpBizMessageRecv->GetErrorNo(), lpBizMessageRecv->GetErrorInfo());
            }
            else if (iReturnCode == 0)
            {
                // ��ȷ
                puts("ҵ������ɹ�");
                int iLen = 0;
                const void * lpBuffer = lpBizMessageRecv->GetContent(iLen);
                IF2UnPacker * lpUnPacker = NewUnPacker((void *)lpBuffer, iLen);
                ShowPacket(lpUnPacker);

                TradeCallback::UnpackBizMessage(lpUnPacker, hstd, TradeCallback::UnpackTradingAccountData);
            }

        }
    }

EXIT:
    // �ͷ�pack��������ڴ�
    if (lpPacker)
    {
        lpPacker->FreeMem(lpPacker->GetPackBuf());
        lpPacker->Release();
    }
    if (lpBizMessage)
    {
        lpBizMessage->Release();
    }

    return rv;
}

int HS_TRADE_STDCALL hstrade_qry_investor(hstrade_t* hstd, HSReqQueryField* qry_field)
{
    return 0;
}

int HS_TRADE_STDCALL hstrade_qry_position(hstrade_t* hstd, HSReqQueryField* qry_field)
{
    hstrade_apidata_t*      apidata;
    CConnectionInterface*   conn;
    int rv;

    apidata = &hstd->apidata;
    conn = hstd->conn;

    // ����ҵ����Ϣ����
    IBizMessage* lpBizMessage = _hstrade_make_bizmsg(hstd, UFX_FUNC_QRY_POSITION);

    // ��ȡ�汾Ϊ2���͵�pack�����
    IF2Packer* lpPacker = _hstrade_make_if2pakcer(hstd);

    // ���ð�����
    lpBizMessage->SetContent(lpPacker->GetPackBuf(), lpPacker->GetPackLen());

    // ����ҵ������
    rv = conn->SendBizMsg(lpBizMessage, hstd->is_async);
    if (!hstrade_is_async(hstd))
    {
        // ͬ��ģʽ
        if (rv < 0)
        {
            printf("���͹���333104ʧ��, �����: %d, ԭ��: %s!\r\n", rv, conn->GetErrorMsg(rv));
            rv = -2;
            goto EXIT;
        }

        printf("���͹���333104�ɹ�, ���ؽ��վ��: %d!\r\n", rv);

        IBizMessage* lpBizMessageRecv = NULL;

        rv = conn->RecvBizMsg(rv, &lpBizMessageRecv, 1000);
        if (rv != 0)
        {
            printf("���չ���333104ʧ��, �����: %d, ԭ��: %s!\r\n", rv, conn->GetErrorMsg(rv));
            goto EXIT;
        }
        else
        {
            int iReturnCode = lpBizMessageRecv->GetReturnCode();
            if (iReturnCode != 0)
            {
                // ����
                printf("���չ���333104ʧ��,errorNo:%d,errorInfo:%s\n", lpBizMessageRecv->GetErrorNo(), lpBizMessageRecv->GetErrorInfo());
            }
            else if (iReturnCode == 0)
            {
                // ��ȷ
                puts("ҵ������ɹ�");
                int iLen = 0;
                const void * lpBuffer = lpBizMessageRecv->GetContent(iLen);
                IF2UnPacker * lpUnPacker = NewUnPacker((void *)lpBuffer, iLen);
                ShowPacket(lpUnPacker);

                // �����ֲ�����
#if 0
                // int i = 0, t = 0, j = 0, k = 0;
                for (int i = 0; i < lpUnPacker->GetDatasetCount(); ++i)
                {
                    // ���õ�ǰ�����
                    // printf("��¼����%d/%d\r\n", i + 1, lpUnPacker->GetDatasetCount());
                    lpUnPacker->SetCurrentDatasetByIndex(i);

                    int rows = lpUnPacker->GetRowCount();
                    for (int k = 0; k < rows; ++k)
                    {
                        HSPositionField pos = { 0 };
                        strncpy(pos.client_id, lpUnPacker->GetStr("fund_account"), sizeof(pos.client_id) - 1);
                        strncpy(pos.stock_code, lpUnPacker->GetStr("stock_code"), sizeof(pos.stock_code) - 1);
                        strncpy(pos.stock_name, lpUnPacker->GetStr("stock_name"), sizeof(pos.stock_name) - 1);
                        pos.current_amount = lpUnPacker->GetInt("current_amount");
                        pos.enable_amount = lpUnPacker->GetInt("enabled_amount");
                        pos.frozen_amount = lpUnPacker->GetInt("frozen_amount");
                        pos.cost_price = lpUnPacker->GetDouble("cost_price");
                        pos.last_price = lpUnPacker->GetDouble("last_price");
                        pos.market_value = lpUnPacker->GetDouble("market_value");
                        // hstd->callback->OnResponseQryPosition(lpUnPacker);

                        // also, we could make a json message

                        lpUnPacker->Next();
                    }
                }
#else
                TradeCallback::UnpackBizMessage(lpUnPacker, hstd, TradeCallback::UnpackPositionData);
#endif
            }

        }
    }

EXIT:
    // �ͷ�pack��������ڴ�
    if (lpPacker)
    {
        lpPacker->FreeMem(lpPacker->GetPackBuf());
        lpPacker->Release();
    }
    if (lpBizMessage)
    {
        lpBizMessage->Release();
    }

    return rv;
}

int HS_TRADE_STDCALL hstrade_qry_trade(hstrade_t* hstd, HSReqQueryField* qry_field)
{
    hstrade_apidata_t*      apidata;
    CConnectionInterface*   conn;
    int rv;

    apidata = &hstd->apidata;
    conn = hstd->conn;

    // ����ҵ����Ϣ����
    IBizMessage* lpBizMessage = _hstrade_make_bizmsg(hstd, UFX_FUNC_QRY_TRADE);

    // ��ȡ�汾Ϊ2���͵�pack�����
    IF2Packer* lpPacker = _hstrade_make_if2pakcer(hstd);

    // ���ð�����
    lpBizMessage->SetContent(lpPacker->GetPackBuf(), lpPacker->GetPackLen());

    // ����ҵ������
    rv = conn->SendBizMsg(lpBizMessage, hstd->is_async);
    if (!hstrade_is_async(hstd))
    {
        // ͬ��ģʽ
        if (rv < 0)
        {
            printf("���͹���333102ʧ��, �����: %d, ԭ��: %s!\r\n", rv, conn->GetErrorMsg(rv));
            rv = -2;
            goto EXIT;
        }

        printf("���͹���333102�ɹ�, ���ؽ��վ��: %d!\r\n", rv);

        IBizMessage* lpBizMessageRecv = NULL;

        rv = conn->RecvBizMsg(rv, &lpBizMessageRecv, 1000);
        if (rv != 0)
        {
            printf("���չ���333102ʧ��, �����: %d, ԭ��: %s!\r\n", rv, conn->GetErrorMsg(rv));
            goto EXIT;
        }
        else
        {
            int iReturnCode = lpBizMessageRecv->GetReturnCode();
            if (iReturnCode != 0)
            {
                // ����
                printf("���չ���333102ʧ��,errorNo:%d,errorInfo:%s\n", lpBizMessageRecv->GetErrorNo(), lpBizMessageRecv->GetErrorInfo());
            }
            else if (iReturnCode == 0)
            {
                // ��ȷ
                puts("ҵ������ɹ�");
                int iLen = 0;
                const void * lpBuffer = lpBizMessageRecv->GetContent(iLen);
                IF2UnPacker * lpUnPacker = NewUnPacker((void *)lpBuffer, iLen);
                ShowPacket(lpUnPacker);

                TradeCallback::UnpackBizMessage(lpUnPacker, hstd, TradeCallback::UnpackQryTradeData);
            }

        }
    }

EXIT:
    // �ͷ�pack��������ڴ�
    if (lpPacker)
    {
        lpPacker->FreeMem(lpPacker->GetPackBuf());
        lpPacker->Release();
    }
    if (lpBizMessage)
    {
        lpBizMessage->Release();
    }

    return rv;
}

int HS_TRADE_STDCALL hstrade_qry_order(hstrade_t* hstd, HSReqQueryField* qry_field)
{
    hstrade_apidata_t*      apidata;
    CConnectionInterface*   conn;
    int rv;

    apidata = &hstd->apidata;
    conn = hstd->conn;

    // ����ҵ����Ϣ����
    IBizMessage* lpBizMessage = _hstrade_make_bizmsg(hstd, UFX_FUNC_QRY_ORDER);

    // ��ȡ�汾Ϊ2���͵�pack�����
    IF2Packer* lpPacker = _hstrade_make_if2pakcer(hstd);

    // ���ð�����
    lpBizMessage->SetContent(lpPacker->GetPackBuf(), lpPacker->GetPackLen());

    // ����ҵ������
    rv = conn->SendBizMsg(lpBizMessage, hstd->is_async);
    if (!hstrade_is_async(hstd))
    {
        // ͬ��ģʽ
        if (rv < 0)
        {
            printf("���͹���333101ʧ��, �����: %d, ԭ��: %s!\r\n", rv, conn->GetErrorMsg(rv));
            rv = -2;
            goto EXIT;
        }

        printf("���͹���333101�ɹ�, ���ؽ��վ��: %d!\r\n", rv);

        IBizMessage* lpBizMessageRecv = NULL;

        rv = conn->RecvBizMsg(rv, &lpBizMessageRecv, 1000);
        if (rv != 0)
        {
            printf("���չ���333101ʧ��, �����: %d, ԭ��: %s!\r\n", rv, conn->GetErrorMsg(rv));
            goto EXIT;
        }
        else
        {
            int iReturnCode = lpBizMessageRecv->GetReturnCode();
            if (iReturnCode != 0)
            {
                // ����
                printf("���չ���333101ʧ��,errorNo:%d,errorInfo:%s\n", lpBizMessageRecv->GetErrorNo(), lpBizMessageRecv->GetErrorInfo());
            }
            else if (iReturnCode == 0)
            {
                // ��ȷ
                puts("ҵ������ɹ�");
                int iLen = 0;
                const void * lpBuffer = lpBizMessageRecv->GetContent(iLen);
                IF2UnPacker * lpUnPacker = NewUnPacker((void *)lpBuffer, iLen);
                ShowPacket(lpUnPacker);

                TradeCallback::UnpackBizMessage(lpUnPacker, hstd, TradeCallback::UnpackQryOrderData);
            }

        }
    }

EXIT:
    // �ͷ�pack��������ڴ�
    if (lpPacker)
    {
        lpPacker->FreeMem(lpPacker->GetPackBuf());
        lpPacker->Release();
    }
    if (lpBizMessage)
    {
        lpBizMessage->Release();
    }

    return rv;
}

int HS_TRADE_STDCALL hstrade_qry_security_info(hstrade_t* hstd, HSReqQueryField* qry_field)
{
    return 0;
}

int HS_TRADE_STDCALL hstrade_qry_md(hstrade_t* hstd, HSReqQueryField* qry_field)
{
    return 0;
}

int HS_TRADE_STDCALL hstrade_qry_cash_hist(hstrade_t* hstd, HSReqQueryField* qry_field)
{
    return 0;
}

int HS_TRADE_STDCALL hstrade_qry_trade_hist(hstrade_t* hstd, HSReqQueryField* qry_field)
{
    return 0;
}

int HS_TRADE_STDCALL hstrade_qry_order_hist(hstrade_t* hstd, HSReqQueryField* qry_field)
{
    return 0;
}


//////////////////////////////////////////////////////////////////////////
void* HS_TRADE_STDCALL hstrade_begin_pack(hstrade_t* hstd)
{
    IF2Packer* lpPacker;
    lpPacker = NewPacker(2);
    if (!lpPacker)
        return NULL;

    lpPacker->AddRef();

    return lpPacker;
}

int HS_TRADE_STDCALL hstrade_end_pack(hstrade_t* hstd, void* packer)
{
    IF2Packer* lpPacker;
    lpPacker = (IF2Packer*)packer;
    lpPacker->EndPack();
    return 0;
}

int HS_TRADE_STDCALL hstrade_send_bizmsg(hstrade_t* hstd, void* packer, int func_id)
{
    IF2Packer* lpPacker;
    lpPacker = (IF2Packer*)packer;

    IBizMessage* lpBizMessage;
    lpBizMessage = NewBizMessage();
    lpBizMessage->AddRef();
    lpBizMessage->SetFunction(func_id);
    lpBizMessage->SetPacketType(REQUEST_PACKET);

    lpBizMessage->SetContent(lpPacker->GetPackBuf(), lpPacker->GetPackBufSize());

    // ��������
    int hSend;
    hSend = hstd->conn->SendBizMsg(lpBizMessage, hstd->is_async);
    if (!hstrade_is_async(hstd))
    {
        // ͬ��ģʽ
        IBizMessage* lpBizMessageRecv = NULL;
        int iRet = hstd->conn->RecvBizMsg(hSend, &lpBizMessageRecv, 5000);
        fprintf(stderr, "hstrade_user_login RecvBizMsg ret:%d\n", iRet);

        int iLen = 0;
        const void * lpBuffer = lpBizMessageRecv->GetContent(iLen);
        IF2UnPacker * lpUnPacker = NewUnPacker((void *)lpBuffer, iLen);
        ShowPacket(lpUnPacker);
        lpUnPacker->Release();
    }

    if (lpPacker)
    {
        lpPacker->FreeMem(lpPacker->GetPackBuf());
        lpPacker->Release();
    }
    if (lpBizMessage)
    {
        lpBizMessage->Release();
    }
    return 0;
}

/// add pack fields
int HS_TRADE_STDCALL hstrade_add_field(hstrade_t* hstd, void* packer, const char* key, char field_type, int field_width)
{
    IF2Packer* lpPacker;
    lpPacker = (IF2Packer*)packer;
    return lpPacker->AddField(key, field_type, field_width);
}


/// add pack values
int HS_TRADE_STDCALL hstrade_pack_char(hstrade_t* hstd, void* packer, const char value)
{
    IF2Packer* lpPacker;
    lpPacker = (IF2Packer*)packer;
    lpPacker->AddChar(value);
    return 0;
}

int HS_TRADE_STDCALL hstrade_pack_string(hstrade_t* hstd, void* packer, const char* value)
{
    IF2Packer* lpPacker;
    lpPacker = (IF2Packer*)packer;
    lpPacker->AddStr(value);
    return 0;
}

int HS_TRADE_STDCALL hstrade_pack_int(hstrade_t* hstd, void* packer, const int value)
{
    IF2Packer* lpPacker;
    lpPacker = (IF2Packer*)packer;
    lpPacker->AddInt(value);
    return 0;
}

int HS_TRADE_STDCALL hstrade_pack_double(hstrade_t* hstd, void* packer, const double value)
{
    IF2Packer* lpPacker;
    lpPacker = (IF2Packer*)packer;
    lpPacker->AddDouble(value);
    return 0;
}


