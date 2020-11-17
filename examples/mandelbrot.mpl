function mandel_calc(a)
    z = 0
    for i in 1:100
        z = z^2 + a
    end
    return z
end

function mandelbrot()
    "starting mandelbrot"
    for i in 0:30
    	y = 0.066666*i - 1
	s = ""
	for j in 0:78
	    x = 0.0315*j - 2
	    if abs(mandel_calc((x, y))) < 2 then
	        s = s + "*"
	    else
	        s = s + " "
	    end
	end
	s
    end
end

mandelbrot()
