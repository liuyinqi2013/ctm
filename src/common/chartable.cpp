#include <assert.h>
#include <math.h>
#include "chartable.h"

namespace ctm
{
    string StrNum(char c, size_t n)
    {
        string tmp;
        for (size_t i = 0; i < n; ++i)
        {
            tmp += c;
        }
        return tmp;
    }

    string FixedString(const string& strIn, size_t fixedLen, int align, char padding)
    {
        if (strIn.size() >= fixedLen) return strIn.substr(0, fixedLen);

        if (align == 1) 
            return strIn + StrNum(padding, fixedLen - strIn.size());
        else if (align == 2)
            return StrNum(padding, fixedLen - strIn.size()) + strIn;
        
        return StrNum(padding, floor((fixedLen - strIn.size()) / 2.0)) + strIn + StrNum(padding, ceil((fixedLen - strIn.size())/2.0));
    }

    CStyle CCharTable::default_style(CStyle::HCENTER | CStyle::VCENTER);

    CCharTable::CCharTable(size_t rowCnt, size_t colCnt)
    {
        m_top = default_top;
        m_side = default_side;
        m_corner = default_corner;
        m_style = &default_style;
        m_rowCnt = rowCnt;
        m_colCnt = colCnt;
        m_gap = 1;

        for (size_t i = 0; i < m_rowCnt; ++i)
        {
            m_rowVec.push_back(new CRow(this));
        }

        for (size_t i = 0; i < m_colCnt; ++i)
        {
            m_colVec.push_back(new CColumn(this));
        }

        for (size_t row = 0; row < m_rowCnt; ++row)
        {
            for (size_t col = 0; col < m_colCnt; ++col) {
                CCell* cell = new CCell(m_rowVec[row], m_colVec[col]);
                m_rowVec[row]->Add(cell);
                m_colVec[col]->Add(cell);
                m_cellList.push_back(cell);
            }
        }
    }

    CCharTable::~CCharTable()
    {
        for (size_t i = 0; i < m_rowVec.size(); ++i)
        {
            delete m_rowVec[i];
        }

        for (size_t i = 0; i < m_colVec.size(); ++i)
        {
            delete m_colVec[i];
        }

        list<CCell*>::iterator it = m_cellList.begin();
        for (; it != m_cellList.end(); it++)
        {
            delete *it;
        }

        for (size_t i = 0; i < m_styleVec.size(); ++i)
        {
            delete m_styleVec[i];
        }
    }

    size_t CCharTable::Hight() const
    {
        return 0;
    }

    size_t CCharTable::Width() const
    {
        return 0;
    }

    CCell* CCharTable::Cell(size_t row, size_t col)
    {
        return m_rowVec[row]->Cell(col);
    }

    CRow* CCharTable::Row(size_t row)
    {
        return m_rowVec[row];
    }

    CColumn* CCharTable::Column(size_t col)
    {
        return m_colVec[col];
    }

    void CCharTable::Write(size_t row, size_t col, const string& text)
    {
        Cell(row, col)->SetText(text);
    }

    void CCharTable::ClearText()
    {

    }

    string CCharTable::ToString() const
    {
        string topLine = TopLine();
        string text = "\n" + topLine + '\n';
        string line;
        for (size_t row = 0; row < m_rowVec.size(); ++row)
        {
            for (size_t i = 0; i < m_rowVec[row]->m_hight; ++i)
            {
                line = m_side;
                for (size_t col = 0; col < m_colVec.size(); ++col){
                    line += StrNum(' ', m_gap) + m_rowVec[row]->Cell(col)->LineString(i) + StrNum(' ', m_gap) + m_side;
                }
                text += line + "\n";
            }
            text += topLine + "\n";
        }

        return text;
    }

    void CCharTable::Print(FILE* out) const
    {
        fprintf(out, ToString().c_str());
    }

    void CCharTable::Print(ostream& out)
    {
        out << ToString();
    }

    CStyle* CCharTable::CreateStyle()
    {
        CStyle* style = new CStyle;
        m_styleVec.push_back(style);

        return style;
    }

    string CCharTable::TopLine() const
    {
        string topLine =  StrNum(m_corner, 1);
        for (size_t i = 0; i < m_colVec.size(); ++i)
        {
            topLine += StrNum(m_top, m_colVec[i]->m_width + 2 * m_gap) + m_corner;
        }
        return topLine;
    }

    void CRow::ClearText()
    {
        for (size_t i = 0; m_cellVec.size(); ++i)
        {
            m_cellVec[i]->ClearText();
        }
    }

    void CColumn::ClearText()
    {
        for (size_t i = 0; m_cellVec.size(); ++i)
        {
            m_cellVec[i]->ClearText();
        }
    }

    string CCell::LineString(size_t line)
    {
        CStyle* style = Style();
        if (style == NULL) style = &CCharTable::default_style;
        size_t width = m_pCol->m_width;
        size_t begin = line * width;
        size_t cnt = (m_text.size() + width - 1) / width;

        if (begin >= m_text.size()) 
        {
            return StrNum(' ', width);
        }
        else if(begin < m_text.size() && begin + width >= m_text.size())
        {
        }
        
        return StrNum(' ', width);
    }

}
