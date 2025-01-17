local Scan = pd.Class:new():register("pongarGui")

function Scan:initialize(sel, atoms)
    --self.inlets = 1
    self:setsize(450)
    self.pdiam = 20
    self.border = 10
    self.pan = 0
    self.tilt = 0
    self.player_width = 30
    self.players = {}
    self.scan_offset = 0
    self.rcv_name = "pongarGui"
    if(atoms and atoms[1]) then self.rcv_name = atoms[1] end
    return true
end

function Scan:postinitialize()
    self.receiver = pd.Receive:new():register(self, self.rcv_name, "receive")
    pd.post("receive name: "..self.rcv_name)
    end

function Scan:finalize()
  self.receiver:destruct()
end

local function clamp(val, lower, upper)
    if lower > upper then lower, upper = upper, lower end -- swap if boundaries supplied the wrong way
    return math.max(lower, math.min(upper, val))
end

function Scan:receive(sel, atoms)
    --pd.post("received "..sel.." : "..tostring(atoms))
    --if(sel == "size") self.in_1_size
    self["in_1_"..sel](self, atoms)
    end

function Scan:setsize(s)
    self.size = s
    self.center = self.size / 2
    self.radius = self.size / 2
    self:set_size(self.size, self.size)
    end

function Scan:in_1_size(d)
    self:setsize(d[1])
    end

function Scan:in_1_pos(d)
    self.pan = d[1]
    self.tilt = d[2]
    --pd.post("new pos " .. self.pan .. " " .. self.tilt)
    self:repaint()
    end

function Scan:in_1_players(d)
    self.players = d
    --pd.post("nb players " .. #(self.players))
    end


local pi = math.pi
local sin = math.sin
local cos = math.cos

function Scan:paint(g)

    --g:set_color(50, 50, 50)
    --g:fill_all()

    -- background
    g:set_color(0 , 0, 70)
    g:fill_ellipse(0, 0, self.size, self.size, 1)

    -- dmx spot position
    local r = (self.radius - self.pdiam / 2 - 2 * self.border) * self.tilt / 45
    local x = self.center + r * cos(self.pan / 180 * pi) - self.pdiam / 2
    local y = self.center + r * sin(self.pan / 180 * pi) - self.pdiam / 2
    g:set_color(255, 50, 0, 0.3)
    g:fill_ellipse(x, y, self.pdiam, self.pdiam)

    -- players
    g:set_color(20, 250, 0, 1)
    local pw = self.player_width
    for i = 1, #(self.players) do
        local c = self.players[i]
        local path = nil
        for c2 = c - pw / 2, c + pw / 2 do
            local a = 2 * pi * (c2 / 360)
            local x, y = (self.radius - 3 * self.border / 2) * cos(a) + self.center, (self.radius - 3 * self.border / 2) * sin(a) + self.center
            if(path) then path:line_to(x, y)
            else path = Path(x, y)
            end
        end
        if(path) then g:stroke_path(path, self.border); end
    end

    -- draw lidar detection
    local t = pd.Table:new():sync("lidar_distance_masked")
    local l = t:length()
    --local a = 2 * pi * (359/360)
    g:set_color(220, 250, 0, 1)
    local path = nil
    for i = 0, l-1 do
        local a = 2 * pi * (i / l)
        local p = t:get(i) < 4000
        local x, y = (self.radius - self.border / 2) * cos(a) + self.center, (self.radius - self.border / 2) * sin(a) + self.center
        if(p) then 
            if(path) then path:line_to(x, y)
            else path = Path(x, y)
            end
        else if(path) then g:stroke_path(path, self.border); path = nil; end
        end
    end
    if(path) then g:stroke_path(path, self.border); path = nil; end
end

function Scan:in_1_bang()
    self:repaint()
end

function Scan:postreload()
   -- stuff to do post-reload goes here
   pd.post("Scan reloaded!")
   -- instead of doing a full initialization, you could also just change the
   -- number of inlets and outlets here
   self:initialize()
end
