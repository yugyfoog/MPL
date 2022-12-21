function compare(a, b)
    if islist(a) and islist(b) then
        if size(a) != size(b) then
	    return 0
	end
	for i in 0:size(a)-1
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
    a-b
    ~exit(1)
end

function test(a, b, s)
    if not compare(a, b) then
        error(a, b, s)
    end
end

function test_addition()
    a = { 5, 1, (2,3), (1,2), [1,2,3], [1,2], [(1,2),(3,4)], \
          [(1,2),(3,4)], \
	  [1,2|3,4], [1,2|3,4], [(1,2),(3,4)|(5,6),(7,8)], \
	  [(1,2),(3,4)|(5,6),(7,8)], "abc" }
    b = { 3, (2,3), 1, (3,4), [4,5,6], [(1,2),(3,4)], [1,2], [(5,6),(7,8)], \
          [5,6|7,8], [(1,2),(3,4)|(5,6),(7,8)], [1,2|3,4], \
	  [(8,7),(6,5)|(4,3),(2,1)], "def" }
    q = { 8, (3,3), (3,3), (4,6), [5,7,9], [(2,2),(5,4)], [(2,2),(5,4)], [(6,8),(10,12)], \
          [6,8|10,12], [(2,2),(5,4)|(8,6),(11,8)], [(2,2),(5,4)|(8,6),(11,8)], \
	  [(9,9),(9,9)|(9,9),(9,9)], "abcdef" }
    s = { "real + real", "real + complex", "complex + real", \
          "complex + complex", "vector + vector", "vector + cvector", \
	  "cvector + vector", "cvector + cvector", \
	  "matrix + matrix", "matrix + cmatrix", "cmatrix + matrix", \
	  "cmatrix + cmatrix", "string + string" }
    c = a + b
    for i in 0:size(a)-1   
        test(q[i], c[i], s[i])
        i = i + 1
    end
end

function test_subtraction()
    a = { 5, 1, (2,3), (1,2), [4,5,6], [1,2], [(1,2),(3,4)], [(1,2),(3,4)], \
          [5,6|7,8], [1,2|3,4], [(1,2),(3,4)|(5,6),(7,8)], \
	  [(8,7),(6,5)|(4,3),(2,1)], "abc" }
    b = { 3, (2,3), 1, (3,4), [1,2,3], [(1,2),(3,4)], [1,2], [(5,6),(7,8)], \
          [1,2|3,4], [(1,2),(3,4)|(5,6),(7,8)], [1,2|3,4], \
	  [(1,2),(3,4)|(5,6),(7,8)], "def" }
    x = { 2, (-1,-3), (1,3), (-2,-2), [3,3,3], [(0,-2),(-1,-4)], \
          [(0,2),(1,4)], [(-4,-4),(-4,-4)], \
	  [4,4|4,4], [(0,-2),(-1,-4)|(-2,-6),(-3,-8)], \
	  [(0,2),(1,4)|(2,6),(3,8)], [(7,5),(3,1)|(-1,-3),(-5,-7)], "abcfed" }
    s = { "real - real", "real - complex", "complex - real", \
          "complex - complex", "vector - vector", "vector - cvector", \
	  "cvector - vector", "cvector - cvector", \
	  "matrix - matrix", "matrix - cmatrix", "cmatrix - matrix", \
	  "cmatrix - cmatrix", "string - string" }
    c = a - b
    for i in 0:size(a)-1
        test(x[i], c[i], s[i])
        i = i + 1
    end
end

