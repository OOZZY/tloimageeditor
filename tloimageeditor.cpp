#include <QApplication>
#include "tlo/imageeditorview.hpp"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  tlo::ImageEditorModel imageEditorModel;
  tlo::ImageEditorView imageEditorView(imageEditorModel);
  imageEditorView.show();
  return app.exec();
}
