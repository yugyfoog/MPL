function compare(a, b)
    if islist(a) and islist(b) then
        if size(a) != size(b) then
	    return 0
	end
	i = 0
	while i < size(a)
	    if compare(a[i], b[i]) == 0 then
	        return 0
	    end
	    i = i + 1
	end
	return 1
    end
    return a == b
end

function error(a, b, s)
    "error " + s
    a
    b
    exit(1)
end

function test(a, b, s)
    if not compare(a, b) then
        error(a, b, s)
    end
end

function test_addition()
    a = { 5, 1, (2,3), (1,2), [1,2,3], [1,2], [(1,2),(3,4)], \
          [(1,2),(3,4)], \
	  [1,2|3,4], "abc" }
    b = { 3, (2,3), 1, (3,4), [4,5,6], [(1,2),(3,4)], [1,2], [(5,6),(7,8)], \
          [5,6|7,8], "def" }
    q = { 8, (3,3), (3,3), (4,6), [5,7,9], [(2,2),(5,4)], [(2,2),(5,4)], [(6,8),(10,12)], \
          [6,8|10,12], "abcdef" }
    s = { "real + real", "real + complex", "complex + real", \
          "complex + complex", "vector + vector", "vector + cvector", \
	  "cvector + vector", "cvector + cvector", \
	  "matrix + matrix", "string + string" }
    c = a + b
    i = 0
    while i < size(a)
        test(q[i], c[i], s[i])
        i = i + 1
    end
end

function test_subtraction()
    a = { 5, 1, (2,3), (1,2), [4,5,6], [1,2], [(1,2),(3,4)], [(1,2),(3,4)], \
          [5,6|7,8], "abc" }
    b = { 3, (2,3), 1, (3,4), [1,2,3], [(1,2),(3,4)], [1,2], [(5,6),(7,8)], \
          [1,2|3,4], "def" }
    x = { 2, (-1,-3), (1,3), (-2,-2), [3,3,3], [(0,-2),(-1,-4)], \
          [(0,2),(1,4)], [(-4,-4),(-4,-4)], \
	  [4,4|4,4], "abcfed" }
    s = { "real - real", "real - complex", "complex - real", \
          "complex - complex", "vector - vector", "vector - cvector", \
	  "cvector - vector", "cvector - cvector", \
	  "matrix - matrix", "string - string" }
    c = a - b
    i = 0
    while i < size(a)
        test(x[i], c[i], s[i])
        i = i + 1
    end
end

~ real * matrix ***
~ matrix * real ***
~ matrix * vector ***
~ matrix * matrix ***

~ complex * matrix
~ cmatrix * real
~ cmatrix * vector
~ cmatrix * matrix

~ real * cmatrix
~ matrix * complex
~ matrix * cvector
~ matrix * cmatrix

~ complex * cmatrix
~ cmatrix * complex
~ cmatrix * cvector
~ cmatrix * cmatrix

~ [ 1 2  [1 3  = [ 5 11
~   3 4 ] 2 4]     11 25 ]

function test_multiplication()
    a = { 5,                  2,                  (2,3),            (1,2), \
          2,                  [1,2,3],            [1,2,3], \
	  2,                  (1,2),              (1,2), \
	  [(1,2),(3,4)],      [1,2],              [(1,2),(3,4)], \
	  [1,2],              [(1,2),(3,4)],      [(1,2),(3,4)], \
	  2,                  [1,2|3,4],          [1,2|3,4], \
	  [1,2|3,4], \
	  2,                  "abc" }
	  
    b = { 3,                  (2,3),              2,                (3,4), \
          [1,2,3],            2,                  [2,3,4], \
          [(1,2),(3,4)],      [1,2],              [(1,2),(3,4)], \
	  2,                  (1,2),              (1,2), \
	  [(1,2),(3,4)],      [1,2],              [(4,3),(2,1)], \
	  [1,2|3,4],          2,                  [1,2], \
	  [1,3|2,4], \
	  "abc",              2 }
	  
    q = { 15,                 (4,6),              (4,6),            (-5,10), \
          [2,4,6],            [2,4,6],            20, \
          [(2,4),(6,8)],      [(1,2),(2,4)],      [(-3,4),(-5,10)], \
	  [(2,4),(6,8)],      [(1,2),(2,4)],      [(-3,4),(-5,10)], \
	  (7,-10),            (7,10),             (20,10), \
	  [2,4|6,8],          [2,4|6,8],          [5,11], \
	  [5,11|11,25], \
	  "abcabc",           "abcabc" }
	  
    s = { "real * real",      "real * complex",   "complex * real", "complex * complex", \
          "real * vector",    "vector * real",    "vector * vector", \
	  "real * cvector",   "complex * vector", "complex * cvector", \
	  "cvector * real",   "vector * complex", "cvector * complex", \
	  "vector * cvector", "cvector * vector", "cvector * cvector", \
	  "real * matrix",    "matrix * real",    "matrix * vector", \
	  "matrix * matrix", \
	  "real * string",    "string * real" }
    c = a*b
    i = 0
    while i < size(a)
        test(q[i], c[i], s[i])
        i = i + 1
    end
