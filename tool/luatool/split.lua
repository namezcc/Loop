require "class"

-- 分割字符串 p1:字符串 p2:分隔符
function Split(szFullString, szSeparator)
    local nFindStartIndex = 1
    local nSplitIndex = 1
    local nSplitArray = {}
    while true do
       local nFindLastIndex = string.find(szFullString, szSeparator, nFindStartIndex)
       if not nFindLastIndex then
        nSplitArray[nSplitIndex] = string.sub(szFullString, nFindStartIndex, string.len(szFullString))
        break
       end
       nSplitArray[nSplitIndex] = string.sub(szFullString, nFindStartIndex, nFindLastIndex - 1)
       nFindStartIndex = nFindLastIndex + string.len(szSeparator)
       nSplitIndex = nSplitIndex + 1
    end
    return nSplitArray
end

function SplitPrint(str,sep)
    local tab = Split(str,sep)
    for i,v in ipairs(tab) do
        print(v)
    end
end

-- 分割字符串2 p1:string p2:分隔符1 优先 p2:分割符2
function Split2(str,sp1,sp2)
	local d1 = Split(str,sp1)
	for i,v in ipairs(d1) do
  		local d2 = Split(v,sp2)
  		print(table.concat( d2, "  "))
	end
end

-- 分割打印
function SplitTabAndP(tab,sp1 )
	for i,v in ipairs(tab) do
		local d = Split(v,sp1)
		print(table.concat( d, " "))
	end
end

-- 模式匹配
function Match(str,pat)
	local data = {}
	for s in string.gmatch(str,pat) do
		if s ~= nil and s ~= "" then
			table.insert(data,s)
		end
	end
	return data
end

-- 打印2层 table lk:链接符
function printTable2( tab,lk )
	for i,v in ipairs(tab) do
		print(table.concat( v, lk))
	end
end

-- 处理文件行
function readFileLine( fname,func )
    local handle = io.open(fname,"r")
    repeat
        line = handle:read('*l')
        if line ~= nil then
            func(line)
        end
    until line == nil
    handle:close()
end

function haveStr( str,mat )
    local x = string.find(str,mat)
    return x ~= nil
end

myfile = class("myfile")

function myfile:open( fname,opt )
    self._hand = io.open(fname,opt)
end

function myfile:write( str )
    self._hand:write(str)
end

function myfile:close()
    self._hand:close()
end