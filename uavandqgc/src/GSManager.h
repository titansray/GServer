#ifndef  __OBJECT_UAV_H__
#define __OBJECT_UAV_H__

#include "ObjectBase.h"

namespace das {
    namespace proto {
        class RequestGSIdentityAuthentication;
        class RequestUavStatus;
        class ParcelDescription;
        class ParcelContracter;
    }
}

namespace google {
    namespace protobuf {
        class Message;
    }
}

class VGMySql;
class ExecutItem;

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

class ProtoMsg;
class ObjectGS : public IObject
{
public:
    ObjectGS(const std::string &id);
    ~ObjectGS();
    bool IsConnect()const;

    virtual void OnConnected(bool bConnected);
    void SetPswd(const std::string &pswd);
    const std::string &GetPswd()const;
    void RespondLogin(int seqno, int res);
protected:
    virtual int GetObjectType()const;
    virtual void ProcessMassage(const IMessage &msg);
    virtual int ProcessReceive(void *buf, int len);
    virtual int GetSenLength()const;
    virtual int CopySend(char *buf, int sz, unsigned form = 0);
    virtual void SetSended(int sended = -1);//-1,������
private:
    void _prcsReqUavs(const das::proto::RequestUavStatus &msg);
    void _prcsPostLand(const das::proto::ParcelDescription &msg, int ack);
    uint64_t _saveContact(const das::proto::ParcelDescription &msg, ExecutItem &item, uint64_t id);
    uint64_t _saveLand(const das::proto::ParcelDescription &msg, ExecutItem &item, uint64_t id);
    void _ackLandOfGs(const std::string &user, int ack);
    void _initialParcelDescription(das::proto::ParcelDescription *msg, const ExecutItem &item);
    das::proto::ParcelContracter *_transPC(const ExecutItem &item);
    void _prcsDeleteLand(const std::string &id, bool del, int ack);

    void _send(const google::protobuf::Message &msg);
protected:
    friend class GSManager;
    bool            m_bConnect;
    std::string     m_pswd;
    ProtoMsg        *m_p;
};

class GSManager : public IObjectManager
{
public:
    GSManager();
    ~GSManager();
    
    VGMySql *GetMySql()const;
protected:
    int GetObjectType()const;
    IObject *ProcessReceive(ISocket *s, const char *buf, int &len);
private:
    IObject *_checkLogin(ISocket *s, const das::proto::RequestGSIdentityAuthentication &rgi);
private:
    VGMySql         *m_sqlEng;
    ProtoMsg        *m_p;
};

#ifdef SOCKETS_NAMESPACE
}
#endif
#endif // __OBJECT_UAV_H__

