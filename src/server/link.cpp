#include "link.h"
#include "units.h"
#include "base_game.h"

namespace ctm {

    uint32 CLinkConn::Uid() 
    { 
        return UUID(id, type); 
    }

    int CLinkConn::Send(uint32 protoId, char* buf, uint32 len)
    {
        return server->Send(this, protoId, buf, len);
    }

    CLinkManger::~CLinkManger()
    {
        LinkIdMAP::iterator it = m_linkMap.begin();
        for(; it != m_linkMap.end(); it++)
        {
            delete it->second;
        }
        m_linkMap.clear();
        m_connMap.clear();
        m_linkTypeMap.clear();
    }

    CLinkConn* CLinkManger::AddLink(uint32 id, uint32 type, CConn* conn, CBaseGame* serv)
    {
        return AddLink(UUID(id, type), conn, serv);
    }

    CLinkConn* CLinkManger::AddLink(uint32 uid, CConn* conn, CBaseGame* serv)
    {
        CLinkConn* link = NULL;
        if (m_linkMap.find(uid) != m_linkMap.end())
        {
            link = m_linkMap[uid];
            link->server = serv;
            if (link->conn != conn)
            {
                m_connMap.erase(link->conn);
                m_connMap[conn] = link;
            }
            link->conn = conn;
        }
        else
        {
            link = new CLinkConn(GET_ID(uid), GET_TYPE(uid), conn, serv);
            m_linkMap[uid] = link;
            m_linkTypeMap[link->type][uid] = link;
            m_connMap[conn] = link;
        }
        return link;
    }

    void CLinkManger::RemoveLink(uint32 uid)
    {
        return RemoveLink(GetLink(uid));
    }

    void CLinkManger::RemoveLink(CLinkConn* link)
    {
        if (!link) return;
        m_linkMap.erase(link->Uid());
        m_connMap.erase(link->conn);
        m_linkTypeMap[link->type].erase(link->Uid());
        delete link;
    }

    void CLinkManger::RemoveLink(CConn* conn)
    {
        return RemoveLink(GetLink(conn));
    }

    void CLinkManger::RemoveLink(uint32 id, uint32 type)
    {
        return RemoveLink(GetLink(id, type));
    }

    CLinkConn* CLinkManger::GetLink(uint32 uid)
    {
        if (m_linkMap.find(uid) != m_linkMap.end())
        {
            return m_linkMap[uid];
        }
        return NULL;
    }

    CLinkConn* CLinkManger::GetLink(uint32 id, uint32 type)
    {
        return GetLink(UUID(id, type));
    }

    CLinkConn* CLinkManger::GetLink(CConn* conn)
    {
        if (m_connMap.find(conn) != m_connMap.end())
        {
            return m_connMap[conn];
        }
        return NULL;
    }

    LinkIdMAP& CLinkManger::GetLinkIdMap(uint32 type)
    {
        return m_linkTypeMap[type];
    }
}