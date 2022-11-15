#include <assert.h>
#include <math.h>
#include "terminal.h"
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

        if (align == CStyle::RIGHT) 
            return StrNum(padding, fixedLen - strIn.size()) + strIn;
        else if (align == CStyle::LIFT)
            return strIn + StrNum(padding, fixedLen - strIn.size());
        
        return StrNum(padding, floor((fixedLen - strIn.size()) / 2.0)) + strIn + StrNum(padding, ceil((fixedLen - strIn.size())/2.0));
    }

    CStyle CCharTable::default_style;

    CCharTable::CCharTable(size_t rowCnt, size_t colCnt)
    {
        m_top = default_top;
        m_side = default_side;
        m_corner = default_corner;
        m_style = &default_style;
        m_rowCnt = rowCnt;
        m_colCnt = colCnt;
        m_gap = 1;
        m_isNoFrame = false;

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

    string CCharTable::ToString(bool bandColor) const
    {
        string side;
        string topLine;

        if (!m_isNoFrame)
        {
            side = m_side;
            topLine = TopLine();
        }

        string text = "\n" + topLine;
        string line;
        for (size_t row = 0; row < m_rowVec.size(); ++row)
        {
            for (size_t i = 0; i < m_rowVec[row]->m_hight; ++i)
            {
                line = side;
                for (size_t col = 0; col < m_colVec.size(); ++col) {
                    if (bandColor) {
                        line += StrNum(' ', m_gap) + m_rowVec[row]->Cell(col)->ColorLineString(i) + StrNum(' ', m_gap) + side;
                    }
                    else {
                        line += StrNum(' ', m_gap) + m_rowVec[row]->Cell(col)->LineString(i) + StrNum(' ', m_gap) + side;
                    }
                }
                text += line + "\n";
            }
            text += topLine;
        }

        return text;
    }

    void CCharTable::Print(FILE* out) const
    {
        fprintf(out, ToString(IsCharDevice(out)).c_str());
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
        return topLine + "\n";
    }

    void CRow::SetHight(size_t hight) 
    { 
        m_hight = std::min(hight, max_hight); 
        m_hight = std::max(m_hight, min_hight); 
    }

    void CRow::ClearText()
    {
        for (size_t i = 0; m_cellVec.size(); ++i)
        {
            m_cellVec[i]->ClearText();
        }
    }

    void  CColumn::SetWidth(size_t width) 
    { 
        m_width = std::min(width, max_width); 
        m_width = std::max(m_width, min_width);
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
        size_t width = m_pCol->m_width;
        size_t hight = m_pPow->m_hight;
        if (m_text.size() == 0) 
        {
            return StrNum(' ', width);
        }

        CStyle* style = Style();
        if (style == NULL) style = &CCharTable::default_style;

        size_t cnt = (m_text.size() + width - 1) / width;
        size_t beginLine = 0;
        size_t endLine = hight;

        switch (style->m_verAlign)
        {
        case CStyle::TOP:
            beginLine = 0;
            break;
        case CStyle::BOTTOM:
            beginLine = max((size_t)0, hight - cnt);
            break;
        case CStyle::VCENTER:
            beginLine = max((size_t)0, (size_t)floor((hight - cnt) / 2.0));
            break;
        default:
            beginLine = 0;
            break;
        }

        endLine = min(hight, beginLine + cnt);

        if ((int)line < (int)beginLine || (int)line >= (int)endLine) 
        {
            return StrNum(' ', width);
        }

        if ((line - beginLine < cnt - 1) || cnt > hight)
        {
            return FixedString(m_text.substr((line - beginLine) * width, width), width, style->m_horAlign);
        }
        else
        {
            return FixedString(m_text.substr((line - beginLine) * width), width, style->m_horAlign);
        }

        return StrNum(' ', width);
    }

    string CCell::ColorLineString(size_t line)
    {
        return ColorString(LineString(line), Style()->m_color);
    }

}
