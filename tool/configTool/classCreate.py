# -*- coding: UTF-8 -*-
import os

class Struct:
    Template = '''
struct {0}:public BaseTable
{{
    static {0}* Get(int key)
    {{
        auto& m = *{0}::m_map;
        auto it = m.find(key);
        if (it == m.end())
            return NULL;
        return it->second;
    }}
    static BaseTable* CreateTable(int key) 
    {{ 
        auto& m = *{0}::m_map;
        auto t = new {0}();
        m[key] = t;
        return t;
    }};
    static TableRegist* m_reg;
    static map<int, {0}*>* m_map;
public:
'''

    TypeMap = {
        "int" : "int32_t",
        "long" : "int64_t",
        "float" : "float",
        "double" : "double",
        "bool" : "bool",
        "string" : "std::string",
        "int[]" : "std::vector<int32_t>",
        "long[]" : "std::vector<int64_t>",
        "float[]" : "std::vector<float>",
        "double[]" : "std::vector<double>",
        "bool[]" : "std::vector<bool>",
        "string[]" : "std::vector<std::string>"
    }

    InitMap = {
        "int" : '       m_{0} = val["{0}"].asInt();',
        "long" : '       m_{0} = val["{0}"].asInt64();',
        "float" : '       m_{0} = val["{0}"].asFloat();',
        "double" : '       m_{0} = val["{0}"].asDouble();',
        "bool" : '       m_{0} = val["{0}"].asBool();',
        "string" : '       m_{0} = val["{0}"].asString();',
        "int[]" : '       SplitVec(val["{0}"].asString(),m_{0});',
        "long[]" : '       SplitVec(val["{0}"].asString(),m_{0});',
        "float[]" : '       SplitVec(val["{0}"].asString(),m_{0});',
        "double[]" : '       SplitVec(val["{0}"].asString(),m_{0});',
        "bool[]" : '       SplitVec(val["{0}"].asString(),m_{0});',
        "string[]" : '       SplitVec(val["{0}"].asString(),m_{0});',
    }

    CppTemplate = '''
TableRegist* {0}::m_reg = new TableRegist("{0}", {0}::CreateTable);
map<int, {0}*>* {0}::m_map = new map<int, {0}*>();
'''

    def __init__(self,name):
        self._name = name
        self._members = {}

    def addMember(self, _name,_type):
        self._members[_name] = _type

    def getDefineCode(self):
        code = Struct.Template.format(self._name)
        for m,t in self._members.items():
            if Struct.TypeMap.has_key(t):
                mem = "    {0} m_{1};\n".format(Struct.TypeMap[t],m)
                code += mem
            else:
                print "error Type {0} in Table->{1} field:{2}".format(t,self._name,m)
        initstr = '''
    void Init(Json::Value& val)
    {
'''
        for m,t in self._members.items():
            if Struct.InitMap.has_key(t):
                mem = Struct.InitMap[t].format(m)
                mem += "\n"
                initstr += mem
        initstr += "    }\n"
        code +=initstr
        code += "};\n"
        return code

    def getCppCode(self):
        code = Struct.CppTemplate.format(self._name)
        return code


class HppCreate:
    fileName = "TableConfig"

    hppHead = '''
#ifndef TABLE_CONFIG_H
#define TABLE_CONFIG_H
    '''

    hppEnd = '''
#endif
    '''

    def __init__(self):
        self._structs = []

    def addStruct(self, s):
        self._structs.append(s)

    def createFile(self, path):
        self._createHpp(os.path.join(path,HppCreate.fileName+".h"))
        self._createCpp(os.path.join(path,HppCreate.fileName+".cpp"))

    def _createHpp(self, path):
        f = open(path,"w")
        f.write(HppCreate.hppHead)
        for s in self._structs:
            code = s.getDefineCode()
            f.write(code)
        f.write(HppCreate.hppEnd)
        f.close()

    def _createCpp(self, path):
        f = open(path,"w")
        f.write('#include "{0}.h"\n'.format(HppCreate.fileName))
        for s in self._structs:
            code = s.getCppCode()
            f.write(code)
        f.close()