end

function test_division()
    a = { 6, 13, (4,6), (-5,10), [2,4,6], [1,2], [(1,2),(3,4)], \
          [(1,2),(3,4)], [2,4|6,8] }
    b = { 2, (2,-3), 2, (3,4), 2, (1,2), 2, (1,2), 2 }
    q = { 3, (2,3), (2,3), (1,2), [1,2,3], [(0.2,-0.4),(0.4,-0.8)], \
          [(0.5,1),(1.5,2)], [(1,0),(2.2,-0.4)], [1,2|3,4] }
    s = { "real / real", "real / complex", "complex / real", \
          "complex / complex", "vector / real", "vector / complex", \
	  "cvector / real", "cvector / complex", "matrix / real" }
    c = a/b
    i = 0
    while i < size(a)
        test(q[i], c[i], s[i])
        i = i + 1
    end
end

function test_floored_division()
    test(2, 8 div 3, "real div real")
end

function test_modulo()
    test(2, 8 mod 3, "real mod real")
end

function test_exponent()
    a = { 3 }
    b = { 2, (2,-3), 2, (3,4), 2 }
    q = { 9, (2,3), (2,3), (1,2), [1,2,3] }
    s = { "real ^ real", "real ^ complex", "complex ^ real", \
          "complex ^ complex", "vector ^ real" }
    c = a^b
    i = 0
    while i < size(a)
        test(q[i], c[i], s[i])
        i = i + 1
    end
end

function test_negation()
    a = {3, (1,2), [1,2,3], [(1,2),(3,4)], [1,2|3,4], "abc"}
    q = {-3, (-1,-2), [-1,-2,-3], [(-1,-2),(-3,-4)], [-1,-2|-3,-4], "cba" }
    s = { "- real", "- complex", "- vector", "- cvector", "- matrix", "- string" }
    c = -a
    i = 0
    while i < size(a)
        test(q[i], c[i], s[i])
	i = i + 1
    end
end

function test_arithmetic()
    test_addition()
    test_subtraction()
    test_multiplication()
    test_division()
    test_floored_division()
    test_modulo()
    test_exponent()
    test_negation()
end

function test_or()
    a = {1, 1, 0, 0}
    b = {1, 0, 1, 0}
    q = {1, 1, 1, 0}
    c = a or b
    i = 0
    while i < size(a)
        test(q[i], c[i], "or")
	i = i + 1
    end
end

function test_and()
    a = {1, 1, 0, 0}
    b = {1, 0, 1, 0}
    q = {1, 0, 0, 0}
    c = a and b
    i = 0
    while i < size(a)
        test(q[i], c[i], "and")
	i = i + 1
    end
end

function test_not()
    a = { 0, 1 }
    q = { 1, 0 }
    c = not a
    i = 0
    while i < size(a)
        test(q[i], c[i], "not")
	i = i + 1
    end
end

function test_logic()
    test_or()
    test_and()
    test_not()
end

function test_eq()
    a = { 3, 1, (1,2), (1,2), [1,2,3], [1,2], [(1,0),(2,0)], \
          [(1,2),(3,4)], [1,2|3,4], "abc" }
    b = { 3, (2,3), 3, (3,4), [1,2,3], [(1,0),(2,0)], [1,2], \
          [(1,2),(3,4)], [1,2|3,4], "def" }
    q = { 1, 0, 0, 0, 1, 1, 1, 1, 1, 0 }
    s = { "real == real", "real == complex", "complex == real", \
           "complex == complex", "vector == vector", "vector == cvector", \
	   "cvector == vector", "cvector == cvector", "matrix == matrix", "string == string" }
    c = a == b
    i = 0
    while i < size(a)
        test(q[i], c[i], s[i])
	i = i + 1
    end
