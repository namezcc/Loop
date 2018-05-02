# -*- coding: UTF-8 -*-

import xlrd
import os
from objectcreate import *

fromPath = "./object"
outPath = "./"

objfile = objectFile()

def ReadExl(name):
    if not name.find(".xlsx"):
        return
    fp = os.path.join(fromPath,name)
    book = xlrd.open_workbook(fp)
    for sh in book.sheets():
        if sh.name.find("sheet") != -1:
            continue
        ExlToObject(sh)

def ExlToObject(sheet):
    objcreate = objcetCreate(sheet.name)
    field = sheet.row_values(0)
    fname = {}
    for i,f in enumerate(field):
        fname[f] = i
    for r in range(1,sheet.nrows):
        row = sheet.row(r)
        name = row[fname["Name"]].value
        desc = row[fname["Desc"]].value
        ftype = row[fname["Type"]].value
        save = row[fname["Save"]].value
        pri = row[fname["Private"]].value
        pub = row[fname["Public"]].value
        pev = row[fname["Event"]].value
        objcreate.addmember(name,desc,ftype,save,pri,pub,pev)
    objfile.addObject(objcreate)

def GetFiles(path):
    return  os.listdir(path)


def main():
    lis = GetFiles(fromPath)
    for f in lis:
        ReadExl(f)
    objfile.CreateFile(outPath)
    print "------------success "

if __name__ == '__main__':
    main()