function test_multiplication()
    a = { 5,                  2,                  (2,3),            (1,2), \
          2,                  [1,2,3],            [1,2,3], \
	  2,                  (1,2),              (1,2), \
	  [(1,2),(3,4)],      [1,2],              [(1,2),(3,4)], \
	  [1,2],              [(1,2),(3,4)],      [(1,2),(3,4)], \
	  2,                  [1,2|3,4],          [1,2|3,4], \
	  [1,2|3,4],          (1,2),              2, \
	  (1,2),              [1,2|3,4],          [(1,2),(3,4)|(5,6),(7,8)], \
	  [(1,2),(3,4)|(5,6),(7,8)],              [(1,2),(3,4)|(5,6),(7,8)], \
	  [1,2|3,4],          [(1,2),(3,4)|(5,6),(7,8)], \
	  [(1,2),(3,4)|(5,6),(7,8)],              [1,2|3,4], \
	  [(1,2),(3,4)|(5,6),(7,8)], \
	  2,                  "abc" }
	  
    b = { 3,                  (2,3),              2,                (3,4), \
          [1,2,3],            2,                  [2,3,4], \
          [(1,2),(3,4)],      [1,2],              [(1,2),(3,4)], \
	  2,                  (1,2),              (1,2), \
	  [(1,2),(3,4)],      [1,2],              [(4,3),(2,1)], \
	  [1,2|3,4],          2,                  [1,2], \
	  [1,3|2,4],          [1,2|3,4],          [(1,2),(3,4)|(5,6),(7,8)], \
	  [(1,2),(3,4)|(5,6),(7,8)], (1,2),       [1,2], \
	  2,                  (1,2), \
	  [(1,2),(3,4)],      [(1,2),(3,4)], \
	  [1,2|3,4],          [(1,2),(3,4)|(5,6),(7,8)], \
	  [(8,7),(6,5)|(4,3),(2,1)], \
	  "abc",              2 }
	  
    q = { 15,                 (4,6),              (4,6),            (-5,10), \
          [2,4,6],            [2,4,6],            20, \
          [(2,4),(6,8)],      [(1,2),(2,4)],      [(-3,4),(-5,10)], \
	  [(2,4),(6,8)],      [(1,2),(2,4)],      [(-3,4),(-5,10)], \
	  (7,-10),            (7,10),             (20,10), \
	  [2,4|6,8],          [2,4|6,8],          [5,11], \
	  [5,11|11,25],       [(1,2),(2,4)|(3,6),(4,8)], [(2,4),(6,8)|(10,12),(14,16)], \
	  [(-3,4),(-5,10)|(-7,16),(-9,22)], [(1,2),(2,4)|(3,6),(4,8)], [(7,10),(19,22)], \
	  [(2,4),(6,8)|(10,12),(14,16)], [(-3,4),(-5,10)|(-7,16),(-9,22)], \
	  [(7,10),(15,22)],   [(-10,28),(-18,68)], \
	  [(10,14),(14,20)|(26,30),(38,44)], [(11,14),(17,20)|(23,30),(37,44)], \
	  [(-6,48),(-2,28)|(2,136),(6,84)], \
	  "abcabc",           "abcabc" }
	  
    s = { "real * real",      "real * complex",   "complex * real", "complex * complex", \
          "real * vector",    "vector * real",    "vector * vector", \
	  "real * cvector",   "complex * vector", "complex * cvector", \
	  "cvector * real",   "vector * complex", "cvector * complex", \
	  "vector * cvector", "cvector * vector", "cvector * cvector", \
	  "real * matrix",    "matrix * real",    "matrix * vector", \
	  "matrix * matrix",  "complex * matrix", "real * cmatrix", \
	  "complex * cmatrix", "matrix * complex", "matrix * complex", \
	  "cmatrix * real",   "cmatrix * complex", \
	  "matrix * cvector", "cmatrix * cvector", \
	  "cmatrix * matrix", "matrix * cmatrix", \
	  "cmatrix * cmatrix", \
	  "real * string",    "string * real" }
    c = a*b
    for i in 0:size(a)-1
        test(q[i], c[i], s[i])
        i = i + 1
    end
end

function test_pointwise_multiplication()
    a = { [1,2,3], [1,2,3], [(1,2),(3,4),(5,6)], [(1,2),(3,4)], \
          [1,2|3,4], [1,2|3,4], [(1,2),(3,4)|(5,6),(7,8)], \
	  [(1,2),(3,4)|(5,6),(7,8)] }

    b = { [4,5,6], [(4,5),(6,7),(8,9)],[7,8,9], [(5,6),(7,8)], \
          [5,6|7,8], [(1,2),(3,4)|(5,6),(7,8)], [1,2|3,4], \
	  [(1,2),(3,4)|(5,6),(7,8)] }

    q = { [4,10,18], [(4,5),(12,14),(24,27)], \
          [(7,14),(24,32),(45,54)], [(-7,16),(-11,52)], \
	  [5,12|21,32], [(1,2),(6,8)|(15,18),(28,32)], \
	  [(1,2),(6,8)|(15,18),(28,32)], \
	  [(-3,4),(-7,24)|(-11,60),(-15,112)] }

    s = { "vector .* vector", "vector .* cvector", "cvector .* vector", \
          "cvector .* cvector", "matrix .* matrix", "matrix .* cmatrix", \
	  "cmatrix .* matrix", "cmatrix .* cmatrix" }

    c = a.*b
    
    for i in 0:size(a)-1
        test(q[i], c[i], s[i])
	i = i + 1
    end
