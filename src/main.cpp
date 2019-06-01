//
//  main.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 3/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#define BUG_TEST 0

#if !BUG_TEST

#include "application.hpp"
#include <iostream>
#include <QtGui/qevent.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlayout.h>
#include <QtWidgets/qmenubar.h>
#include <QtWidgets/qscrollarea.h>
#include <QtWidgets/qdockwidget.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qmainwindow.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qapplication.h>
#include <QtWidgets/qstylepainter.h>
#include <QtWidgets/qdesktopwidget.h>

/*
In case I decide that we need KC filters

#include <pixman-1/pixman.h>

#define d2f pixman_double_to_fixed
    const double frac = 1.0/5.0;
    pixman_fixed_t filterParams[] = {
      d2f(3), d2f(3),
      d2f(-1), d2f(-1), d2f(0),
      d2f(-1), d2f(0), d2f(1),
      d2f(0), d2f(1), d2f(1)
 
      //d2f(5), d2f(5),
      //d2f(1), d2f(0), d2f(0), d2f(0), d2f(0),
      //d2f(0), d2f(1), d2f(0), d2f(0), d2f(0),
      //d2f(0), d2f(0), d2f(0), d2f(0), d2f(0),
      //d2f(0), d2f(0), d2f(0), d2f(-1), d2f(0),
      //d2f(0), d2f(0), d2f(0), d2f(0), d2f(-1),
 
      //d2f(3), d2f(3),
      //d2f(frac), d2f(frac), d2f(frac),
      //d2f(frac), d2f(frac), d2f(frac),
      //d2f(frac), d2f(frac), d2f(frac),
    };
 
    pixman_image_set_filter(
      pixman_img,
      PIXMAN_FILTER_CONVOLUTION,
      filterParams,
      std::size(filterParams)
    );
    pixman_image_composite32(
      PIXMAN_OP_SRC,
      pixman_img,
      nullptr,
      pixman_filtered,
      0, 0, 0, 0, 0, 0,
      w, h
    );
    QImage filtered(filteredDat, w, h, QImage::Format_ARGB32);
 
    pixman_image *pixman_img = pixman_image_create_bits(
      PIXMAN_a8r8g8b8,
      w, h,
      reinterpret_cast<uint32_t *>(img.bits()),
      img.bytesPerLine()
    );
    uint8_t *filteredDat = new uint8_t[w * h * 4];
    pixman_image *pixman_filtered = pixman_image_create_bits(
      PIXMAN_a8r8g8b8,
      w, h,
      reinterpret_cast<uint32_t *>(filteredDat),
      w * 4
    );
*/


/*
 
 Performing basic graphics operations on QImages and rendering them
 
    setWindowTitle("Pixel 2");
    setupMenubar();
 
    int w = 256;
    int h = 256;
 
    uint8_t *imgDat = new uint8_t[4 * w * h];
    QImage img{imgDat, w, h, QImage::Format_ARGB32};
    std::memset(imgDat, 0, 4 * w * h);

    int x = 50;
    int y = 40;

    {
      QPainter painter{&img};
      painter.setRenderHint(QPainter::Antialiasing, false);
      painter.setCompositionMode(QPainter::CompositionMode_Source);
      painter.fillRect(x, y, 101, 101, QColor{0, 0, 255, 127});
 
      painter.setBrush(QBrush{QColor{255, 0, 0}});
      painter.setPen(QColor{0, 255, 0});
      painter.drawEllipse({x + 50, y + 50}, 50, 50);
 
      painter.setPen(QColor{255, 0, 255});
      painter.drawLine(x + 20, y + 30, x + 68, y + 85);
    }

    int index = ((y + 0) * w + (x + 0)) * 4;
    std::cout << int(imgDat[index]) << ' ' << int(imgDat[index + 1]) << ' ' << int(imgDat[index + 2]) << ' ' << int(imgDat[index + 3]) << '\n';

    union {
      uint32_t rgba;
      uint8_t comp[4];
    };

    rgba = (127 << 24) | (255 << 0);
    std::cout << int(comp[0]) << ' ' << int(comp[1]) << ' ' << int(comp[2]) << ' ' << int(comp[3]) << '\n';

    uint8_t *maskDat = new uint8_t[w * h];
    START_TIMER(MemsetMask);
    std::memset(maskDat, 255, w * h);
    STOP_TIMER(MemsetMask);
    QImage mask{maskDat, w, h, QImage::Format_Alpha8}; // for drawing and masking
    QImage realMask{maskDat, w, h, QImage::Format_Grayscale8}; // for rendering

    START_TIMER(RenderMask);
    {
      QPainter painter{&mask};
      painter.setRenderHint(QPainter::Antialiasing, false);
      painter.setCompositionMode(QPainter::CompositionMode_Source);
      painter.fillRect(x, y, 70, 70, QColor{0, 0, 0, 0});
    }
    STOP_TIMER(RenderMask);

    QImage maskAnd = mask;
    maskAnd.detach();

    START_TIMER(AndMask);
    andMask(maskAnd, mask);
    STOP_TIMER(AndMask);

    START_TIMER(CopyMask);
    QImage copy = mask;
    copy.detach();
    STOP_TIMER(CopyMask);

    START_TIMER(NotMask);
    notMask(mask);
    notMask(mask);
    STOP_TIMER(NotMask);

    START_TIMER(ApplyMask);
    applyMask(img, mask);
    STOP_TIMER(ApplyMask);

    START_TIMER(ToTexture);
    pixmap = QPixmap::fromImage(img, Qt::NoFormatConversion);
    STOP_TIMER(ToTexture);
    //pixmap = QPixmap::fromImage(realMask, Qt::NoFormatConversion);

    //pixmap = QPixmap::fromImage(realMask);
    //pixmap.load("/Users/indikernick/Library/Developer/Xcode/DerivedData/Pixel_2-gqoblrlhvynmicgniivandqktune/Build/Products/Debug/Pixel 2.app/Contents/Resources/icon.png");

    label.setPixmap(pixmap);
    label.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    label.setMinimumSize(0, 0);
    label.resize(w, h);
    label.show();

    layout.addWidget(&button, 0, 0);
    layout.addWidget(&label, 1, 0);
    setLayout(&layout);

    show();

*/

#include <cmath>
#include "formats.hpp"
#include "porter duff.hpp"

struct ColorF {
  float r, g, b, a;
};

float toFloat(const uint8_t component) {
  return static_cast<float>(component) / 255.0f;
}

uint8_t fromFloat(const float component) {
  return static_cast<uint8_t>(std::clamp(std::round(component * 255.0f), 0.0f, 255.0f));
}

ColorF toFloat(const Color color) {
  return {toFloat(color.r), toFloat(color.g), toFloat(color.b), toFloat(color.a)};
}

Color fromFloat(const ColorF color) {
  return {fromFloat(color.r), fromFloat(color.g), fromFloat(color.b), fromFloat(color.a)};
}

// straight alpha with float precision
Color compositeF(const Color aInt, const Color bInt, const uint8_t afInt, const uint8_t bfInt) {
  const float aF = toFloat(afInt);
  const float bF = toFloat(bfInt);
  const ColorF a = toFloat(aInt);
  const ColorF b = toFloat(bInt);
  
  const float cA = a.a*aF + b.a*bF;
  if (cA == 0.0f) {
    return {0, 0, 0, 0};
  } else {
    const float cR = (a.a*aF*a.r + b.a*bF*b.r) / cA;
    const float cG = (a.a*aF*a.g + b.a*bF*b.g) / cA;
    const float cB = (a.a*aF*a.b + b.a*bF*b.b) / cA;
    return fromFloat({cR, cG, cB, cA});
  }
}

// straight alpha with uint8 precision
Color compositeI(const Color a, const Color b, const uint8_t aF, const uint8_t bF) {
  const uint32_t cA = a.a*aF + b.a*bF;
  if (cA == 0) {
    return {0, 0, 0, 0};
  } else {
    const uint8_t cR = (a.a*aF*a.r + b.a*bF*b.r) / cA;
    const uint8_t cG = (a.a*aF*a.g + b.a*bF*b.g) / cA;
    const uint8_t cB = (a.a*aF*a.b + b.a*bF*b.b) / cA;
    return {cR, cG, cB, static_cast<uint8_t>(cA / 255)};
  }
}

// premultiplied alpha with uint8 precision
Color compositeM(const Color a, const Color b, const uint8_t aF, const uint8_t bF) {
  const uint8_t cR = (aF*a.r + bF*b.r) / 255;
  const uint8_t cG = (aF*a.g + bF*b.g) / 255;
  const uint8_t cB = (aF*a.b + bF*b.b) / 255;
  const uint8_t cA = (aF*a.a + bF*b.a) / 255;
  return {cR, cG, cB, cA};
}

Color mulAlpha(const Color color) {
  const uint8_t r = (color.r * color.a) / 255;
  const uint8_t g = (color.g * color.a) / 255;
  const uint8_t b = (color.b * color.a) / 255;
  return {r, g, b, color.a};
}

Color divAlpha(const Color color) {
  if (color.a == 0) {
    return {0, 0, 0, 0};
  } else {
    const uint8_t r = (color.r * 255) / color.a;
    const uint8_t g = (color.g * 255) / color.a;
    const uint8_t b = (color.b * 255) / color.a;
    return {r, g, b, color.a};
  }
}

std::ostream &operator<<(std::ostream &stream, const Color color) {
  stream.width(3);
  stream << int(color.r) << ' ';
  stream.width(3);
  stream << int(color.g) << ' ';
  stream.width(3);
  stream << int(color.b) << ' ';
  stream.width(3);
  stream << int(color.a);
  return stream;
}

