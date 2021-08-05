local pb = mpb

local Lex = class()
function Lex:ctor(str)
	self.str = str
	self.pos = 1
end

function Lex:__call(pat,pos)
	return self.str:match(pat,pos or self.pos)
end

function Lex:pos2loc()
	local linenr = 1
	local pos = self.pos
   	for start, stop in self.str:gmatch "()[^\n]*()\n?" do
      	if start <= pos and pos <= stop then
         	return linenr, pos - start + 1
      	end
      	linenr = linenr + 1
   	end
end

function Lex:error(fmt,...)
	local l,c = self:pos2loc()
	return error(("%d:%d "..fmt):format(l,c,...))
end

function Lex:jumpspace()
	local p,c = self "^[%s\n]*()(%/?)"
	self.pos = p
	if c == '' then return self end
	return self:jumpcomment()
end

function Lex:jumpcomment()
	local p = self "^%/%/[^\n]*\n?()"
	if p then
		if self "^%/%*" then
			p = self "^%/%*.-%*%/()"
			if not p then
				self:error "error comment"
			end
		end
	end
	if not p then return self end
	self.pos = p
	return self:jumpspace()
end

function Lex:check(pat)
	self:jumpspace()
	local s,p = self("^("..pat..")%s*()")
	return s ~= nil
end

function Lex:test(pat)
	self:jumpspace()
	local s,p = self("^("..pat..")%s*()")
	if not p then return end
	self.pos = p
	return s
end

function Lex:line_end()
	local p = self "[^;]*;[%s\n]*()"
	if not p then
		self:error "';' expected"
	end
	self.pos = p
	return p
end

function Lex:eof()
	self:jumpspace()
	return self.pos > self.str:len()
end

function Lex:quote()
	local q = self:test("[\"\']")
	if not q then
		self:error "error quote"
	end
	local w = self:test("[%w_%.]*")
	if not w then
		self:error "error quote"
	end
	local q2 = self:test("[\"\']")
	if not q2 then
		self:error "error quote"
	end
	return w
end

function Lex:expected(pat)
	local w = self:test(pat)
	if not w then
		self:error("error expected "..pat)
	end
end

function Lex:keyword()
	local w = self:test("[%a_][%w_]*")
	if not w then
		self:error "error word"
	end
	return w
end

function Lex:integer()
	local n = self:test("%d+")
	if not n then
		self:error "error number"
	end
	return tonumber(n)
end

local SupportPacking = {
	int32 = true,
	uint32 = true,
	int64 = true,
	uint64 = true,
	bool = true,
	float = true,
	double = true,
	enum = true,
}

local WriteType = {
	varint = 0,
	fix64 = 1,
	lengthDelimited = 2,
	fix32 = 5,
}

local TypeWriteType = {
	int32 = 0,
	uint32 = 0,
	int64 = 0,
	uint64 = 0,
	bool = 0,
	float = 5,
	double = 1,
	string = 2,
	bytes = 2,
	enum = 0,
}

local TypeDefaultValue = {
	int32 = 0,
	uint32 = 0,
	int64 = 0,
	uint64 = 0,
	bool = 0,
	float = 0.0,
	double = 0.0,
	string = "",
	bytes = "",
	enum = 0,
}

local TypeSizeFunc = {
	int32 = function(v)
		return pb.sizeInt(v)
	end,
	uint32 = function(v)
		return pb.sizeUInt(v)
	end,
	int64 = function(v)
		return pb.sizeInt64(v)
	end,
	uint64 = function(v)
		return pb.sizeUInt64(v)
	end,
	bool = function(v)
		return 1
	end,
	float = function(v)
		return pb.sizeFloat(v)
	end,
	double = function(v)
		return pb.sizeDouble(v)
	end,
	string = function(v)
		return pb.sizeString(v)
	end,
	bytes = function(v)
		return pb.sizeString(v)
	end,
	enum = function(v)
		return pb.sizeInt(v)
	end,
}

