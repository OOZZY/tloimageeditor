#include "tlo/imageeditormodel.hpp"
#include <cmath>

namespace tlo {
void ImageEditorModel::emitImageModified() {
  revision++;
  emit imageModified();
}

void ImageEditorModel::copyConvertedOriginalToImage() {
  if (originalImage_.hasAlphaChannel()) {
    image_ = originalImage_.convertToFormat(QImage::Format_ARGB32);
  } else {
    image_ = originalImage_.convertToFormat(QImage::Format_RGB32);
  }
  emitImageModified();
}

ImageEditorModel::ImageEditorModel(QObject *parent) : QObject(parent) {}

bool ImageEditorModel::load(const QString &filePath) {
  bool loaded = originalImage_.load(filePath);
  if (!loaded) {
    return false;
  }

  this->filePath_ = filePath;
  copyConvertedOriginalToImage();
  return true;
}

bool ImageEditorModel::save(const QString &filePath) const {
  return image_.save(filePath);
}

const QString &ImageEditorModel::filePath() const { return filePath_; }
const QImage &ImageEditorModel::originalImage() const { return originalImage_; }
const QImage &ImageEditorModel::image() const { return image_; }

namespace {
int min(int a, int b) { return b < a ? b : a; }
int min(int a, int b, int c) { return min(min(a, b), c); }
int max(int a, int b) { return b > a ? b : a; }
int max(int a, int b, int c) { return max(max(a, b), c); }

template <typename Function>
void recolor(QImage &image, Function computeNewColor) {
  QRgb *pixels = reinterpret_cast<QRgb *>(image.bits());
  int pixelCount = image.byteCount() / static_cast<int>(sizeof(QRgb));
  for (int i = 0; i < pixelCount; ++i) {
    int red = qRed(pixels[i]);
    int green = qGreen(pixels[i]);
    int blue = qBlue(pixels[i]);
    int alpha = qAlpha(pixels[i]);
    pixels[i] = computeNewColor(red, green, blue, alpha);
  }
}

const auto grayscaleLightness = [](int red, int green, int blue,
                                   int alpha) -> QRgb {
  int lightness =
      static_cast<int>((max(red, green, blue) + min(red, green, blue)) / 2.0);
  return qRgba(lightness, lightness, lightness, alpha);
};

const auto grayscaleAverage = [](int red, int green, int blue,
                                 int alpha) -> QRgb {
  int average = static_cast<int>((red + green + blue) / 3.0);
  return qRgba(average, average, average, alpha);
};

const auto grayscaleLuminosity = [](int red, int green, int blue,
                                    int alpha) -> QRgb {
  int luminosity = static_cast<int>(0.21 * red + 0.72 * green + 0.07 * blue);
  return qRgba(luminosity, luminosity, luminosity, alpha);
};

struct GammaCorrect {
  double gamma;

  QRgb operator()(int red, int green, int blue, int alpha) const {
    int redCorrected =
        static_cast<int>(std::pow(red / 255.0, 1.0 / gamma) * 255.0);
    int greenCorrected =
        static_cast<int>(std::pow(green / 255.0, 1.0 / gamma) * 255.0);
    int blueCorrected =
        static_cast<int>(std::pow(blue / 255.0, 1.0 / gamma) * 255.0);
    return qRgba(redCorrected, greenCorrected, blueCorrected, alpha);
  }
};

struct Middle {
  int operator()(int value, double newIncrementSize) const {
    double index = std::floor(value / newIncrementSize);
    double lowest = index * newIncrementSize;
    double highest = (index + 1) * newIncrementSize - 1;
    return static_cast<int>(lowest + 0.5 * (highest - lowest));
  }
};

struct Lowest {
  int operator()(int value, double newIncrementSize) const {
    double index = std::floor(value / newIncrementSize);
    return static_cast<int>(index * newIncrementSize);
  }
};

struct Highest {
  int operator()(int value, double newIncrementSize) const {
    double index = std::floor(value / newIncrementSize);
    return static_cast<int>((index + 1) * newIncrementSize - 1);
  }
};

struct Dynamic {
  int operator()(int value, double newIncrementSize) const {
    double index = std::floor(value / newIncrementSize);
    double lowest = index * newIncrementSize;
    double highest = (index + 1) * newIncrementSize - 1;
    double maxIndex = std::floor(255.0 / newIncrementSize);
    return static_cast<int>(lowest + (index / maxIndex) * (highest - lowest));
  }
};

template <typename Function>
struct ReduceColorDepth {
  int redDepth;
  int greenDepth;
  int blueDepth;
  int alphaDepth;

