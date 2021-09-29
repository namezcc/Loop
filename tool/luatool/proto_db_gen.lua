local out_path = "E:/git/Loop/Loop/Servers/Common"
local out_path2 = "./"

require "split"
local conf = require "proto_db_conf"

function getCtype( sqlf_type )
    if sqlf_type == SQL_INT then
        return "int32_t"
    elseif sqlf_type == SQL_INT64 then
        return "int64_t"
    elseif sqlf_type == SQL_STRING then
        return "std::string"
    elseif sqlf_type == SQL_FLOAT then
        return "float"
    elseif sqlf_type == SQL_TIMESTAMP then
        return "std::string"
    end
end

function getCformate( sqlf_type )
    if sqlf_type == SQL_INT then
        return "%d"
    elseif sqlf_type == SQL_INT64 then
        return "%lld"
    elseif sqlf_type == SQL_STRING then
        return "'%s'"
    elseif sqlf_type == SQL_FLOAT then
        return "%f"
    elseif sqlf_type == SQL_TIMESTAMP then
        return "%s"
    end
end

function get_get_val_func( sqlf_type )
    if sqlf_type == SQL_INT then
        return "getInt32"
    elseif sqlf_type == SQL_INT64 then
        return "getInt64"
    elseif sqlf_type == SQL_STRING then
        return "getString"
    elseif sqlf_type == SQL_FLOAT then
        return "getDouble"
    elseif sqlf_type == SQL_TIMESTAMP then
        return "getString"
    end
end

function getKeyStr( cfg,_key,noType )
    local str = ""
    for _1,kindex in ipairs(_key) do
        local finfo = cfg._field[kindex]
        if noType then
            str = str..", _"..finfo[1]
            if finfo[3] == SQL_STRING or finfo[3] == SQL_TIMESTAMP then
                str = str..".c_str()"
            end
        else
            str = str..","..getCtype(finfo[3]).." _"..finfo[1]
        end
    end
    return str
end

function getFuncStr(cfg,v,_vec)
    local fs = "bool %s_%s(%s& pb,MysqlModule* db,char* _buff,size_t _size%s)"
    local sqltype = ""
    local fname = cfg._table
    local className = "LPMsg::"..cfg._proto
    local keyStr = ""

    if _vec then 
        className = "gopb::RepeatedPtrField<LPMsg::"..cfg._proto..">" 
    end

    if v._type == SQL_TYPE_INSERT then
        sqltype = "insert"
    elseif v._type == SQL_TYPE_DELETE then
        sqltype = "delete"
    elseif v._type == SQL_TYPE_UPDATE then
        sqltype = "update"
    elseif v._type == SQL_TYPE_SELECT then
        sqltype = "select"
        -- if v._key then
        --     keyStr = getKeyStr(cfg,v._key)
        -- end
        keyStr = getKeyStr(cfg,v._key or cfg._key)
    elseif v._type == SQL_TYPE_REPLACE then
        sqltype = "replace"
    end

    fs = string.format(fs,sqltype,fname,className,keyStr)
    return fs
end

function getFileStr( cfg )
    local str = "("
    for i,v in ipairs(cfg._field) do
        if i > 1 then
            str = str..","
        end
        str = str.."`"..v[1].."`"
    end
    return str..")"
end

function getValueStr( cfg )
    local str = "("
    for i,v in ipairs(cfg._field) do
        if i > 1 then
            str = str..","
        end
        str = str..getCformate(v[3])
    end
    return str..")"
end

function get_k_v( f )
    local str = "`%s`=%s"
    str = string.format(str,f[1],getCformate(f[3]))
    return str
end

function getWhereStr( cfg,keys )
    local str = ""
    for i,v in ipairs(keys) do
        if i > 1 then
            str = str.." AND "
        end
        str = str..get_k_v(cfg._field[v])
    end
    return str
end

function isKey( index,_key )
    for i,v in ipairs(_key) do
        if index == v then
            return true
        end
    end
    return false
end

function getSetStr( cfg,_key )
    local str = ""
    local first = nil
    for index,v in ipairs(cfg._field) do
        if isKey(index,_key) == false then
            if first then
                str = str..", "
            end
            str = str..get_k_v(v)
            first = true
        end
    end
    return str
end

