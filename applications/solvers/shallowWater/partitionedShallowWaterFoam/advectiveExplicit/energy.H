{
    // Potential energy
    volScalarField energyField("energyField", 0.5*g*sqr(hSum));
    
    PE = fvc::domainIntegrate(energyField)/Vtot;
    
    // Kinetic energy
    energyField = 0.5*h[0]*magSqr(u[0]);
    for(label ip = 1; ip < nParts; ip++)
    {
        energyField += 0.5*h[ip]*magSqr(u[ip]);
    }

    KE = fvc::domainIntegrate(energyField)/Vtot;

    energy = KE + PE;

    normalEnergyChange = ((energy - energy0)/energy0).value();
    
    os << runTime.timeName() << " " << KE.value() << " " << PE.value() << " "
       << normalEnergyChange << endl;
}