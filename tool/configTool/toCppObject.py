# -*- coding: UTF-8 -*-
import xlrd
import os
from structToCpp import *

import sys
reload(sys)
sys.setdefaultencoding('utf-8')

fromPath = "../../cfgdata/struct/"
outPath = "../../Servers/struct/"
fileMaker = objectFile()

dataStart = 2

def ReadExl(name):
    if not name.find(".xlsx") and not name.find(".xls"):
        return
    fp = os.path.join(fromPath,name)
    book = xlrd.open_workbook(fp)
    for sh in book.sheets():
        if sh.name.find("sheet") != -1:
            continue
        ExlToObject(sh)

def ExlToObject(sheet):
    objcreate = objectCpp(sheet.name)
    field = sheet.row_values(0)
    fname = {}
    for i,f in enumerate(field):
        fname[f] = i
    for r in range(dataStart,sheet.nrows):
        row = sheet.row(r)
        rowtab = {
            "Name":row[fname["Name"]],
            "Desc":row[fname["Desc"]],
            "Type":row[fname["Type"]],
            "DefVal":row[fname["DefVal"]],
            "Private":row[fname["Private"]],
            "Public":row[fname["Public"]],
            "Event":row[fname["Event"]],
            "Db":row[fname["Db"]],
            "Primary":row[fname["Primary"]],
            "SqlType":row[fname["SqlType"]],
            "Len":row[fname["Len"]],
            "Nullable":row[fname["Nullable"]],
            "Default":row[fname["Default"]],
            "Index":row[fname["Index"]],
            "Comment":row[fname["Comment"]],
            "Increment":row[fname["Increment"]],
        }
        objcreate.AddMember(rowtab)
    fileMaker.AddStruct(objcreate)

def GetFiles(path):
    return  os.listdir(path)


def main():
    lis = GetFiles(fromPath)
    for f in lis:
        ReadExl(f)
    fileMaker.CreateFile(outPath)
    print "------------success "

if __name__ == '__main__':
    main()