local TypeEncodeFunc = {
	int32 = function(s,v)
		pb.wInt(s,v)
	end,
	uint32 = function(s,v)
		pb.wUInt(s,v)
	end,
	int64 = function(s,v)
		pb.wInt64(s,v)
	end,
	uint64 = function(s,v)
		pb.wUInt64(s,v)
	end,
	bool = function(s,v)
		pb.wInt(s,v)
	end,
	float = function(s,v)
		pb.wFloat(s,v)
	end,
	double = function(s,v)
		pb.wDouble(s,v)
	end,
	string = function(s,v)
		pb.wString(s,v)
	end,
	bytes = function(s,v)
		pb.wString(s,v)
	end,
	enum = function(s,v)
		pb.wInt(s,v)
	end,
}

local TypeDecodeFunc = {
	int32 = function(s)
		return pb.rInt(s)
	end,
	uint32 = function(s)
		return pb.rUInt(s)
	end,
	int64 = function(s)
		return pb.rInt64(s)
	end,
	uint64 = function(s)
		return pb.rUInt64(s)
	end,
	bool = function(s)
		return pb.rInt(s)
	end,
	float = function(s)
		return pb.rFloat(s)
	end,
	double = function(s)
		return pb.rDouble(s)
	end,
	string = function(s)
		return pb.rString(s)
	end,
	bytes = function(s)
		return pb.rString(s)
	end,
	enum = function(s)
		return pb.rInt(s)
	end,
}

local function GetWriteType(tname,rept)
	if rept then
		return WriteType.lengthDelimited
	else
		return TypeWriteType[tname] or WriteType.lengthDelimited
	end
end

local function IsPackted(tname,wt)
	return (SupportPacking[tname] and wt == WriteType.lengthDelimited)
end

local Parse = {
	loadFile = {},
	message = {}
}

local function ComputeMessageSize(tname,v)
	local msg = Parse.message[tname]
	local size = msg.calcSize(v)
	return size + pb.sizeInt(size)
end

local function encodeMessage(tname,stream,v)
	local msg = Parse.message[tname]
	local size = msg.calcSize(v)
	pb.wInt(stream,size)
	msg.encodeValue(stream,v,size)
end

local function encodeT(T,stream,t)
	for i,v in ipairs(T.fields) do
		if t[v.fname] then
			v.encode(stream,t[v.fname])
		end
	end
end

local function decodeMessage(tname,stream)
	local msg = Parse.message[tname]
	local len = pb.rUInt(stream)
	local oldLimit = pb.PushLismit(stream,len)
	local obj = msg.decodeValue(stream)
	pb.PopLimit(stream,oldLimit)
	return obj
end

local function decodeT(T,stream,t)
	local index = 1
	local num = #T.fields
	for i,f in ipairs(T.fields) do
		t[f.fname] = f:DefaultValue()
	end
	while(true)
	do
		local tag = pb.rTag(stream)
		if tag == 0 then
			break
		end
		for i=index,num do
			local f = T.fields[i]
			if f.tag == tag then
				t[f.fname] = f.decode(stream)
				index = i + 1
				break
			else
				index = i
			end
		end
	end
end

local Field = class()

function Field:encodeFunc(tname)	
	local enfunc = TypeEncodeFunc[tname] or function(s,v) return encodeMessage(tname,s,v) end
	local resfunc = nil
	if self.rept then
		if self.packted then
			local sizefunc = TypeSizeFunc[tname]
			if not sizefunc then error("error "..tname.." size func") end
			resfunc = function(s,vlist)
				if #vlist == 0 then return end
				local dsize = 0
				for i,v in ipairs(vlist) do
					dsize = dsize + sizefunc(v)
				end
				pb.wInt(s,self.tag)
				pb.wInt(s,dsize)
				for i,v in ipairs(vlist) do
					enfunc(s,v)
				end
			end
		else
			resfunc = function(s,vlist)
				for i,v in ipairs(vlist) do
					pb.wInt(s,self.tag)
					enfunc(s,v)
				end
			end
		end
	else
		resfunc = function(s,v)
			pb.wInt(s,self.tag)
			enfunc(s,v)
		end
	end
	return resfunc
