local Scan = pd.Class:new():register("scan")

local starX1, starY1 = 300, 300
local starRadius = 300
local maxDist = 8000
local subdiv = 1

function Scan:initialize(sel, atoms)
    self.inlets = 1
    self.delay_time = 40
    self:set_size(600, 600)
    return true
end

local function clamp(val, lower, upper)
    if lower > upper then lower, upper = upper, lower end -- swap if boundaries supplied the wrong way
    return math.max(lower, math.min(upper, val))
end

function Scan:postinitialize()
    self.clock = pd.Clock:new():register(self, "tick")
    self.clock:delay(self.delay_time)
end

function Scan:finalize()
    self.clock:destruct()
end

function Scan:in_1_maxdist(d)
    maxDist = d[1]
    subdiv = 16000 / maxDist
    if(subdiv < 2) then subdiv = 1
    elseif(subdiv < 5) then subdiv = 2
    elseif(subdiv < 10) then subdiv = 5
    else subdiv = 10
    end

    --pd.post(tostring(d[1]))
end

--[[
function Scan:mouse_down(x, y)
    if x > self.draggable_rect_x and y > self.draggable_rect_y and x < self.draggable_rect_x + self.draggable_rect_size and y < self.draggable_rect_y + self.draggable_rect_size then
        dragging_rect = true
        self.mouse_down_pos[0] = x
        self.mouse_down_pos[1] = y
        self.rect_down_pos[0] = self.draggable_rect_x
        self.rect_down_pos[1] = self.draggable_rect_y
    else
        dragging_rect = false
    end
end

function Scan:mouse_drag(x, y)
    if dragging_rect == true then
        local w, h = self:get_size()
        self.draggable_rect_x = self.rect_down_pos[0] + (x - self.mouse_down_pos[0])
        self.draggable_rect_y = self.rect_down_pos[1] + (y - self.mouse_down_pos[1])
        self.draggable_rect_x = math.clamp(self.draggable_rect_x, 0, w - self.draggable_rect_size)
        self.draggable_rect_y = math.clamp(self.draggable_rect_y, 0, h - self.draggable_rect_size)
        self:repaint()
    end
end
--]]
local pi = math.pi
local sin = math.sin
local cos = math.cos

function Scan:paint(g)

    g:set_color(50, 50, 50)
    g:fill_all()

    g:set_color(50, 200, 20, 0.3)
    g:fill_ellipse(290, 290, 20, 20)
--    g:set_color(200, 20, 20, 0.5)
    g:fill_rect(300, 298, 30, 4)


    for i = 0, maxDist / 1000 do
        if(i ~= 0) then
            g:set_color(220 , 150 + 70 * (i % 2), 250)
            local r = starRadius * (i * 1000) / maxDist
            if(r < starRadius) then
                g:stroke_ellipse(starX1 - r, starY1 - r, 2 * r, 2 * r, 1)
                g:draw_text(string.format("%d m", i), 290, starY1 - r - 14, 0, 12)
            end
        end
        if(subdiv > 1) then
            for j = 1, subdiv - 1 do
                g:set_color(30 + 70 * (j % 2), 90, 200)
                local r = starRadius * ((i + (j / subdiv)) * 1000) / maxDist
                if(r < starRadius) then
                    g:stroke_ellipse(starX1 - r, starY1 - r, 2 * r, 2 * r, 1)
                    g:draw_text(string.format("%.1f m", i + j / subdiv), 290, starY1 - r - 14, 0, 12)
                end
            end
        end
    end

    g:set_color(252, 218, 81, 1)
    local t = pd.Table:new():sync("lidar_distance_masked")
    local l = t:length()
    local a = 2 * pi * (359/360)

    if false then
        local function coord(i)
            local a = 2 * pi * (i / l)
            --local r = clamp(starRadius * t:get(i) / maxDist, 0, starRadius)
            local r = starRadius * ((t:get(i) < 4000) and 0.9 or 0)
            return r * cos(a) + starX1, r * sin(a) + starY1
        end
        --local r = starRadius * t:get(l-1) / maxDist
        --local star = Path(r * cos(a) + starX1, r * sin(a) + starY1)
        local star = Path(coord(l - 1))
        for i = 0, l-1 do
            --local a = 2 * pi * (i / l)
            --local r = clamp(starRadius * t:get(i) / maxDist, 0, starRadius)
            --star:line_to(r * cos(a) + starX1, r * sin(a) + starY1)
            star:line_to(coord(i))
        end
        star:close()
        --g:fill_path(star)
        g:stroke_path(star, 1)
    end

    g:set_color(255, 150, 120, 1)
    local path = nil
    for i = 0, l-1 do
        local a = 2 * pi * (i / l)
        local p = t:get(i) < 4000
        local x, y = 0.9 * starRadius * cos(a) + starX1, 0.9 * starRadius * sin(a) + starY1
        if(p) then 
            if(path) then path:line_to(x, y)
            else path = Path(x, y)
            end
        else if(path) then g:stroke_path(path, 20); path = nil; end
        end
        --star:line_to(r * cos(a) + starX1, r * sin(a) + starY1)
    end
    if(path) then g:stroke_path(path, 20); path = nil; end

    -- Titles
    -- g:set_color(252, 118, 81, 1)
    -- g:draw_text("Ellipse", 300, 10, 120, 18)
end

function Scan:tick()
    --self.circle_y = self.circle_y + self.animation_speed
    --if self.circle_y > 160 + self.circle_radius then
    --    self.circle_y = 0 -- -self.circle_radius
    --end
    self:repaint()
    self.clock:delay(self.delay_time)
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