std::ostream &operator<<(std::ostream &stream, const ColorF color) {
  stream.width(3);
  stream << color.r << ' ';
  stream.width(3);
  stream << color.g << ' ';
  stream.width(3);
  stream << color.b << ' ';
  stream.width(3);
  stream << color.a;
  return stream;
}

bool operator==(const Color a, const Color b) {
  return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}

bool operator!=(const Color a, const Color b) {
  return !(a == b);
}

constexpr int difference_threshold = 0;

bool different(const uint8_t a, const uint8_t b) {
  const int aI = a;
  const int bI = b;
  return aI + difference_threshold < bI || bI + difference_threshold < aI;
}

bool different(const Color a, const Color b) {
  return different(a.r, b.r) ||
         different(a.g, b.g) ||
         different(a.b, b.b) ||
         different(a.a, b.a);
}

void testComposite() {
  std::cout << "A                   B                   Float               Int                 Premul\n";

  uint8_t vals[] = {0, 63, 127, 191, 255};
  for (int cA = 0; cA != sizeof(vals); ++cA) {
    for (int aA = 0; aA != sizeof(vals); ++aA) {
      for (int cB = 0; cB != sizeof(vals); ++cB) {
        for (int aB = 0; aB != sizeof(vals); ++aB) {
          const Color a = {vals[cA], 0, 0, vals[aA]};
          const Color b = {vals[cB], 0, 0, vals[aB]};
          const uint8_t aF = 255;
          const uint8_t bF = 255 - a.a;
          const Color cF = compositeF(a, b, aF, bF);
          const Color cI = compositeI(a, b, aF, bF);
          //const Color cM = divAlpha(compositeM(mulAlpha(a), mulAlpha(b), aF, bF));
          //if (different(cF, cI) || different(cI, cM) || different(cF, cM)) {
          //  std::cout << a << " \t" << b << " \t" << cF << " \t" << cI << " \t" << cM << '\n';
          //}
          if (different(cF, cI)) {
            std::cout << a << " \t" << b << " \t" << cF << " \t" << cI << '\n';
          }
        }
      }
    }
  }
}

void applyMask(QImage &image, const QImage &mask) {
  assert(image.size() == mask.size());
  assert(image.format() == QImage::Format_ARGB32);
  assert(mask.format() == QImage::Format_Alpha8);
  
  image.detach();
  
  const ptrdiff_t dstPitch = image.bytesPerLine();
  uchar *dstData = image.bits();
  const ptrdiff_t maskPitch = mask.bytesPerLine();
  const uchar *maskData = mask.bits();
  const ptrdiff_t width = image.width();
  const ptrdiff_t height = image.height();
  
  for (ptrdiff_t y = 0; y != height; ++y) {
    for (ptrdiff_t x = 0; x != width; ++x) {
      uchar *const dstPixel = dstData + x * 4;
      const uchar maskPixel = maskData[x];
      dstPixel[0] &= maskPixel;
      dstPixel[1] &= maskPixel;
      dstPixel[2] &= maskPixel;
      dstPixel[3] &= maskPixel;
    }
    dstData += dstPitch;
    maskData += maskPitch;
  }
}

// each scanline is 32-bit aligned

void notMask(QImage &dst) {
  assert(dst.format() == QImage::Format_Alpha8);
  
  dst.detach();
  
  const ptrdiff_t pitch = dst.bytesPerLine();
  uchar *data = dst.bits();
  const ptrdiff_t width = dst.width() / 4;
  const ptrdiff_t height = dst.height();
  
  for (ptrdiff_t y = 0; y != height; ++y) {
    for (ptrdiff_t x = 0; x != width; ++x) {
      uint32_t *const pixel = reinterpret_cast<uint32_t *>(data) + x;
      *pixel ^= 0xFFFFFFFF;
    }
    data += pitch;
  }
}

template <typename Op>
void binaryMaskOp(QImage &dst, const QImage &src, Op &&op) {
  assert(dst.size() == src.size());
  assert(dst.format() == QImage::Format_Alpha8);
  assert(src.format() == QImage::Format_Alpha8);
  assert(dst.bytesPerLine() == src.bytesPerLine());
  
  dst.detach();
  
  const ptrdiff_t pitch = dst.bytesPerLine();
  uchar *dstData = dst.bits();
  const uchar *srcData = src.bits();
  const ptrdiff_t width = dst.width() / 4;
  const ptrdiff_t height = dst.height();
  
  for (ptrdiff_t y = 0; y != height; ++y) {
    for (ptrdiff_t x = 0; x != width; ++x) {
      uint32_t *const dstPixel = reinterpret_cast<uint32_t *>(dstData) + x;
      const uint32_t srcPixel = reinterpret_cast<const uint32_t *>(srcData)[x];
      op(dstPixel, srcPixel);
    }
    dstData += pitch;
    srcData += pitch;
  }
}

void andMask(QImage &dst, const QImage &src) {
  binaryMaskOp(dst, src, [](uint32_t *const dstPixel, const uint32_t srcPixel) {
    *dstPixel &= srcPixel;
  });
}

void orMask(QImage &dst, const QImage &src) {
  binaryMaskOp(dst, src, [](uint32_t *const dstPixel, const uint32_t srcPixel) {
    *dstPixel |= srcPixel;
  });
}

void xorMask(QImage &dst, const QImage &src) {
  binaryMaskOp(dst, src, [](uint32_t *const dstPixel, const uint32_t srcPixel) {
    *dstPixel ^= srcPixel;
  });
}

uint32_t composeRGBA(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a = 255) {
  return (uint32_t{a} << 24) | (uint32_t{r} << 16) | (uint32_t{g} << 8) | uint32_t{b};
}

class Timer {
public:
  using Clock = std::chrono::high_resolution_clock;
  using OutputDuration = std::chrono::duration<double, std::milli>;
  
  void start(const char *timerName) {
    assert(timerName);
    assert(!name);
    name = timerName;
    startTime = Clock::now();
  }
  void stop() {
    assert(name);
    const auto endTime = Clock::now();
    const auto diff = std::chrono::duration_cast<OutputDuration>(
      endTime - startTime
    );
    std::cout.width(16);
    std::cout << std::left << name << ' ';
    std::cout.precision(3);
    std::cout << diff.count() << "ms\n";
    name = nullptr;
  }
  void stopAndStart(const char *timerName) {
    stop();
    start(timerName);
  }

private:
  const char *name = nullptr;
  Clock::time_point startTime;
};
/*
#include <QtCore/qtimer.h>

class StatusBar : public QWidget {
public:
  explicit StatusBar(QWidget *parent)
    : QWidget{parent}, label{this} {
    timer.setInterval(5000);
    timer.setSingleShot(true);
    CONNECT(&timer, timeout, this, hideTemp);
    label.setMinimumWidth(400);
  }
  
  void showTemp(const QString &text) {
    tempText = text;
    timer.start();
    setText();
  }
  void showPerm(const QString &text) {
    permText = text;
    setText();
  }
  
private:
  QLabel label;
  QString permText;
  QString tempText;
  QTimer timer;
  
  void setText() {
    if (tempText.isEmpty()) {
      label.setText(permText);
    } else {
      label.setText(permText + " | " + tempText);
    }
  }
  void hideTemp() {
    tempText = "";
    setText();
  }
};

class ToolsWidget : public QWidget {
public:
  ToolsWidget(QWidget *parent)
    : QWidget{parent} {
    layout = new QVBoxLayout{this};
    setLayout(layout);
    setFixedSize(24*2+2, (24*2+2) * 11);
  }

private:
  QVBoxLayout *layout = nullptr;
};

class TimelineWidget : public QWidget {
public:
  explicit TimelineWidget(QWidget *parent)
    : QWidget{parent}, status{this} {
    setMinimumHeight(128);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    status.showPerm("Permanent message");
    status.showTemp("Temporary message");
  }

private:
  StatusBar status;
};

class RenderWidget : public QScrollArea {
public:
  RenderWidget(QWidget *parent)
    : QScrollArea{parent} {
    setMinimumSize(128, 128);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setAlignment(Qt::AlignCenter);
    view = new QLabel{this};
    img.load("/Users/indikernick/Library/Developer/Xcode/DerivedData/Pixel_2-gqoblrlhvynmicgniivandqktune/Build/Products/Debug/Pixel 2.app/Contents/Resources/icon.png");
    view->setPixmap(img.scaled(64, 64));
    view->resize(64, 64);
    setWidget(view);
  }
  
private:
  QLabel *view = nullptr;
  QPixmap img;
};

class EditorWidget : public QScrollArea {
public:
  EditorWidget(QWidget *parent)
    : QScrollArea{parent},
      cursor{Qt::CrossCursor} {
    setMinimumSize(128, 128);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setAlignment(Qt::AlignCenter);
    view = new QLabel{this};
    img.load("/Users/indikernick/Library/Developer/Xcode/DerivedData/Pixel_2-gqoblrlhvynmicgniivandqktune/Build/Products/Debug/Pixel 2.app/Contents/Resources/icon.png");
    view->setPixmap(img.scaled(1024, 1024));
    view->resize(1024, 1024);
    setWidget(view);
    setCursor(cursor);
  }
  
private:
  QLabel *view = nullptr;
  QPixmap img;
  QCursor cursor;
};

class CentralWidget : public QWidget {
public:
  CentralWidget(QWidget *parent)
    : QWidget{parent} {
    layout = new QHBoxLayout{this};
    layout->addWidget(new EditorWidget{this});
    layout->addWidget(new RenderWidget{this});
    setLayout(layout);
  }

private:
  QHBoxLayout *layout = nullptr;
};

class Window : public QMainWindow {
public:
  Window() {
    setupMenubar();
    setupUI();
    show();
  }

  void fileOpen(QFileOpenEvent *event) {
    //button.setText(static_cast<QFileOpenEvent *>(event)->file());
  }

private:
  QMenuBar *menubar = nullptr;
  QDockWidget *toolDock = nullptr;
  QDockWidget *timelineDock = nullptr;
  
  void setupUI() {
    toolDock = new QDockWidget{this};
    toolDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    toolDock->setAllowedAreas(Qt::LeftDockWidgetArea);
    toolDock->setWidget(new ToolsWidget{this});
    toolDock->setTitleBarWidget(new QWidget{});
    addDockWidget(Qt::LeftDockWidgetArea, toolDock);
    
    timelineDock = new QDockWidget{this};
    timelineDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    timelineDock->setAllowedAreas(Qt::BottomDockWidgetArea);
    timelineDock->setWidget(new TimelineWidget{this});
    timelineDock->setTitleBarWidget(new QWidget{});
    addDockWidget(Qt::BottomDockWidgetArea, timelineDock);
    
    setCentralWidget(new CentralWidget{this});
  }
  
  void setupMenubar() {
    menubar = new QMenuBar{this};
    QMenu *file = menubar->addMenu("File");
    QAction *open = file->addAction("Open");
    QAction *save = file->addAction("Save");
    file->addSeparator();
    file->addAction("Export");
    CONNECT(open, triggered, this, openDoc);
    CONNECT(save, triggered, this, saveDoc);
  }

  void openDoc() {
    std::cout << "Open document\n";
    QString path = QFileDialog::getOpenFileName(
      this,
      "Open Image",
      QDir::homePath(),
      "PNG Files (*.png)",
      nullptr,
      QFileDialog::ReadOnly | QFileDialog::HideNameFilterDetails
    );
    if (!path.isEmpty()) {
      //pixmap.load(path);
      //label.setPixmap(pixmap);
      //label.resize(pixmap.size());
    }
  }
  void saveDoc() {
    std::cout << "Save document\n";
    QString path = QFileDialog::getSaveFileName(
      this,
      "Save Image",
      QDir::homePath() + "/Frame 0.png",
      "",
      nullptr,
      QFileDialog::HideNameFilterDetails
    );
    if (!path.isEmpty()) {
      //pixmap.save(path);
    }
  }
  
  void resizeEvent(QResizeEvent *event) override {
    
  }
};

class Application : public QApplication {
public:
  Application(int &argc, char **argv)
    : QApplication{argc, argv} {
    window.setMinimumSize(640, 360);
    window.setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    window.setGeometry(
      QStyle::alignedRect(
        Qt::LeftToRight,
        Qt::AlignCenter,
        window.size(),
        desktop()->availableGeometry()
      )
    );
  }
  
private:
  Window window;
  
  bool event(QEvent *event) override {
    if (event->type() == QEvent::FileOpen) {
      window.fileOpen(static_cast<QFileOpenEvent *>(event));
      return true;
    } else {
      return QApplication::event(event);
    }
  }
};*/

