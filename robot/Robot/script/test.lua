

local str = "asd ldkf df   sdf o  okds "

for s in string.gmatch( str,"[^%s]*" ) do
    print(s)
end