#include "VGTable.h"
#if !defined _WIN32 && !defined _WIN64
#include <mysql/mysql.h>
#else
#include <mysql.h>
#endif
#include <map>
#include <string.h>

#if !defined _WIN32 && !defined _WIN64
#include <strings.h>
#define strnicmp strncasecmp
#endif
#include "VGDBManager.h"
#include <tinyxml.h>

////////////////////////////////////////////////////////////////////////////////
//VGConstraint
////////////////////////////////////////////////////////////////////////////////
class VGForeignKey
{
public:
    bool IsValid()const
    {
        return m_bValid;
    }
    string GetForeignStr()const
    {
        return m_tableForeign + "(" + m_keyForeign + ")";
    }
    string ToString() const
    {
        if (!IsValid())
            return string();

        string foreign = GetForeignStr();
        string ret;
        ret.resize(40 + foreign.length() + m_keyLocal.length() * 2);
        sprintf(&ret.at(0), "constraint %s foreign key(%s) references %s"
            , m_keyLocal.c_str(), m_keyLocal.c_str(), foreign.c_str());

        return string(ret.c_str());
    }
public:
    static VGForeignKey *ParseForeignKey(TiXmlElement &e, VGTable *parent)
    {
        const char *tmp = e.Attribute("name");
        if (!tmp || !parent)
            return NULL;

        string local = tmp;
        tmp = e.Attribute("foreign");
        if (!tmp || !parent->FindFieldByName(local))
            return NULL;

        string foreign = tmp;
        tmp = e.Attribute("ref");
        if (!tmp)
            return NULL;

        if (VGTable *tb = VGDBManager::Instance().GetTableByName(tmp))
        {
            if (!tb->FindFieldByName(foreign))
                return NULL;

            if(VGForeignKey *ret = new VGForeignKey())
            {
                ret->m_bValid = true;
                ret->m_keyForeign = foreign;
                ret->m_keyLocal = local;
                ret->m_tableForeign = tmp;
                return ret;
            }
        }
        return NULL;
    }
protected:
    VGForeignKey() :m_bValid(false){}
private:
    bool    m_bValid;
    string  m_keyForeign;
    string  m_keyLocal;
    string  m_tableForeign;
};
////////////////////////////////////////////////////////////////////////////////
//VGTableField
////////////////////////////////////////////////////////////////////////////////
VGTableField::VGTableField(VGTable *tb, const string &n)
: m_table(tb), m_type(0), m_name(n)
{
}

string VGTableField::GetName(bool bTable) const
{
    if(bTable && m_table)
        return m_table->GetName()+"."+m_name;

    return m_name;
}

void VGTableField::SetName(const string &n)
{
    m_name = n;
}

void VGTableField::SetConstrains(const string &n)
{
    m_constraints.clear();
    
    for (const string &itr : VGDBManager::SplitString(n, ";"))
    {
        if (itr.length()>0)
            m_constraints.push_back(itr);
    }
}

const StringList & VGTableField::GetConstrains() const
{
    return m_constraints;
}

const string &VGTableField::GetTypeName()const
{
    return m_typeName;
}

void VGTableField::SetTypeName(const string &type)
{
    m_type = 0;
    if (0 == strnicmp(type.c_str(), "TINYINT", 8))
        m_type = (1 << 16) | MYSQL_TYPE_TINY;
    else if (0 == strnicmp(type.c_str(), "SMALLINT", 9))
        m_type = (2 << 16) | MYSQL_TYPE_SHORT;
    else if (0 == strnicmp(type.c_str(), "INTEGER", 8) || 0 == strnicmp(type.c_str(), "INT", 4))
        m_type = (4 << 16) | MYSQL_TYPE_LONG;
    else if (0 == strnicmp(type.c_str(), "BIGINT", 8))
        m_type = (8 << 16) | MYSQL_TYPE_LONGLONG;
    else if (0 == strnicmp(type.c_str(), "FLOAT", 6))
        m_type = (4 << 16) | MYSQL_TYPE_FLOAT;
    else if (0 == strnicmp(type.c_str(), "DOUBLE", 6))
        m_type = (8 << 16) | MYSQL_TYPE_DOUBLE;
    else if (0 == strnicmp(type.c_str(), "DATE", 5))
        m_type = (4 << 16) | MYSQL_TYPE_DATE;
    else if (0 == strnicmp(type.c_str(), "TIME", 5))
        m_type = (4 << 16) | MYSQL_TYPE_TIME;
    else if (0 == strnicmp(type.c_str(), "DATETIME", 9))
        m_type = (8 << 16) | MYSQL_TYPE_DATETIME;
    else if (0 == strnicmp(type.c_str(), "TIMESTAMP", 10))
        m_type = (4 << 16) | MYSQL_TYPE_DATETIME;
    else if (0 == strnicmp(type.c_str(), "CHAR", 4))
        _parseChar(type.substr(4));
    else if (0 == strnicmp(type.c_str(), "VARCHAR", 7))
        _parseVarChar(type.substr(7));
    else if (0 == strnicmp(type.c_str(), "BINARY", 6))
        _parseBinary(type.substr(6));
    else if (0 == strnicmp(type.c_str(), "VARBINARY", 9))
        _parseVarBinary(type.substr(9));
    else if (0 == strnicmp(type.c_str(), "BLOB", 8))
        m_type = (1 << 16) | MYSQL_TYPE_BLOB;
    else if (0 == strnicmp(type.c_str(), "TEXT", 5))
        m_type = (1 << 16) | MYSQL_TYPE_STRING;

    if (m_type != 0)
        m_typeName = type;
}

