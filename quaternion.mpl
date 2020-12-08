~ quaternions are represented as a vector(4)
~      r + xi + yj + zk => [r, x, y, z]

function quaternion_rotation(theta, v)
    st = sin(theta/2)
    return [cos(theta/2), st*v[0], st*v[1], st*v[2]]
end

function quaternion_add(x, y)
    return x + y
end

function quaternion_negate(x)
    return -x
end

function quaternion_subtract(x, y)
    return x - y
end

function quaternion_multiply(x, y)
    return [ x[0]*y[0] - x[1]*y[1] - x[2]*y[2] - x[3]*y[3], \
             x[0]*y[1] + x[1]*y[0] + x[2]*y[3] - x[3]*y[2], \
	     x[0]*y[2] + x[2]*y[0] + x[3]*y[1] - x[1]*y[3], \
	     x[0]*y[3] + x[3]*y[0] + x[1]*y[2] - x[2]*y[1] ]
end

function quaternion_conjugate(x)
    return [x[0], -x[1], -x[2], -x[3]]
end

function quaternion_normalize(x)
    return x/sqrt(x*x)
end

function quaternion_inverse(x)
    return quaternion_conjugate(x)/(x*x)
end

function quaternion_divide(x, y)
    return quaternion_multiply(x, quaternion_inverse(y))
end

function quaternion_rotate(x, y)
    return quaternion_multiply(quaternion_multiply(x, y), quaternion_inverse(x))
end

function quaternion_from_matrix(m)
    if m[2,2] < 0 then
        if m[0,0] > m[1,1] then
	    t = 1 + m[0,0] - m[1,1] - m[2,2]
	    q = [m[2,1] - m[1,2], t, m[1,0] + m[0,1], m[0,2] + m[2,0]]
	else
	    t = 1 - m[0,0] + m[1,1] - m[2,2]
	    q = [m[0,2] - m[2,0], m[1,0] + m[0,1], t, m[2,1] + m[1,2]]
	end
    else if m[0,0] < -m[1,1] then
        t = 1 - m[0,0] - m[1,1] + m[2,2]
	q = [m[1,0] - m[0,1], m[0,2] + m[2,0], m[2,1] + m[1,2], t]
    else
        t = 1 + m[0,0] + m[1,1] + m[2,2]
	q = [t, m[2,1] - m[1,2], m[0,2] - m[2,0], m[1,0] - m[0,1]]
    end
    return q/(2*sqrt(t))
end

function quaternion_to_matrix(q)
    return [ 1 - 2*q[2]*q[2] - 2*q[3]*q[3], 2*q[1]*q[2] - 2*q[3]*q[0], 2*q[1]*q[3] + 2*q[2]*q[0]
             2*q[1]*q[2] + 2*q[3]*q[0], 1 - 2*q[1]*q[1] - 2*q[3]*q[3], 2*q[2]*q[3] - 2*q[1]*q[0]
	     2*q[1]*q[3] - 2*q[2]*q[0], 2*q[2]*q[3] + 2*q[1]*q[0], 1 - 2*q[1]*q[1] - 2*q[2]*q[2] ]
end

function quaternion_slerp(xa, ya, t)
    x = quaternion_normalize(xa)
    y = quaternion_normalize(ya)
    dot = x*y
    if dot < 0 then
        x = -x
	dot = -dot
    end
    if dot > 0.9995 then
        return quaternion_normalize((1-t)*x + t*y)
    end
    theta0 = acos(dot)
    theta = t*theta0
    st = sin(theta)
    st0 = sin(theta0)
    s0 = cos(theta) - dot*st/st0
    s1 = st/st0
    return s0*x + s1*y
end
        
