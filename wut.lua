#!/usr/bin/env lua

require("xmplay")
xmplay.init()
xmplay.play("xm/lastrain.xm")

print("Press ^C when tired, or when music ends.")
print("Since we made xmplay.play asynchronous, this script just ends,\
if we don't sleep or spin in endless loop")

local position = 1
local len = 77
local cyan = string.char(27) .. "[36m"
local reset = string.char(27) .. "[0m"
local step = 1

while true do
   local prefix = string.rep(" ", position)
   local suffix = string.rep(" ", len - position)
   io.write("[" ..  prefix .. cyan .. "***" .. reset .. suffix .. "]\r")
   io.flush()
   position = position + step
   if position == len - 1 or position == 0 then
      step = -step
   end
   sleep(30)
end