end

function test_division()
    a = { 6, 13, (4,6), (-5,10), [2,4,6], [1,2], [(1,2),(3,4)], \
          [(1,2),(3,4)], [2,4|6,8], [1,2|3,4], \
	  [(1,2),(3,4)|(5,6),(7,8)], [(1,2),(3,4)|(5,6),(7,8)] }
    b = { 2, (2,-3), 2, (3,4), 2, (1,2), 2, (1,2), 2, (1,2), 2, (1,2) }
    q = { 3, (2,3), (2,3), (1,2), [1,2,3], [(0.2,-0.4),(0.4,-0.8)], \
          [(0.5,1),(1.5,2)], [(1,0),(2.2,-0.4)], [1,2|3,4], \
	  [(0.2,-0.4),(0.4,-0.8)|(0.6,-1.2),(0.8,-1.6)], \
	  [(0.5,1),(1.5,2)|(2.5,3),(3.5,4)], \
	  [(1,0),(2.2,-0.4)|(3.4,-0.8),(4.6,-1.2)] }
    s = { "real / real", "real / complex", "complex / real", \
          "complex / complex", "vector / real", "vector / complex", \
	  "cvector / real", "cvector / complex", "matrix / real", \
	  "matrix / complex", "cmatrix / real", "cmatrix / complex" }
    c = a/b
    for i in 0:size(a)-1
        test(q[i], c[i], s[i])
        i = i + 1
    end
end

function test_pointwise_division()
    a = { [4,10,18], [5,25,100], \
          [(7,14),(24,32),(45,54)], [(11,52),(-11,52)], \
	  [5,12|21,32], [5,25|100,113], \
	  [(1,2),(6,8)|(15,18),(28,32)], \
	  [(-3,4),(-7,24)|(11,52),(-15,112)] }

    b = { [4,5,6], [(1,2),(3,4),(6,8)], [7,8,9], [(8,7),(7,8)], \
          [5,6|7,8], [(1,2),(3,4)|(6,8),(7,8)], [1,2|3,4], \
	  [(1,2),(3,4)|(8,7),(7,8)] }

    q = { [1,2,3], [(1,-2),(3,-4),(6,-8)], [(1,2),(3,4),(5,6)], [(4,3),(3,4)], \
          [1,2|3,4], [(1,-2),(3,-4)|(6,-8),(7,-8)], [(1,2),(3,4)|(5,6),(7,8)], \
	  [(1,2),(3,4)|(4,3),(7,8)] }
	  
    c = a./b

     s = { "vector ./ vector", "vector ./ cvector", "cvector ./ vector", \
          "cvector ./ cvector", "matrix ./ matrix", "matrix ./ cmatrix", \
	  "cmatrix ./ matrix", "cmatrix ./ cmatrix" }

    for i in 0:size(a)-1
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
    a = { 3, 3, (3,0), (3,0) }
    b = { 2, (2,0), 2, (2,0) }
    q = { 9, (9,0), (9,0), (9.00000000000000177636,0) }
    s = { "real ^ real", "real ^ complex", "complex ^ real", \
          "complex ^ complex" }
    c = a^b
    for i in 0:size(a)-1
        test(q[i], c[i], s[i])
        i = i + 1
    end
end

function test_negation()
    a = {3, (1,2), [1,2,3], [(1,2),(3,4)], [1,2|3,4], \
         [(1,2),(3,4)|(5,6),(7,8)], "abc"}
    q = {-3, (-1,-2), [-1,-2,-3], [(-1,-2),(-3,-4)], [-1,-2|-3,-4], \
         [(-1,-2),(-3,-4)|(-5,-6),(-7,-8)], "cba" }
    s = { "- real", "- complex", "- vector", "- cvector", "- matrix", \
          "- cmatrix", "- string" }
    c = -a
    for i in 0:size(a)-1
        test(q[i], c[i], s[i])
	i = i + 1
    end
