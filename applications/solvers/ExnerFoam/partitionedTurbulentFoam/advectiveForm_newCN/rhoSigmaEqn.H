// update rhoSigma according to the continuity equation (without transfers)
for(label ip = 0; ip < nParts; ip++)
{
    // Calculate the rate of change using old sigmaRho for bounded advection
    flux[ip] = fvc::interpolate(rho[ip])*volFlux[ip];
    
    sigmaRho.ddt()[ip] = -fvc::div
    (
        (flux[ip] + ocCoeff*flux[ip].oldTime())/(1+ocCoeff),
        sigma[ip].oldTime(),
        "sigmaRho"
    );

    // Update sigmaRho based on the rate of change
    sigmaRho[ip] = sigmaRho[ip].oldTime() + dt*sigmaRho.ddt()[ip];
    sigmaRho[ip].correctBoundaryConditions();
    
    // Update the sums and face values
    sigmaRhof[ip] = fvc::interpolate(sigmaRho[ip], "sigmaRho");
    
    Info << "sigmaRho["  << ip << "] goes from " << min(sigmaRho[ip]).value()
         << " to " << max(sigmaRho[ip]).value() << endl;
}

flux.updateSum();
sigmaRho.updateSum();
sigmaRhof.updateSum();

