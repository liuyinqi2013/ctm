#include "manger.h"

namespace ctm
{
    CConn* CConnManger::GetConnById(unsigned int id)
    {
        if (m_idConnMap.find(id) != m_idConnMap.end())
        {
            return m_idConnMap[id];
        } 
        return NULL;
    }

    ConnMAP* CConnManger::GetConnsByType(unsigned int type)
    {
        if (m_typeConnMaps.find(type) != m_typeConnMaps.end())
        {
            return &m_typeConnMaps[type];
        } 
        return NULL;
    }

    void CConnManger::SetConn(unsigned int id, unsigned int type, CConn* conn)
    {
        m_idConnMap[id] = conn;
        m_typeConnMaps[type][id] = conn;
    }

    void CConnManger::RemoveConn(unsigned int id, unsigned int type)
    {
        m_idConnMap.erase(id);
        if (m_typeConnMaps.find(type) != m_typeConnMaps.end())
        {
            m_typeConnMaps[type].erase(id);
        } 
    }
}