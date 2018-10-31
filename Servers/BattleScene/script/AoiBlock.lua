local AoiBlock = class()

function AoiBlock:ctor(width,height,blocksize,viewSize)
    self._width = width
    self._height = height
    self._bsize = blocksize
    self._entity = {}
    self._vsize = math.ceil(blocksize/viewSize)
    self._clear = {}

    self._xBlock = {}
    local xnum = math.ceil(width/blocksize)
    local ynum = math.ceil(height/blocksize)
    self._xnum = xnum
    self._ynum = ynum

    for i=1,xnum do
        local yblock = {}
        for j=1,ynum do
            yblock[j] = {}
        end
        self._xBlock[i] = yblock
    end
end

function AoiBlock:SetCallBack(onEnter,onLeave)
    self._onEnter = onEnter
    self._onLeave = onLeave
end

function AoiBlock:AddEntity(entity)
    if entity._x < 0 or entity._x > self._width then
        entity._x = 0
    end
    if entity._y < 0 or entity._y > self._height then
        entity._y = 0
    end

    local xid = math.floor(entity._x/self._bsize) + 1
    local yid = math.floor(entity._y/self._bsize) + 1

    local aoiobj = {_xid = xid,_yid = yid,_enter = {},_leave = {},
        _ent = entity,
        _view = {}
    }
    self._entity[entity._id] = aoiobj
    local xblock = self._xBlock[xid]
    if xblock then
        local yblock = xblock[yid]
        if yblock then
            yblock[entity._id] = aoiobj
            self:EnterAoi(entity._id)
        end
    end
end

function AoiBlock:EnterAoi(eid)
    local eaoi = self._entity[eid]

    for i = eaoi._xid - self._vsize,eaoi._xid + self._vsize,1 do
        local xblock = self._xBlock[i]
        if xblock then
            for j= eaoi._yid - self._vsize ,eaoi._yid + self._vsize,1 do
                local yblock = xblock[j]
                if yblock then
                    for k,v in pairs(yblock) do
                        if k ~= eid then
                            self:EnterView(eaoi,v)
                        end 
                    end
                end
            end
        end
    end
end

function AoiBlock:UpdateEntity(eid)
    local eaoi = self._entity[eid]
    if eaoi == nil then
        return
    end
    local ent = eaoi._ent
    local nxid = math.ceil(ent._x/self._bsize)
    local nyid = math.ceil(ent._y/self._bsize)

    local oxid = eaoi._xid
    local oyid = eaoi._yid

    if nxid ~= eaoi._xid then
        local leaveblock = nil
        local addblock = nil

        if nxid > eaoi._xid then
            leaveblock = self._xBlock[oxid - self._vsize]
            addblock = self._xBlock[nxid + self._vsize]
        else
            leaveblock = self._xBlock[oxid + self._vsize]
            addblock = self._xBlock[nxid - self._vsize]
        end
        
        if leaveblock then
            for i=oyid - self.vsize,oyid + self._vsize,1 do
                local block = leaveblock[i]
                if block then
                    for k,v in pairs(block) do
                        self:LeaveView(eaoi,v)
                    end
                end
            end
        end

        if addblock then
            for i=nyid - self._vsize,nyid + self._vsize,1 do
                local block = addblock[i]
                if block then
                    for k,v in pairs(block) do
                        self:EnterView(eaoi,v)
                    end
                end
            end
        end
        eaoi._xid = nxid
    end

    if nyid ~= eaoi._yid then
        local bx = oxid >= nxid and oxid - self._vsize or nxid - self._vsize
        local ex = oxid >= nxid and nxid + self._vsize or oxid + self._vsize
        for x=bx,ex,1 do
            local xblock = self._xBlock[x]
            if xblock then
                local leaveblock = nil
                local addblock = nil
                if nyid > oyid then
                    leaveblock = xblock[oyid - self._vsize]
                    addblock = xblock[nyid + self._vsize]
                else
                    leaveblock = xblock[oyid + self._vsize]
                    addblock = xblock[nyid - self._vsize]
                end

                if leaveblock then
                    for k,v in pairs(leaveblock) do
                        self:LeaveView(eaoi,v)
                    end
                end

                if addblock then
                    for k,v in pairs(addblock) do
                        self:EnterView(eaoi,v)
                    end
                end
            end
        end
        eaoi._yid = nyid
    end

    if nxid ~= oxid or nyid ~= oyid then
        self._xBlock[oxid][oyid] = nil
        self._xBlock[nxid][nyid] = eaoi
    end
end

function AoiBlock:EnterView(ent1,ent2)
    local eid1 = ent1._ent._id
    local eid2 = ent2._ent._id
    if ent1._leave[eid2] then
        ent1._leave[eid2] = nil
    else
        ent1._enter[eid2] = true
        ent1._view[eid2] = true
        self:AddClearList(eid1)
    end

    if ent2._leave[eid1] then
        ent2._leave[eid1] = nil
    else
        ent2._enter[eid1] = true
        ent2._view[eid1] = true
        self:AddClearList(eid2)
    end
end

function AoiBlock:LeaveView(ent1,ent2)
    local eid1 = ent1._ent._id
    local eid2 = ent2._ent._id
    if ent1._enter[eid2] then
        ent1._enter[eid2] = nil
    else
        ent1._leave[eid2] = true
        ent1._view[eid2] = nil
        self:AddClearList(eid1)
    end

    if ent2._enter[eid1] then
        ent2._enter[eid1] = nil
    else
        ent2._leave[eid1] = true
        ent2._view[eid1] = nil
        self:AddClearList(eid2)
    end
end

function AoiBlock:AddClearList(eid)
    self._clear[eid] = true
end

function AoiBlock:ClearView()
    for k,_1 in pairs(self._clear) do
        local v = self._entity[k]
        if v then
            v._enter = {}
            v._leave = {}
        end
    end
end

function AoiBlock:GetViewEntityPB(pid)
    local eaoi = self._entity[pid]
    local list = {}
    
    if eaoi == nil then
        print("nil eaoi")
        return list
    end
    
    table.insert(list,eaoi._ent:GetInfoPB())
    return list
end

return AoiBlock