#include "animation.hpp"
#include "cell impls.hpp"

template <typename CellClass>
auto getCell(const Animation &anim, const LayerIdx l, const FrameIdx f) {
  Cell *cell = anim.getCell(l, f);
  assert(cell);
  auto *derived = dynamic_cast<CellClass *>(cell);
  assert(derived);
  return derived;
};

QImage dupImage(const QImage &img) {
  QImage dup = makeCompatible(img);
  std::memcpy(dup.bits(), img.constBits(), img.sizeInBytes());
  return dup;
}

#include "tool impls.hpp"
#include "composite.hpp"
#include "painting.hpp"

void drawSectors(
  QImage &image,
  const QPoint c,
  const QPoint p,
  const QRgb color
) {
  image.setPixel(c.x()+p.x(), c.y()+p.y(), color);
  image.setPixel(c.x()-p.x(), c.y()+p.y(), color);
  image.setPixel(c.x()+p.x(), c.y()-p.y(), color);
  image.setPixel(c.x()-p.x(), c.y()-p.y(), color);
  image.setPixel(c.x()+p.y(), c.y()+p.x(), color);
  image.setPixel(c.x()-p.y(), c.y()+p.x(), color);
  image.setPixel(c.x()+p.y(), c.y()-p.x(), color);
  image.setPixel(c.x()-p.y(), c.y()-p.x(), color);
}

void bresenhamStroked(
  QImage &img,
  const QRgb col,
  const QPoint ctr,
  const int rad,
  CircleShape
) {
  QPoint pos = {0, rad};
  int err = 3 - 2 * rad;
  drawSectors(img, ctr, pos, col);
  while (pos.y() >= pos.x()) {
    ++pos.rx();
    if (err > 0) {
      --pos.ry();
      err = err + 4 * (pos.x() - pos.y()) + 10;
    } else {
      err = err + 4 * pos.x() + 6;
    }
    drawSectors(img, ctr, pos, col);
  }
}

void moreGreen(QRgb &color) {
  color = qRgba(qRed(color), qGreen(color) + 12, qBlue(color) - 12, qAlpha(color));
}

void midpointStroked(
  QImage &img,
  QRgb col,
  const QPoint ctr,
  const int rad,
  CircleShape
) { // radius 6
  QPoint pos = {rad, 0};
  int err = 1 - rad;
  while (pos.x() >= pos.y()) {
    img.setPixel(ctr.x() + pos.x(), ctr.y() + pos.y(), col);
    img.setPixel(ctr.x() - pos.x(), ctr.y() + pos.y(), col);
    img.setPixel(ctr.x() + pos.x(), ctr.y() - pos.y(), col);
    img.setPixel(ctr.x() - pos.x(), ctr.y() - pos.y(), col);
    
    img.setPixel(ctr.x() + pos.y(), ctr.y() + pos.x(), col);
    img.setPixel(ctr.x() - pos.y(), ctr.y() + pos.x(), col);
    img.setPixel(ctr.x() + pos.y(), ctr.y() - pos.x(), col);
    img.setPixel(ctr.x() - pos.y(), ctr.y() - pos.x(), col);
    
    ++pos.ry();
    
    if (err < 0) {
      err += 2 * pos.y() + 1;
    } else {
      --pos.rx();
      err += 2 * (pos.y() - pos.x()) + 1;
    }
  }
}

void fillScanLine(QImage &img, const QRgb col, const QPoint first, const int last) {
  uchar *rowBytes = img.bits() + first.y() * img.bytesPerLine() + first.x() * sizeof(QRgb);
  QRgb *row = reinterpret_cast<QRgb *>(rowBytes);
  QRgb *const rowEnd = row + (last - first.x() + 1);
  while (row != rowEnd) {
    *row++ = col;
  }
}

void fillVertLine(QImage &img, const QRgb col, const QPoint first, const int last) {
  uchar *row = img.bits() + first.y() * img.bytesPerLine() + first.x() * sizeof(QRgb);
  const uintptr_t bbl = img.bytesPerLine();
  uchar *const rowEnd = row + (last - first.y() + 1) * bbl;
  while (row != rowEnd) {
    *reinterpret_cast<QRgb *>(row) = col;
    row += bbl;
  }
}

void midpointFilled(
  QImage &img,
  QRgb col,
  const QPoint ctr,
  const int rad,
  CircleShape
) { // radius 6
  QPoint pos = {rad, 0};
  int err = 1 - rad;
  while (pos.x() >= pos.y()) {
    fillScanLine(img, col, {ctr.x() - pos.x(), ctr.y() + pos.y()}, ctr.x() + pos.x());
    fillScanLine(img, col, {ctr.x() - pos.x(), ctr.y() - pos.y()}, ctr.x() + pos.x());
    fillScanLine(img, col, {ctr.x() - pos.y(), ctr.y() + pos.x()}, ctr.x() + pos.y());
    fillScanLine(img, col, {ctr.x() - pos.y(), ctr.y() - pos.x()}, ctr.x() + pos.y());
    
    ++pos.ry();
    
    if (err < 0) {
      err += 2 * pos.y() + 1;
    } else {
      --pos.rx();
      err += 2 * (pos.y() - pos.x()) + 1;
    }
  }
}

void midpointCircleThick(
  QImage &img,
  const QRgb col,
  const QPoint ctr,
  const int innerRad,
  const int outerRad
) { // radius 6
  int innerX = innerRad;
  int outerX = outerRad;
  int y = 0;
  int innerErr = 1 - innerRad;
  int outerErr = 1 - outerRad;
  
  while (outerX >= y) {
    fillScanLine(img, col, {ctr.x() + innerX, ctr.y() + y}, ctr.x() + outerX);
    fillVertLine(img, col, {ctr.x() + y,      ctr.y() + innerX}, ctr.y() + outerX);
    fillScanLine(img, col, {ctr.x() - outerX, ctr.y() + y}, ctr.x() - innerX);
    fillVertLine(img, col, {ctr.x() - y,      ctr.y() + innerX}, ctr.y() + outerX);
    
    fillScanLine(img, col, {ctr.x() - outerX, ctr.y() - y}, ctr.x() - innerX);
    fillVertLine(img, col, {ctr.x() - y,      ctr.y() - outerX}, ctr.y() - innerX);
    fillScanLine(img, col, {ctr.x() + innerX, ctr.y() - y}, ctr.x() + outerX);
    fillVertLine(img, col, {ctr.x() + y,      ctr.y() - outerX}, ctr.y() - innerX);
    
    ++y;
    
    if (outerErr < 0) {
      outerErr += 2 * y + 1;
    } else {
      --outerX;
      outerErr += 2 * (y - outerX) + 1;
    }
    
    if (y > innerRad) {
      innerX = y;
    } else {
      if (innerErr < 0) {
        innerErr += 2 * y + 1;
      } else {
        --innerX;
        innerErr += 2 * (y - innerX) + 1;
      }
    }
  }
}

