local CmdManager = {
    cmdList = {}
}

function CmdManager:AddCmdCall(_func,cmdName,_desc,_ext)
    if self.cmdList[cmdName] ~= nil then
        return
    end
    self.cmdList[cmdName] = {
        func = _func,
        desc = _desc,
		ext = _ext,
    }
end

function CmdManager:ShowHelp(cmdName)
    if cmdName == nil then
        if GameType ~= GAME_TYPE.SINGLE then
            print("cmds: first game Id 0:all >0:one game then cmd name")
        else
            print("cmds:")
        end
        for k,v in pairs(self.cmdList) do
            print("\t"..k..":"..(v.desc or " "))
        end
    else
		if self.cmdList[cmdName] == nil then
			local find = false
			for name, v in pairs(self.cmdList) do
				if string.find(name,cmdName) then
					find = true
					print("\t"..name.."\t"..v.desc)
				end
			end
			if not find then
				print("nil cmdName ",cmdName)
			end
            return
        end
        print("\t"..self.cmdList[cmdName].desc)
    end
end

function CmdManager:DoCmd( cmdName,ply,pams )
    local cmd = self.cmdList[cmdName]
    if cmd == nil then
        print("no this cmdName",cmdName)
		self:ShowHelp(cmdName)
        return
    end
	local res = cmd.func(ply,pams)
	if res == false then
		print("error param:")
		self:ShowHelp(cmdName)
	end
end

return CmdManager