end

function test_ne()
    a = { 3, 1, (1,2), (1,2), [1,2,3], [1,2], [(1,0),(2,0)], \
          [(1,2),(3,4)], [1,2|3,4], "abc" }
    b = { 3, (2,3), 3, (3,4), [1,2,3], [(1,0),(2,0)], [1,2], \
          [(1,2),(3,4)], [1,2|3,4], "def" }
    q = { 0, 1, 1, 1, 0, 0, 0, 0, 0, 1 }
    s = { "real != real", "real != complex", "complex != real", \
           "complex != complex", "vector != vector", "vector != cvector", \
	   "cvector != vector", "cvector != cvector", "matrix != matrix", \
	   "string != string" }
    c = a != b
    i = 0
    while i < size(a)
        test(q[i], c[i], s[i])
	i = i + 1
    end
end

function test_lt()
    a = { 3, "abc" }
    b = { 3, "def" }
    q = { 0, 1 }
    s = { "real < real", "string < string" }
    c = a < b
    i = 0
    while i < size(a)
        test(q[i], c[i], s[i])
	i = i + 1
    end
end

function test_le()
    a = { 3, "abc" }
    b = { 3, "def" }
    q = { 1, 1 }
    s = { "real <= real", "string <= string" }
    c = a <= b
    i = 0
    while i < size(a)
        test(q[i], c[i], s[i])
	i = i + 1
    end
end

function test_gt()
    a = { 3, "abc" }
    b = { 3, "def" }
    q = { 0, 0 }
    s = { "real > real", "string > string" }
    c = a > b
    i = 0
    while i < size(a)
        test(q[i], c[i], s[i])
	i = i + 1
    end
end

function test_ge()
    a = { 3, "abc" }
    b = { 3, "def" }
    q = { 1, 0 }
    s = { "real >= real", "string >= string" }
    c = a >= b
    i = 0
    while i < size(a)
        test(q[i], c[i], s[i])
	i = i + 1
    end
end

function test_relational_operators()
    test_eq()
    test_ne()
    test_lt()
    test_le()
    test_gt()
    test_ge()
end

function test_vector_index()
    a = [1, 2, 3, 4]
    test(2, a[1], "vector[i]")
    test([2, 3], a[1:2], "vector[i:i]")
    test([2, 4], a[1:2:2], "vector[i:i:i]")
    a[1] = 20
    test([1, 20, 3, 4], a, "vector[i] = real")
    a[1:2] = [200, 300]
    test([1, 200, 300, 4], a, "vector[i:i] = vector")
    a[1:2:2] = [2000, 4000]
    test([1, 2000, 300, 4000], a, "vector[i:i:i] = vector")
end

function test_cvector_index()
    a = [(1,2),(3,4),(5,6),(7,8)]
    test((3,4), a[1], "cvector[i]")
    test([(3,4),(5,6)], a[1:2], "cvector[i:i]")
    test([(3,4),(7,8)], a[1:2:2], "cvector[i:i:i]")
    a[1] = (30,40)
    test([(1,2),(30,40),(5,6),(7,8)], a, "cvector[i] = real")
    a[1:2] = [(300,400),(500,600)]
    test([(1,2),(300,400),(500,600),(7,8)], a, "cvector[i:i] = cvector")
    a[1:2:2] = [(3000,4000), (7000,8000)]
    test([(1,2),(3000,4000), (500,600), (7000,8000)], a, "cvector[i:i:i] = vector")
end