void midpointEllipse(
  QImage &img,
  const QRgb col,
  QPoint ctr,
  const QPoint rad
) {
  QPoint pos = {0, rad.y()};
  QPoint del = {2 * rad.y() * rad.y() * pos.x(), 2 * rad.x() * rad.x() * pos.y()};
  int err = rad.y() * rad.y()
          - rad.x() * rad.x() * rad.y()
          + (rad.x() * rad.x()) / 4;
  
  while (del.x() < del.y()) {
    img.setPixel(ctr.x() + pos.x(), ctr.y() + pos.y(), col);
    img.setPixel(ctr.x() - pos.x(), ctr.y() + pos.y(), col);
    img.setPixel(ctr.x() + pos.x(), ctr.y() - pos.y(), col);
    img.setPixel(ctr.x() - pos.x(), ctr.y() - pos.y(), col);
    
    ++pos.rx();
    
    if (err < 0) {
      del.rx() += 2 * rad.y() * rad.y();
      err += del.x() + rad.y() * rad.y();
    } else {
      --pos.ry();
      del.rx() += 2 * rad.y() * rad.y();
      del.ry() -= 2 * rad.x() * rad.x();
      err += del.x() - del.y() + rad.y() * rad.y();
    }
  }
  
  err = rad.y() * rad.y() * (pos.x() * pos.x() + pos.x())
      + rad.x() * rad.x() * (pos.y() - 1) * (pos.y() - 1)
      - rad.x() * rad.x() * rad.y() * rad.y();
  
  while (pos.y() >= 0) {
    img.setPixel(ctr.x() + pos.x(), ctr.y() + pos.y(), col);
    img.setPixel(ctr.x() - pos.x(), ctr.y() + pos.y(), col);
    img.setPixel(ctr.x() + pos.x(), ctr.y() - pos.y(), col);
    img.setPixel(ctr.x() - pos.x(), ctr.y() - pos.y(), col);
    
    --pos.ry();
    
    if (err > 0) {
      del.ry() -= 2 * rad.x() * rad.x();
      err += rad.x() * rad.x() - del.y();
    } else {
      ++pos.rx();
      del.rx() += 2 * rad.y() * rad.y();
      del.ry() -= 2 * rad.x() * rad.x();
      err += del.x() - del.y() + rad.x() * rad.x();
    }
  }
}

void midpointEllipseFilled(
  QImage &img,
  const QRgb col,
  QPoint ctr,
  const QPoint rad
) {
  QPoint pos = {0, rad.y()};
  QPoint del = {2 * rad.y() * rad.y() * pos.x(), 2 * rad.x() * rad.x() * pos.y()};
  int err = rad.y() * rad.y()
          - rad.x() * rad.x() * rad.y()
          + (rad.x() * rad.x()) / 4;
  
  while (del.x() < del.y()) {
    if (err >= 0) {
      fillScanLine(img, col, {ctr.x() - pos.x(), ctr.y() + pos.y()}, ctr.x() + pos.x());
      fillScanLine(img, col, {ctr.x() - pos.x(), ctr.y() - pos.y()}, ctr.x() + pos.x());
    }
    
    ++pos.rx();
    
    if (err < 0) {
      del.rx() += 2 * rad.y() * rad.y();
      err += del.x() + rad.y() * rad.y();
    } else {
      --pos.ry();
      del.rx() += 2 * rad.y() * rad.y();
      del.ry() -= 2 * rad.x() * rad.x();
      err += del.x() - del.y() + rad.y() * rad.y();
    }
  }
  
  err = rad.y() * rad.y() * (pos.x() * pos.x() + pos.x())
      + rad.x() * rad.x() * (pos.y() - 1) * (pos.y() - 1)
      - rad.x() * rad.x() * rad.y() * rad.y();
  
  while (pos.y() >= 0) {
    fillScanLine(img, col, {ctr.x() - pos.x(), ctr.y() + pos.y()}, ctr.x() + pos.x());
    fillScanLine(img, col, {ctr.x() - pos.x(), ctr.y() - pos.y()}, ctr.x() + pos.x());
    
    --pos.ry();
    
    if (err > 0) {
      del.ry() -= 2 * rad.x() * rad.x();
      err += rad.x() * rad.x() - del.y();
    } else {
      ++pos.rx();
      del.rx() += 2 * rad.y() * rad.y();
      del.ry() -= 2 * rad.x() * rad.x();
      err += del.x() - del.y() + rad.x() * rad.x();
    }
  }
}

void midpointEllipseThick(
  QImage &img,
  const QRgb col,
  QPoint ctr,
  const int radX,
  const int radY
) {
  int x = 0;
  int y = radY;
  int err = radY * radY
          - radX * radX * radY
          + (radX * radX) / 4;
  int dx = 2 * radY * radY * x;
  int dy = 2 * radX * radX * y;
  
  while (dx < dy) {
    img.setPixel(ctr.x() + x, ctr.y() + y, col);
    img.setPixel(ctr.x() - x, ctr.y() + y, col);
    img.setPixel(ctr.x() + x, ctr.y() - y, col);
    img.setPixel(ctr.x() - x, ctr.y() - y, col);
    
    ++x;
    
    if (err < 0) {
      dx += 2 * radY * radY;
      err += dx + radY * radY;
    } else {
      --y;
      dx += 2 * radY * radY;
      dy -= 2 * radX * radX;
      err += dx - dy + radY * radY;
    }
  }
  
  err = radY * radY * (x * x + x)
      + radX * radX * (y - 1) * (y - 1)
      - radX * radX * radY * radY;
  
  while (y >= 0) {
    img.setPixel(ctr.x() + x, ctr.y() + y, col);
    img.setPixel(ctr.x() - x, ctr.y() + y, col);
    img.setPixel(ctr.x() + x, ctr.y() - y, col);
    img.setPixel(ctr.x() - x, ctr.y() - y, col);
    
    --y;
    
    if (err > 0) {
      dy -= 2 * radX * radX;
      err += radX * radX - dy;
    } else {
      ++x;
      dx += 2 * radY * radY;
      dy -= 2 * radX * radX;
      err += dx - dy + radX * radX;
    }
  }
}

void midpointLilPos(QImage &img, const QRgb col, const QPoint p1, const QPoint p2) {
  const int dx = p2.x() - p1.x();
  const int dy = p2.y() - p1.y();
  const int incE = 2 * dy;
  const int incSE = 2 * (dy - dx);
  int err = 2 * dy - dx;
  QPoint pos = p1;
  
  img.setPixel(pos, col);
  while (pos.x() < p2.x()) {
    if (err < 0) {
      err += incE;
    } else {
      err += incSE;
      ++pos.ry();
    }
    ++pos.rx();
    img.setPixel(pos, col);
  }
}

void midpointBigPos(QImage &img, const QRgb col, const QPoint p1, const QPoint p2) {
  const int dx = p2.x() - p1.x();
  const int dy = p2.y() - p1.y();
  const int incS = 2 * dx;
  const int incSE = 2 * (dx - dy);
  int err = 2 * dx - dy;
  QPoint pos = p1;
  
  img.setPixel(pos, col);
  while (pos.y() < p2.y()) {
    if (err < 0) {
      err += incS;
    } else {
      err += incSE;
      ++pos.rx();
    }
    ++pos.ry();
    img.setPixel(pos, col);
  }
}

void midpointLilNeg(QImage &img, const QRgb col, const QPoint p1, const QPoint p2) {
  const int dx = p2.x() - p1.x();
  const int dy = p1.y() - p2.y();
  const int incE = 2 * dy;
  const int incNE = 2 * (dy - dx);
  int err = 2 * dy - dx;
  
  QPoint pos = p1;
  img.setPixel(pos, col);
  while (pos.x() < p2.x()) {
    if (err < 0) {
      err += incE;
    } else {
      err += incNE;
      --pos.ry();
    }
    ++pos.rx();
    img.setPixel(pos, col);
  }
}

void midpointBigNeg(QImage &img, const QRgb col, const QPoint p1, const QPoint p2) {
  const int dx = p2.x() - p1.x();
  const int dy = p1.y() - p2.y();
  const int incN = 2*dx;
  const int incNE = 2*(dx-dy);
  int err = 2*dx - dy;
  QPoint pos = p1;
  
  img.setPixel(pos, col);
  while (pos.y() > p2.y()) {
    if (err < 0) {
      err += incN;
    } else {
      err += incNE;
      ++pos.rx();
    }
    --pos.ry();
    img.setPixel(pos, col);
  }
}

void midpointLine(
  QImage &img,
  const QRgb col,
  QPoint p1,
  QPoint p2
) {
  if (p2.x() < p1.x()) {
    std::swap(p1, p2);
  }
  
  const int dx = p2.x() - p1.x();
  const int dy = p2.y() - p1.y();
  
  if (dy == 0) {
    //return fillScanLine(img, col, p1, p2.x());
  } else if (dx == 0) {
    if (dy > 0) {
      //return fillVertLine(img, col, p1, p2.y());
    } else {
      //return fillVertLine(img, col, {p1.x(), p2.y()}, p1.y());
    }
  }
  
  if (0 < dy && dy <= dx) {
    return midpointLilPos(img, col, p1, p2);
  } else if (dx < dy) {
    return midpointBigPos(img, col, p1, p2);
  } else if (-dy <= dx) {
    return midpointLilNeg(img, col, p1, p2);
  } else {
    return midpointBigNeg(img, col, p1, p2);
  }
}

#include <QtGui/qpainter.h>
#include <QtWidgets/qscrollbar.h>
#include <QtWidgets/qmainwindow.h>
#include <QtWidgets/qscrollarea.h>
#include <QtWidgets/qapplication.h>

