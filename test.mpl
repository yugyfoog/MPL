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
    a = { 5, 1, (2,3), (1,2), [1,2,3], "abc" }
    b = { 3, (2,3), 1, (3,4), [4,5,6], "def" }
    q = { 8, (3,3), (3,3), (4,6), [5,7,9], "abcdef" }
    s = { "real + real", "real + complex", "complex + real", \
          "complex + complex", "vector + vector", "string + string" }
    c = a + b
    i = 0
    while i < size(a)
        test(q[i], c[i], s[i])
        i = i + 1
    end
end

function test_subtraction()
    a = { 5, 1, (2,3), (1,2), [4,5,6], "abc" }
    b = { 3, (2,3), 1, (3,4), [1,2,3], "def" }
    x = { 2, (-1,-3), (1,3), (-2,-2), [3,3,3], "abcfed" }
    s = { "real - real", "real - complex", "complex - real", \
          "complex - complex", "vector - vector", "string - string" }
    c = a - b
    i = 0
    while i < size(a)
        test(x[i], c[i], s[i])
        i = i + 1
    end
end

function test_multiplication()
    a = { 5, 2, (2,3), (1,2), 2, [1,2,3], [1,2,3], 2 }
    b = { 3, (2,3), 2, (3,4), [1,2,3], 2, [2,3,4], "abc" }
    q = { 15, (4,6), (4,6), (-5,10), [2,4,6], [2,4,6], 20, "abcabc" }
    s = { "real * real", "real * complex", "complex * real", \
          "complex * complex", "real * vector", "vector * real", \
	  "vector * vector", "real * string" }
    c = a*b
    i = 0
    while i < size(a)
        test(q[i], c[i], s[i])
        i = i + 1
    end
end

function test_division()
    a = { 6, 13, (4,6), (-5,10), [2,4,6] }
    b = { 2, (2,-3), 2, (3,4), 2 }
    q = { 3, (2,3), (2,3), (1,2), [1,2,3] }
    s = { "real / real", "real / complex", "complex / real", \
          "complex / complex", "vector / real" }
    c = a/b
    i = 0
    while i < size(a)
        test(q[i], c[i], s[i])
        i = i + 1
    end
end

function test_floored_division()
    a = { 8 }
    b = { 3 }
    q = { 2 }
    s = { "real div real" }
    c = a div b
    i = 0
    while i < size(a)
        test(q[i], c[i], s[i])
        i = i + 1
    end
end

function test_modulo()
    a = { 8 }
    b = { 3 }
    q = { 2 }
    s = { "real mod real" }
    c = a mod b
    i = 0
    while i < size(a)
        test(q[i], c[i], s[i])
        i = i + 1
    end
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
    a = {3, (1,2), [1,2,3], "abc"}
    q = {-3, (-1,-2), [-1,-2,-3], "cba" }
    s = { "- real", "- complex", "- vector", "- string" }
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
    a = { 3, 1, (1,2), (1,2), [1,2,3], "abc" }
    b = { 3, (2,3), 3, (3,4), [1,2,3], "def" }
    q = { 1, 0, 0, 0, 1, 0 }
    s = { "real == real", "real == complex", "complex == real", \
           "complex == complex", "vector == vector", "string == string" }
    c = a == b
    i = 0
    while i < size(a)
        test(q[i], c[i], s[i])
	i = i + 1
    end
end

function test_ne()
    a = { 3, 1, (1,2), (1,2), [1,2,3], "abc" }
    b = { 3, (2,3), 3, (3,4), [1,2,3], "def" }
    q = { 0, 1, 1, 1, 0, 1 }
    s = { "real != real", "real != complex", "complex != real", \
           "complex != complex", "vector != vector", "string != string" }
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
    test_string_index()
    test_list_index()
end

function mpl_test()
    test_arithmetic()
    test_logic()
    test_relational_operators()
    test_index()
end

mpl_test()

"Done testing"
