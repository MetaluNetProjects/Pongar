local Scan = pd.Class:new():register("dmxspot")

function Scan:initialize(sel, atoms)
    self.inlets = 1
    self:setsize(250)
    self.pdiam = 20
    self.pan = 0
    self.tilt = 0
    self.pan_offset = 0

    return true
end

local function clamp(val, lower, upper)
    if lower > upper then lower, upper = upper, lower end -- swap if boundaries supplied the wrong way
    return math.max(lower, math.min(upper, val))
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

local pi = math.pi
local sin = math.sin
local cos = math.cos

function Scan:paint(g)

    --g:set_color(50, 50, 50)
    --g:fill_all()

    g:set_color(220 , 220, 250)
    g:fill_ellipse(0, 0, self.size, self.size, 1)

    local r = (self.radius - self.pdiam / 2) * self.tilt / 45
    local x = self.center + r * cos(self.pan / 180 * pi) - self.pdiam / 2
    local y = self.center + r * sin(self.pan / 180 * pi) - self.pdiam / 2
    g:set_color(50, 200, 20, 0.3)
    g:fill_ellipse(x, y, self.pdiam, self.pdiam)
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