class InnerWidget final : public QWidget {
public:
  explicit InnerWidget(QScrollArea *parent)
    : QWidget{parent}, parent{parent}, scale{1} {
    updateSize();
    setFocusPolicy(Qt::StrongFocus);
  }

private:
  QScrollArea *parent;
  int scale;
  
  void updateSize() {
    setFixedSize(256 * scale, 256 * scale);
  }

  void paintEvent(QPaintEvent *) override {
    QPainter painter{this};
    const QColor green = {0, 255, 0};
    painter.fillRect(0, 0, width(), height(), {255, 255, 255});
    painter.fillRect(32 * scale, 32 * scale, 16 * scale, 16 * scale, green);
    painter.fillRect(128 * scale, 128 * scale, 16 * scale, 16 * scale, green);
  }
  
  void adjustScroll(const int oldScale) {
    if (scale == oldScale) return;
    QScrollBar *hbar = parent->horizontalScrollBar();
    QScrollBar *vbar = parent->verticalScrollBar();
    /*if (width() >= parent->width()) {
      const int halfWidth = parent->width() / 2;
      hbar->setValue((hbar->value() + halfWidth) * scale / oldScale - halfWidth);
    }
    if (height() >= parent->height()) {
      const int halfHeight = parent->height() / 2;
      vbar->setValue((vbar->value() + halfHeight) * scale / oldScale - halfHeight);
    }*/
    hbar->setValue((hbar->minimum() + hbar->maximum()) / 2);
    vbar->setValue((vbar->minimum() + vbar->maximum()) / 2);
  }
  
  void keyPressEvent(QKeyEvent *event) override {
    const int oldScale = scale;
    if (event->key() == Qt::Key_Z) {
      scale = std::min(scale + 1, 64);
      updateSize();
      adjustScroll(oldScale);
    } else if (event->key() == Qt::Key_X) {
      scale = std::max(scale - 1, 1);
      updateSize();
      adjustScroll(oldScale);
    }
  }
};

namespace test {

#if 1

struct Point {
  int x, y;
};

using Image = QImage;
using Color = QRgb;

void setPixel(Image &image, Color color, Point pos) {
  image.setPixel(pos.x, pos.y, color);
}

#else

struct Point {
  int x, y;
};

struct Image {};
using Color = int;

void setPixel(Image &, Color, Point pos) {
  std::cout << pos.x << ' ' << pos.y << '\n';
}

#endif

void horiLine(Image &img, Color color, Point first, int last) {
  assert(first.x <= last);
  assert(img.isDetached());
  const uintptr_t ppl = img.bytesPerLine() / sizeof(QRgb);
  QRgb *row = reinterpret_cast<QRgb *>(img.bits()) + first.y * ppl + first.x;
  QRgb *const rowEnd = row + (last - first.x);
  while (row != rowEnd) {
    *row++ = color;
  }
}

void vertLine(Image &image, Color color, Point first, int last) {
  assert(first.y <= last);
  while (first.y <= last) {
    setPixel(image, color, first);
    first.y++;
  }
}

void midpointCircle(
  Image &image,
  Color color,
  Point center,
  int radius,
  CircleShape shape
) {
  Point pos = {radius, 0};
  int err = 1 - radius;
  const int extraX = centerOffsetX(shape);
  const int extraY = centerOffsetY(shape);
  
  while (pos.x >= pos.y) {
    setPixel(image, color, {center.x + pos.x + extraX, center.y + pos.y + extraY});
    setPixel(image, color, {center.x - pos.x,          center.y + pos.y + extraY});
    setPixel(image, color, {center.x + pos.x + extraX, center.y - pos.y});
    setPixel(image, color, {center.x - pos.x,          center.y - pos.y});
    
    setPixel(image, color, {center.x + pos.y + extraX, center.y + pos.x + extraY});
    setPixel(image, color, {center.x - pos.y,          center.y + pos.x + extraY});
    setPixel(image, color, {center.x + pos.y + extraX, center.y - pos.x});
    setPixel(image, color, {center.x - pos.y,          center.y - pos.x});
    
    pos.y++;
    
    if (err < 0) {
      err += 2 * pos.y + 1;
    } else {
      pos.x--;
      err += 2 * (pos.y - pos.x) + 1;
    }
  }
}

void midpointCircleThick(
  Image &image,
  Color color,
  Point center,
  int innerRadius,
  int outerRadius,
  CircleShape shape
) {
  assert(0 <= innerRadius);
  assert(innerRadius <= outerRadius);

  int innerX = innerRadius;
  int outerX = outerRadius;
  int posY = 0;
  int innerErr = 1 - innerRadius;
  int outerErr = 1 - outerRadius;
  const int extraX = centerOffsetX(shape);
  const int extraY = centerOffsetY(shape);
  
  while (outerX >= posY) {
    horiLine(image, color, {center.x + innerX + extraX, center.y + posY + extraY},   center.x + outerX + extraX); // right down
    vertLine(image, color, {center.x + posY + extraX,   center.y + innerX + extraY}, center.y + outerX + extraY); // right down
    horiLine(image, color, {center.x - outerX,          center.y + posY + extraY},   center.x - innerX);          //       down
    vertLine(image, color, {center.x - posY,            center.y + innerX + extraY}, center.y + outerX + extraY); //       down
    
    horiLine(image, color, {center.x - outerX,          center.y - posY},   center.x - innerX);                   //
    vertLine(image, color, {center.x - posY,            center.y - outerX}, center.y - innerX);                   //
    horiLine(image, color, {center.x + innerX + extraX, center.y - posY},   center.x + outerX + extraX);          // right
    vertLine(image, color, {center.x + posY + extraX,   center.y - outerX}, center.y - innerX);                   // right
    
    posY++;
    
    if (outerErr < 0) {
      outerErr += 2 * posY + 1;
    } else {
      outerX--;
      outerErr += 2 * (posY - outerX) + 1;
    }
    
    if (posY > innerRadius) {
      innerX = posY;
    } else {
      if (innerErr < 0) {
        innerErr += 2 * posY + 1;
      } else {
        innerX--;
        innerErr += 2 * (posY - innerX) + 1;
      }
    }
  }
}

void midpointEllipse(
  Image &image,
  Color color,
  Point center,
  Point radius
) {
  Point pos = {radius.x, 0};
  Point delta = {
    2 * radius.y * radius.y * pos.x,
    2 * radius.x * radius.x * pos.y
  };
  int err = radius.x * radius.x
          - radius.y * radius.y * radius.x
          + (radius.y * radius.y) / 4;
  
  while (delta.y < delta.x) {
    setPixel(image, color, {center.x + pos.x, center.y + pos.y});
    setPixel(image, color, {center.x + pos.x, center.y - pos.y});
    setPixel(image, color, {center.x - pos.x, center.y + pos.y});
    setPixel(image, color, {center.x - pos.x, center.y - pos.y});
    
    pos.y++;
    
    if (err < 0) {
      delta.y += 2 * radius.x * radius.x;
      err += delta.y + radius.x * radius.x;
    } else {
      pos.x--;
      delta.y += 2 * radius.x * radius.x;
      delta.x -= 2 * radius.y * radius.y;
      err += delta.y - delta.x + radius.x * radius.x;
    }
  }
  
  err = radius.x * radius.x * (pos.y * pos.y + pos.y)
      + (radius.x * radius.x) / 4
      + radius.y * radius.y * (pos.x - 1) * (pos.x - 1)
      - radius.y * radius.y * radius.x * radius.x;
  
  while (pos.x >= 0) {
    setPixel(image, color, {center.x + pos.x, center.y + pos.y});
    setPixel(image, color, {center.x + pos.x, center.y - pos.y});
    setPixel(image, color, {center.x - pos.x, center.y + pos.y});
    setPixel(image, color, {center.x - pos.x, center.y - pos.y});
    
    pos.x--;
    
    if (err > 0) {
      delta.x -= 2 * radius.y * radius.y;
      err += radius.y * radius.y - delta.x;
    } else {
      pos.y++;
      delta.y += 2 * radius.x * radius.x;
      delta.x -= 2 * radius.y * radius.y;
      err += delta.y - delta.x + radius.y * radius.y;
    }
  }
}

void midpointEllipseThick(
  Image &image,
  Color color,
  Point center,
  Point innerRadius,
  Point outerRadius
) {
  
}

void midpointEllipseThick(
  Point center,
  Point innerRadius,
  Point outerRadius,
  std::function<void(const Color&, const Point&, int)> horiLine)
{
  /// @todo validate/correct innerRadius and outerRadius
  Point pos = { outerRadius.x, 0 };
  Point deltaOuter = {
    2 * outerRadius.y * outerRadius.y * pos.x,
    2 * outerRadius.x * outerRadius.x * pos.y
  };
  auto errOuterYX
    = [&]() {
      return outerRadius.x * outerRadius.x
        - outerRadius.y * outerRadius.y * outerRadius.x
        + (outerRadius.y * outerRadius.y) / 4;
    };
  auto errOuterXY
    = [&]() {
      return outerRadius.x * outerRadius.x * (pos.y * pos.y + pos.y)
        + (outerRadius.x * outerRadius.x) / 4
        + outerRadius.y * outerRadius.y * (pos.x - 1) * (pos.x - 1)
        - outerRadius.y * outerRadius.y * outerRadius.x * outerRadius.x;
    };
  int errOuter = errOuterYX();
  int xInner = innerRadius.x;
  Point deltaInner = {
    2 * innerRadius.y * innerRadius.y * xInner,
    2 * innerRadius.x * innerRadius.x * pos.y
  };
  auto errInnerYX
    = [&]() {
      return innerRadius.x * innerRadius.x
        - innerRadius.y * innerRadius.y * innerRadius.x
        + (innerRadius.y * innerRadius.y) / 4;
    };
  auto errInnerXY
    = [&]() {
      return innerRadius.x * innerRadius.x * (pos.y * pos.y + pos.y)
        + (innerRadius.x * innerRadius.x) / 4
        + innerRadius.y * innerRadius.y * (xInner - 1) * (xInner - 1)
        - innerRadius.y * innerRadius.y * innerRadius.x * innerRadius.x;
    };
  int errInner = errInnerYX();
  // helpers (to reduce code duplication)
  auto stepOuterYX
    = [&]() {
      ++pos.y;
      if (errOuter < 0) {
        deltaOuter.y += 2 * outerRadius.x * outerRadius.x;
        errOuter += deltaOuter.y + outerRadius.x * outerRadius.x;
      } else {
        --pos.x;
        deltaOuter.y += 2 * outerRadius.x * outerRadius.x;
        deltaOuter.x -= 2 * outerRadius.y * outerRadius.y;
        errOuter += deltaOuter.y - deltaOuter.x + outerRadius.x * outerRadius.x;
      }
    };
  auto stepOuterXY
    = [&]() {
      while (--pos.x > 0) {
        if (errOuter > 0) {
          deltaOuter.x -= 2 * outerRadius.y * outerRadius.y;
          errOuter += outerRadius.y * outerRadius.y - deltaOuter.x;
        } else {
          ++pos.y;
          deltaOuter.y += 2 * outerRadius.x * outerRadius.x;
          deltaOuter.x -= 2 * outerRadius.y * outerRadius.y;
          errOuter += deltaOuter.y - deltaOuter.x + outerRadius.y * outerRadius.y;
          break;
        }
      }
    };
  auto stepInnerYX
    = [&]() {
      if (errInner < 0) {
        deltaInner.y += 2 * innerRadius.x * innerRadius.x;
        errInner += deltaInner.y + innerRadius.x * innerRadius.x;
      } else {
        --xInner;
        deltaInner.y += 2 * innerRadius.x * innerRadius.x;
        deltaInner.x -= 2 * innerRadius.y * innerRadius.y;
        errInner += deltaInner.y - deltaInner.x + innerRadius.x * innerRadius.x;
      }
    };
  auto stepInnerXY
    = [&]() {
      while (--xInner >= 0) {
        if (errInner > 0) {
          deltaInner.x -= 2 * innerRadius.y * innerRadius.y;
          errInner += innerRadius.y * innerRadius.y - deltaInner.x;
        } else {
          deltaInner.y += 2 * innerRadius.x * innerRadius.x;
          deltaInner.x -= 2 * innerRadius.y * innerRadius.y;
          errInner += deltaInner.y - deltaInner.x + innerRadius.y * innerRadius.y;
          break;
        }
      }
    };
  // 1st phase
  while (deltaOuter.y < deltaOuter.x && deltaInner.y < deltaInner.x) {
    horiLine(Qt::blue, { center.x - pos.x, center.y + pos.y }, center.x - xInner);
    horiLine(Qt::blue, { center.x + pos.x, center.y + pos.y }, center.x + xInner); // hori swap
    horiLine(Qt::blue, { center.x - pos.x, center.y - pos.y }, center.x - xInner);
    horiLine(Qt::blue, { center.x + pos.x, center.y - pos.y }, center.x + xInner); // hori swap
    stepOuterYX();
    stepInnerYX();
  }

  // 2nd phase
  if (deltaOuter.y < deltaOuter.x) { // inner flipped
    //errOuter = errOuterYX();
    errInner = errInnerXY();
    while (deltaOuter.y < deltaOuter.x && xInner >= 0) {
      horiLine(Qt::green, { center.x - pos.x, center.y + pos.y }, center.x - xInner);
      horiLine(Qt::green, { center.x + pos.x, center.y + pos.y }, center.x + xInner); // hori swap
      horiLine(Qt::green, { center.x - pos.x, center.y - pos.y }, center.x - xInner);
      horiLine(Qt::green, { center.x + pos.x, center.y - pos.y }, center.x + xInner); // hori swap
      stepOuterYX();
      stepInnerXY();
    }
    //errOuter = errOuterYX();
    while (deltaOuter.y < deltaOuter.x) {
      horiLine(Qt::red, { center.x - pos.x, center.y + pos.y }, center.x + pos.x);
      horiLine(Qt::red, { center.x - pos.x, center.y - pos.y }, center.x + pos.x);
      stepOuterYX();
    }
  } else { // outer flipped
    errOuter = errOuterXY();
    //errInner = errInnerYX();
    while (deltaInner.y < deltaInner.x) {
      horiLine(Qt::cyan, { center.x - pos.x, center.y + pos.y }, center.x - xInner);
      horiLine(Qt::cyan, { center.x + pos.x, center.y + pos.y }, center.x + xInner);
      horiLine(Qt::cyan, { center.x - pos.x, center.y - pos.y }, center.x - xInner);
      horiLine(Qt::cyan, { center.x + pos.x, center.y - pos.y }, center.x + xInner);
      stepOuterXY();
      stepInnerYX();
    }
    //errOuter = errOuterXY();
  }
  // 3rd phase
  errOuter = errOuterXY();
  errInner = errInnerXY();
  while (xInner >= 0) {
    horiLine(Qt::yellow, { center.x - pos.x, center.y + pos.y }, center.x - xInner);
    horiLine(Qt::yellow, { center.x + pos.x, center.y + pos.y }, center.x + xInner); // hori swap
    horiLine(Qt::yellow, { center.x - pos.x, center.y - pos.y }, center.x - xInner);
    horiLine(Qt::yellow, { center.x + pos.x, center.y - pos.y }, center.x + xInner); // hori swap
    stepOuterXY();
    stepInnerXY();
  }
  // 4th phase
  //errOuter = errOuterXY();
  while (pos.x >= 0) {
    horiLine(Qt::magenta, { center.x - pos.x, center.y + pos.y }, center.x + pos.x);
    horiLine(Qt::magenta, { center.x - pos.x, center.y - pos.y }, center.x + pos.x);
    stepOuterXY();
  }
}

}

