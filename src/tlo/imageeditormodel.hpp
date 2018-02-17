#ifndef TLO_IMAGEEDITORMODEL_HPP
#define TLO_IMAGEEDITORMODEL_HPP

#include <QImage>
#include <QMap>
#include <QObject>

namespace tlo {
class ImageEditorModel : public QObject {
  Q_OBJECT

 private:
  QString filePath_;
  QImage originalImage_;
  QImage image_;

  int revision = 0;
  int computedInfoRevision = -1;
  QMap<int, int> redHistogram_;
  QMap<int, int> greenHistogram_;
  QMap<int, int> blueHistogram_;
  QMap<int, int> alphaHistogram_;
  double redEntropy_;
  double greenEntropy_;
  double blueEntropy_;
  double alphaEntropy_;

  void emitImageModified();
  void copyConvertedOriginalToImage();

 public:
  explicit ImageEditorModel(QObject *parent = nullptr);
  bool load(const QString &filePath);
  bool save(const QString &filePath) const;
  const QString &filePath() const;
  const QImage &originalImage() const;
  const QImage &image() const;
  void revertToOriginal();
  void convertToGrayscaleLightness();
  void convertToGrayscaleAverage();
  void convertToGrayscaleLuminosity();
  void gammaCorrect(double gamma);
  void reduceColorDepthMiddle(int redDepth, int greenDepth, int blueDepth,
                              int alphaDepth);
  void reduceColorDepthLowest(int redDepth, int greenDepth, int blueDepth,
                              int alphaDepth);
  void reduceColorDepthHighest(int redDepth, int greenDepth, int blueDepth,
                               int alphaDepth);
  void reduceColorDepthDynamic(int redDepth, int greenDepth, int blueDepth,
                               int alphaDepth);

  void computeImageInformation();
  const QMap<int, int> &redHistogram();
  const QMap<int, int> &greenHistogram();
  const QMap<int, int> &blueHistogram();
  const QMap<int, int> &alphaHistogram();
  double redEntropy();
  double greenEntropy();
  double blueEntropy();
  double alphaEntropy();

 signals:
  void imageModified();
};
}  // namespace tlo

#endif  // TLO_IMAGEEDITORMODEL_HPP