function test_matrix_index()

    ~ simple index

    a = [1,2,3|4,5,6|7,8,9]
    test([4,5,6], a[1], "matrix[i]")
    test([4,5,6|7,8,9], a[1:2], "matrix[i:i]")
    test([1,2,3|7,8,9], a[0:2:2], "matrix[i:i:i]")
    a[1] = [40,50,60]
    test([1,2,3|40,50,60|7,8,9], a, "matrix[i] = vector")
    a[1:2] = [44,55,66|77,88,99]
    test([1,2,3|44,55,66|77,88,99], a, "matrix[i:i] = matrix")
    a[0:2:2] = [10,20,30|70,80,90]
    test([10,20,30|44,55,66|70,80,90], a, "matrix[i:i:i] = matrix")

    ~ row index

    a = [1,2,3|4,5,6|7,8,9]
    test([4,5,6], a[1,], "matrix[i,]")
    test([4,5,6|7,8,9], a[1:2,], "matrix[i:i,]")
    test([1,2,3|7,8,9], a[0:2:2,], "matrix[i:i:i,]")
    a[1,] = [40,50,60]
    test([1,2,3|40,50,60|7,8,9], a, "matrix[i,] = vector")
    a[1:2,] = [44,55,66|77,88,99]
    test([1,2,3|44,55,66|77,88,99], a, "matrix[i:i,] = matrix")
    a[0:2:2,] = [10,20,30|70,80,90]
    test([10,20,30|44,55,66|70,80,90], a, "matrix[i:i:i,] = matrix")

    ~ column index
    
    a = [1,2,3|4,5,6|7,8,9]
    test([2, 5, 8], a[,1], "matrix[,i]")
    test([2,3|5,6|8,9], a[,1:2], "matrix[,i:i]")
    test([1,3|4,6|7,9], a[,0:2:2], "matrix[,i:i:i]")
    a[,1] = [20,50,80]
    test([1,20,3|4,50,6|7,80,9], a, "matrix[,i] = vector")
    a[,1:2] = [22,33|55,66|88,99]
    test([1,22,33|4,55,66|7,88,99], a, "matrix[,i:i] = matrix")
    a[,0:2:2] = [10,30|40,60|70,90]
    test([10,22,30|40,55,60|70,88,90], a, "matrix[,i:i:i] = matrix")

    ~ matrix index


    ~ 1 2 3
    ~ 4 5 6
    ~ 7 8 9

    a = [1,2,3|4,5,6|7,8,9]
    test(6, a[1,2], "matrix[i,i]")
    test([5,6], a[1,1:2], "matrix[i,i:i]")
    test([4,6], a[1,0:2:2], "matrix[i,i:i:i]")
    test([5,8], a[1:2,1], "matrix[i:i,i]")
    test([5,6|8,9], a[1:2,1:2], "matrix[i:i,i:i]")
    test([4,6|7,9], a[1:2,0:2:2], "matrix[i:i,i:i:i]")
    test([2,8], a[0:2:2,1], "matrix[i:i:i,i]")
    test([2,3|8,9], a[0:2:2,1:2], "matrix[i:i:i,i:i]")
    test([1,3|7,9], a[0:2:2,0:2:2], "matrix[i:i:i,i:i:i]")

end

function test_string_index()
    a = "abcdefghijklmnopqrstuvwxyz"
    test("m", a[12], "string[i]")
    test("bcd", a[1:3], "string[i:i]")
    test("bdf", a[1:3:2], "string[i:i:i]")
    b = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    b[12] = "mark"
    test("ABCDEFGHIJKLmNOPQRSTUVWXYZ", b, "string[i] = s")
    b[0:13] = a[13:13]
    test("nopqrstuvwxyzNOPQRSTUVWXYZ", b, "string[i:i] = s")
    b = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    c = string(26)
    c[1:13:2] = a
    c[0:13:2] = b
    test("AaBbCcDdEeFfGgHhIiJjKkLlMm", c, "string[i:i:i]")
end

function test_list_index()
    a = {1, 2, 3, 4}
    test(2, a[1], "list[i]")
    test({2, 3}, a[1:2], "list[i:i]")
    test({2, 4}, a[1:2:2], "list[i:i:i]")
    a[1] = {2, 2.5}
    test({1, {2, 2.5}, 3, 4}, a, "list[i] = value")
    a[1:2] = {20, 30}
    test({1, 20, 30,4}, a, "list[i:i] = list")
    a[1:2:2] = {200, 400}
    test({1, 200, 30, 400}, a, "list[i:i:i] = list")
end

function test_index()
    test_vector_index()
    test_cvector_index()
    test_matrix_index()
    test_string_index()
    test_list_index()
end

