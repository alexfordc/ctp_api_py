#include <time.h>
#include <codecvt>
#include <locale>
#include <iostream>
// #include <condition_variable>

#include "xctd.h"

#ifdef _MSC_VER
#else

#include <unistd.h>
#include <iconv.h>
#define sleepms(x)  usleep((x) * 1000)

int code_convert(char* from, char* to, char* inbuf, size_t inlen, char* outbuf, size_t outlen)
{
    iconv_t cd;
    char** pin = &inbuf;
    char** pout = &outbuf;

    cd = iconv_open(to, from);
    if (cd == 0)
        return -1;
    if (iconv(cd, pin, &inlen, pout, &outlen) == -1)
        return -1;
    iconv_close(cd);
    return 0;
}

#endif//_MSC_VER


#define MY_DEBUG 0

#if (MY_DEBUG)
#define XcDebugInfo(fd,fmt,...) fprintf(fd,fmt,##__VA_ARGS__)
#else
static  inline void XcPrintNull(FILE* fd, const char* fmt, ...) { (void)fd; (void)fmt; }
#define XcDebugInfo(fd,fmt,...) XcPrintNull(fd,fmt,##__VA_ARGS__)
#endif//MY_DEBUG
#define XcDbgFd stdout


#define XC_OP_BRANCH_NO         "0"
#define XC_OP_ENTRUST_WAY       "7"     // 网上委托
#define XC_PASSWORD_TYPE        "2"     // 1-资金密码 2-交易密码

#define XC_EXCHANGE_TYPE_CASH   "0"     // 0-资金 1-上海 2-深圳
#define XC_EXCHANGE_TYPE_SSE    "1"
#define XC_EXCHANGE_TYPE_SZSE   "2"

#define XC_ENTRUST_TYPE_BUY     "1"     // 1-买入 2-卖出
#define XC_ENTRUST_TYPE_SELL    "2"
#define XC_ENTRUST_PROP         "0"     // 0-买卖

#if 0
//从字典中获取某个建值对应的字符串，并赋值到请求结构体对象的值上
// template <uint32_t size>
// void get_string(const pybind11::dict &d, const char *key, string_literal<size> &value)
void get_string(const pybind11::dict &d, const char *key, std::string &value)
{
    if (d.contains(key))
    {
        object o = d[key];
        std::string s = o.cast<std::string>();
        // const char *buf = s.c_str();
        // strcpy(value, buf);
        value = s;
    }
};
#endif//0

static const char* func_id_desc(int func_id)
{
    switch (func_id)
    {
    case XC_FUNC_QRY_SECINFO:       return "qry_secifo";
    case XC_FUNC_QRY_CASH_FAST:     return "qry_cash_fast";
    case XC_FUNC_QRY_CASH:          return "qry_cash";
    case XC_FUNC_PLACE_ORDER:       return "place_order";
    case XC_FUNC_CANCEL_ORDER:      return "cancel_order";
    case XC_FUNC_QRY_PURCHASE:      return "qry_purchase";
    case XC_FUNC_QRY_ORDER:         return "qry_order";
    case XC_FUNC_QRY_TRADE:         return "qry_trade";
    case XC_FUNC_QRY_POSITION:      return "qry_position";
    case XC_FUNC_QRY_MD:            return "qrt_md";
    case XC_FUNC_LOGIN:             return "client_login";
    case XC_FUNC_QRY_ACCOUNTINFO:   return "qry_account_info";
    case XC_FUNC_SUB_ORDER:         return "subscribe_order";
    default:                        return "unknown";
    }
}

//将GBK编码的字符串转换为UTF8
static std::string to_utf(const std::string &gb2312)
{
#ifdef _MSC_VER
    const static locale loc("zh-CN");

    vector<wchar_t> wstr(gb2312.size());
    wchar_t* wstrEnd = nullptr;
    const char* gbEnd = nullptr;
    mbstate_t state = {};
    int res = use_facet<codecvt<wchar_t, char, mbstate_t> >
        (loc).in(state,
            gb2312.data(), gb2312.data() + gb2312.size(), gbEnd,
            wstr.data(), wstr.data() + wstr.size(), wstrEnd);

    if (codecvt_base::ok == res)
    {
        wstring_convert<codecvt_utf8<wchar_t>> cutf8;
        return cutf8.to_bytes(wstring(wstr.data(), wstrEnd));
    }

    return std::string();
#else
    // const static locale loc("zh_CN.GB18030");
    if ((int)gb2312.length() < 4080)
    {
        char outbuf[4080] = "";
        int outlen = sizeof(outbuf) - 1;
        int rv = code_convert("gb2312", "utf-8", (char*)gb2312.c_str(), (int)gb2312.length(), outbuf, outlen);
        if (rv == -1)
            return string();
        return string(outbuf);
    }
    else
    {
        int outlen = gb2312.length() * 2 + 2;
        char* outbuf = (char*)malloc(outlen);
        memset(outbuf, 0, outlen);
        int rv = code_convert("gb2312", "utf-8", (char*)gb2312.c_str(), (int)gb2312.length(), outbuf, outlen);

        string ret(outbuf);
        free(outbuf);

        if (rv == -1)
            return string();
        else
            return ret;
    }
#endif
}


