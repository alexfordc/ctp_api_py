#pragma warning(disable:4200)
#pragma once

#include <stdio.h>
#include <string.h>

#include <string>
#include <map>

#include <pybind11/pybind11.h>
#include <XcTradeApi/XcTradeApi.h>
#include <XcTradeApi/XcTrade_Define.h>

// #include <nlohmann/json.hpp>
// using json = nlohmann::json;

// using std::string;
// using std::map;
// using namespace std;
using namespace pybind11;


#define XC_TDAPI_VERSION        "0.0.1"

#define XC_FUNC_QRY_SECINFO     330300      // ֤ȯ������Ϣ��ѯ
#define XC_FUNC_PLACE_ORDER     333002      // ��ͨί��
#define XC_FUNC_CANCEL_ORDER    333017      // ί�г���
#define XC_FUNC_QRY_PRUCHASE    333030      // �¹��깺��ѯ
#define XC_FUNC_QRY_ORDER       333101      // ֤ȯί�в�ѯ
#define XC_FUNC_QRY_TRADE       333102      // ֤ȯ�ɽ���ѯ
#define XC_FUNC_QRY_POS_FAST    333103      // ֤ȯ�ֲֿ��ٲ�ѯ
#define XC_FUNC_QRY_POSITION    333104      // ֤ȯ�ֲֲ�ѯ
#define XC_FUNC_QRY_MD          400         // ֤ȯ�����ѯ
#define XC_FUNC_LOGIN           331100      // �ͻ���¼
#define XC_FUNC_TRADING_ACCOUNT 331155      // �ʽ��˻���ȡ
#define XC_FUNC_SUB_ORDER       620001      // ���Ļر�����
#define XC_FUNC_RTN_ORDER       620003      // �ر�����


class XcTdApi :public CXcTradeSpi
{
public:
    XcTdApi();
    ~XcTdApi();

    CXcTradeApi*    api;
    FILE*           fp;

    std::string     account_id;

public:
    /* �Ͽ���ʾ*/
    virtual void OnClose(void);

    /* �����У����ڳ������� */
    virtual void OnDisConnect(void);

    /* �����ɹ� */
    virtual void OnConnected(void);

    /* ����JSONģʽӦ�����ݣ�JSONģʽ��*/
    virtual void OnRecvJsonMsg(char* pJsonMsg);

public:
    virtual void on_front_connected() {}
    virtual void on_front_disconnected(int reason) {}
    virtual void on_recv_msg(const std::string& msg) {}

public:
    void create_td_api(std::string str = "", int reserve=0);

    void release();

    int init(std::string user_id, std::string server_ip, std::string server_port, std::string license);

    int send_data(int func_id, const std::string& data, int subsystem_no = 0, int branch_no = 0);

    // ��ȡ�û����ֵ
    int get_space();
    // ��ȡ�û�Ƶ��ֵ
    int get_frequency();
    // ��ȡ���������Ϣ
    std::string get_last_error_msg();

    // my extend functions
    std::string get_api_version();

    int open_debug(std::string log_file, int log_level);

    // some api for field map
    std::string get_field_buy();
    std::string get_field_sell();
    std::string get_field_exchange_sse();
    std::string get_field_exchange_szse();

private:
    int write_data(int reserve, const char* fmt, ...);
};
