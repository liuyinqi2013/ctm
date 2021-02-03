#ifndef __CTM_SERVER_LINK_H_H_
#define __CTM_SERVER_LINK_H_H_

#include <unordered_map>
#include "common/macro.h"

namespace ctm {

    class CConn;
    class CBaseGame;
    class CLinkConn;

    typedef std::unordered_map<uint32, CLinkConn*> LinkIdMAP;
    typedef std::unordered_map<uint32, LinkIdMAP>  LinkTypeMAP;
    typedef std::unordered_map<CConn*, CLinkConn*> LinkConnMAP;

    class CLinkConn
    {
    public:
        CLinkConn() 
        : id(0), type(0), conn(NULL), server(NULL) {}
        CLinkConn(uint32 id_, uint32 type_, CConn* conn_, CBaseGame* serv_) 
        : id(id_), type(type_), conn(conn_), server(serv_) {}
        
        uint32 Uid();
        int Send(uint32 protoId, char* buf, uint32 len);

        uint32 id;
        uint32 type;
        CConn* conn;
        CBaseGame* server;
    };

    class CLinkManger
    {
    public:
        CLinkManger() {}
        ~CLinkManger() ;
        CLinkConn* AddLink(uint32 id, uint32 type, ctm::CConn* conn, CBaseGame* serv);
        CLinkConn* AddLink(uint32 uid, ctm::CConn* conn, CBaseGame* serv);
        void RemoveLink(uint32 uid);
        void RemoveLink(CLinkConn* link);
        void RemoveLink(CConn* conn);
        void RemoveLink(uint32 id, uint32 type);
        CLinkConn* GetLink(uint32 uid);
        CLinkConn* GetLink(uint32 id, uint32 type);
        CLinkConn* GetLink(CConn* conn);
        LinkIdMAP& GetLinkIdMap(uint32 type);
    private:
        LinkIdMAP   m_linkMap;
        LinkConnMAP m_connMap;
        LinkTypeMAP m_linkTypeMap;
    };
}

#endif