function test_standard_functions()
    ~ not testing matvec(), exit(), read(), eof()
    ~ some tests may fail due to machine rounding
    test([0, 0, 0], vector(3), "vector()")
    test("   ", string(3), "string()")
    test({0, 0, 0}, list(3), "list()")
    test(3, size([0, 0, 0]), "size(vector)")
    test(3, size("abc"), "size(string)")
    test(3, size({1, 2, 3}), "size(list)")
    test({1,2,3,4,5,6}, concat({1,2,3},{4,5,6}), "concat(list,list)")
    test(REAL_TYPE, type(0), "type(e)")
    test(65, ascii("A"), "ascii(string)")
    test(1, real((1,2)), "real(complex)")
    test(2, imag((1,2)), "imag(complex)")
    test(2, abs(-2), "abs(real)")
    test(5, abs((3,4)), "abs(complex)")
    test(0, arg((1,0)), "arg(complex)")
    test(25, norm((3,4)), "norm(complex)")
    test((1,-2), conj((1,2)), "conj(complex)")
    test((1,0), polar(1,0), "polar(real,real)")
    test(5, sqrt(25), "sqrt(real)")
    test((1,2), sqrt((-3,4)), "sqrt(complex)")
    test(1, exp(0), "exp(real)")
    test((1,0), exp((0,0)), "exp(complex)")
    test(0, expm1(0), "exp(real)")
    test(0, log(1), "log(real)")
    test((0,0), log((1,0)), "log(complex)")
    test(0, log1p(0), "log1p(real)")
    test(1, log10(10), "log10(real)")
    test(0, sin(0), "sin(real)")
    test((0,0), sin((0,0)), "sin(complex)")
    test(1, cos(0), "cos(real)")
    test((1,0), cos((0,0)), "cos(complex)")
    test(0, tan(0), "tan(real)")
    test((0,0), tan((0,0)), "tan(complex)")
    test(1, csc(PI/2), "csc(real)")
    test((1,0), csc((PI/2,0)), "csc(complex)")
    test(1, sec(0), "sec(real)")
    test((1,0), sec((0,0)), "sec(complex")
    test(1+EPS, cot(PI/4), "cot(real)")
    test((1+EPS,0), cot((PI/4,0)), "cot(complex)")    
    test(0, asin(0), "asin(real)")
    test((0,0), asin((0,0)), "asin(complex)")
    test(0, acos(1), "acos(real)")
    test((0,0), acos((1,0)), "acos(complex)")
    test(0, atan(0), "atan(real)")
    test((0,0), atan((0,0)), "atan(complex)")
    test(PI/2, acsc(1), "acsc(real)")
    test((PI/2,0), acsc((1,0)), "acsc(comple)")
    test(0, asec(1), "sec(real)")
    test((0,0), asec((1,0)), "sec(complex)")
    test(PI/4, acot(1), "acot(real)")
    test((PI/4,0), acot((1,0)), "acot(complex)")
    test(0, sinh(0), "sinh(real)")
    test((0,0), sinh((0,0)), "sinh(complex)")
    test(1, cosh(0), "cosh(real)")
    test((1,0), cosh((0,0)), "cosh(complex)")
    test(0, tanh(0), "tanh(real)")
    test((0,0), tanh((0,0)), "tanh(complex)")
    test(0.850918128239321521, csch(1), "csch(real)")
    test((0.850918128239321521,0), csch((1,0)), "csch(complex)")
    test(1, sech(0), "sech(real)")
    test((1,0), sech((0,0)), "sech(complex)")
    test(0.642092615934330624, coth(1), "coth(real)")
    test((0.642092615934330735,0), coth((1,0)), "coth(real)")
    test(0, asinh(0), "asinh(real)")
    test((0,0), asinh((0,0)), "asinh(complex)")
    test(0, acosh(1), "acosh(real)")
    test((0,0), acosh((1,0)), "acosh(complex)")
    test(0, atanh(0), "atanh(real)")
    test((0,0), atanh((0,0)), "atanh(complex)")
    test(0.88137358701954303, acsch(1), "acsch(real)")
    test((0.881373587019542899,0), acsch((1,0)), "acsch(complex)")
    test(0, asech(1), "asech(real)")
    test((0,0), asech((1,0)), "asech(complex)")
    test(0.549306144334054734, acoth(2), "acoth(real)")
    test((0.549306144334054845,0), acoth((2,0)), "acoth(complex)")
    test(120, gamma(6), "gamma(real)")
end

function mpl_test()
    test_arithmetic()
    test_logic()
    test_relational_operators()
    test_index()
    test_standard_functions()
end

mpl_test()

"Done testing"