XcTdApi::XcTdApi()
    : api()
    , fp()
{
}

XcTdApi::~XcTdApi()
{
    if (fp)
    {
        fclose(fp);
        fp = NULL;
    }
}

void XcTdApi::OnClose()
{
    XcDebugInfo(XcDbgFd, "xctdapi OnClose\n");
    this->on_front_disconnected(-1);
}

void XcTdApi::OnDisConnect(void)
{
    XcDebugInfo(XcDbgFd, "xctdapi OnDisConnect\n");
    this->on_front_disconnected(-1);
}

void XcTdApi::OnConnected(void)
{
    XcDebugInfo(XcDbgFd, "xctdapi OnConnected\n");
    this->on_front_connected();
}

void XcTdApi::OnRecvJsonMsg(char* pJsonMsg)
{
    // XcDebugInfo(XcDbgFd, pJsonMsg);
    // write_data(0, pJsonMsg);

    std::string msg = to_utf(pJsonMsg);
    this->on_recv_msg(msg);
}

void XcTdApi::OnRecvPackMsg(int iFunid, int iRefid, int iIssueType, int iSet, int iRow, int iCol, char* szName, char* szValue)
{
    std::string name = to_utf(szName);
    std::string value = to_utf(szValue);
    this->on_recv_pack_msg(iFunid, iRefid, iIssueType, iSet, iRow, iCol, name, value);
}

void XcTdApi::OnRecvPackEndSet(int iFunid, int iRefid, int iIssueType, int iSet)
{
    this->on_recv_pack_end_set(iFunid, iRefid, iIssueType, iSet);
}

void XcTdApi::OnRecvPackEndRow(int iFunid, int iRefid, int iIssueType, int iSet, int iRow)
{
    this->on_recv_pack_end_row(iFunid, iRefid, iIssueType, iSet, iRow);
}


//////////////////////////////////////////////////////////////////////////
void XcTdApi::create_td_api(int async_mode, int data_proto)
{
    this->api = CXcTradeApi::CreateTradeApi();
    this->api->Register(async_mode, data_proto, this);
}

void XcTdApi::release()
{
    if (this->api)
    {
        this->api->Release();
        this->api = NULL;
    }
}

int XcTdApi::connect(std::string server_port, std::string license_path, std::string fund_account)
{
    int rv;
    XcDebugInfo(XcDbgFd, "xctdapi init user_id:%s, server_port:%s, liscense:%s\n",
        fund_account.c_str(), server_port.c_str(), license_path.c_str());

    // keep it
    account_id = fund_account;

    // write_data(0, "xctdapi connecting to server_port:%s, license_path:%s, fund_account:%s",
    //     server_port.c_str(), license_path.c_str(), fund_account.c_str());
    rv = this->api->Connect((char*)server_port.c_str(), (char*)license_path.c_str(), System_UFX, (char*)fund_account.c_str());
    if (rv < 0)
    {
        XcDebugInfo(XcDbgFd, "xctdapi Connect to %s failed:%d\n", server_port.c_str(), rv);
        write_data(0, "xctdapi %s connect to %s failed rv:%d", fund_account.c_str(), server_port.c_str(), rv);
    }

    return rv;
}

void XcTdApi::begin_pack(void)
{
    api->BeginPack();
}

void XcTdApi::end_pack(void)
{
    api->EndPack();
}

int XcTdApi::set_pack_value(const std::string& key_name, const std::string& value)
{
    return api->SetPackValue(key_name.c_str(), value.c_str());
}

int XcTdApi::set_json_value(const std::string& json_str)
{
    return api->SetJsonValue(json_str.c_str());
}

int XcTdApi::send_msg(int func_id, int subsystem_no, int branch_no)
{
    int rv = api->SendMsg(func_id, subsystem_no, branch_no);
    return rv;
}

int XcTdApi::send_json_data(int func_id, const std::string& data, int subsystem_no, int branch_no)
{
    // write_data(0, "send_data func_id:%d(%s),subsystem_no:%d,branch_no:%d,data:\n%s",
    //     func_id, func_id_desc(func_id), subsystem_no, branch_no, data.c_str());

    int rv;
    rv = api->SetJsonValue(data.c_str());
    if (rv < 0) {
        return rv;
    }

    rv = api->SendMsg(func_id, subsystem_no, branch_no);
    return rv;
}

