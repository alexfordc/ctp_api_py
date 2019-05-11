//˵������

//ϵͳ
#ifdef WIN32
#include "stdafx.h"
#endif
#include <stdint.h>
#include <string>
#include <queue>

//Boost
#define BOOST_PYTHON_STATIC_LIB
#include <boost/python/module.hpp>    //python��װ
#include <boost/python/def.hpp>        //python��װ
#include <boost/python/dict.hpp>    //python��װ
#include <boost/python/list.hpp>    //python��װ
#include <boost/python/object.hpp>    //python��װ
#include <boost/python.hpp>            //python��װ
// #include <boost/thread.hpp>            //������е��̹߳���
// #include <boost/bind.hpp>            //������е��̹߳���
// #include <boost/any.hpp>            //������е�����ʵ��
// #include <boost/locale.hpp>            //�ַ���ת��
//API
#include "KDMdUserApi.h"

//�����ռ�
using namespace std;
using namespace boost::python;
using namespace boost;

//����
#define ONFRONTCONNECTED 1
#define ONFRONTDISCONNECTED 2
#define ONHEARTBEATWARNING 3
#define ONRSPUSERLOGIN 4
#define ONRSPUSERLOGOUT 5
#define ONRSPERROR 6
#define ONRSPSUBMARKETDATA 7
#define ONRSPUNSUBMARKETDATA 8
#define ONRSPSUBFORQUOTERSP 9
#define ONRSPUNSUBFORQUOTERSP 10
#define ONRTNDEPTHMARKETDATA 11
#define ONRTNFORQUOTERSP 12



//����ṹ��
struct Task
{
    int task_name;        //�ص��������ƶ�Ӧ�ĳ���
    kd_md_recved_data_t* task_data;
};


///�̰߳�ȫ�Ķ���
template<typename Data>
class ConcurrentQueue
{
private:
    queue<Data> the_queue;                              //��׼�����
    mutable mutex the_mutex;                            //boost������
    condition_variable the_condition_variable;          //boost��������

public:

    //�����µ�����
    void push(Data const& data)
    {
        mutex::scoped_lock lock(the_mutex);             //��ȡ������
        the_queue.push(data);                           //������д�������
        lock.unlock();                                  //�ͷ���
        the_condition_variable.notify_one();            //֪ͨ���������ȴ����߳�
    }

    //�������Ƿ�Ϊ��
    bool empty() const
    {
        mutex::scoped_lock lock(the_mutex);
        return the_queue.empty();
    }

    //ȡ��
    Data wait_and_pop()
    {
        mutex::scoped_lock lock(the_mutex);

        while (the_queue.empty())                        //������Ϊ��ʱ
        {
            the_condition_variable.wait(lock);           //�ȴ���������֪ͨ
        }

        Data popped_value = the_queue.front();          //��ȡ�����е����һ������
        the_queue.pop();                                //ɾ��������
        return popped_value;                            //���ظ�����
    }

};


///-------------------------------------------------------------------------------------
///API�еĲ������
///-------------------------------------------------------------------------------------

//GILȫ�����򻯻�ȡ�ã�
//���ڰ���C++�̻߳��GIL�����Ӷ���ֹpython����
class PyLock
{
private:
    PyGILState_STATE gil_state;

public:
    //��ĳ�����������д����ö���ʱ�����GIL��
    PyLock()
    {
        gil_state = PyGILState_Ensure();
    }

    //��ĳ��������ɺ����ٸö���ʱ�����GIL��
    ~PyLock()
    {
        PyGILState_Release(gil_state);
    }
};



//���ֵ��л�ȡĳ����ֵ��Ӧ������������ֵ������ṹ������ֵ��
void getInt(dict d, string key, int* value);


//���ֵ��л�ȡĳ����ֵ��Ӧ�ĸ�����������ֵ������ṹ������ֵ��
void getDouble(dict d, string key, double* value);


//���ֵ��л�ȡĳ����ֵ��Ӧ���ַ�������ֵ������ṹ������ֵ��
void getChar(dict d, string key, char* value);


//���ֵ��л�ȡĳ����ֵ��Ӧ���ַ���������ֵ������ṹ������ֵ��
void getStr(dict d, string key, char* value);


///-------------------------------------------------------------------------------------
///C++ SPI�Ļص���������ʵ��
///-------------------------------------------------------------------------------------

//API�ļ̳�ʵ��
class KDMdApi
{
private:
    kd_md_api_t* api;                   // API����
    thread* task_thread;                //�����߳�ָ�루��python���������ݣ�
    ConcurrentQueue<Task> task_queue;   //�������

public:
    KDMdApi() : api()
    {
        function0<void> f = boost::bind(&MdApi::processTask, this);
        thread t(f);
        this->task_thread = &t;
    }

    ~KDMdApi()
    {
    }

    ConcurrentQueue* taskQueue() { return &task_queue; }

    //-------------------------------------------------------------------------------------
    //API�ص�����
    //-------------------------------------------------------------------------------------
    static void mdApiHandlerStatic(kd_md_api_t* apMdApi, uint32_t aMsgType, kd_md_recved_data_t* apData);
    void mdApiHandler(kd_md_api_t* apMdApi, uint32_t aMsgType, kd_md_recved_data_t* apData);

    //-------------------------------------------------------------------------------------
    //task������
    //-------------------------------------------------------------------------------------
    void processTask()

    void processFrontConnected();

    void processFrontDisconnected(kd_md_recved_data_t* apData);

    void processRspUserLogin(kd_md_recved_data_t* apData);

    void processRspUserLogout(kd_md_recved_data_t* apData);

    void processRspSubMarketData(kd_md_recved_data_t* apData);

    void processRspUnSubMarketData(kd_md_recved_data_t* apData);

    void processRspQueryData(kd_md_recved_data_t* apData);

    void processRtnData(kd_md_recved_data_t* apData);

    //-------------------------------------------------------------------------------------
    //data���ص������������ֵ�
    //error���ص������Ĵ����ֵ�
    //id������id
    //last���Ƿ�Ϊ��󷵻�
    //i������
    //-------------------------------------------------------------------------------------

    virtual void onFrontConnected() {};

    virtual void onFrontDisconnected(int i) {};

    virtual void onRspUserLogin(dict data) {};

    virtual void onRspUserLogout(dict data) {};

    virtual void onRspSubMarketData(dict data, bool last) {};

    virtual void onRspUnSubMarketData(dict data, bool last) {};

    virtual void onRspQryData(dict data, dict error, bool last) {};

    virtual void onRtnMarketData(uint16_t aMarketId, uint16_t aServiceId, dict data) {};

    //-------------------------------------------------------------------------------------
    //req:���������������ֵ�
    //-------------------------------------------------------------------------------------

    void createMdApi(string pszFlowPath = "");

    string getApiVersion();

    void release();

    void init(uint32_t aTimeoutMS = 0);

    int exit();

    int setOption(int32_t aOptionKey, int32_t aOptionValue);

    int getOption(int32_t aOptionKey, int32_t* aOptionValue);

    void registerFront(string pszFrontAddress, uint16_t port);

    int subscribeMarketData(dict req);

    int unSubscribeMarketData(dict req);

    int unSubscribeAll();

    int reqQryData(dict req);

    int reqGetData(dict req, dict outData, int aTimeoutMS);
    
    int isConnected();

    int isLogined();

    int openDebug(string aLogFile, int aLogLevel);

    int reqUserLogin(dict req, int aTimeoutMS = 0);

    int reqUserLogout(dict req);

    void  setUserData(void* apUserData);

    void* getUserData();
};
