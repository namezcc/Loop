# -*- coding: UTF-8 -*-
import os
import xlrd

class UP_FLAG:
    up_db = 1
    up_pri = 2
    up_pub = 4
    up_eve = 8

class SQL_TYPE:
    SQL_INT = 0
    SQL_BIGINT = 1
    SQL_VARCHAR = 2
    SQL_TIMESTAMP = 3

TypeMap = {
    "bool":"bool",
    "int8":"int8_t",
    "int16":"int16_t",
    "int32":"int32_t",
    "int64":"int64_t",
    "uint8":"uint8_t",
    "uint16":"uint16_t",
    "uint32":"uint32_t",
    "uint64":"uint64_t",
    "string":"std::string",
    "float":"float",
    "double":"double",
}

SqlTypeMap = {
    "int":SQL_TYPE.SQL_INT,
    "bigint":SQL_TYPE.SQL_BIGINT,
    "varchar":SQL_TYPE.SQL_VARCHAR,
    "timestamp":SQL_TYPE.SQL_TIMESTAMP,
}

SqlTypeStr = {
    SQL_TYPE.SQL_INT:"SQL_FIELD_TYPE::SQL_INT",
    SQL_TYPE.SQL_BIGINT:"SQL_FIELD_TYPE::SQL_BIGINT",
    SQL_TYPE.SQL_VARCHAR:"SQL_FIELD_TYPE::SQL_VARCHAR",
    SQL_TYPE.SQL_TIMESTAMP:"SQL_FIELD_TYPE::SQL_TIMESTAMP",
}

BoolStr = {
    False:"false",
    True:"true",
}

class objectField:
    def __init__(self,row,nid):
        self._id = nid
        self._name = row["Name"].value
        self._type = TypeMap[row["Type"].value]
        self._defval = self.CheckStr(row,"DefVal")
        isPri = self.CheckBool(row,"Private")
        isPub = self.CheckBool(row,"Public")
        isEve = self.CheckBool(row,"Event")
        isDb = self.CheckBool(row,"Db")
        flag = 0
        if isPri:
            flag |= UP_FLAG.up_pri
        if isPub:
            flag |= UP_FLAG.up_pub
        if isEve:
            flag |= UP_FLAG.up_eve
        if isDb:
            flag |= UP_FLAG.up_db
        self._isdb = isDb
        self._flag = flag
        self._primary = self.CheckBool(row,"Primary")
        sqltype = self.CheckStr(row,"SqlType")
        if self._isdb:
            if len(sqltype)==0:
                assert False,"error sqltype null"
            self._sqlType = SqlTypeMap[sqltype]
        else:
            self._sqlType = sqltype
        
        self._len = self.GetSqlFieldLen(row)
        self._nullable = self.CheckBool(row,"Nullable")
        self._default = self.CheckStr(row,"Default")
        self._index = self.CheckBool(row,"Index")
        self._comment = self.CheckStr(row,"Comment")
        self._increment = self.CheckBool(row,"Increment")

    def CheckBool(self,row,field):
        ret = False
        cell = row[field]
        if cell.ctype!=0:
            val = int(cell.value)
            if val==1:
                ret = True
        return ret
    
    def CheckStr(self,row,field):
        ret = ""
        cell = row[field]
        if cell.ctype == xlrd.XL_CELL_NUMBER:
            if cell.value % 1==0:
                ret = str(int(cell.value))
            else:
                ret = str(cell.value)
        elif cell.ctype == xlrd.XL_CELL_TEXT:
            ret = cell.value
        elif cell.ctype == xlrd.XL_CELL_EMPTY:
            return ret
        else:
            assert False,"error cell value"+str(cell.value)
        return ret

    def GetSqlFieldLen(self,row):
        nlen = ""
        cell = row["Len"]
        if cell.ctype==0:
            if self._sqlType == SQL_TYPE.SQL_INT:
                nlen = "11"
            elif self._sqlType == SQL_TYPE.SQL_BIGINT:
                nlen = "20"
            elif self._sqlType == SQL_TYPE.SQL_VARCHAR:
                assert False,"varchar len null"
            elif self._sqlType == SQL_TYPE.SQL_TIMESTAMP:
                nlen = "0"
        else:
            nlen = cell.value
        return nlen

    def GetComment(self):
        if len(self._comment)>0:
            ret = "'{0}'".format(self._comment)
            if self._increment:
                ret += " AUTO_INCREMENT"
            return ret
        else:
            return ""


