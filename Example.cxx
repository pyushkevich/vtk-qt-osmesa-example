#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkNew.h>
#include <vtkCylinderSource.h>
#include <vtkProperty.h>
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>
#include <vtkTIFFWriter.h>
#include <vtkImageData.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <QVTKInteractor.h>

#include <QWidget>
#include <QPainter>
#include <QApplication>
#include <QImage>
#include <QResizeEvent>
#include <QVTKInteractorAdapter.h>

class MyWidget : public QWidget
{
public:

  MyWidget(QWidget *parent) : QWidget(parent)
    {
    qad = new QVTKInteractorAdapter(this);
    qad->SetDevicePixelRatio(this->devicePixelRatio());
    }

  void setRenderWindow(vtkRenderWindow *rwin)
    {
    this->rwin = rwin;
    this->wfil = vtkSmartPointer<vtkWindowToImageFilter>::New();
    this->wfil->SetInput(rwin);
    }

  void resizeEvent(QResizeEvent *evt) override
    {
    this->size = evt->size();
    }

  void paintEvent(QPaintEvent *evt) override 
    {
    std::cout << "Paint start: " << clock() * 1.0 / CLOCKS_PER_SEC << std::endl;
    this->rwin->SetSize(this->size.width(), this->size.height());
    this->rwin->Render();
    std::cout << "Render done " << clock() * 1.0 / CLOCKS_PER_SEC << std::endl;
    this->wfil->Modified();
    this->wfil->Update();
    vtkImageData *id = this->wfil->GetOutput();
    std::cout << "Win2Img done " << clock() * 1.0 / CLOCKS_PER_SEC << std::endl;

    QImage img((const unsigned char *)id->GetScalarPointer(), 
               id->GetDimensions()[0], id->GetDimensions()[1], 
               3 * id->GetDimensions()[0], QImage::Format_RGB888);
    std::cout << "QImage made " << clock() * 1.0 / CLOCKS_PER_SEC << std::endl;


    QPainter p(this);
    p.drawImage(0, 0, img.mirrored());
    std::cout << "Paint done: " << clock() * 1.0 / CLOCKS_PER_SEC << std::endl;
    }

  void mousePressEvent(QMouseEvent *evt) override
    {
    std::cout << "Press: " << clock() * 1.0 / CLOCKS_PER_SEC << std::endl;
    if(rwin) 
      qad->ProcessEvent(evt, rwin->GetInteractor());
    this->update();
    std::cout << "Press done: " << clock() * 1.0 / CLOCKS_PER_SEC << std::endl;
    }

  void mouseMoveEvent(QMouseEvent *evt) override
    {
    std::cout << "Move: " << clock() * 1.0 / CLOCKS_PER_SEC << std::endl;
    if(rwin) 
      qad->ProcessEvent(evt, rwin->GetInteractor());
    this->update();
    }

  void mouseReleaseEvent(QMouseEvent *evt) override
    {
    std::cout << "Release: " << clock() * 1.0 / CLOCKS_PER_SEC << std::endl;
    if(rwin) 
      qad->ProcessEvent(evt, rwin->GetInteractor());
    this->update();
    }

  void enterEvent(QEvent *evt) override
    {
    if(rwin) 
      qad->ProcessEvent(evt, rwin->GetInteractor());
    }

  void leaveEvent(QEvent *evt) override
    {
    if(rwin) 
      qad->ProcessEvent(evt, rwin->GetInteractor());
    }

  vtkSmartPointer<vtkRenderWindow> rwin;
  vtkSmartPointer<vtkWindowToImageFilter> wfil;
  QSize size;
  QVTKInteractorAdapter *qad;
};

int main(int argc, char *argv[])
{
  vtkNew<vtkCylinderSource> cyl;
  cyl->SetResolution(20);

  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputConnection(cyl->GetOutputPort());

  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);
  actor->GetProperty()->SetColor(1.0, 0.0, 0.0);

  vtkNew<vtkRenderer> rend;
  rend->AddActor(actor);
  rend->ResetCamera();

  vtkNew<vtkRenderWindow> rwin;
  rwin->SetSize(300,300);
  rwin->AddRenderer(rend);
  rwin->Render();

  vtkNew<QVTKInteractor> inter;
  inter->SetRenderWindow(rwin);
  inter->Initialize();
  vtkNew<vtkInteractorStyleTrackballCamera> style;
  inter->SetInteractorStyle(style);
  inter->SetSize(300,300);

  vtkNew<vtkWindowToImageFilter> wfil;
  wfil->SetInput(rwin);
  wfil->Update();

  vtkNew<vtkTIFFWriter> writer;
  writer->SetInputConnection(wfil->GetOutputPort());
  writer->SetFileName("test.tiff");
  writer->Update();

  QApplication app(argc, argv);
  MyWidget widget(nullptr);
  widget.resize(300, 300);
  widget.setWindowTitle("Test Window");
  widget.setRenderWindow(rwin);
  widget.show();

  inter->Start();
  return app.exec();
}
