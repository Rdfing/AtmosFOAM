{
    for(label ip = 0; ip < nParts; ip++)
    {
        Psi[ip] = pow(rho[ip],(2*kappa-1)/(kappa-1))
                 *pow(R/pRef*theta[ip], kappa/(kappa-1));
    }
    Psi.updateSum();
    
    gradPcoeffSum = Cp*thetaf.sum();

    for (int corr=0; corr<nCorr; corr++)
    {
        // Explicit part of velocity for each partition (without mass tranfers)
        for(label ip = 0; ip < nParts; ip++)
        {
            // Rate of change of flux without gravity
            volFlux.ddt()[ip] = mesh.Sf() &
            (
              -fvc::interpolate
               (
                   fvc::div(volFlux[ip], u[ip], "u")
                 - u[ip] * fvc::div(volFlux[ip])
                 //+ ((turbulence[ip].divDevRhoReff(u[ip])) & u[ip])/sigmaRho[ip],
                 - nu * fvc::laplacian(u[ip]),
                   "convection"
               )
             - 2*(Omega ^ Uf[ip])
            );
            
            // Flux with gravity
            volFlux[ip] = volFlux[ip].oldTime() + dt/(1+ocCoeff)*
            (
                (volFlux.ddt()[ip] + gSf)
              + ocCoeff*volFlux.ddt()[ip].oldTime()
            );
            sigmaFlux[ip] = sigmaRhof[ip]*volFlux[ip];
        }
        sigmaFlux.updateSum();
        // Needed for Exner boundary conditions
        surfaceScalarField rhog("rhog", sigmaRhof.sum()*gSf);

        // Helmholtz equation
        for (int nonOrth=0; nonOrth<=nNonOrthCorr; nonOrth++)
        {
            fvScalarMatrix ExnerEqn
            (
                fvm::ddt(Psi.sum(), Exner)
              + fvc::div(sigmaFlux.sum())
              - fvm::laplacian(dt/(1+ocCoeff)*gradPcoeffSum, Exner)
            );

            if(ucorr==nOuterCorr-1 && nonOrth==nNonOrthCorr && corr==nCorr-1)
            {
                ExnerEqn.solve(Exner.name() + "Final");
            }
            else
            {
                ExnerEqn.solve(Exner.name());
            }

            // Back-substitutions
            if (nonOrth == nNonOrthCorr)
            {
                for(label ip = 0; ip < nParts; ip++)
                {
                    volFlux[ip] -= dt/(1+ocCoeff)*Cp*thetaf[ip]
                                   *fvc::snGrad(Exner)*mesh.magSf();

                    volFlux.ddt()[ip] += gSf - Cp*thetaf[ip]
                                        *fvc::snGrad(Exner)*mesh.magSf();
                }

                // Include the mass transfer and drag implicitly (for nParts=2)
                transferf[0]
                    = fvc::interpolate(transfer[0], "massTransfer")
                    + dt*dragCommon
                     *fvc::interpolate(sigma[0]/rho[1], "massTransfer");
                transferf[1]
                    = fvc::interpolate(transfer[1], "massTransfer")
                    + dt*dragCommon
                      *fvc::interpolate(sigma[1]/rho[0], "massTransfer");
                transferf.updateSum();
                
                volFlux[0] =
                (
                    (1+transferf[0])*volFlux[0] + transferf[1]*volFlux[1]
                )/(1+transferf.sum());
                volFlux[1] = (volFlux[1] + transferf[0]*volFlux[0])
                            /(1+transferf[0]);

                // Update mass flux and velocities based on the flux
                for(label ip = 0; ip < nParts; ip++)
                {
                    sigmaFlux[ip] = sigmaRhof[ip]*volFlux[ip];
                    u[ip] = fvc::reconstruct(volFlux[ip]);
                    Uf[ip] = fvc::interpolate(u[ip], "Uf");
                }
                sigmaFlux.updateSum();
                volFlux.updateSum();
            }
        }
        // Update the pressure and temperature based on the new Exner
        for(label ip = 0; ip < nParts; ip++)
        {
            thermos[ip].p() = pRef*pow(Exner, 1/kappa);
            thermos[ip].T() = theta[ip]*Exner;
            thermos[ip].he() == thermos[ip].he(thermos[ip].p(),thermos[ip].T());
            thermos[ip].correct();
        }
    }
}
u.updateSum();
Uf.updateSum();

