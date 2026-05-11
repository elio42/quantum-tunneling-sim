
module QuantumDynamics

    using .PSolver: System, solve
    using .SpStFT: fastMultiElectronSystem, MultiElectronSystem, SingleElectronSystem, step, comp_ϕ
    using .WaveFunc



    function Plot(ψs, ϕs,sys::anySys)
        nsteps = length(ψs)
        x = range(-length(ψs[1])*sys.dx/2, length(ψs[1])*sys.dx/2, length=length(ψs[1]))

        anim = @animate for i in 1:nsteps
            plot(x, abs.(ψs[i]).^2, title="Wave Function Probability Density", xlabel="x", ylabel="|ψ|^2")
            plot!(x, real.(ψs[i]), label="Real Part")
            plot!(x, imag.(ψs[i]), label="Imaginary Part")
            plot!(x, ϕs[i], label="Potential", linestyle=:dash)
        end
        mp4(anim, "wave_evolution.mp4", fps=60)
        
    end
end