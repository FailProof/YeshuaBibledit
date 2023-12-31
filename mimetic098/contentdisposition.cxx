#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"

#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Wuseless-cast"

#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Wuseless-cast"

#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Wuseless-cast"

#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wswitch-default"

#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsuggest-override"

#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wold-style-cast"

#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wold-style-cast"

/***************************************************************************
    copyright            : (C) 2002-2008 by Stefano Barbato
    email                : stefano@codesink.org

    $Id: contentdisposition.cxx,v 1.4 2008-10-07 11:06:25 tat Exp $
 ***************************************************************************/
#include <mimetic098/contentdisposition.h>
#include <mimetic098/tokenizer.h>
#include <mimetic098/utils.h>

namespace mimetic
{
using namespace std;

const char ContentDisposition::label[] = "Content-Disposition";

ContentDisposition::ContentDisposition()
{
}


ContentDisposition::ContentDisposition(const char* cstr)
{
    set(cstr);
}

ContentDisposition::ContentDisposition(const string& val)
{
    set(val);
}

void ContentDisposition::type(const string& stype)
{
    m_type.assign(stype);
}

const istring& ContentDisposition::type() const
{
    return m_type;
}

const ContentDisposition::ParamList& ContentDisposition::paramList() const
{
    return m_paramList;
}

ContentDisposition::ParamList& ContentDisposition::paramList()
{
    return m_paramList;
}

const string& ContentDisposition::param(const string& field) const
{
    ParamList::const_iterator bit = m_paramList.begin(),  eit = m_paramList.end();
    for(; bit != eit; ++bit)
    {
        if(bit->name() == field)
            return bit->value();    
    }
    return nullstring;
}

void ContentDisposition::param(const std::string& name, const std::string& val)
{
    ParamList::iterator bit = m_paramList.begin(),  eit = m_paramList.end();
    for(; bit != eit; ++bit)
    {
        if(bit->name() == name)
        {
            bit->value(val);    
            return;
        }
    }
    m_paramList.push_back(Param(name, val));
}

void ContentDisposition::set(const string& val)
{
    string type;
    StringTokenizer stok(&val, ";");
    if(!stok.next(type))
        return;
    m_type.assign(type);

    string sparam;
    while(stok.next(sparam))
    {
        Param p(sparam);
        m_paramList.push_back(p);
    }
}

string ContentDisposition::str() const
{
    string ostr = m_type;
    ParamList::const_iterator bit, eit;
    bit = m_paramList.begin();
    eit = m_paramList.end();
    for(; bit != eit; ++bit)
        ostr += "; " + bit->name() +  "=\"" 
               + bit->value() + "\"";
    return ostr;
}

ostream& ContentDisposition::write(ostream& os, int fold) const
{
    os << "Content-Disposition: " << m_type;
    ParamList::const_iterator bit, eit;
    bit = m_paramList.begin();
    eit = m_paramList.end();
    for(; bit != eit; ++bit)
        if(fold)
            os << ";" << crlf << "\t" << bit->name() 
               << "=\"" << bit->value() << "\"";
        else
            os << "; " << bit->name() << "=\"" 
               << bit->value() << "\"";

    os << crlf;
    return os;
}


FieldValue* ContentDisposition::clone() const
{
    return new ContentDisposition(*this);
}

}