int centerWidth(const CircleShape shape) {
  return (shape == CircleShape::c2x1 || shape == CircleShape::c2x2) ? 2 : 1;
}

int centerHeight(const CircleShape shape) {
  return (shape == CircleShape::c1x2 || shape == CircleShape::c2x2) ? 2 : 1;
}

QRect circleToRect(const QPoint center, const int radius, const CircleShape shape) {
  return QRect{
    center.x() - radius,
    center.y() - radius,
    radius * 2 + centerWidth(shape),
    radius * 2 + centerHeight(shape)
  };
}

QRect adjustStrokedEllipse(const QRect rect, const int thickness) {
  return QRect{
    rect.left() + thickness / 2,
    rect.top() + thickness / 2,
    rect.width() - thickness,
    rect.height() - thickness
  };
}

const QPen round_pen{
  Qt::NoBrush, 1.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin
};
const QPen square_pen{
  Qt::NoBrush, 1.0, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin
};

QPen makePen(const QPen &base, const QRgb color, const int width) {
  QPen pen = base;
  pen.setColor(QColor::fromRgba(color));
  pen.setWidth(width);
  return pen;
}

void preparePainter(QPainter &painter) {
  painter.setCompositionMode(QPainter::CompositionMode_Source);
}

bool drawFilledEllipse(QImage &img, const QRgb color, const QRect ellipse) {
  assert(ellipse.isValid());
  if (!img.rect().intersects(ellipse)) return false;
  QPainter painter{&img};
  preparePainter(painter);
  painter.setBrush(QColor::fromRgba(color));
  painter.setPen(makePen(round_pen, color, 1));
  painter.drawEllipse(adjustStrokedEllipse(ellipse, 1));
  return true;
}

bool drawStrokedEllipse(QImage &img, const QRgb color, const QRect ellipse) {
  assert(ellipse.isValid());
  if (!img.rect().intersects(ellipse)) return false;
  QPainter painter{&img};
  preparePainter(painter);
  painter.setPen(makePen(round_pen, color, 1));
  painter.drawEllipse(adjustStrokedEllipse(ellipse, 1));
  return true;
}

#include "masking.hpp"
#include "geometry.hpp"
#include "surface factory.hpp"

void blitMaskImageNew(QImage &dst, const QImage &mask, const QImage &src, const QPoint pos) {
  assert(mask.size() == src.size());
  maskCopyRegion(
    makeSurface<QRgb>(dst),
    makeSurface<QRgb>(src),
    makeSurface<uint8_t>(mask),
    pos,
    pos
  );
}

void blitMaskImageOld(QImage &dst, const QImage &mask, const QImage &src, const QPoint pos) {
  assert(mask.size() == src.size());
  assert(mask.format() == mask_format);
  const QRect rect = mask.rect().intersected(dst.rect().translated(-pos));
  for (int y = rect.top(); y <= rect.bottom(); ++y) {
    for (int x = rect.left(); x <= rect.right(); ++x) {
      if (mask.pixel(x, y) == mask_color_on) {
        dst.setPixel(x + pos.x(), y + pos.y(), src.pixel(x, y));
      }
    }
  }
}

void blitImageNew(QImage &dst, const QImage &src, const QPoint pos) {
  copyRegion(
    makeSurface<QRgb>(dst),
    makeSurface<QRgb>(src),
    pos
  );
}

void blitImageOld(QImage &dst, const QImage &src, const QPoint pos) {
  QPainter painter{&dst};
  painter.setCompositionMode(QPainter::CompositionMode_Source);
  painter.drawImage(pos, src);
}

#endif

#if BUG_TEST

#include <QtWidgets/qdockwidget.h>
#include <QtWidgets/qmainwindow.h>
#include <QtWidgets/qapplication.h>

