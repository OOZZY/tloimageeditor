#ifndef TLO_IMAGEEDITORVIEW_HPP
#define TLO_IMAGEEDITORVIEW_HPP

#include <QGraphicsScene>
#include <QMainWindow>
#include "imageeditormodel.hpp"

namespace tlo {
namespace Ui {
class ImageEditorView;
}

class ImageEditorView : public QMainWindow {
  Q_OBJECT

 private:
  Ui::ImageEditorView *ui;
  ImageEditorModel *imageEditorModel;
  QGraphicsScene graphicsScene;

 private slots:
  void updateGraphicsScene();
  void on_actionOpen_triggered();
  void on_actionSave_As_triggered();
  void on_actionQuit_triggered();
  void on_actionRevert_to_Original_triggered();
  void on_actionGrayscale_Lightness_triggered();
  void on_actionGrayscale_Average_triggered();
  void on_actionGrayscale_Luminosity_triggered();
  void on_actionGamma_Correct_triggered();
  void on_actionReduce_Color_Depth_Middle_triggered();
  void on_actionReduce_Color_Depth_Lowest_triggered();
  void on_actionReduce_Color_Depth_Highest_triggered();
  void on_actionReduce_Color_Depth_Dynamic_triggered();
  void on_actionCompute_Image_Information_triggered();

 public:
  explicit ImageEditorView(ImageEditorModel &model, QWidget *parent = nullptr);
  ~ImageEditorView();
};
}  // namespace tlo

#endif  // TLO_IMAGEEDITORVIEW_HPP
