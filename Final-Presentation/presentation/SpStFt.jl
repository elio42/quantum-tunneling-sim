module SpStFt
    using  FFTW
    using  LinearAlgebra
    
    struct System
        ψ::Vector{ComplexF64}
        ϕ::Vector{Float64}
        dx::Float64
        kinetic_phase::Vector{ComplexF64}
    end

    # CHANGE 1: Added dt to the arguments so it isn't hardcoded to 0.005
    function System(Ψ::Vector{ComplexF64}, ϕ::Vector{Float64}, dx::Float64, dt::Float64)
        N = length(Ψ)
        k = 2π .* fftfreq(N, 1/dx)
        # Calculate phase using the passed dt
        kinetic_phase = exp.(-im .* (k.^2 ./ 2) .* dt)
        return System(Ψ, ϕ, dx, kinetic_phase)
    end

    function step(sys::System, dt::Real)
        kick(sys, dt/2)
        drift(sys, dt)
        kick(sys, dt/2)
    end
    
    function kick(sys::System, dt::Real)
        sys.ψ .*= exp.(-im .* sys.ϕ .* dt)        
    end

    function drift(sys::System, dt::Real)
        ψ̂ = fft(sys.ψ)
        
        # CHANGE 2: Apply the precomputed kinetic phase in momentum space (to ψ̂)
        ψ̂ .*= sys.kinetic_phase
        
        # Then transform back to position space
        sys.ψ .= ifft(ψ̂)
    end

    function evolve(sys::System, steps::Int64, dt, dtout)
        out = []
        for i in 1:steps
            step(sys, dt)
            if i % dtout == 0
                push!(out, copy(sys.ψ))
            end
        end
        return out
    end
    
end