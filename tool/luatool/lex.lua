require "class"
local Lex = class()
function Lex:ctor(str)
    self.str = str
    self.pos = 1
end

function Lex:__call(pat,pos)
    return self.str:match(pat,pos or self.pos)
end

function Lex:read_file( fname )
    local handle = io.open(fname,"r")
    self.str = handle:read("*a")
    self.pos = 1
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

function Lex:jumpcomment()
    local p = self "^%/%/[^\n]*\n?()"
    if not p then
        if self "^%/%*" then
            p = self "^%/%*.-%*%/()"
            if not p then
                local _str = self:nextAll()
                self:error(_str)
            end
        end
    end
    if not p then return self end
    self.pos = p
    return self:jumpspace()
end

function Lex:jumpspace(nocomm)
    local p,c = self "^[%s\n]*()(%/?)"
    self.pos = p
    if c == '' then return self end
    if nocomm then
        return
    end
    return self:jumpcomment()
end

function Lex:test(pat,nocomm)
    self:jumpspace(nocomm)
    local s,p = self("^("..pat..")%s*()")
    if not p then return end
    self.pos = p
    return s
end

function Lex:eof()
    self:jumpspace()
    return self.pos > self.str:len()
end

function Lex:word()
    local w = self:test("[%w_]*")
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

function Lex:nextChar()
    self:jumpspace()
    return self.str:sub(self.pos,self.pos)
end

function Lex:nextAll()
    self:jumpspace(true)
    return self:test(".*",true)
end

function Lex:cutChar()
    if not self:eof() then
        self.pos = self.pos + 1
    end
end

function Lex:line_end()
	local p = self "[^;]*;[%s\n]*()"
	if not p then
		-- self:error "';' expected"
		return
	end
	self.pos = p
	return p
end

function Lex:match_pat(pat)
	local w = self:test(pat)
	return w
end

function Lex:keyword()
	local w = self:test("[%a_][%w_]*")
	if not w then
		self:error "error word"
	end
	return w
end

return Lex