class objectCpp:
    Begin = '''
class {0}:public GameObject
{{
public:
    {0}();
    virtual void init(FactorManager * fm);
'''

    GetSet = '''
    void Set_{0}(const {1}& v){{ SetValue<{2}>({3},v); }}
    {1} Get_{0}(){{ return GetValue<{1}>({3});}}
'''

    GetSetSql = '''
    void Set_{0}(const {1}& v);
    {1} Get_{0}(){{ return {0};}}
'''

    GetSetSqlCpp = '''
void {0}::Set_{1}(const {2}& v)
{{
    SetValue<{0}>({3},v);
    m_sql->Set_{1}(v);
}}
'''

    SqlField = '    {0} {1};\n'
    SqlReflect = '''    std::shared_ptr<Reflect<{0}>> m_sql;\n'''

    End = '''
};
'''

    def __init__(self,name):
        self._name = name
        self._member = []
        self._sqlmember = []
    
    def AddMember(self,row):
        objf = objectField(row,len(self._member))
        self._member.append(objf)
        if objf._isdb:
            self._sqlmember.append(objf)
    
    def MakeClass(self):
        if len(self._member)==0:
            assert False,"struct member 0"
        code = objectCpp.Begin.format(self._name)
        for field in self._member:
            code += self.MakeGetSet(field)
        code += "\n"
        for field in self._sqlmember:
            code += objectCpp.SqlField.format(field._type,field._name)
        if len(self._sqlmember) > 0:
            code += objectCpp.SqlReflect.format(self._name)
        code += objectCpp.End
        if len(self._sqlmember)>0:
            code += self.MakeReflect()
            code += self.MakeTable()
        return code

    def MakeGetSet(self,field):
        if field._isdb:
            return objectCpp.GetSetSql.format(field._name,field._type)
        else:
            return objectCpp.GetSet.format(field._name,field._type,self._name,field._id)

    def MakeGetSetCpp(self,field):
        return objectCpp.GetSetSqlCpp.format(self._name,field._name,field._type,field._id)

    def MakeClassCpp(self):
        code = '''
{0}::{0}()
{{
'''
        code = code.format(self._name)
        if len(self._sqlmember)>0:
            tmp = '''   m_sql = std::shared_ptr<Reflect<{0}>>(new Reflect<{0}>(this));\n'''
            code += tmp.format(self._name)
        tmp = '''   AddPropery<{0}>({1});\n'''
        for f in self._member:
            code += tmp.format(f._type,f._flag)
        code += '}\n'
        tmp = '''
void {0}::init(FactorManager * fm)
{{
    GameObject::init(fm);
'''
        code += tmp.format(self._name)
        tmp ='''    Set_{0}({1});\n'''
        for f in self._member:
            if len(f._defval)>0:
                code += tmp.format(f._name,f._defval)
        if len(self._sqlmember)>0:
            code +=  '''\n    m_sql->flag = 0;\n'''
        tmp = '''
    Clear();
}
'''     
        code += tmp
        for f in self._sqlmember:
            code += self.MakeGetSetCpp(f)
        if len(self._sqlmember)>0:
            code += self.MakeReflectCpp()
        return code

    def MakeReflect(self):
        tmpcode = "REFLECT("+self._name
        for field in self._sqlmember:
            tmpcode += ","+field._name
        tmpcode += ")\n"
        return tmpcode
    
    def MakeTable(self):
        tmpcode = "TABLE_DESC_BEGAN("+self._name
        for field in self._sqlmember:
            if field._primary:
                tmpcode += ","+field._name
        tmpcode += ")\n"
        fdesc = 'FIELD_DESC({0},{1},{2},{3},"{4}",{5},"{6}")\n'
        for f in self._sqlmember:
            tmpcode += fdesc.format(f._name,SqlTypeStr[f._sqlType],f._len,BoolStr[f._nullable],f._default,BoolStr[f._index],f.GetComment())
        tmpcode += "TABLE_DESC_END\n"
        return tmpcode

    def MakeReflectCpp(self):
        code = '''
REFLECT_CPP_DEFINE({0})
TABLE_CPP_DEFINE({0})
'''
        return code.format(self._name)

class objectFile:
    FileName = "ConfigObjects"

    H_head = '''
#ifndef CONFIG_OBJECTS_H
#define CONFIG_OBJECTS_H

#include "GameObject.h"
#include "Reflection.h"

'''
    H_end = '''

#endif

'''
    CPP_head = '''
#include "{0}"
'''

    def __init__(self):
        self._struct = []

    def AddStruct(self,obj):
        self._struct.append(obj)

    def CreateFile(self,path):
        self.CreateHpp(os.path.join(path,objectFile.FileName+".h"))
        self.CreateCpp(os.path.join(path,objectFile.FileName+".cpp"))

    def CreateHpp(self,path):
        f = open(path,"w")
        f.write(objectFile.H_head)
        for obj in self._struct:
            code = obj.MakeClass()
            f.write(code)
        f.write(objectFile.H_end)
        f.close()

    def CreateCpp(self,path):
        f = open(path,"w")
        f.write(objectFile.CPP_head.format(objectFile.FileName+".h"))
        for obj in self._struct:
            code = obj.MakeClassCpp()
            f.write(code)
        f.close()