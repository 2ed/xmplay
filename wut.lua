#!/usr/bin/env lua

require("xmplay")
xmplay.init()
xmplay.play("xm/lastrain.xm")

print("Press ^C when tired, or when music ends.")
print("Since we made xmplay.play asynchronous, this script just ends,\
if we don't sleep or spin in endless loop")

while true do
end
