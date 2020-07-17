#ifndef CTM_COMMON_CHAR_TABLE_H__
#define CTM_COMMON_CHAR_TABLE_H__
#include <stdlib.h>
#include <string>
#include <list>
#include <vector>
#include <iostream>
#include <stdio.h>

namespace ctm
{
    class CCell;
    class CRow;
    class CColumn;
    class CStyle;

    using namespace std;

    string StrNum(char c, size_t n);
    string FixedString(const string& strIn, size_t fixedLen, int align, char padding = ' ');

    class CCharTable
    {
        friend class CRow;
        friend class CCell;
        friend class CColumn;
    public:
        const static char default_top = '-';
        const static char default_side = '|';
        const static char default_corner = '+';
        static CStyle default_style;

        CCharTable(size_t rowCnt, size_t colCnt);
        ~CCharTable();

        size_t Hight() const;
        size_t Width() const;

        CCell* Cell(size_t row, size_t col);
        CRow* Row(size_t row);
        CColumn* Column(size_t col);

        void Write(size_t row, size_t col, const string& text);
        void ClearText();

        string ToString() const;
        void Print(FILE* out = stdout) const;
        void Print(ostream& out);

        CStyle* CreateStyle();
        void SetStyle(CStyle* style) { m_style = style; }
        string TopLine() const;
    
    public:
        string m_tabName;
        char m_top;
        char m_side;
        char m_corner;
    
    protected:
        size_t m_rowCnt;
        size_t m_colCnt;
        CStyle* m_style;
        vector<CRow*> m_rowVec;
        vector<CColumn*> m_colVec;
        vector<CStyle*> m_styleVec;
        list<CCell*> m_cellList;
        size_t m_gap;
    };

    class CRow
    {
        friend class CCell;
        friend class CCharTable;
    public:
        static const size_t max_hight = 16;
        static const size_t default_hight = 1;

        ~CRow() {}

        void SetHight(size_t hight) { m_hight = std::min(hight, max_hight); }
        void SetStyle(CStyle* style) { m_style = style; }
        CCell* Cell(size_t col) { return m_cellVec[col]; }
        void Add(CCell* cell) { m_cellVec.push_back(cell); }

        void ClearText();

        CCell* operator [](size_t col) { return m_cellVec[col]; }

    private:
        CRow(CCharTable* pParent) 
        : m_pParent(pParent), m_hight(default_hight), m_style(NULL) {}

    private:  
        CCharTable* m_pParent;
        size_t m_hight;
        CStyle* m_style;
        vector<CCell*> m_cellVec;
    };

    class CColumn
    {
        friend class CCell;
        friend class CCharTable;
    public:
        static const size_t max_width = 128;
        static const size_t default_width = 8;

        ~CColumn() { }

        void SetWidth(size_t width) { m_width = std::min(width, max_width); }
        void SetStyle(CStyle* style) { m_style = style; }
        CCell* Cell(size_t row) { return m_cellVec[row]; }
        void Add(CCell* cell) { m_cellVec.push_back(cell); }

        void ClearText();

        CCell* operator [](size_t row) { return m_cellVec[row]; }

    private:
        CColumn(CCharTable* pParent)
        : m_pParent(pParent), m_width(default_width), m_style(NULL) {}

    private:
        CCharTable* m_pParent;
        size_t m_width;
        CStyle* m_style;
        vector<CCell*> m_cellVec;
    };

    class CCell
    {
        friend class CRow;
        friend class CColumn;
        friend class CCharTable;
    public:
        ~CCell() {}

        void ClearText() { m_text.clear(); m_offset = 0; }
        void SetText(const string& text) { m_text = text; }
        void SetStyle(CStyle* style) { m_style = style; }

        string LineString(size_t line);

    private:
        CCell(CRow* pRow, CColumn* pCol) 
        : m_pPow(pRow), m_pCol(pCol), m_text(""), m_style(NULL), m_offset(0) { }

        CStyle* Style() 
        {
            if (m_style) return m_style;
            if (m_pCol->m_style) return m_pCol->m_style;
            if (m_pPow->m_style) return m_pPow->m_style;
            return m_pPow->m_pParent->m_style;
        }

    protected:
        CRow* m_pPow;
        CColumn* m_pCol;
        string m_text;
        CStyle* m_style;
        size_t m_offset;
    };

    class CStyle
    {
        friend class CCharTable;
    public:
        enum Alignment
        {
            TOP = 0x01,
            BOTTOM = 0x02,
            HCENTER = 0x04,
            LIFT = 0x10,
            RIGHT = 0x20,
            VCENTER = 0x40,
        };

        int m_align;

    private:
        CStyle(int align = HCENTER | VCENTER)
        : m_align(align) {}
    };
};

#endif

