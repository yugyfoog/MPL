INF = #7ff0000000000000
NAN = #7ff8000000000000
EPS = #3cb0000000000000

TRUE = 1
FALSE = 0

E = 2.71828182845904523536
PI = 3.141592653589793236462643
GAMMA = 0.577215664901632860606512 ~ Euler-Mascheroni constant
I = (0,1)

function string(x)
    return x*" "
end

function value(x)
    return x
end

REAL_TYPE = 1
COMPLEX_TYPE = 2
VECTOR_TYPE = 3
CVECTOR_TYPE = 4
MATRIX_TYPE = 5
CMATRIX_TYPE = 6
STRING_TYPE = 7
LIST_TYPE = 8

function isreal(x)
    return type(x) == REAL_TYPE
end

function iscomplex(x)
    return type(x) == COMPLEX_TYPE
end

function isvector(x)
    return type(x) == VECTOR_TYPE
end

function iscvector(x)
    return type(x) == CVECTOR_TYPE
end

function ismatrix(x)
    return type(x) == MATRIX_TYPE
end

function iscmatrix(x)
    return type(x) == CMATRIX_TYPE
end

function isstring(x)
    return type(x) == STRING_TYPE
end

function islist(x)
    return type(x) == LIST_TYPE
end

function left(s, n)
    if n <= size(s) then
        return s
    end
    return s + (n-size(s))*" "
end

function right(s, n)
    if n <= size(s) then
        return s
    end
    return (n-size(s))*" " + s
end

function center(s, n)
    if n <= size(s) then
        return s
    end
    d = n - size(s)
    l = round(d/2)
    r = d - l
    return l*" " + s + r*" "
end

function min(x, y)
    if x <= y then
        return x
    end
    return y
end

function max(x, y)
    if x >= y then
        return x
    end
    return y
end

function cis(x)
    return (cos(x), sin(x))
end

function pow(x, y)
    return x^y
end

function csc(x)
    return 1/sin(x)
end

function sec(x)
    return 1/cos(x)
end

function cot(x)
    return 1/tan(x)
end

function acsc(x)
    return asin(1/x)
end

function asec(x)
    return acos(1/x)
end

function acot(x)
    return atan(1/x)
end

function csch(x)
    return 1/sinh(x)
end

function sech(x)
    return 1/cosh(x)
end

function coth(x)
    return 1/tan(x)
end

function acsch(x)
    return asinh(1/x)
end

function asech(x)
    return acosh(1/x)
end

function acoth(x)
    return atanh(1/x)
end

