#include "tlo/imageeditorview.hpp"
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGraphicsPixmapItem>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QTextStream>
#include <cfloat>
#include "tlo/ui_imageeditorview.h"

namespace tlo {
void ImageEditorView::updateGraphicsScene() {
  auto *item =
      new QGraphicsPixmapItem(QPixmap::fromImage(imageEditorModel->image()));
  graphicsScene.clear();

  /*
   * without this, whenever a new image is loaded and the new image is smaller
   * than the previous image, its top-left will be where the top-left of the
   * previous image was. this means the new image will not be centered. this
   * happens because the QGraphicsScene's sceneRect doesn't shrink and is still
   * the size of previous image and the new image's top-left will be at the
   * top-left of the sceneRect. with this, the QGraphicsScene's sceneRect is
   * resized to the size of the new image and so the new image will be
   * centered. by default, a QGraphicsView's sceneRect is synced with its
   * QGraphicsScene's sceneRect.
   */
  qreal x = 0;
  qreal y = 0;
  graphicsScene.setSceneRect(QRectF(x, y, imageEditorModel->image().width(),
                                    imageEditorModel->image().height()));

  /*
   * while doing this, graphicsScene takes ownership of item so no need to
   * manually delete item
   */
  graphicsScene.addItem(item);
}

void ImageEditorView::on_actionOpen_triggered() {
  QString filePath = QFileDialog::getOpenFileName(this);
  if (filePath.isEmpty()) {
    return;
  }

  bool loaded = imageEditorModel->load(filePath);
  if (!loaded) {
    QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
    return;
  }
}

void ImageEditorView::on_actionSave_As_triggered() {
  QString filePath = QFileDialog::getSaveFileName(this);
  if (filePath.isEmpty()) {
    return;
  }

  bool saved = imageEditorModel->save(filePath);
  if (!saved) {
    QMessageBox::critical(this, tr("Error"), tr("Could not save file"));
    return;
  }
}

void ImageEditorView::on_actionQuit_triggered() { QCoreApplication::quit(); }

void ImageEditorView::on_actionRevert_to_Original_triggered() {
  imageEditorModel->revertToOriginal();
}

void ImageEditorView::on_actionGrayscale_Lightness_triggered() {
  imageEditorModel->convertToGrayscaleLightness();
}

void ImageEditorView::on_actionGrayscale_Average_triggered() {
  imageEditorModel->convertToGrayscaleAverage();
}

void ImageEditorView::on_actionGrayscale_Luminosity_triggered() {
  imageEditorModel->convertToGrayscaleLuminosity();
}

void ImageEditorView::on_actionGamma_Correct_triggered() {
  const char *title = "Gamma Correct";
  const char *label = "Gamma";
  double defaultGamma = 2.2;
  double minGamma = DBL_EPSILON;  // gamma should not be 0
  double maxGamma = 4;
  int maxDecimalPlaces = 2;
  bool ok;

  double gamma =
      QInputDialog::getDouble(this, tr(title), tr(label), defaultGamma,
                              minGamma, maxGamma, maxDecimalPlaces, &ok);
  if (!ok) {
    return;
  }

  imageEditorModel->gammaCorrect(gamma);
}

namespace {
QSpinBox *makeSpinBox(QWidget *parent, int defaultValue, int minValue,
                      int maxValue) {
  QSpinBox *spinBox = new QSpinBox(parent);
  spinBox->setValue(defaultValue);
  spinBox->setMinimum(minValue);
  spinBox->setMaximum(maxValue);
  return spinBox;
}

const int RED_INDEX = 0;
const int GREEN_INDEX = 1;
const int BLUE_INDEX = 2;
const int ALPHA_INDEX = 3;

std::tuple<int, int, int, int> getColorDepths(QWidget *parent,
                                              const QString &title, bool &ok) {
  ok = false;

  QDialog dialog(parent);
  dialog.setWindowTitle(title);

  QFormLayout formLayout(&dialog);
  // formLayout takes ownership of the new QLabel
  formLayout.addRow(new QLabel(QObject::tr("Color Depth")));
  QList<QSpinBox *> spinBoxes;

  int defaultValue = 8;
  int minValue = 1;
  int maxValue = 8;

  QSpinBox *spinBox = makeSpinBox(&dialog, defaultValue, minValue, maxValue);
  formLayout.addRow(QObject::tr("Red"), spinBox);
  spinBoxes << spinBox;

  spinBox = makeSpinBox(&dialog, defaultValue, minValue, maxValue);
  formLayout.addRow(QObject::tr("Green"), spinBox);
  spinBoxes << spinBox;

  spinBox = makeSpinBox(&dialog, defaultValue, minValue, maxValue);
  formLayout.addRow(QObject::tr("Blue"), spinBox);
  spinBoxes << spinBox;

  spinBox = makeSpinBox(&dialog, defaultValue, minValue, maxValue);
  formLayout.addRow(QObject::tr("Alpha"), spinBox);
  spinBoxes << spinBox;

  QDialogButtonBox dialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
  formLayout.addRow(&dialogButtonBox);
  QObject::connect(&dialogButtonBox, SIGNAL(accepted()), &dialog,
                   SLOT(accept()));
  QObject::connect(&dialogButtonBox, SIGNAL(rejected()), &dialog,
                   SLOT(reject()));

  int result = dialog.exec();
  if (result != QDialog::Accepted) {
    return std::tuple<int, int, int, int>();
  }

  int redDepth = spinBoxes[RED_INDEX]->value();
  int greenDepth = spinBoxes[GREEN_INDEX]->value();
  int blueDepth = spinBoxes[BLUE_INDEX]->value();
  int alphaDepth = spinBoxes[ALPHA_INDEX]->value();

  ok = true;
  return std::tuple<int, int, int, int>(redDepth, greenDepth, blueDepth,
                                        alphaDepth);
}
}  // namespace

void tlo::ImageEditorView::on_actionReduce_Color_Depth_Middle_triggered() {
  bool ok;
  auto colorDepths =
      getColorDepths(this, tr("Reduce Color Depth (Middle)"), ok);
  if (!ok) {
    return;
  }

  int redDepth = std::get<RED_INDEX>(colorDepths);
  int greenDepth = std::get<GREEN_INDEX>(colorDepths);
  int blueDepth = std::get<BLUE_INDEX>(colorDepths);
  int alphaDepth = std::get<ALPHA_INDEX>(colorDepths);
  imageEditorModel->reduceColorDepthMiddle(redDepth, greenDepth, blueDepth,
                                           alphaDepth);
}

void tlo::ImageEditorView::on_actionReduce_Color_Depth_Lowest_triggered() {
  bool ok;
  auto colorDepths =
      getColorDepths(this, tr("Reduce Color Depth (Lowest)"), ok);
  if (!ok) {
    return;
  }

  int redDepth = std::get<RED_INDEX>(colorDepths);
  int greenDepth = std::get<GREEN_INDEX>(colorDepths);
  int blueDepth = std::get<BLUE_INDEX>(colorDepths);
  int alphaDepth = std::get<ALPHA_INDEX>(colorDepths);
  imageEditorModel->reduceColorDepthLowest(redDepth, greenDepth, blueDepth,
                                           alphaDepth);
}

void tlo::ImageEditorView::on_actionReduce_Color_Depth_Highest_triggered() {
  bool ok;
  auto colorDepths =
      getColorDepths(this, tr("Reduce Color Depth (Highest)"), ok);
  if (!ok) {
    return;
  }

  int redDepth = std::get<RED_INDEX>(colorDepths);
  int greenDepth = std::get<GREEN_INDEX>(colorDepths);
  int blueDepth = std::get<BLUE_INDEX>(colorDepths);
  int alphaDepth = std::get<ALPHA_INDEX>(colorDepths);
  imageEditorModel->reduceColorDepthHighest(redDepth, greenDepth, blueDepth,
                                            alphaDepth);
}

void tlo::ImageEditorView::on_actionReduce_Color_Depth_Dynamic_triggered() {
  bool ok;
  auto colorDepths =
      getColorDepths(this, tr("Reduce Color Depth (Dynamic)"), ok);
  if (!ok) {
    return;
  }

  int redDepth = std::get<RED_INDEX>(colorDepths);
  int greenDepth = std::get<GREEN_INDEX>(colorDepths);
  int blueDepth = std::get<BLUE_INDEX>(colorDepths);
  int alphaDepth = std::get<ALPHA_INDEX>(colorDepths);
  imageEditorModel->reduceColorDepthDynamic(redDepth, greenDepth, blueDepth,
                                            alphaDepth);
}

void tlo::ImageEditorView::on_actionCompute_Image_Information_triggered() {
  QString text;
  QTextStream textStream(&text);

  textStream << "Red Channel:" << endl;
  textStream << "  Histogram: " << endl;
  const auto &redHistogram = imageEditorModel->redHistogram();
  for (auto i = redHistogram.cbegin(); i != redHistogram.cend(); ++i) {
    textStream << "    " << i.key() << ": " << i.value() << endl;
  }
  textStream << "  Entropy: " << imageEditorModel->redEntropy() << endl;
  textStream << endl;

  textStream << "Green Channel:" << endl;
  textStream << "  Histogram: " << endl;
  const auto &greenHistogram = imageEditorModel->greenHistogram();
  for (auto i = greenHistogram.cbegin(); i != greenHistogram.cend(); ++i) {
    textStream << "    " << i.key() << ": " << i.value() << endl;
  }
  textStream << "  Entropy: " << imageEditorModel->greenEntropy() << endl;
  textStream << endl;

  textStream << "Blue Channel:" << endl;
  textStream << "  Histogram: " << endl;
  const auto &blueHistogram = imageEditorModel->blueHistogram();
  for (auto i = blueHistogram.cbegin(); i != blueHistogram.cend(); ++i) {
    textStream << "    " << i.key() << ": " << i.value() << endl;
  }
  textStream << "  Entropy: " << imageEditorModel->blueEntropy() << endl;
  textStream << endl;

  textStream << "Alpha Channel:" << endl;
  textStream << "  Histogram: " << endl;
  const auto &alphaHistogram = imageEditorModel->alphaHistogram();
  for (auto i = alphaHistogram.cbegin(); i != alphaHistogram.cend(); ++i) {
    textStream << "    " << i.key() << ": " << i.value() << endl;
  }
  textStream << "  Entropy: " << imageEditorModel->alphaEntropy() << endl;
  textStream << endl;

  QDialog dialog(this);
  dialog.setWindowTitle(tr("Image Information"));

  QTextEdit textEdit;
  textEdit.setText(text);
  textEdit.setReadOnly(true);

  QVBoxLayout vBoxLayout(&dialog);
  vBoxLayout.addWidget(&textEdit);

  QDialogButtonBox dialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal,
                                   &dialog);
  vBoxLayout.addWidget(&dialogButtonBox);
  QObject::connect(&dialogButtonBox, SIGNAL(accepted()), &dialog,
                   SLOT(accept()));

  dialog.exec();
}

ImageEditorView::ImageEditorView(ImageEditorModel &model, QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::ImageEditorView),
      imageEditorModel(&model) {
  ui->setupUi(this);
  ui->graphicsView->setScene(&graphicsScene);

  connect(imageEditorModel, SIGNAL(imageModified()), this,
          SLOT(updateGraphicsScene()));
}

ImageEditorView::~ImageEditorView() { delete ui; }
}  // namespace tlo
