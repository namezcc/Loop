# -*- coding: UTF-8 -*-

import xlrd
import os
import json
from enum import IntEnum
from classCreate import *

exlpath = "../../cfgdata/cfgexcl/"
dataRow = 5     #数据开始行数 0开始
dataTypeRow = 2 # 数据类型
outTypeRow = 4  #输出类型控制行   0开始 

server = True
client = False

class OUT_TYPE(IntEnum):
    NO_OUT = 0
    CLIENT = 1
    SERVER = 2
    BOTH = 3

outServer = "../../cfgdata/cfgjson/"
outClient = "./out/client"

hppcreate = HppCreate()
hppPath = "../../configHpp/"

def ReadExl(name):
    if not name.find(".xlsx"):
        return
    fp = os.path.join(exlpath,name)
    book = xlrd.open_workbook(fp)
    for sh in book.sheets():
        if sh.name.find("Sheet") != -1:
            continue
        ExlToJson(sh)

def Init():
    if not os.path.exists(outServer):
        os.makedirs(outServer)
    if not os.path.exists(outClient):
        os.makedirs(outClient)
    if not os.path.exists(hppPath):
        os.makedirs(hppPath)

def ExlToJson(sheet):
    jsServer = []
    jsClient = []
    field = sheet.row_values(0)
    outType = sheet.row_values(outTypeRow)
    dataType = sheet.row_values(dataTypeRow)
    objStruct = Struct(sheet.name)
    for i,f in enumerate(field):
        if int(outType[i]) == OUT_TYPE.SERVER or int(outType[i]) == OUT_TYPE.BOTH:
            objStruct.addMember(f,dataType[i])
    hppcreate.addStruct(objStruct)
    for r in range(dataRow,sheet.nrows):
        objServer = {}
        objClient = {}
        for i,cell in enumerate(sheet.row(r)):
            v = cell.value
            if cell.ctype == 2 and v % 1 == 0:
                v = int(v)
            elif cell.ctype == 0:   #空单元格
                v = ""
            elif cell.ctype == 5:   #error
                continue
            if int(outType[i]) == OUT_TYPE.CLIENT:
                objClient[field[i]] = v
            elif int(outType[i]) == OUT_TYPE.SERVER:
                objServer[field[i]] = v
            elif int(outType[i]) == OUT_TYPE.BOTH:
                objClient[field[i]] = v
                objServer[field[i]] = v
        if len(objServer)>0:
            jsServer.append(objServer)
        if len(objClient)>0:
            jsClient.append(objClient)
    serp = os.path.join(outServer,sheet.name+".json")
    clip = os.path.join(outClient,sheet.name+".json")
    if server:
        WriteJson(jsServer,serp)
    if client:
        WriteJson(jsClient,clip)

def WriteJson(js,fpath):
    if len(js) <= 0:
        return
    f = open(fpath,"w")
    jstr = json.dumps(js,ensure_ascii=False,indent=2)
    f.write(jstr.encode("UTF-8"))
    f.close()
    print "write %s to json success" % fpath


def GetFiles(path):
    return  os.listdir(path)

def main():
    Init()
    lis = GetFiles(exlpath)
    for f in lis:
        ReadExl(f)
    hppcreate.createFile(hppPath)
    print "-------------- all success"

if __name__ == '__main__':
    main()