// This code reads a legacy VTK file containing a rectilinear grid and volume
// renders it. The test benchmarks the frame rate of volume rendering

// VTK includes
#include <vtkCamera.h>
#include <vtkColorTransferFunction.h>
#include <vtkDataArray.h>
#include <vtkFixedPointVolumeRayCastMapper.h>
#include <vtkGenericDataObjectReader.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkImageData.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkNew.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPointData.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkTimerLog.h>
#include <vtkVolume.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>

int main (int argc, char* argv[])
{
  if (argc < 2)
    {
    std::cerr << "Usage: " << argv[0] << " <input VTK file> [--cpu] [--shade] [-I]" << std::endl;
    return EXIT_FAILURE;
    }

  bool shade = false;
  bool interactive = false;
  bool cpu = false;
  for (int i = 2; i < argc; ++i)
    {
    if (strcmp(argv[i], "--shade") == 0)
      {
      shade = true;
      }
    else if (strcmp(argv[i], "-I") == 0)
      {
      interactive = true;
      }
    else if (strcmp(argv[i], "--cpu") == 0)
      {
      cpu = true;
      }
    }

  vtkNew<vtkGenericDataObjectReader> reader;
  reader->SetFileName(argv[1]);
  reader->Update();

  vtkImageData* im = vtkImageData::SafeDownCast(reader->GetOutput());
  if (!im)
    {
    std::cerr << "Input file " << argv[1] <<
      " does not contain a dataset of type vtkImageData" << std::endl;
    return EXIT_FAILURE;
    }

  std::cout << "Input dataset : " << argv[1] << std::endl;
  std::cout << "Dimensions : (" << im->GetDimensions()[0] << ", " <<
    im->GetDimensions()[1] << ", " << im->GetDimensions()[2] << ")" << std::endl;
  std::cout << "Data Type : " <<
    im->GetPointData()->GetScalars()->GetDataTypeAsString() << std::endl;
  std::cout << std::endl <<  "Mapper: " << (cpu ? "CPU" : "GPU") << std::endl;
  std::cout << "Shading : " << (shade ? "ON" : "OFF") << std::endl;

  vtkNew<vtkGPUVolumeRayCastMapper> gpumapper;
  vtkNew<vtkFixedPointVolumeRayCastMapper> cpumapper;
  vtkVolumeMapper* mapper;
  if (cpu)
    {
    mapper = cpumapper.GetPointer();
    }
  else
    {
    mapper = gpumapper.GetPointer();
    }

  mapper->SetInputConnection(reader->GetOutputPort());

  vtkNew<vtkVolume> volume;
  volume->SetMapper(mapper);

  vtkNew<vtkColorTransferFunction> ctf;
  ctf->AddRGBPoint(-3024, 0, 0, 0);
  ctf->AddRGBPoint(67.0106, 0.54902, 0.25098, 0.14902);
  ctf->AddRGBPoint(251.105, 0.882353, 0.603922, 0.290196);
  ctf->AddRGBPoint(439.291, 1, 0.937033, 0.954531);
  ctf->AddRGBPoint(3071, 0.827451, 0.658824, 1);

  vtkNew<vtkPiecewiseFunction> pf;
  pf->AddPoint(-3024, 0);
  pf->AddPoint(67.0106, 0);
  pf->AddPoint(251.105, 0.446429);
  pf->AddPoint(439.291, 0.625);
  pf->AddPoint(3071, 0.616071);

  vtkNew<vtkPiecewiseFunction> gf;
  gf->AddPoint(0, 1);
  gf->AddPoint(255, 1);

  vtkNew<vtkVolumeProperty> property;
  property->SetColor(ctf.GetPointer());
  property->SetScalarOpacity(pf.GetPointer());
  property->SetGradientOpacity(gf.GetPointer());
  property->SetShade(shade);
  volume->SetProperty(property.GetPointer());

  vtkNew<vtkRenderWindow> renWin;
  renWin->SetSize(400,400);
  vtkNew<vtkRenderWindowInteractor> iren;
  iren->SetRenderWindow(renWin.GetPointer());
  vtkNew<vtkInteractorStyleTrackballCamera> style;
  iren->SetInteractorStyle(style.GetPointer());

  vtkNew<vtkRenderer> ren;
  renWin->AddRenderer(ren.GetPointer());

  ren->AddVolume(volume.GetPointer());
  ren->ResetCamera();

  double startTime = vtkTimerLog::GetUniversalTime();
  renWin->Render();
  double firstFrameTime = vtkTimerLog::GetUniversalTime() - startTime;
  int frameCount = 80;
  for (int i = 0; i < frameCount; i++)
    {
    renWin->Render();
    ren->GetActiveCamera()->Azimuth(1);
    ren->GetActiveCamera()->Elevation(1);
    ren->ResetCameraClippingRange();
    }

  double subsequentFrameTime = (vtkTimerLog::GetUniversalTime()
    - startTime - firstFrameTime)/frameCount;
  double frameRate = 1.0 / subsequentFrameTime;

  std::cout << std::endl;
  std::cout << "First Frame Time: " << firstFrameTime << " sec." << std::endl;
  std::cout << "Subsequent Frame Time: " << subsequentFrameTime <<
    " sec." << std::endl;
  std::cout << "Frame Rate: " << frameRate << " fps" << std::endl;
  std::cout << std::endl;

  if (interactive)
    {
    std::cout << std::endl << "Entering interactive mode..." << std::endl;
    iren->Initialize();
    iren->Start();
    }
  return EXIT_SUCCESS;
}