end

function test_arithmetic()
    test_addition()
    test_subtraction()
    test_multiplication()
    test_pointwise_multiplication()
    test_division()
    test_pointwise_division()
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
    for i in 0:size(a)-1
        test(q[i], c[i], "or")
	i = i + 1
    end
end

function test_and()
    a = {1, 1, 0, 0}
    b = {1, 0, 1, 0}
    q = {1, 0, 0, 0}
    c = a and b
    for i in 0:size(a)-1
        test(q[i], c[i], "and")
	i = i + 1
    end
end

function test_not()
    a = { 0, 1 }
    q = { 1, 0 }
    c = not a
    for i in 0:size(a)-1
        test(q[i], c[i], "not")
	i = i + 1
    end
end

function test_bor()
    test(14, 10 bor 12, "bor")
end

function test_bxor()
    test(6, 10 bxor 12, "bxor")
end

function test_band()
    test(8, 10 band 12, "band")
end

function test_bnot()
    test(-2, bnot 1, "bnot")
end

function test_logic()
    test_or()
    test_and()
    test_not()
    test_bor()
    test_bxor()
    test_band()
    test_bnot()
end

function test_eq()
    a = { 3, 1, (1,2), (1,2), [1,2,3], [1,2], [(1,0),(2,0)], \
          [(1,2),(3,4)], [1,2|3,4], [1,2|3,4], [(1,0),(2,0)|(3,0),(4,0)], \
	  [(1,2),(3,4)|(5,6),(7,8)], "abc" }
    b = { 3, (2,3), 3, (3,4), [1,2,3], [(1,0),(2,0)], [1,2], \
          [(1,2),(3,4)], [1,2|3,4], [(1,0),(2,0)|(3,0),(4,0)], [1,2|3,4], \
	  [(1,2),(3,4)|(5,6),(7,8)], "def" }
    q = { 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0 }
    s = { "real == real", "real == complex", "complex == real", \
           "complex == complex", "vector == vector", "vector == cvector", \
	   "cvector == vector", "cvector == cvector", "matrix == matrix", \
	   "matrix == cmatrix", "cmatrix == matrix", "cmatrix == cmatrix", "string == string" }
    c = a == b
    for i in 0:size(a)-1
        test(q[i], c[i], s[i])
	i = i + 1
    end
end

function test_ne()
    a = { 3, 1, (1,2), (1,2), [1,2,3], [1,2], [(1,0),(2,0)], \
          [(1,2),(3,4)], [1,2|3,4], [1,2|3,4], [(1,0),(2,0)|(3,0),(4,0)], \
	  [(1,2),(3,4)|(5,6),(7,8)], "abc" }
    b = { 3, (2,3), 3, (3,4), [1,2,3], [(1,0),(2,0)], [1,2], \
          [(1,2),(3,4)], [1,2|3,4], [(1,0),(2,0)|(3,0),(4,0)], [1,2|3,4], \
	  [(1,2),(3,4)|(5,6),(7,8)], "def" }
    q = { 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 }
    s = { "real != real", "real != complex", "complex != real", \
           "complex != complex", "vector != vector", "vector != cvector", \
	   "cvector != vector", "cvector != cvector", "matrix != matrix", \
	   "matrix == cmatrix", "cmatrix == matrix", "cmatrix == cmatrix", "string != string" }
    c = a != b
    for i in 0:size(a)-1
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
    for i in 0:size(a)-1
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
    for i in 0:size(a)-1
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
    for i in 0:size(a)-1
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
    for i in 0:size(a)-1
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

    a[1,2] = 60
    test([1,2,3|4,5,60|7,8,9], a, "matrix[i,i] = real")
    a[1,1:2] = [55,66]
    test([1,2,3|4,55,66|7,8,9], a, "matrix[i,i:i] = vector")
    a[1,0:2:2] = [400,600]
    test([1,2,3|400,55,600|7,8,9], a, "matrix[i,i:i:i] = vector")
    a[1:2,1] = [50,80]
    test([1,2,3|400,50,600|7,80,9], a, "matrix[i:i,i] = vector")
    a[1:2,1:2] = [555,666|888,999]
    test([1,2,3|400,555,666|7,888,999], a, "matrix[i:i,i:i] = matrix")
    a[1:2,0:2:2] = [404,606|707,909]
    test([1,2,3|404,555,606|707,888,909], a, "matrix[i:i,i:i:i] = matrix")
    a[0:2:2,1] = [20,80]
    test([1,20,3|404,555,606|707,80,909], a, "matrix[i:i:i,i] = vector")
    a[0:2:2,1:2] = [22,33|88,99]
    test([1,22,33|404,555,606|707,88,99], a, "matrix[i:i:i,i:i] = matrix")
    a[0:2:2,0:2:2] = [10,30|70,90]
    test([10,22,30|404,555,606|70,88,90], a, "matrix[i:i:i,i:i:i] = matrix")