end

function Field:decodeFunc(tname)
	local defunc = TypeDecodeFunc[tname] or function(s) return decodeMessage(tname,s) end
	local resfunc = nil

	if self.rept then
		if self.packted then
			resfunc = function(s)
				local list = {}
				local len = pb.rInt(s)
				local oldLimit = pb.PushLismit(s,len)
				while(not pb.IsEndLimit(s))
				do
					list[#list+1] = defunc(s)
				end
				pb.PopLimit(s,oldLimit)
				return list
			end
		else
			resfunc = function(s)
				local list = {}
				repeat
					list[#list+1] = defunc(s)
				until(not pb.MaybeConsumeTag(s,self.tag))
				return list
			end
		end
	else
		resfunc = function(s)
			return defunc(s)
		end
	end
	return resfunc
end

function Field:ctor(tname,fname,idx,rept)
	self.tname = tname
	self.fname = fname
	self.rept = rept
	local wt = GetWriteType(tname,rept)
	self.tag = idx*8+wt
	-- if not rept then
	-- 	self.defaultValue = TypeDefaultValue[tname]
	-- end
	-- self.tagSize = pb.sizeInt(self.tag)
	-- self.packted = IsPackted(tname,wt)
	-- local sizefunc = TypeSizeFunc[tname] or function(v) return ComputeMessageSize(tname,v) end

	-- if self.rept then
	-- 	if self.packted then
	-- 		self.calcSize = function(vlist)
	-- 			if #vlist == 0 then
	-- 				return 0
	-- 			end
	-- 			local dsize = 0
	-- 			for i,v in ipairs(vlist) do
	-- 				dsize = dsize + sizefunc(v)
	-- 			end
	-- 			dsize = pb.sizeInt(dsize) + self.tagSize + dsize
	-- 			return dsize
	-- 		end
	-- 	else
	-- 		self.calcSize = function(vlist)
	-- 			local n = #vlist
	-- 			local size = n*self.tagSize
	-- 			for i,v in ipairs(vlist) do
	-- 				size = size + sizefunc(v)
	-- 			end
	-- 			return size
	-- 		end
	-- 	end
	-- else
	-- 	self.calcSize = function(v)
	-- 		return self.tagSize + sizefunc(v)
	-- 	end
	-- end

	self.encode = self:encodeFunc(tname)
	self.decode = self:decodeFunc(tname)
end

function Field:DefaultValue()
	if self.rept then
		return {}
	else
		return TypeDefaultValue[self.tname]
	end
end

local function calcSizeT(T,t)
	local size = 0
	for i,v in ipairs(T.fields) do
		if t[v.fname] then
			size = size + v.calcSize(t[v.fname])
		end
	end
	return size
end

local function CreateMessage(enum)
	local msg = class()
	msg.enum = enum
	msg.subT = {}
	msg.fields = {}
	msg.calcSize = function(data)
		return calcSizeT(msg,data)
	end

	msg.encode = function(data)
		local size = msg.calcSize(data)
		local s = pb.newStream(size)
		msg.encodeValue(s,data)
		return s
	end

	msg.encodeValue = function(stream,val)
		encodeT(msg,stream,val)
	end

	msg.decode = function(data)
		local s = pb.fromString(data)
		local obj = msg.decodeValue(s)
		pb.clearStream(s)
		return obj
	end

	msg.decodeValue = function(stream)
		local obj = {}
		decodeT(msg,stream,obj)
		return obj
	end
	return msg
end

local function CreateField(msg,tname,fname,idx,rept)
	local f = Field.new(tname,fname,idx,rept)
	table.insert(msg.fields,f)
	return f
end

function Parse:SetProtoPath(path)
	self.proPath = path
end

function Parse:ParseFile(filename)
	local fname = string.match(filename,"[%w_%.]*$")
	if self.loadFile[fname] then
		return
	end

	local file = io.open(self.proPath..filename,"r")
	print(self.proPath..filename)
	local content = file:read("*a")
	io.close(file)
	local lex = Lex.new(content)
	self.loadFile[fname] = true
	self:ParseContent(lex)
end

function Parse:ParseString(text)
	local lex = Lex.new(text)
	self:ParseContent(lex)
end

function Parse:ParseContent(lex)
	while(not lex:eof())
	do
		local keyword = lex:keyword()
		if keyword == "syntax" then
			self:ParseSyntax(lex)
		elseif keyword == "package" then
			self:ParsePackage(lex)
		elseif keyword == "message" then
			self:ParseMessage(lex)
		elseif keyword == "enum" then
			self:ParseEnum(lex)
		elseif keyword == "import" then
			self:ImportFile(lex)
		else
			lex:error "error keyword"
		end
	end
end

function Parse:ImportFile(lex)
	local filename = lex:quote()
	self:ParseFile(filename)
	lex:line_end()
end

function Parse:ParseSyntax(lex)
	lex:expected("=")
	if lex:quote() ~= 'proto3' then
		lex:error "just support proto3"
	end
	lex:line_end()
end

function Parse:ParsePackage(lex)
	local pak = lex:keyword()
	lex:line_end()
end

function Parse:ParseMessage(lex)
	local mname = lex:keyword()
	if self.message[mname] then
		lex:error("error double message "..mname)
	end
	lex:expected("{")
	self:ParseMsgBody(lex,mname)
	lex:expected("}")
end

function Parse:ParseMsgBody(lex,mname)
	local msg = CreateMessage()
	local fileNum = {}
	while(not lex:check("}"))
	do
		local keyword = lex:keyword()
		if keyword == "message" then
			self:ParseMessage(lex)
		elseif keyword == "enum" then
			self:ParseEnum(lex)
		elseif keyword == "repeated" then
			local ktype = lex:keyword()
			local kname = lex:keyword()
			lex:expected("=")
			local knum = lex:integer()
			if fileNum[knum] then
				lex:error("double file num"..knum)
			end
			fileNum[knum] = true
			CreateField(msg,ktype,kname,knum,true)
			lex:line_end()
		else
			local kname = lex:keyword()
			lex:expected("=")
			local knum = lex:integer()
			if fileNum[knum] then
				lex:error("double file num"..knum)
			end
			fileNum[knum] = true
			CreateField(msg,keyword,kname,knum)
			lex:line_end()
		end
	end
	if #msg.fields > 1 then
		table.sort( msg.fields, function(f1,f2)
			return f1.tag < f2.tag
		end )
	end
	self.message[mname] = msg
end

function Parse:ParseEnum(lex)
	local ename = lex:keyword()
	if self.message[ename] then
		lex:error "error double enum"
	end
	local msg = CreateMessage(true)
	lex:expected("{")
	
	while(not lex:check("}"))
	do
		local estr = lex:keyword()
		lex:expected("=")
		msg[estr] = lex:integer()
		lex:line_end()
	end
	lex:expected("}")
	self.message[ename] = msg
end

function Parse:NewMsg( msgName )
	return self.message[msgName].new()
end

function Parse:EncodeToString(msgName,data)
	local s = self.message[msgName].encode(data)
	local str = pb.toStringbyte(s)
	pb.clearStream(s)
	return str
end

function Parse:EncodeToStream(msgName,data)
	return self.message[msgName].encode(data)
end

function Parse:clearStream( s )
	if s then pb.clearStream(s) end
end

function Parse:DecodeToObject( msgName,str )
	return self.message[msgName].decode(str)
end

function Parse:GetMessage(mname)
	return self.message[mname]
end

return Parse