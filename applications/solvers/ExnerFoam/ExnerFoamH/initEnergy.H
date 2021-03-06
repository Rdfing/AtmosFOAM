Info << "Initialising total energy file energy.dat\n" << endl;

OFstream es(args.rootPath() / args.caseName() / "energy.dat");
es << "#time   KE       internal potential normalisedEnergy  max_mag_w max_theta heatError entropyError thetaVariance thetaRhoVariance"
   << endl;

dimensionedScalar Vtot = sum(mesh.V());

volScalarField KE
(
    IOobject("KE", runTime.timeName(), mesh),
    0.25*fvc::surfaceIntegrate(U*V/rhof)
);
volScalarField internalEnergy
(
    IOobject("internalEnergy", runTime.timeName(), mesh),
    rho*theta*Exner*Cv
);
volScalarField potentialEnergy
(
    IOobject("potentialEnergy", runTime.timeName(), mesh),
    -rho*(g & mesh.C())
);

const dimensionedScalar KEInitTot = fvc::domainIntegrate(KE)/Vtot;
const dimensionedScalar internalInitTot
     = fvc::domainIntegrate(internalEnergy)/Vtot;
const dimensionedScalar potentialInitTot
     = fvc::domainIntegrate(potentialEnergy)/Vtot;

Info << "Vtot = " << Vtot << "\nKEInitTot = " << KEInitTot
     << "\ninternalInitTot = " << internalInitTot << "\npotentialInitTot = "
     << potentialInitTot << endl;

const dimensionedScalar energyInitTot = KEInitTot + internalInitTot
                                      + potentialInitTot;
scalar normalisedHeatDiff = 0;
scalar entropyError = 0;
