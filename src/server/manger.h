#ifndef CTM_SERVER_MANGER_H__
#define CTM_SERVER_MANGER_H__

#include <unordered_map>
#include <vector>
#include <stdint.h>
#include "common/singleton.h"

namespace ctm
{
    class CConn;
    typedef std::unordered_map<unsigned int, CConn*> ConnMAP;

    class CConnManger : public CSingleton<CConnManger>
    {
    public:
        CConn* GetConnById(unsigned int id);
        ConnMAP* GetConnsByType(unsigned int type);
        void SetConn(unsigned int id, unsigned int type, CConn* conn);
        void RemoveConn(unsigned int id, unsigned int type);

    private:
        ConnMAP m_idConnMap;
        std::unordered_map<unsigned int, ConnMAP> m_typeConnMaps;
    };
}

#endif