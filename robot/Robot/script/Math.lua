_Math = {}

function _Math.Normalise(x,y)
    local len = math.sqrt(x*x+y*y)
    return x/len,y/len
end