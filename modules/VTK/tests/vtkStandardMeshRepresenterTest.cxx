/*
 * itkVectorImageRepresenterTest.cxx
 *
 *  Created on: May 3, 2012
 *      Author: luethi
 */

#include <vtkPolyDataReader.h>
#include <vtkSmartPointer.h>

#include "statismo/core/genericRepresenterTest.hxx"
#include "statismo/VTK/vtkStandardMeshRepresenter.h"

using statismo::vtkStandardMeshRepresenter;
using statismo::vtkPoint;

typedef GenericRepresenterTest<vtkStandardMeshRepresenter> RepresenterTestType;

vtkPolyData* loadPolyData(const std::string& filename) {
    vtkSmartPointer<vtkPolyDataReader> reader = vtkSmartPointer<vtkPolyDataReader>::New();
    reader->SetFileName(filename.c_str());
    reader->Update();
    vtkPolyData* pd = vtkPolyData::New();
    pd->ShallowCopy(reader->GetOutput());
    return pd;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " datadir" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::string datadir = std::string(argv[1]);

    const std::string referenceFilename = datadir + "/hand_polydata/hand-0.vtk";
    const std::string testDatasetFilename = datadir + "/hand_polydata/hand-1.vtk";

    vtkPolyData* reference = loadPolyData(referenceFilename);
    vtkStandardMeshRepresenter* representer = vtkStandardMeshRepresenter::Create(reference);

    // choose a test dataset, a point (on the reference) and the associated point on the test example

    vtkPolyData* testDataset = loadPolyData(testDatasetFilename);
    unsigned testPtId = 0;
    vtkPoint testPt(reference->GetPoints()->GetPoint(testPtId));
    vtkPoint testValue(testDataset->GetPoints()->GetPoint(testPtId));

    RepresenterTestType representerTest(representer, testDataset, std::make_pair(testPt, testValue));

    bool testsOk = representerTest.runAllTests();
    delete representer;
    reference->Delete();
    testDataset->Delete();

    if (testsOk == true) {
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }

}