  QRgb operator()(int red, int green, int blue, int alpha) const {
    int numRedValues = static_cast<int>(std::pow(2, redDepth));
    int numGreenValues = static_cast<int>(std::pow(2, greenDepth));
    int numBlueValues = static_cast<int>(std::pow(2, blueDepth));
    int numAlphaValues = static_cast<int>(std::pow(2, alphaDepth));

    double redIncrementSize = 256.0 / numRedValues;
    double greenIncrementSize = 256.0 / numGreenValues;
    double blueIncrementSize = 256.0 / numBlueValues;
    double alphaIncrementSize = 256.0 / numAlphaValues;

    Function computeNewValue;
    int redReduced = computeNewValue(red, redIncrementSize);
    int greenReduced = computeNewValue(green, greenIncrementSize);
    int blueReduced = computeNewValue(blue, blueIncrementSize);
    int alphaReduced = computeNewValue(alpha, alphaIncrementSize);
    return qRgba(redReduced, greenReduced, blueReduced, alphaReduced);
  }
};
}  // namespace

void ImageEditorModel::revertToOriginal() { copyConvertedOriginalToImage(); }

void ImageEditorModel::convertToGrayscaleLightness() {
  recolor(image_, grayscaleLightness);
  emitImageModified();
}

void ImageEditorModel::convertToGrayscaleAverage() {
  recolor(image_, grayscaleAverage);
  emitImageModified();
}

void ImageEditorModel::convertToGrayscaleLuminosity() {
  recolor(image_, grayscaleLuminosity);
  emitImageModified();
}

void ImageEditorModel::gammaCorrect(double gamma) {
  recolor(image_, GammaCorrect{gamma});
  emitImageModified();
}

void ImageEditorModel::reduceColorDepthMiddle(int redDepth, int greenDepth,
                                              int blueDepth, int alphaDepth) {
  recolor(image_, ReduceColorDepth<Middle>{redDepth, greenDepth, blueDepth,
                                           alphaDepth});
  emitImageModified();
}

void ImageEditorModel::reduceColorDepthLowest(int redDepth, int greenDepth,
                                              int blueDepth, int alphaDepth) {
  recolor(image_, ReduceColorDepth<Lowest>{redDepth, greenDepth, blueDepth,
                                           alphaDepth});
  emitImageModified();
}

void ImageEditorModel::reduceColorDepthHighest(int redDepth, int greenDepth,
                                               int blueDepth, int alphaDepth) {
  recolor(image_, ReduceColorDepth<Highest>{redDepth, greenDepth, blueDepth,
                                            alphaDepth});
  emitImageModified();
}

void ImageEditorModel::reduceColorDepthDynamic(int redDepth, int greenDepth,
                                               int blueDepth, int alphaDepth) {
  recolor(image_, ReduceColorDepth<Dynamic>{redDepth, greenDepth, blueDepth,
                                            alphaDepth});
  emitImageModified();
}

void ImageEditorModel::computeImageInformation() {
  if (computedInfoRevision == revision) {
    return;
  }

  redHistogram_.clear();
  greenHistogram_.clear();
  blueHistogram_.clear();
  alphaHistogram_.clear();
  QRgb *pixels = reinterpret_cast<QRgb *>(image_.bits());
  int pixelCount = image_.byteCount() / static_cast<int>(sizeof(QRgb));
  for (int i = 0; i < pixelCount; ++i) {
    int red = qRed(pixels[i]);
    int green = qGreen(pixels[i]);
    int blue = qBlue(pixels[i]);
    int alpha = qAlpha(pixels[i]);
    redHistogram_[red]++;
    greenHistogram_[green]++;
    blueHistogram_[blue]++;
    alphaHistogram_[alpha]++;
  }

  redEntropy_ = 0;
  greenEntropy_ = 0;
  blueEntropy_ = 0;
  alphaEntropy_ = 0;
  for (int i = 0; i < 256; ++i) {
    if (redHistogram_.contains(i)) {
      double redProbability =
          static_cast<double>(redHistogram_[i]) / pixelCount;
      redEntropy_ += -redProbability * std::log2(redProbability);
    }

    if (greenHistogram_.contains(i)) {
      double greenProbability =
          static_cast<double>(greenHistogram_[i]) / pixelCount;
      greenEntropy_ += -greenProbability * std::log2(greenProbability);
    }

    if (blueHistogram_.contains(i)) {
      double blueProbability =
          static_cast<double>(blueHistogram_[i]) / pixelCount;
      blueEntropy_ += -blueProbability * std::log2(blueProbability);
    }

    if (alphaHistogram_.contains(i)) {
      double alphaProbability =
          static_cast<double>(alphaHistogram_[i]) / pixelCount;
      alphaEntropy_ += -alphaProbability * std::log2(alphaProbability);
    }
  }

  computedInfoRevision = revision;
}

const QMap<int, int> &ImageEditorModel::redHistogram() {
  computeImageInformation();
  return redHistogram_;
}

const QMap<int, int> &ImageEditorModel::greenHistogram() {
  computeImageInformation();
  return greenHistogram_;
}

const QMap<int, int> &ImageEditorModel::blueHistogram() {
  computeImageInformation();
  return blueHistogram_;
}

const QMap<int, int> &ImageEditorModel::alphaHistogram() {
  computeImageInformation();
  return alphaHistogram_;
}

double ImageEditorModel::redEntropy() {
  computeImageInformation();
  return redEntropy_;
}

double ImageEditorModel::greenEntropy() {
  computeImageInformation();
  return greenEntropy_;
}

double ImageEditorModel::blueEntropy() {
  computeImageInformation();
  return blueEntropy_;
}

double ImageEditorModel::alphaEntropy() {
  computeImageInformation();
  return alphaEntropy_;
}
}  // namespace tlo