function getPbValue( cfg,_key , _outKey)
    local str = "\t"

    if _key and _outKey == nil then
        for i,index in ipairs(_key) do
            if i > 1 then
                str = str..","
            end
            local v = cfg._field[index]
            str = str.."pb."..string.lower(v[2]).."()"
            if v[3] == SQL_STRING or v[3] == SQL_TIMESTAMP then
                str = str..".c_str()"
            end
        end
    else
        local first = nil
        for i,v in ipairs(cfg._field) do
            if _outKey == nil or isKey(i,_key) == false then
                if first then
                    str = str..","
                end
                first = true
                str = str.."pb."..string.lower(v[2]).."()"
                if v[3] == SQL_STRING or v[3] == SQL_TIMESTAMP then
                    str = str..".c_str()"
                end
            end
        end
    end
    return str
end

function get_insert_func( cfg,v,_vec )
    local fstr = getFuncStr(cfg,v,_vec)
    if _vec == nil then
        local str = [[
{
    %s
    return db->Query(_buff);
}

]]

        local ptf = 'snprintf(_buff, _size,'
        ptf = ptf..'"INSERT INTO `'..cfg._table..'`'..getFileStr(cfg)
        if v._update then
            ptf = ptf.." VALUES "..getValueStr(cfg).." ON DUPLICATE KEY UPDATE "..getSetStr(cfg,cfg._key)..';",\n'
            ptf = ptf..getPbValue(cfg)..","..getPbValue(cfg,cfg._key,true)..");\n"
        else
            ptf = ptf.." VALUES "..getValueStr(cfg)..';",\n'
            ptf = ptf..getPbValue(cfg)..");\n"
        end
        str = string.format(str,ptf)
        fstr = fstr..str
    else
        local str = [[
{
    for(auto& v:pb)
    {
        insert_%s(v,db,_buff,_size);
    }
    return true;
}

]]
        str = string.format(str,cfg._table)
        fstr = fstr..str
    end

    return fstr
end

function get_replace_func( cfg,v,_vec )
    local fstr = getFuncStr(cfg,v,_vec)
    if _vec == nil then
        local str = [[
{
    %s
    return db->Query(_buff);
}

]]

        local ptf = 'snprintf(_buff, _size,'
        ptf = ptf..'"REPLACE INTO `'..cfg._table..'`'..getFileStr(cfg)
        ptf = ptf.." VALUES "..getValueStr(cfg)..';",\n'
        ptf = ptf..getPbValue(cfg)..");\n"
        str = string.format(str,ptf)
        fstr = fstr..str
    else
        local str = [[
{
    for(auto& v:pb)
    {
        replace_%s(v,db,_buff,_size);
    }
    return true;
}

]]
        str = string.format(str,cfg._table)
        fstr = fstr..str
    end

    return fstr
end

function get_delete_func( cfg,v,_vec )
    local fstr = getFuncStr(cfg,v,_vec)
    if _vec == nil then
        local str = [[
{
    %s
    return db->Query(_buff);
}

]]

        local ptf = 'snprintf(_buff, _size,'
        ptf = ptf..'"DELETE FROM `'..cfg._table..'` WHERE '..getWhereStr(cfg,cfg._key)..';",'
        ptf = ptf..getPbValue(cfg,cfg._key)..");\n"
        str = string.format(str,ptf)
        fstr = fstr..str
    else
        local str = [[
{
    for(auto& v:pb)
    {
        delete_%s(v,db,_buff,_size);
    }
    return true;
}

]]
        str = string.format(str,cfg._table)
        fstr = fstr..str
    end

    return fstr
end

function get_update_func( cfg,v,_vec )
    local fstr = getFuncStr(cfg,v,_vec)
    if _vec == nil then
        local str = [[
{
    %s
    return db->Query(_buff);
}

]]

        local ptf = 'snprintf(_buff, _size,'
        ptf = ptf..'"UPDATE `'..cfg._table..'` SET '..getSetStr(cfg,cfg._key)..' WHERE '..getWhereStr(cfg,cfg._key)..';",\n'
        ptf = ptf..getPbValue(cfg,cfg._key,true)..","..getPbValue(cfg,cfg._key)..");\n"
        str = string.format(str,ptf)
        fstr = fstr..str
    else
        local str = [[
{
    for(auto& v:pb)
    {
        update_%s(v,db,_buff,_size);
    }
    return true;
}

]]
        str = string.format(str,cfg._table)
        fstr = fstr..str
    end

    return fstr
