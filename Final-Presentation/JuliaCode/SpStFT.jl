
module SpStFT
    using Pkg
    Pkg.add("FFTW")
    using FFTW


    struct fastMultiElectronSystem
        ψ::Matrix{ComplexF64}
        ϕ::Vector{Real}
        dx::Real
    end

    struct MultiElectronSystem
        ψ::Matrix{ComplexF64}
        ϕ::Matrix{Real}
        dx::Real
    end

    struct SingleElectronSystem
        ψ::Vector{ComplexF64}
        ϕ::Vector{Real}
        dx::Real
    end

    AllSys = Union{MultiElectronSystem, fastMultiElectronSystem, SingleElectronSystem}


    function step(sys::AllSys, dt::Real)
        kick(sys, dt/2)
        drift(sys, dt)
        kick(sys, dt/2)
    end
    


    function kick(sys::SingleElectronSystem, dt::Real)
        sys.ψ .*= exp.(-im .* sys.ϕ .* dt)        
    end




    function drift(sys::SingleElectronSystem, dt::Real)
        ψ̂ = fft(sys.ψ)
        ψ̂ .*= exp.(-im .* (0:length(sys.ψ)-1) .* dt)
        sys.ψ .= ifft(ψ̂)
        
    end


    anySys = Union{SingleElectronSystem, MultiElectronSystem, fastMultiElectronSystem}

    function Plot(sys::anySys)
        x = range(-length(sys.ψ)*sys.dx/2, length(sys.ψ)*sys.dx/2, length=length(sys.ψ))
        plot(x, abs.(sys.ψ).^2, title="Wave Function Probability Density", xlabel="x", ylabel="|ψ|^2")
        plot!(x, real.(sys.ψ), label="Real Part")
        plot!(x, imag.(sys.ψ), label="Imaginary Part")
        plot!(x, sys.ϕ, label="Potential", linestyle=:dash)
        
    end


    function steps(sys::anySys, dt::Real, nsteps::Integer)
        ψs = Vector{typeof(sys.ψ)}(undef, nsteps)
        ϕs = Vector{typeof(sys.ϕ)}(undef, nsteps)

        for i in 1:nsteps
            step(sys, dt)
            ψs[i] = copy(sys.ψ)
            ϕs[i] = copy(sys.ϕ)
        end
        return ψs, ϕs
    end
    
    
    
end