#include <cstdlib>

#include "fvCFD.H"

int main(int argc, char *argv[])
{
#   include "setRootCase.H"
#   include "createTime.H"
#   include "createMesh.H"
    
    // New point locations layered over the mountain
    IOField<point> newPoints
    (
        IOobject("points", mesh.time().constant(), "polyMesh", mesh),
        mesh.points()
    );

	forAll(newPoints, pointIdx)
	{
        newPoints[pointIdx].x() += (rand() % 200) - 100;
        newPoints[pointIdx].z() += (rand() % 200) - 100;
    }

    newPoints.write();
}