end

function get_pb_set_str( cfg,pb )
    local pb = pb or "pb"
    local str = ""
    for i,v in ipairs(cfg._field) do
        str = str..string.format('\t\t%s.set_%s(r->%s("%s"));\n',pb,string.lower(v[2]),get_get_val_func(v[3]),v[1])
    end
    return str
end

function get_select_func( cfg,v,_vec )
    local fstr = getFuncStr(cfg,v,_vec)
    local key = v._key or cfg._key
    local str = ""
    if _vec == nil then
        str = [[
{
    %s
    auto& r = db->query(_buff);

    if (!r->eof())
    {
%s
        return true;
    }else return false;
}

]]
    else
        str = [[
{
    %s
    auto& r = db->query(_buff);

    while (!r->eof())
    {
        auto& e = *pb.Add();
%s
        r->nextRow();
    }
    return true;
}

]]
    end

    local ptf = 'snprintf(_buff, _size,'
    ptf = ptf..'"SELECT * FROM `'..cfg._table..'` WHERE '..getWhereStr(cfg,key)..';"'
    ptf = ptf..getKeyStr(cfg,key,true)..");\n"

    if _vec == nil then
        str = string.format(str,ptf,get_pb_set_str(cfg))
    else
        str = string.format(str,ptf,get_pb_set_str(cfg,"e"))
    end
    fstr = fstr..str

    return fstr
end

function getCppStr( cfg )
    local str = ""
    for i,v in ipairs(cfg._sql) do
        local fstr = ""
        if v._type == SQL_TYPE_INSERT then
            fstr = get_insert_func(cfg,v)
            if v._vec then
                fstr = fstr..get_insert_func(cfg,v,true)
            end
        elseif v._type == SQL_TYPE_REPLACE then
            fstr = get_replace_func(cfg,v)
            if v._vec then
                fstr = fstr..get_replace_func(cfg,v,true)
            end
        elseif v._type == SQL_TYPE_DELETE then
            fstr = get_delete_func(cfg,v)
            if v._vec then
                fstr = fstr..get_delete_func(cfg,v,true)
            end
        elseif v._type == SQL_TYPE_UPDATE then
            fstr = get_update_func(cfg,v)
            if v._vec then
                fstr = fstr..get_update_func(cfg,v,true)
            end
        elseif v._type == SQL_TYPE_SELECT then
            if v._vec == true then
                fstr = get_select_func(cfg,v,v._vec)
            else
                fstr = get_select_func(cfg,v)
            end
        end
        str = str..fstr
    end
    return str
end

function getHstr( cfg )
    local str = ""
    for i,v in ipairs(cfg._sql) do
        local fs = ""
        if v._type == SQL_TYPE_SELECT then
            fs = getFuncStr(cfg,v,v._vec)..";\n"
        else
            fs = getFuncStr(cfg,v)..";\n"
            if v._vec then
                fs = fs..getFuncStr(cfg,v,true)..";\n"
            end
        end
        str = str..fs
    end
    return str.."\n"
end

function genCpp(path)

    local file_h = myfile.new()
    local file_cpp = myfile.new()
    local file_enum = myfile.new()

    file_h:open(path.."/proto_sql.h","w")
    file_cpp:open(path.."/proto_sql.cpp","w")
    file_enum:open(path.."/proto_table.h","w")

file_h:write([[
#ifndef PROTO_SQL_H
#define PROTO_SQL_H

namespace gopb = google::protobuf;
class MysqlModule;

]])


file_cpp:write([[
#include "protoPB/server/dbdata.pb.h"
#include "proto_sql.h"
#include "MysqlModule.h"

]])

file_enum:write([[
#ifndef PROTO_TABLE_H
#define PROTO_TABLE_H

enum TABLE_ENUM{

]])

    for i,v in ipairs(conf) do
        file_h:write(getHstr(v))
        file_cpp:write(getCppStr(v))
        file_enum:write("TAB_"..v._table.."="..i..",\n")
    end

    file_h:write("#endif")
    file_h:close()
    file_cpp:close()

    file_enum:write("};\n#endif")
    file_enum:close()
end

genCpp(out_path)
genCpp(out_path2)