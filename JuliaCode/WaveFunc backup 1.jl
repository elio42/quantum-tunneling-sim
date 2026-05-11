module WaveFunc

    function wave_func(n::Integer, σ::Real, x₀::Real, k₀::Real)
        x = range(-10.0, 10.0, length=n)
        return wave_func(x, σ, x₀, k₀)
    end

    function wave_func(x, σ::Real, x₀::Real, k₀::Real)
        ψ = @. (1.0 / (σ * sqrt(pi))^0.5) * exp(-(x - x₀)^2 / (2 * σ^2)) * exp(im * k₀ * x)
        return ψ
    end
end