int XcTdApi::recv_msg()
{
    return api->RecvMsg();
}

int XcTdApi::get_data_set_count()
{
    return api->GetDataSetCount();
}

int XcTdApi::get_cur_row_count()
{
    return api->GetCurRowCount();
}

int XcTdApi::get_cur_col_count()
{
    return api->GetCurColCount();
}

int XcTdApi::get_cur_data_set(int index)
{
    return api->GetCurDataSet(index);
}

std::string XcTdApi::get_col_name(int col)
{
    char name[1024] = "";
    if (api->GetColName(col, name))
    {
        return std::string(name);
    }
    return to_utf(name);
}

std::string XcTdApi::get_col_value(int col)
{
    char value[4096] = "";
    if (api->GetColValue(col, value))
    {
    }
    return to_utf(value);
}

void XcTdApi::get_next_row()
{
    api->GetNextRow();
}

std::string XcTdApi::get_json_value(int len)
{
    char* buf = new char[len + 1];
    api->GetJsonValue(buf);
    buf[len] = '\0';

    std::string msg = to_utf(buf);
    return msg;
}

std::string XcTdApi::recv_json_data()
{
    int ret;
    ret = api->RecvMsg();
    if (ret <= 0)
    {
        // error
        return "";
    }

    return get_json_value(ret);
}

int XcTdApi::get_space()
{
    if (api)
    {
        return api->GetSpace();
    }
    return -1;
}

int XcTdApi::get_frequency()
{
    if (api)
    {
        return api->GetFrequency();
    }
    return -1;
}

std::string XcTdApi::get_last_error_msg()
{
    if (api)
    {
        std::string errmsg = to_utf(api->GetLastErrorMsg());
        return errmsg;
    }
    return std::string("");
}

std::string XcTdApi::get_api_version()
{
    return std::string(XC_TDAPI_VERSION);
}

int XcTdApi::open_debug(std::string log_file, int log_level)
{
    (void)log_level;

    if (fp)
    {
        fflush(fp);
        fclose(fp);
        fp = NULL;
    }

    time_t now = time(NULL);
    struct tm* ptm = localtime(&now);
    char filename[512] = "";
    if (log_file.empty())
    {
        sprintf(filename, "xctdapi_%04d%02d%02d.con",
            ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday);
    }
    else
    {
        strncpy(filename, log_file.c_str(), sizeof(filename) - 1);
    }

    fp = fopen(filename, "a+");
    if (!fp)
    {
        XcDebugInfo(XcDbgFd, "open serialize file:%s failed!\n", filename);
        return -1;
    }

    return 0;
}

std::string XcTdApi::get_field_buy()
{
    return XC_ENTRUST_TYPE_BUY;
}
std::string XcTdApi::get_field_sell()
{
    return XC_ENTRUST_TYPE_SELL;
}

std::string XcTdApi::get_field_exchange_sse()
{
    return XC_EXCHANGE_TYPE_SSE;
}
std::string XcTdApi::get_field_exchange_szse()
{
    return XC_EXCHANGE_TYPE_SZSE;
}

