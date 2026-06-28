using Roots # For finding the transcendental roots

"""
    solve_finite_well(V0, a; m=1.0, hbar=1.0)

Calculates the allowed wave vectors 'k' for a 1D finite potential well.
- V0: Depth of the well (positive value)
- a:  Half-width of the well
"""
function solve_finite_well(V0, a; m=1.0, hbar=1.0)
    # Scaling constant for the transcendental equations
    z0 = sqrt(2 * m * V0 * a^2 / hbar^2)
    
    # Transcendental functions to find zeros: f(z) = 0
    # Even states: z * tan(z) = sqrt(z0^2 - z^2)
    f_even(z) = z * tan(z) - sqrt(z0^2 - z^2)
    # Odd states: -z * cot(z) = sqrt(z0^2 - z^2)
    f_odd(z) = -z * (1/tan(z)) - sqrt(z0^2 - z^2)
    
    ks = Float64[]
    
    # We search for roots in intervals of pi/2
    num_intervals = Int(floor(z0 / (pi/2)))
    for n in 0:num_intervals
        lower = n * pi/2 + 1e-5
        upper = min((n+1) * pi/2 - 1e-5, z0 - 1e-5)
        
        if lower < upper
            if n % 2 == 0 # Even
                try push!(ks, find_zero(f_even, (lower, upper)) / a) catch end
            else # Odd
                try push!(ks, find_zero(f_odd, (lower, upper)) / a) catch end
            end
        end
    end
    return ks
end

"""
    psi(x, k, V0, a; m=1.0, hbar=1.0, is_even=true)

Calculates the value of the wave function at position x for a given k.
"""
function psi(x, k, V0, a; m=1.0, hbar=1.0, is_even=true)
    alpha = sqrt(2 * m * V0 / hbar^2 - k^2)
    
    if abs(x) <= a
        return is_even ? cos(k * x) : sin(k * x)
    else
        # Match boundary condition: C*exp(-alpha*|x|)
        amplitude = is_even ? cos(k * a) : (x > 0 ? sin(k * a) : sin(k * -a))
        return amplitude * exp(-alpha * (abs(x) - a))
    end
end
V0 = 20.0
a = 1.0
ks = solve_finite_well(V0, a)

println("Found $(length(ks)) bound states.")
for (i, k) in enumerate(ks)
    println("State $i: k = $(round(k, digits=3))")
end


using Plots
x_vals = range(-4a, 4a, length=4000)
p = plot(title="Finite Well Standing Waves")

i=1 
k= ks[i]
    y_vals = [psi(x, k, V0, a, is_even=(i%2!=0)) for x in x_vals]
    plot(p, x_vals, y_vals, label="n=$i")

display(p)