void addDock(QMainWindow *window, Qt::DockWidgetArea area, QWidget *widget) {
  QDockWidget *dock = new QDockWidget{window};
  dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
  dock->setAllowedAreas(area);
  dock->setWidget(widget);
  dock->setTitleBarWidget(new QWidget{dock});
  window->addDockWidget(area, dock);
}

int main(int argc, char **argv) {
  QApplication app{argc, argv};
  QMainWindow window;
  
  window.setStyleSheet(R"(
    QMainWindow::separator {
      width: 10px;
      height: 10px;
      background-color: #F00;
    }
  )");
  
  QWidget bottom{&window};
  bottom.setMinimumHeight(100);
  bottom.setStyleSheet("background-color: #0F0");
  addDock(&window, Qt::BottomDockWidgetArea, &bottom);
  
  QWidget left{&window};
  left.setFixedWidth(100);
  left.setStyleSheet("background-color: #00F");
  addDock(&window, Qt::LeftDockWidgetArea, &left);
  
  QWidget center{&window};
  center.setMinimumSize(300, 300);
  center.setStyleSheet("background-color: #0FF");
  window.setCentralWidget(&center);
  
  window.show();
  
  return app.exec();
}

#endif

#if !BUG_TEST

int main(int argc, char **argv) {
  /*Image img;
  img.data.load("/Users/indikernick/Library/Developer/Xcode/DerivedData/Pixel_2-gqoblrlhvynmicgniivandqktune/Build/Products/Debug/Pixel 2.app/Contents/Resources/icon.png");
  img.xform.angle = 1;
  img.xform.posX = 3;
  img.xform.posY = 5;
  img.xform.flipX = true;
  QImage xformed = img.transformed();
  xformed.save("/Users/indikernick/Desktop/Test/test.png");*/
  
  /*QImage idxImg{2, 2, QImage::Format_Indexed8};
  idxImg.detach();
  idxImg.bits()[0] = 0;
  idxImg.bits()[1] = 20;
  idxImg.bits()[4] = 40;
  idxImg.bits()[5] = 60;
  idxImg.reinterpretAsFormat(QImage::Format_Grayscale8);
  idxImg.save("/Users/indikernick/Desktop/Test/idx_test.png");
  
  QImage loaded("/Users/indikernick/Desktop/idx_test.png");
  std::cout << (loaded.format() == QImage::Format_Grayscale8) << '\n';
  std::cout << (loaded.format() == QImage::Format_Indexed8) << '\n';
  std::cout << (loaded.format() == QImage::Format_Alpha8) << '\n';
  std::cout << loaded.format() << '\n';
  std::cout << '\n';
  
  QImage colImg(1, 1, QImage::Format_ARGB32);
  colImg.detach();
  colImg.bits()[0] = 63;
  colImg.bits()[1] = 127;
  colImg.bits()[2] = 0;
  colImg.bits()[3] = 0;
  colImg.save("/Users/indikernick/Desktop/Test/col_test.png");
  
  const int multiplied = ((127 * 63) / 255);
  std::cout << multiplied << '\n';
  std::cout << ((multiplied * 255) / 63) << '\n';
  std::cout << '\n';
  
  QImage loadedCol("/Users/indikernick/Desktop/Test/col_test.png");
  std::cout << (loadedCol.format() == QImage::Format_ARGB32) << '\n';
  std::cout << (loadedCol.format() == QImage::Format_ARGB32_Premultiplied) << '\n';
  std::cout << loadedCol.format() << '\n';
  std::cout << int(loadedCol.constBits()[0]) << ' '
            << int(loadedCol.constBits()[1]) << ' '
            << int(loadedCol.constBits()[2]) << ' '
            << int(loadedCol.constBits()[3]) << '\n';
  
  std::cout << "\n\n";*/
  
  Timer timer;
  // 128
  // painter filled   0.0975ms
  // midpoint filled  0.0121ms
  // 2048
  // painter filled   4.31ms
  // midpoint filled  1.77ms
  // 4096
  // painter filled   17.7ms
  // midpoint filled  7.59ms
  timer.start("Alloc");
  QImage image{2048, 2048, QImage::Format_ARGB32};
  timer.stop();
  
  timer.start("Clear");
  image.fill(0);
  timer.stop();
  
  timer.start("Memset");
  image.detach();
  std::memset(image.bits(), 0, image.sizeInBytes());
  timer.stop();
  
  timer.start("Loop clear");
  clearImage(image, 0x11223344);
  timer.stop();
  
  timer.start("Floodfill");
  drawFloodFill(image, QRgb{0xFFFFFFFF}, {image.width() / 2, image.height() / 2});
  timer.stop();
  
  timer.start("Refer");
  QImage copy = image;
  timer.stop();
  
  timer.start("Copy");
  copy.detach();
  timer.stop();
  
  timer.start("Make");
  QImage compat = makeCompatible(image);
  timer.stop();
  
  timer.start("Detach unique");
  compat.detach();
  timer.stop();
  
  timer.start("Memcpy");
  std::memcpy(compat.bits(), image.constBits(), image.sizeInBytes());
  timer.stop();
  
  timer.start("Memcpy again");
  std::memcpy(compat.bits(), image.constBits(), image.sizeInBytes());
  timer.stop();
  
  timer.start("Duplicate");
  QImage dup = dupImage(image);
  timer.stop();
  
  const QRgb fillColor = qRgba(0, 0, 255, 255);
  const QRect fillRect{
    image.width() / 8,
    image.height() / 7,
    (image.width() * 2) / 3,
    (image.height() * 3) / 4
  };
  
  image.detach();
  dup.detach();
  
  timer.start("drawFilledRect");
  drawFilledRect(image, fillColor, fillRect);
  timer.stop();
  
  image.fill(0);
  
  timer.start("painter.fillRect");
  {
    QPainter painter{&image};
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(fillRect, QColor{qRed(fillColor), qGreen(fillColor), qBlue(fillColor), qAlpha(fillColor)});
  }
  timer.stop();
  
  clearImage(image);
  
  for (int r = 0; r <= 24; ++r) {
    auto horiLine = [&](test::Color color, test::Point first, int last) {
      if (last < first.x) {
       
      }
      QPainter painter{&image};
      painter.setPen(color);
      painter.setBrush(Qt::NoBrush);
      painter.drawLine(first.x, first.y, last, first.y);
    };
    
    const int x = 64 * (r + 1);
    const int rx = 4 * r / 3;
    const int ry = r;
    drawStrokedEllipse(image, fillColor, circleToRect({x, 64 * 1}, r, CircleShape::c1x1));
    bresenhamStroked(image, fillColor, {x, 64 * 2}, r, CircleShape::c1x1);
    test::midpointCircle(image, fillColor, {x, 64 * 3}, r, CircleShape::c1x1);
    test::midpointCircleThick(image, fillColor, {x, 64 * 4}, std::max(r - 3 + 1, 0), r, CircleShape::c1x1);
    midpointEllipse(image, fillColor, {x, 64 * 5}, {r, r});
    test::midpointEllipse(image, fillColor, {x, 64 * 6}, {r, r});
    test::midpointEllipseThick({x, 64 * 7}, {rx, ry}, {rx, ry}, horiLine);
    const QRect ellipseRect = {
      x - rx,
      64 * 8 - ry,
      rx * 2 + 1,
      ry * 2 + 1
    };
    drawStrokedEllipse(image, fillColor, ellipseRect);
    midpointEllipse(image, fillColor, {x, 64 * 9}, {rx, ry});
    test::midpointEllipseThick({x, 64 * 10}, {std::max(rx - 3 + 1, 0), std::max(ry - 3 + 1, 0)}, {rx, ry}, horiLine);
    midpointEllipseFilled(image, fillColor, {x, 64 * 11}, {rx, ry});
    midpointFilled(image, fillColor, {x, 64 * 12}, r, CircleShape::c1x1);
    drawFilledEllipse(image, fillColor, circleToRect({x, 64 * 13}, r, CircleShape::c1x1));
  }
  
  const QPoint circPos = {dup.width() / 2, dup.height() / 2};
  const int circRad = std::min(dup.width(), dup.height()) / 2 - 1;
  
  timer.start("painter stroked");
  drawStrokedEllipse(dup, fillColor, circleToRect(circPos, circRad, CircleShape::c1x1));
  timer.stop();
  
  dup.fill(0);
  
  timer.start("bres stroked");
  bresenhamStroked(dup, fillColor, circPos, circRad, CircleShape::c1x1);
  timer.stop();
  
  dup.fill(0);
  
  timer.start("midpoint stroked");
  midpointStroked(dup, fillColor, circPos, circRad, CircleShape::c1x1);
  timer.stop();
  
  dup.fill(0);
  
  timer.start("painter filled");
  drawFilledEllipse(dup, fillColor, circleToRect(circPos, circRad, CircleShape::c1x1));
  timer.stop();
  
  dup.fill(0);
  
  timer.start("midpoint filled");
  drawFilledCircle(dup, fillColor, circPos, circRad);
  timer.stop();
  
  dup.save("/Users/indikernick/Desktop/Test/circle.png");
  image.save("/Users/indikernick/Desktop/Test/circles.png");
  
  dup.fill(0);
  image.fill(0);
  
  timer.start("painter brush");
  {
    QPen pen{Qt::NoBrush, 64.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin};
    pen.setColor(QColor{fillColor});
    QPainter painter{&image};
    painter.setPen(pen);
    painter.drawLine(64, 64, image.width() - 65, image.height() - 65);
  }
  timer.stop();
  
  image.save("/Users/indikernick/Desktop/Test/smear_0.png");
  image.fill(0);
  
  timer.start("filled smear");
  for (int x = 64; x < image.width() - 64; ++x) {
    midpointFilled(image, fillColor, {x, x}, 32, CircleShape::c1x1);
  }
  timer.stop();
  
  image.save("/Users/indikernick/Desktop/Test/smear_1.png");
  image.fill(0);
  
  timer.start("1 stroked smear");
  for (int x = 64; x < image.width() - 64; ++x) {
    drawStrokedCircle(image, fillColor, {x, x}, 32, 1);
  }
  timer.stop();
  
  image.save("/Users/indikernick/Desktop/Test/smear_2.png");
  image.fill(0);
  
  timer.start("2 stroked smear");
  for (int x = 64; x < image.width() - 64; ++x) {
    drawStrokedCircle(image, fillColor, {x, x}, 32, 2);
  }
  timer.stop();
  
  image.save("/Users/indikernick/Desktop/Test/smear_3.png");
  image.fill(0);
  
  timer.start("painter smear");
  for (int x = 64; x < image.width() - 64; ++x) {
    drawFilledEllipse(image, fillColor, circleToRect({x, x}, 32, CircleShape::c1x1));
  }
  timer.stop();
  
  image.save("/Users/indikernick/Desktop/Test/smear_4.png");
  image.fill(0);
  
  QImage mask{image.size() / 2, mask_format};
  makeSurface<uint8_t>(mask).fillRect(0xFF, {toPoint(image.size() / 8), image.size() / 4});
  QImage sauce{image.size() / 2, image.format()};
  
  timer.start("new blit mask");
  blitMaskImageNew(image, mask, sauce, toPoint(image.size() / 4));
  timer.stop();
  
  timer.start("old blit mask");
  blitMaskImageOld(image, mask, sauce, toPoint(image.size() / 4));
  timer.stop();
  
  timer.start("new blit mask");
  blitMaskImageNew(image, mask, sauce, toPoint(image.size() / 4));
  timer.stop();
  
  timer.start("new blit");
  blitImageNew(image, sauce, toPoint(image.size() / 4));
  timer.stop();
  
  timer.start("old blit");
  blitImageOld(image, sauce, toPoint(image.size() / 4));
  timer.stop();
  
  timer.start("painter source-over");
  {
    QPainter painter{&image};
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawImage(0, 0, dup);
  }
  timer.stop();
  
  timer.start("surface source-over");
  porterDuff<ARGB_Format>(mode_src_over, makeSurface<QRgb>(image), makeCSurface<QRgb>(dup));
  timer.stop();
  /*
  midpointLine(image, fillColor, {10, 10}, {20, 10});
  midpointLine(image, fillColor, {20, 20}, {10, 20});
  midpointLine(image, fillColor, {10, 30}, {10, 40});
  midpointLine(image, fillColor, {10, 60}, {10, 50});
  midpointLine(image, fillColor, {10, 70}, {20, 75});
  midpointLine(image, fillColor, {10, 90}, {20, 85});
  midpointLine(image, fillColor, {10, 100}, {15, 110});
  midpointLine(image, fillColor, {10, 130}, {15, 120});
  midpointLine(image, fillColor, {10, 140}, {20, 150});
  midpointLine(image, fillColor, {10, 170}, {20, 160});
  
  image.save("/Users/indikernick/Desktop/Test/lines.png");
  image.fill(0);*/
  
  //testComposite();
  
  /*SourceCell source({32, 32}, Format::color);
  source.image.xform.angle = 0;
  QImage overlay({32, 32}, getImageFormat(Format::color));
  overlay.fill(0);
  
  ToolMouseEvent event;
  event.button = ButtonType::primary;
  event.pos = QPoint{16, 16};
  event.colors.primary = qRgba(0, 255, 0, 255);
  event.colors.erase = qRgba(0, 0, 0, 0);
  event.overlay = &overlay;
  
  ToolKeyEvent keyEvent;
  keyEvent.colors = event.colors;
  keyEvent.overlay = event.overlay;
  
  FilledCircleTool sct;
  sct.attachCell(&source);
  event.pos.rx() += 3;
  event.pos.ry() += 2;
  sct.mouseDown(event);
  event.pos.rx() -= 3;
  event.pos.ry() -= 2;
  sct.mouseUp(event);
  sct.detachCell();
  
  RotateTool tool;
  //tool.setMode(SymmetryMode::both);
  //tool.setWidth(2);
  //tool.setShape(CircleShape::c1x1);
  [[maybe_unused]] const bool ok = tool.attachCell(&source);
  assert(ok);
  
  event.colors.primary = qRgba(191, 63, 127, 191);
  
  timer.start("MouseDown");
  tool.mouseDown(event);
  timer.stop();
  QImage drawing = compositeFrame(Palette{}, Frame{&source});
  compositeOverlay(drawing, overlay);
  drawing.save("/Users/indikernick/Desktop/Test/overlay_0.png");
  
  keyEvent.key = Qt::Key_Down;
  tool.keyPress(keyEvent);
  
  event.pos = QPoint{24, 20};
  timer.start("MouseMove");
  tool.mouseMove(event);
  timer.stop();
  drawing = compositeFrame(Palette{}, Frame{&source});
  compositeOverlay(drawing, overlay);
  drawing.save("/Users/indikernick/Desktop/Test/overlay_1.png");
  
  keyEvent.key = Qt::Key_Left;
  tool.keyPress(keyEvent);
  
  event.pos = QPoint{20, 20};
  tool.mouseMove(event);
  drawing = compositeFrame(Palette{}, Frame{&source});
  compositeOverlay(drawing, overlay);
  drawing.save("/Users/indikernick/Desktop/Test/overlay_2.png");
  
  tool.detachCell();
  
  event.pos = QPoint{20, 20};
  timer.start("MouseMove");
  tool.mouseUp(event);
  timer.stop();
  drawing = compositeFrame(Palette{}, Frame{&source});
  compositeOverlay(drawing, overlay);
  drawing.save("/Users/indikernick/Desktop/Test/overlay_3.png");
  
  //tool.setMode(SelectMode::paste);
  tool.mouseMove(event);
  
  tool.mouseMove(event);
  drawing = compositeFrame(Palette{}, Frame{&source});
  compositeOverlay(drawing, overlay);
  drawing.save("/Users/indikernick/Desktop/Test/overlay_4.png");
  
  event.pos = QPoint{16, 16};
  timer.start("MouseUp");
  tool.mouseMove(event);
  timer.stop();
  drawing = compositeFrame(Palette{}, Frame{&source});
  compositeOverlay(drawing, overlay);
  drawing.save("/Users/indikernick/Desktop/Test/overlay_5.png");
  
  tool.mouseDown(event);
  drawing = compositeFrame(Palette{}, Frame{&source});
  compositeOverlay(drawing, overlay);
  drawing.save("/Users/indikernick/Desktop/Test/overlay_6.png");
  
  tool.mouseUp(event);
  
  drawing = compositeFrame(Palette{}, Frame{&source});
  compositeOverlay(drawing, overlay);
  drawing.save("/Users/indikernick/Desktop/Test/overlay_7.png");
  
  source.image.data.save("/Users/indikernick/Desktop/Test/brush.png");*/
  
  /*QFile file{"/Users/indikernick/Desktop/project.px2"};
  
  file.open(QIODevice::WriteOnly | QIODevice::Truncate);
  Animation anim({1, 1}, Format::color);
  anim.appendTransform(0);
  anim.appendLayer();
  anim.appendDuplicate(1);
  anim.appendDuplicate(1);
  anim.appendLayer();
  anim.appendSource(2);
  auto *src = getCell<SourceCell>(anim, 2, 0);
  src->image.xform.posX = 123;
  src->image.xform.posY = 456;
  src->image.xform.angle = 1;
  src->image.xform.flipY = true;
  src->image.data.bits()[0] = 191;
  src->image.data.bits()[1] = 160;
  src->image.data.bits()[2] = 63;
  src->image.data.bits()[3] = 2;
  
  anim.serialize(&file);
  file.close();
  
  file.open(QIODevice::ReadOnly);
  Animation newAnim{&file};
  file.close();
  
  assert(newAnim.size.width() == anim.size.width());
  assert(newAnim.size.height() == anim.size.height());
  assert(newAnim.format == anim.format);
  assert(newAnim.layers.size() == anim.layers.size());
  for (LayerIdx l = 0; l != newAnim.layers.size(); ++l) {
    assert(newAnim.layers[l].size() == anim.layers[l].size());
  }
  if (newAnim.format == Format::palette) {
    assert(newAnim.palette.size() == anim.palette.size());
    assert(newAnim.palette == anim.palette);
  }
  
  auto *trans_0_0 = getCell<TransformCell>(newAnim, 0, 0);
  auto *dup_1_0 = getCell<DuplicateCell>(newAnim, 1, 0);
  auto *dup_1_1 = getCell<DuplicateCell>(newAnim, 1, 1);
  auto *src_2_0 = getCell<SourceCell>(newAnim, 2, 0);
  assert(src_2_0->image.xform.posX == src->image.xform.posX);
  assert(src_2_0->image.xform.posY == src->image.xform.posY);
  assert(src_2_0->image.xform.angle == src->image.xform.angle);
  assert(src_2_0->image.xform.flipX == src->image.xform.flipX);
  assert(src_2_0->image.xform.flipY == src->image.xform.flipY);
  assert(src_2_0->image.data.bits()[0] == src->image.data.bits()[0]);
  assert(src_2_0->image.data.bits()[1] == src->image.data.bits()[1]);
  assert(src_2_0->image.data.bits()[2] == src->image.data.bits()[2]);
  assert(src_2_0->image.data.bits()[3] == src->image.data.bits()[3]);
  */
  
  Application app{argc, argv};
  return app.exec();
}

#endif