end

function test_cmatrix_index()

    ~ simple index

    a = [(1,2),(3,4),(5,6)
         (7,8),(9,10),(11,12)
	 (13,14),(15,16),(17,18)]

    test([(7,8),(9,10),(11,12)], a[1], "cmatrix[i]")
    test([(7,8),(9,10),(11,12)|(13,14),(15,16),(17,18)], a[1:2], "cmatrix[i:i]")
    test([(1,2),(3,4),(5,6)|(13,14),(15,16),(17,18)], a[0:2:2], "cmatrix[i:i:i]")
    a[1] = [(70,80),(90,100),(110,120)]
    test([(1,2),(3,4),(5,6)|(70,80),(90,100),(110,120)|(13,14),(15,16),(17,18)], a, "cmatrix[i] = cvector")
    a[1:2] = [(77,88),(99,101),(111,122)|(133,144),(155,166),(177,188)]
    test([(1,2),(3,4),(5,6)|(77,88),(99,101),(111,122)|(133,144),(155,166),(177,188)], a, "cmatrix[i:i] = cmatrix")
    a[0:2:2] = [(11,22),(33,44),(55,66)|(31,41),(51,61),(71,81)]
    test([(11,22),(33,44),(55,66)|(77,88),(99,101),(111,122)|(31,41),(51,61),(71,81)], a, "cmatrix[i:i:i] = cmatrix")
    
    ~ row index

    a = [(1,2),(3,4),(5,6)
         (7,8),(9,10),(11,12)
	 (13,14),(15,16),(17,18)]

    test([(7,8),(9,10),(11,12)], a[1,], "cmatrix[i,]")
    test([(7,8),(9,10),(11,12)|(13,14),(15,16),(17,18)], a[1:2,], "cmatrix[i:i,]")
    test([(1,2),(3,4),(5,6)|(13,14),(15,16),(17,18)], a[0:2:2,], "cmatrix[i:i:i,]")
    a[1,] = [(70,80),(90,100),(110,120)]
    test([(1,2),(3,4),(5,6)|(70,80),(90,100),(110,120)|(13,14),(15,16),(17,18)], a, "cmatrix[i,] = cvector")
    a[1:2,] = [(77,88),(99,101),(111,122)|(133,144),(155,166),(177,188)]
    test([(1,2),(3,4),(5,6)|(77,88),(99,101),(111,122)|(133,144),(155,166),(177,188)], a, "cmatrix[i:i,] = cmatrix")
    a[0:2:2,] = [(11,22),(33,44),(55,66)|(31,41),(51,61),(71,81)]
    test([(11,22),(33,44),(55,66)|(77,88),(99,101),(111,122)|(31,41),(51,61),(71,81)], a, "cmatrix[i:i:i,] = cmatrix")
    
    ~ column index

    a = [(1,2),(3,4),(5,6)
         (7,8),(9,10),(11,12)
	 (13,14),(15,16),(17,18)]

    test([(3,4),(9,10),(15,16)], a[,1], "cmatrix[,i]")
    test([(3,4),(5,6)|(9,10),(11,12)|(15,16),(17,18)], a[,1:2], "cmatrix[,i]")
    test([(1,2),(5,6)|(7,8),(11,12)|(13,14),(17,18)], a[,0:2:2], "cmatrix[,i:i:i]")
    a[,1] = [(30,40),(90,100),(150,160)]
    test([(1,2),(30,40),(5,6)|(7,8),(90,100),(11,12)|(13,14),(150,160),(17,18)], a, "cmatrix[,i] = cvector")
    a[,1:2] = [(33,44),(55,66)|(99,101),(111,122)|(155,166),(177,188)]
    test([(1,2),(33,44),(55,66)|(7,8),(99,101),(111,122)|(13,14),(155,166),(177,188)], a, "cmatrix[,i:i] = cmatrix")
    a[,0:2:2] = [(11,22),(50,60)|(77,88),(11,21)|(31,41),(71,81)]
    test([(11,22),(33,44),(50,60)|(77,88),(99,101),(11,21)|(31,41),(155,166),(71,81)], a, "cmatrix[,i:i:i] = cmatrix")

    ~ matrix index

    a = [(1,2),(3,4),(5,6)
         (7,8),(9,10),(11,12)
	 (13,14),(15,16),(17,18)]

    test((11,12), a[1,2], "cmatrix[i,i]")
    test([(9,10),(11,12)], a[1,1:2], "cmatrix[i,i:i]")
    test([(7,8),(11,12)], a[1,0:2:2], "cmatrix[i,i:i:i]")
    test([(9,10),(15,16)], a[1:2,1], "cmatrix[i:i,i]")
    test([(9,10),(11,12)|(15,16),(17,18)], a[1:2,1:2], "cmatrix[i:i,i:i]")
    test([(7,8),(11,12)|(13,14),(17,18)], a[1:2,0:2:2], "cmatrix[i:i,i:i:i]")
    test([(3,4),(15,16)], a[0:2:2,1], "cmatrix[i:i:i,i]")
    test([(3,4),(5,6)|(15,16),(17,18)], a[0:2:2,1:2], "cmatrix[i:i:i,i:i]")
    test([(1,2),(5,6)|(13,14),(17,18)], a[0:2:2,0:2:2], "cmatrix[i:i:i,i:i:i]")

    a[1,2] = (111,112)
    test([(1,2),(3,4),(5,6)|(7,8),(9,10),(111,112)|(13,14),(15,16),(17,18)], a, "cmatrix[i,i] = complex")
    a[1,1:2] = [(209,210),(211,212)]
    test([(1,2),(3,4),(5,6)|(7,8),(209,210),(211,212)|(13,14),(15,16),(17,18)], a, "cmatrix[i,i:i] = cvector")
    a[1,0:2:2] = [(307,308),(311,312)]
    test([(1,2),(3,4),(5,6)|(307,308),(209,210),(311,312)|(13,14),(15,16),(17,18)], a, "cmatrix[i,i:i:i] = cvector")
    a[1:2,1] = [(409,410),(415,416)]
    test([(1,2),(3,4),(5,6)|(307,308),(409,410),(311,312)|(13,14),(415,416),(17,18)], a, "cmatrix[i:i,i] = cvector")
    a[1:2,1:2] = [(509,510),(511,512)|(515,516),(517,518)]
    test([(1,2),(3,4),(5,6)|(307,308),(509,510),(511,512)|(13,14),(515,516),(517,518)], a, "cmatrix[i:i,i:i] = cmatrix")
    a[1:2,0:2:2] = [(607,608),(611,612)|(613,614),(617,618)]
    test([(1,2),(3,4),(5,6)|(607,608),(509,510),(611,612)|(613,614),(515,516),(617,618)], a, "cmatrix[i:i,i:i:i] = cmatrix")
    a[0:2:2,1] = [(703,704),(715,716)]
    test([(1,2),(703,704),(5,6)|(607,608),(509,510),(611,612)|(613,614),(715,716),(617,618)], a, "cmatrix[i:i:i,i] = cvector")
    a[0:2:2,1:2] = [(803,804),(805,806)|(815,816),(817,818)]
    test([(1,2),(803,804),(805,806)|(607,608),(509,510),(611,612)|(613,614),(815,816),(817,818)], a, "cmatrix[i:i:i,i:i] = cmatrix")
    a[0:2:2,0:2:2] = [(901,902),(905,906)|(913,914),(917,918)]
    test([(901,902),(803,804),(905,906)|(607,608),(509,510),(611,612)|(913,914),(815,816),(917,918)], a, "cmatrix[i:i:i,i:i:i] = cmatrix")
    
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
    test_cmatrix_index()
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