int XcTdApi::write_line(int reserve, const std::string& line)
{
    (void)reserve;
    if (!fp)
    {
        return -1;
    }

#if 1
    char buf[1000] = "";
    int  len = 0;

    time_t now = time(NULL);
    struct tm* ptm = localtime(&now);
    len = sprintf(buf + len, "%04d-%02d-%02d %02d:%02d:%02d \n",
        ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,
        ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

    fwrite(buf, len, 1, fp);
    fwrite(line.c_str(), line.length(), 1, fp);
    fwrite("\n", 1, 1, fp);
    fflush(fp);
#endif
    return 0;
}

int XcTdApi::write_data(int reserve, const char* fmt, ...)
{
    (void)reserve;
    if (!fp)
    {
        return -1;
    }

#if 1
    char buf[16300] = "";
    int  len = 0;

    time_t now = time(NULL);
    struct tm* ptm = localtime(&now);
    len = sprintf(buf + len, "%04d-%02d-%02d %02d:%02d:%02d \n",
        ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,
        ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

    va_list args;
    va_start(args, fmt);
    len += vsnprintf(buf + len, sizeof(buf) - len - 2, fmt, args);
    va_end(args);
    // [len++] = '\r';
    buf[len++] = '\n';
    buf[len] = '\0';

    fwrite(buf, len, 1, fp);
    fflush(fp);
#endif
    return 0;
}

///-------------------------------------------------------------------------------------
///Boost.Python封装
///-------------------------------------------------------------------------------------

class PyTdApi : public XcTdApi
{
public:
    using XcTdApi::XcTdApi;

    void on_front_connected() override
    {
        try
        {
            PYBIND11_OVERLOAD(void, XcTdApi, on_front_connected);
        }
        catch (const error_already_set &e)
        {
            std::cout << "on_front_connected: " << e.what() << std::endl;
        }
    };

    void on_front_disconnected(int reqid) override
    {
        try
        {
            PYBIND11_OVERLOAD(void, XcTdApi, on_front_disconnected, reqid);
        }
        catch (const error_already_set &e)
        {
            std::cout << "on_front_disconnected: " << e.what() << std::endl;
        }
    };

    void on_recv_msg(const std::string& msg) override
    {
        try
        {
            PYBIND11_OVERLOAD(void, XcTdApi, on_recv_msg, msg);
        }
        catch (const error_already_set &e)
        {
            std::cout << "on_recv_msg: " << e.what() << std::endl;
        }
    }

    void on_recv_pack_msg(int iFunid, int iRefid, int iIssueType, int iSet, int iRow, int iCol, const std::string& name, const std::string& value) override
    {
        try
        {
            PYBIND11_OVERLOAD(void, XcTdApi, on_recv_pack_msg, iFunid, iRefid, iIssueType, iSet, iRow, iCol, name, value);
        }
        catch (const error_already_set &e)
        {
            std::cout << "on_recv_pack_msg: " << e.what() << std::endl;
        }
    }

    void on_recv_pack_end_set(int iFunid, int iRefid, int iIssueType, int iSet) override
    {
        try
        {
            PYBIND11_OVERLOAD(void, XcTdApi, on_recv_pack_end_set, iFunid, iRefid, iIssueType, iSet);
        }
        catch (const error_already_set &e)
        {
            std::cout << "on_recv_pack_end_set: " << e.what() << std::endl;
        }
    }

    void on_recv_pack_end_row(int iFunid, int iRefid, int iIssueType, int iSet, int iRow) override
    {
        try
        {
            PYBIND11_OVERLOAD(void, XcTdApi, on_recv_pack_end_row, iFunid, iRefid, iIssueType, iSet, iRow);
        }
        catch (const error_already_set &e)
        {
            std::cout << "on_recv_pack_end_row: " << e.what() << std::endl;
        }
    }

};


PYBIND11_MODULE(xctd, m)
{
    class_<XcTdApi, PyTdApi> tdapi(m, "XcTdApi", module_local());
    tdapi
        .def(init<>())
        .def("create_td_api", &XcTdApi::create_td_api)
        .def("create_api", &XcTdApi::create_td_api)
        .def("release", &XcTdApi::release)
        .def("connect", &XcTdApi::connect)
        .def("begin_pack", &XcTdApi::begin_pack)
        .def("end_pack", &XcTdApi::end_pack)
        .def("set_pack_value", &XcTdApi::set_pack_value)
        .def("set_json_value", &XcTdApi::set_json_value)
        .def("send_msg", &XcTdApi::send_msg)
        .def("recv_msg", &XcTdApi::recv_msg)
        .def("get_data_set_count", &XcTdApi::get_data_set_count)
        .def("get_cur_row_count", &XcTdApi::get_cur_row_count)
        .def("get_cur_col_count", &XcTdApi::get_cur_col_count)
        .def("get_cur_data_set", &XcTdApi::get_cur_data_set)
        .def("get_col_name", &XcTdApi::get_col_name)
        .def("get_col_value", &XcTdApi::get_col_value)
        .def("get_next_row", &XcTdApi::get_next_row)
        .def("get_json_value", &XcTdApi::get_json_value)
        .def("get_space", &XcTdApi::get_space)
        .def("get_frequency", &XcTdApi::get_frequency)
        .def("get_last_error_msg", &XcTdApi::get_last_error_msg)

        .def("send_json_data", &XcTdApi::send_json_data)
        .def("recv_json_data", &XcTdApi::recv_json_data)
        .def("get_api_version", &XcTdApi::get_api_version)
        .def("open_debug", &XcTdApi::open_debug)

        .def("get_field_buy", &XcTdApi::get_field_buy)
        .def("get_field_sell", &XcTdApi::get_field_sell)
        .def("get_field_exchange_sse", &XcTdApi::get_field_exchange_sse)
        .def("get_field_exchange_szse", &XcTdApi::get_field_exchange_szse)
        .def("write_line", &XcTdApi::write_line)

        .def("on_front_connected", &XcTdApi::on_front_connected)
        .def("on_front_disconnected", &XcTdApi::on_front_disconnected)
        .def("on_recv_msg", &XcTdApi::on_recv_msg)
        .def("on_recv_pack_msg", &XcTdApi::on_recv_pack_msg)
        .def("on_recv_pack_end_set", &XcTdApi::on_recv_pack_end_set)
        .def("on_recv_pack_end_row", &XcTdApi::on_recv_pack_end_row)
        ;
}