int VGTableField::GetType()
{
    return m_type & 0xff;
}

int VGTableField::GetLength() const
{
    return (m_type >> 16) & 0x00ffff;
}

bool VGTableField::IsValid() const
{
    return m_name.length() > 0 && GetLength() > 0 && m_table;
}

string VGTableField::ToString() const
{
    if (!IsValid())
        return string();

    string ret = m_name + " " + m_typeName;
    for (const string &itr : m_constraints)
    {
        ret += " " + itr;
    }
    return ret;
}

VGTableField *VGTableField::ParseFiled(TiXmlElement &e, VGTable *parent)
{
    const char *tmp = e.Attribute("name");
    if (!tmp || !parent)
        return NULL;

    if (VGTableField *fd = new VGTableField(parent, tmp))
    {
        if (const char *s = e.Attribute("type"))
            fd->SetTypeName(s);
        if (const char *s = e.Attribute("constraint"))
            fd->SetConstrains(s);

        if (fd->IsValid())
            return fd;
        else
            delete fd;
    }

    return NULL;
}

void VGTableField::_parseChar(const string &n)
{
    if (n.length() < 3 || n.at(0) != '(' || *(--n.end()) != ')')
        return;

    int nTmp = VGDBManager::str2int(n.substr(1, n.length()-2));
    if(nTmp>0 && nTmp<256)
        m_type = (nTmp << 16) | MYSQL_TYPE_STRING;
}

void VGTableField::_parseVarChar(const string &n)
{
    if (n.length() < 3 || n.at(0) != '(' || *(--n.end()) != ')')
        return;

    int nTmp = VGDBManager::str2int(n.substr(1, n.length() - 2));
    if (nTmp > 0 && nTmp<0xffff)
        m_type = (nTmp << 16) | MYSQL_TYPE_VAR_STRING;
}

void VGTableField::_parseBinary(const string &n)
{
    if (n.length() < 3 || n.at(0) != '(' || *(--n.end()) != ')')
        return;

    int nTmp = VGDBManager::str2int(n.substr(1, n.length() - 2));
    if (nTmp > 0 && nTmp<256)
        m_type = (nTmp << 16) | MYSQL_TYPE_VAR_STRING;
}

void VGTableField::_parseVarBinary(const string &n)
{
    if (n.length() < 3 || n.at(0) != '(' || *(--n.end()) != ')')
        return;

    int nTmp = VGDBManager::str2int(n.substr(1, n.length() - 2));
    if (nTmp > 0 && nTmp<0xffff)
        m_type = (nTmp << 16) | MYSQL_TYPE_VAR_STRING;
}
////////////////////////////////////////////////////////////////////////////////
//VGTableField
////////////////////////////////////////////////////////////////////////////////
VGTable::VGTable(const string &n/*=string()*/) : m_name(n)
{
}

void VGTable::AddForeign(VGForeignKey *f)
{
    if (f)
        m_foreigns.push_back(f);
}

const string &VGTable::GetName() const
{
    return m_name;
}

const list<VGTableField *> &VGTable::Fields() const
{
    return m_fields;
}

VGTableField *VGTable::FindFieldByName(const string &n)
{
    for (VGTableField *itr : m_fields)
    {
        if (n == itr->GetName())
            return itr;
    }
    return NULL;
}

string VGTable::ToCreateSQL() const
{
    string ret;
    for (VGTableField *itr : m_fields)
    {
        ret += ret.length() > 0 ? ", ":"(";
        ret += itr->ToString();
    }

    for (VGForeignKey *itr : m_foreigns)
    {
        ret += ", " + itr->ToString();
    }
    ret += ")";

    return string("CREATE TABLE IF NOT EXISTS ") + m_name + ret;
}

void VGTable::AddField(VGTableField *fd)
{
    if (!fd || !fd->IsValid())
        return;

    for (VGTableField *itr : m_fields)
    {
        if (fd == itr || fd->GetName() == itr->GetName())
            return;
    }
    m_fields.push_back(fd);
}

VGTable *VGTable::ParseTable(TiXmlElement &e)
{
    const char *tmp = e.Attribute("name");
    string name = tmp ? tmp : string();
    if (name.length() < 1)
        return NULL;

    VGTable *tb = new VGTable(name);
    TiXmlNode *field = e.FirstChild("field");
    while (field)
    {
        if(VGTableField *fd = VGTableField::ParseFiled(*field->ToElement(), tb))
            tb->AddField(fd);

        field = field->NextSibling("field");
    }

    field = e.FirstChild("foreign");
    while (field)
    {
        if(VGForeignKey *fk = VGForeignKey::ParseForeignKey(*field->ToElement(), tb))
            tb->AddForeign(fk);

        field = field->NextSibling("foreign");
    }
    return tb;
}
