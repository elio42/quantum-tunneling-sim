using Interpolations
x = 0.5:1.0:9.5
y = rand(10)
itp = LinearInterpolation(x, y, extrapolation_bc=Line())
println(itp(0.0))
