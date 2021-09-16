Schedule = {}

function Schedule:init()
    self._pl_ms = {}
    self._pl_s = {}
    self._pl_m = {}
    self._pl_h = {}
    self._pl_d = {}
    self._pl_w = {}
    self._pl_M = {}

    self.lastMsdt = 0
end

function Schedule:Run(dt)
    local sec = math.ceil( dt/1000 )
    self._tm = os.date("*t",sec)

    self:RunInterval(dt)
end

function Schedule:RunInterval( dt )
    self.lastMsdt = dt
    local tmp = self._pl_ms.next

    self._pl_ms.next = nil
    self._pl_ms.prev = nil

    while tmp do
        local next = tmp.next
        tmp.next = nil
        if dt >= tmp.dopoint then
            tmp.func()
            if tmp.count > 0 then
                tmp.count = tmp.count - 1
            end
            if tmp.count ~= 0 then
                tmp.dopoint = dt + tmp.interval
                self:AddIntervalNode(tmp)
            end
        else
            self:AddIntervalNode(tmp)
        end
        tmp = next
    end
end

function Schedule:AddIntervalTask(_func,_interval,_delay,_count)
    if _count == 0 then
        return
    end
    local node = {
        func = _func,
        interval = _interval,
        dopoint = self.lastMsdt + _interval + (_delay or 0);
        count = _count or -1;
    }

    self:AddIntervalNode(node)
end

function Schedule:AddIntervalNode(node)
    if self._pl_ms.prev then
        self._pl_ms.prev.next = node
        self._pl_ms.prev = node
    else
        self._pl_ms.next = node
        self._pl_ms.prev = node
    end
end

Schedule:init()
