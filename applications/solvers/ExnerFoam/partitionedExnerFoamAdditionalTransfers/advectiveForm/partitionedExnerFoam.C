/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 1991-2009 OpenCFD Ltd.
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM; if not, write to the Free Software Foundation,
    Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

Application
    partitionedExnerFoamAdv

Description
    Transient Solver for dry,buoyant, inviscid, incompressible, non-hydrostatic
    partitioned flow, advective form momentum equation

\*---------------------------------------------------------------------------*/

#include "HodgeOps.H"
#include "fvCFD.H"
#include "ExnerTheta.H"
#include "PartitionedFields.H"
#include "moreListOps.H"
#include "unitVectors.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    #include "setRootCase.H"
    #include "createTime.H"
    #include "createMesh.H"
    #include "readEnvironmentalProperties.H"
    #include "readThermoProperties.H"
    #include "readTransferCoeffs.H"
    HodgeOps H(mesh);
    surfaceScalarField gd("gd", g & H.delta());
    #define dt runTime.deltaT()
    #include "createFields.H"
    #include "initContinuityErrs.H"
    #include "initDiags.H"
    #include "calcDiags.H"
    
    const dictionary& itsDict = mesh.solutionDict().subDict("iterations");
    const int nOuterCorr = itsDict.lookupOrDefault<int>("nOuterCorrectors", 2);
    const int nCorr = itsDict.lookupOrDefault<int>("nCorrectors", 1);
    const int nNonOrthCorr =
        itsDict.lookupOrDefault<int>("nNonOrthogonalCorrectors", 0);
    const scalar offCentre = readScalar(mesh.schemesDict().lookup("offCentre"));

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

    Info<< "\nStarting time loop\n" << endl;

    while (runTime.loop())
    {
        Info<< "Time = " << runTime.timeName() << nl << endl;

        for (int ucorr=0; ucorr < nOuterCorr; ucorr++)
        {
            transfer[0][1] = 0;
            transfer[1][0] = 0;
            #include "partitionedCourantNo.H"
            #include "sigma.H"
            
            #include "rhoSigmaEqn.H"
            #include "thetaEqn.H"
            #include "sigma.H"
            #include "calculateDrag.H"
            #include "exnerEqn.H"
            for(label ip = 0; ip < nParts; ip++)
            {
                Info << "Min sigmaRho now " << ip << ": " << min(sigmaRho[ip]).value() << endl;
            }
        }
        
        #include "calcDiagsPreTransfer.H"
        #include "massTransfers.H"
        //#include "sigma.H"
        
        Info << "sigma[0] goes from " << min(sigma[0]).value() << " to "
             << max(sigma[0]).value() << endl;

        #include "compressibleContinuityErrs.H"
        #include "correctContinuityErrs.H"
        #include "calcDiags.H"
        runTime.write();

        Info<< "ExecutionTime = " << runTime.elapsedCpuTime() << " s"
            << "  ClockTime = " << runTime.elapsedClockTime() << " s"
            << nl << endl;
    }

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
