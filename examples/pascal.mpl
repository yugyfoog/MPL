function pascal(n)
    if n < 7 then
        field_width = 4
    else
        field_width = floor((lgamma(n+1) - 2*lgamma((n+1)/2))/log(10) + 1)
        if field_width mod 2 == 1 then
            field_width = field_width + 1
        end
    end
    line_width = (n+1)*field_width
    p = list(n+1)
    for i in 0:n  ~ for each row
        p[i] = 1
	for j in i-1:1:-1
	    p[j] = p[j-1] + p[j]
	end
        s = ""
	for j in 0:i
	    s = s + center(ffmt(p[j], 0), field_width)
	end
	center(s, line_width)	
    end
end
