#pragma warning(disable:4200)
#pragma once

#include <stdio.h>
#include <string.h>

#include <string>
#include <map>

#include <pybind11/pybind11.h>
#include <XcMarketApi/XcMarketApi.h>

using namespace std;
using namespace pybind11;


#define XC_MDAPI_VERSION    "0.0.2"


// �Զ���֤ȯ������Ϣ
struct XcSecurityInfo
{
    char        InstrumentID[31];
    char        ExchangeID[8];
    char        SecName[50];            // ���
    char        EndDate[20];            // ���������
    char        StartDate[10];          // ��������
    int32_t     BuyUnit;                // ��������λ
    int32_t     SellUnit;               // ��������λ
    double      PreClosePrice;          // ǰ���̼� SecurityClosePx
    double      PriceTick;              // ��С�۸�䶯�� MinMovement
    double      UpperLimitPrice;        // �Ƿ����޼۸� DailyPriceUpLimit
    double      LowerLimitPrice;        // �������޼۸� DailyPriceDownLimit
    double      ExRightRatio;           // ��Ȩ����
    double      DividendAmount;         // ��Ϣ���
    char        SecurityStatusFlag[50]; // ��Ʒ״̬��־
    char        Bz[200];                // ��ע
};

// �Զ�������ṹ��
struct XcDepthMarketData
{
    ///������
    char    TradingDay[9];
    ///��Լ����
    char    InstrumentID[31];
    ///����������
    char    ExchangeID[8];
    ///���¼�
    double  LastPrice;
    ///�ϴν����
    double  PreSettlementPrice;
    ///������
    double  PreClosePrice;
    ///��ֲ���
    double  PreOpenInterest;
    ///����
    double  OpenPrice;
    ///��߼�
    double  HighestPrice;
    ///��ͼ�
    double  LowestPrice;
    ///����
    int     Volume;
    ///�ɽ����
    double  Turnover;
    ///�ֲ���
    double  OpenInterest;
    ///������
    double  ClosePrice;
    ///���ν����
    double  SettlementPrice;
    ///��ͣ���
    double  UpperLimitPrice;
    ///��ͣ���
    double  LowerLimitPrice;
    ///����ʵ��
    double  PreDelta;
    ///����ʵ��
    double  CurrDelta;
    ///����޸�ʱ��
    char    UpdateTime[9];
    ///����޸ĺ���
    int     UpdateMillisec;
    ///�����һ
    double  BidPrice1;
    ///������һ
    int     BidVolume1;
    ///������һ
    double  AskPrice1;
    ///������һ
    int     AskVolume1;
    ///����۶�
    double  BidPrice2;
    ///��������
    int     BidVolume2;
    ///�����۶�
    double  AskPrice2;
    ///��������
    int     AskVolume2;
    ///�������
    double  BidPrice3;
    ///��������
    int     BidVolume3;
    ///��������
    double  AskPrice3;
    ///��������
    int     AskVolume3;
    ///�������
    double  BidPrice4;
    ///��������
    int     BidVolume4;
    ///��������
    double  AskPrice4;
    ///��������
    int     AskVolume4;
    ///�������
    double  BidPrice5;
    ///��������
    int     BidVolume5;
    ///��������
    double  AskPrice5;
    ///��������
    int     AskVolume5;
    ///���վ���
    double  AveragePrice;
    ///ҵ������
    char    ActionDay[9];
};
typedef map<string, XcDepthMarketData>   MDMapType;
typedef map<QWORD, XcDepthMarketData*>   QuoteIDMapType;

typedef map<string, XcSecurityInfo>      SecInfoMapType;


class XcMdApi :public CXcMarketSpi
{
public:
    XcMdApi();
    ~XcMdApi();

    CXcMarketApi*   api;
    QuoteIDMapType  qidmap;
    MDMapType       mdmap;
    SecInfoMapType  secmap;
    QWORD           refid;

public:
    void OnUserLogin(socket_struct_Msg* pMsg);
    void OnRespMarket(QWORD qQuoteID, socket_struct_Market* pMarket);
    void OnRespSecurity(QWORD qQuoteID, void* pParam);
    void OnRespSecurity_Opt(QWORD qQuoteID, void* pParam);
    void OnRespSecurity_Sz(QWORD qQuoteID, void* pParam);
    void OnRespDyna(QWORD qQuoteID, void* pParam);
    void OnRespDepth(QWORD qQuoteID, char* MarketCode, char* SecCode, socket_struct_DepthDetail* pDepth);
    void OnRespDepthOrder(QWORD qQuoteID, char* MarketCode, char* SecCode, int Grade, DWORD Price, socket_struct_DepthOrderDetail* pDepthOrder);
    void OnRespEachOrder(QWORD qQuoteID, void* pParam);
    void OnRespEachDeal(QWORD qQuoteID, void* pParam);
    void OnHeart();
    void OnIssueEnd(QWORD qQuoteID);
    void OnMsg(QWORD qRefID, socket_struct_Msg* pMsg);
    void OnClose();

public:
    virtual void on_front_connected() {}

    virtual void on_front_disconnected(int reason) {}

    // just notify login result
    virtual void on_rsp_user_login(const dict &data) {}

    virtual void on_rsp_qry_data(const dict& data) {}

    // rtn md fields reference to XcDepthMarketData
    virtual void on_rtn_market_data(const dict &data) {}

public:
    void create_md_api(string str = "");

    void release();

    // connect to server
    int init(string user_id, string server_ip, string server_port, string license);

    // subscribe eg. 000001.SZSE or 600000.SSE
    int subscribe_md(string instrument);
    int unsubscribe_md(string instrument);

    int subscribe_md_batch(const std::vector<std::string>& reqs);
    int unsubscribe_md_batch(const std::vector<std::string>& reqs);
    int unsubscribe_all();

    // request query data
    int req_qry_data(string instrument);
    int req_qry_data_batch(const std::vector<std::string>& reqs);

    // my extend functions
    string get_api_version();

    int open_debug(string log_file, int log_level);
};
