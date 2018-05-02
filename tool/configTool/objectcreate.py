# -*- coding: UTF-8 -*-
import os

class objcetCreate:
    Begin = '''
class {0}:MObject
{{
'''
    Create = '''
    public {0} Create()
    {{
        {0} obj = new {0}();
        obj.Init({1});
'''

    AddPro = '        obj.AddProperty({0},new Varpro<{1}>({2}));\n'

    EnumPro = '    public const {0} p_{1}={2};//{3} \n'

    FilePro = '    public {1} {0} {{ get{{return GetProperty<{1}>({2});}} set{{SetValue<{3},{1}>({2},value);}} }}\n'

    End = '''
}
'''

    def __init__(self,name):
        self._name = name
        self._member = []
    
    def addmember(self,name,desc,_t,save,pri,pub,ev):
        flag = 0
        if save:
            flag |= 1
        if pri:
            flag |= 2
        if pub:
            flag |= 4
        if ev:
            flag |= 8
        self._member.append({"_n":name,"_t":_t,"_f":flag,"_d":desc})
        
    def getCode(self):
        code = objcetCreate.Begin.format(self._name)
        code += objcetCreate.Create.format(self._name,len(self._member))
        for i,v in enumerate(self._member):
            code += objcetCreate.AddPro.format(i,v["_t"],v["_f"])
        code +='''
        return obj;
    }
'''
        for i,v in enumerate(self._member):
            code += objcetCreate.EnumPro.format(v["_t"],v["_n"],i,v["_d"])
        for i,v in enumerate(self._member):
            code += objcetCreate.FilePro.format(v["_n"],v["_t"],i,self._name)
        code += objcetCreate.End
        return code

class objectFile:
    filename = "GameObject.cs"
    head = '''
using System;
using System.Diagnostics;
using System.Linq;
using System.Reflection;
using System.Collections.Generic;
using System.Threading;
'''

    def __init__(self):
        self._object = []

    def addObject(self,obj):
        self._object.append(obj)

    def CreateFile(self,path):
        f = open(os.path.join(path,objectFile.filename),"w")
        f.write(objectFile.head)
        for obj in self._object:
            code = obj.getCode()
            f.write(code)
        f.close()