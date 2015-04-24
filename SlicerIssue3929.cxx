// This code creates a rectilinear grid and volume renders it with a plane
// in the render view

// VTK includes
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkColorTransferFunction.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkImageData.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkNew.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPlaneSource.h>
#include <vtkPointData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRTAnalyticSource.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkTimerLog.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>

int main (int, char**)
{
  vtkNew<vtkRTAnalyticSource> source;
  source->SetWholeExtent(-49,50,-49,50,0,1);
  source->SetCenter(0.0, 0.0, 0.0);

  vtkNew<vtkGPUVolumeRayCastMapper> volumeMapper;
  volumeMapper->SetInputConnection(source->GetOutputPort());

  vtkNew<vtkVolumeProperty> volumeProperty;
  vtkNew<vtkColorTransferFunction> ctf;
  ctf->AddRGBPoint(37.3531, 0.2, 0.29, 1);
  ctf->AddRGBPoint(157.091, 0.87, 0.87, 0.87);
  ctf->AddRGBPoint(276.829, 0.7, 0.015, 0.15);

  vtkNew<vtkPiecewiseFunction> pwf;
  pwf->AddPoint(37.3531, 0.0);
  pwf->AddPoint(276.829, 1.0);

  volumeProperty->SetColor(ctf.GetPointer());
  volumeProperty->SetScalarOpacity(pwf.GetPointer());

  vtkNew<vtkVolume> volume;
  volume->SetMapper(volumeMapper.GetPointer());
  volume->SetProperty(volumeProperty.GetPointer());

  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->SetSize(900, 900);
  renderWindow->Render(); // make sure we have an OpenGL context.

  vtkNew<vtkPlaneSource> plane;
  plane->SetCenter(0,0,0.5);
  plane->SetOrigin(-70, -70, 0.5);
  plane->SetPoint1(70, -70, 0.5);
  plane->SetPoint2(-70, 70, 0.5);

  vtkNew<vtkPolyDataMapper> planeMapper;
  planeMapper->SetInputConnection(plane->GetOutputPort());

  vtkNew<vtkActor> planeActor;
  planeActor->SetMapper(planeMapper.GetPointer());

  vtkNew<vtkRenderer> renderer;
  renderer->SetBackground(0.3, 0.35, 0.45);
  renderWindow->AddRenderer(renderer.GetPointer());

  renderer->AddActor(planeActor.GetPointer());
  renderer->AddVolume(volume.GetPointer());
  renderer->ResetCamera();

  vtkNew<vtkRenderWindowInteractor> iren;
  iren->SetRenderWindow(renderWindow.GetPointer());
  vtkNew<vtkInteractorStyleTrackballCamera> style;
  iren->SetInteractorStyle(style.GetPointer());

  renderWindow->Render();

  iren->Initialize();
  iren->Start();

  return EXIT_SUCCESS;
}