//
//  main.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 3/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

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

struct Color {
  uint8_t r, g, b, a;
};

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

#include <QtCore/qtimer.h>

class StatusBar : public QWidget {
public:
  explicit StatusBar(QWidget *parent)
    : QWidget{parent}, label{this} {
    timer.setInterval(5000);
    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, this, &StatusBar::hideTemp);
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
    connect(open, &QAction::triggered, this, &Window::openDoc);
    connect(save, &QAction::triggered, this, &Window::saveDoc);
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
};

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

#include "paint tool impls.hpp"
#include "composite.hpp"
#include "painting.hpp"

void freshFilledCircle(
  QImage &image,
  const QRgb color,
  const QPoint pos,
  const int radius,
  const CircleShape shape
) {
  const QRect rect = circleToRect(pos, radius, shape);
  const int radius2 = radius * radius;
  for (int y = rect.top(); y <= rect.bottom(); ++y) {
    for (int x = rect.left(); x <= rect.right(); ++x) {
      const int dx = std::abs(x - pos.x());
      const int dy = std::abs(y - pos.y());
      if (dx*dx + dy*dy < radius2 + radius) {
        image.setPixel(x, y, color);
      }
    }
  }
}

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
) {
  QPoint pos = {rad, 0};
  img.setPixel(ctr.x() + pos.x(), ctr.y() + pos.y(), col);
  if (rad > 0) {
    img.setPixel(ctr.x() - pos.x(), ctr.y() + pos.y(), col);
    img.setPixel(ctr.x() + pos.y(), ctr.y() + pos.x(), col);
    img.setPixel(ctr.x() + pos.y(), ctr.y() - pos.x(), col);
  }
  int mid = 1 - rad;
  while (pos.x() > pos.y()) {
    ++pos.ry();
    if (mid < 0) {
      mid = mid + 2 * pos.y() + 1;
    } else {
      --pos.rx();
      mid = mid + 2 * pos.y() - 2 * pos.x() + 1;
    }
    if (pos.x() < pos.y()) break;
    img.setPixel(ctr.x() + pos.x(), ctr.y() + pos.y(), col);
    img.setPixel(ctr.x() - pos.x(), ctr.y() + pos.y(), col);
    img.setPixel(ctr.x() + pos.x(), ctr.y() - pos.y(), col);
    img.setPixel(ctr.x() - pos.x(), ctr.y() - pos.y(), col);
    moreGreen(col);
    if (pos.x() != pos.y()) {
      img.setPixel(ctr.x() + pos.y(), ctr.y() + pos.x(), col);
      img.setPixel(ctr.x() - pos.y(), ctr.y() + pos.x(), col);
      img.setPixel(ctr.x() + pos.y(), ctr.y() - pos.x(), col);
      img.setPixel(ctr.x() - pos.y(), ctr.y() - pos.x(), col);
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
) {
  QPoint pos = {rad, 0};
  img.setPixel(ctr.x() + pos.x(), ctr.y() + pos.y(), col);
  if (rad > 0) {
    fillScanLine(img, col, {ctr.x() - pos.x(), ctr.y() + pos.y()}, ctr.x() + pos.x());
    img.setPixel(ctr.x() + pos.y(), ctr.y() + pos.x(), col);
    img.setPixel(ctr.x() + pos.y(), ctr.y() - pos.x(), col);
  }
  int mid = 1 - rad;
  while (pos.x() > pos.y()) {
    ++pos.ry();
    if (mid < 0) {
      mid = mid + 2 * pos.y() + 1;
    } else {
      --pos.rx();
      mid = mid + 2 * pos.y() - 2 * pos.x() + 1;
    }
    if (pos.x() < pos.y()) break;
    fillScanLine(img, col, {ctr.x() - pos.x(), ctr.y() + pos.y()}, ctr.x() + pos.x());
    fillScanLine(img, col, {ctr.x() - pos.x(), ctr.y() - pos.y()}, ctr.x() + pos.x());
    if (pos.x() != pos.y()) {
      fillScanLine(img, col, {ctr.x() - pos.y(), ctr.y() + pos.x()}, ctr.x() + pos.y());
      fillScanLine(img, col, {ctr.x() - pos.y(), ctr.y() - pos.x()}, ctr.x() + pos.y());
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

int main(int argc, char **argv) {
  /*Image img;
  img.data.load("/Users/indikernick/Library/Developer/Xcode/DerivedData/Pixel_2-gqoblrlhvynmicgniivandqktune/Build/Products/Debug/Pixel 2.app/Contents/Resources/icon.png");
  img.xform.angle = 1;
  img.xform.posX = 3;
  img.xform.posY = 5;
  img.xform.flipX = true;
  QImage xformed = img.transformed();
  xformed.save("/Users/indikernick/Desktop/test.png");*/
  
  /*QImage idxImg{2, 2, QImage::Format_Indexed8};
  idxImg.detach();
  idxImg.bits()[0] = 0;
  idxImg.bits()[1] = 20;
  idxImg.bits()[4] = 40;
  idxImg.bits()[5] = 60;
  idxImg.reinterpretAsFormat(QImage::Format_Grayscale8);
  idxImg.save("/Users/indikernick/Desktop/idx_test.png");
  
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
  colImg.save("/Users/indikernick/Desktop/col_test.png");
  
  const int multiplied = ((127 * 63) / 255);
  std::cout << multiplied << '\n';
  std::cout << ((multiplied * 255) / 63) << '\n';
  std::cout << '\n';
  
  QImage loadedCol("/Users/indikernick/Desktop/col_test.png");
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
  
  timer.start("Floodfill");
  drawFloodFill(image, QRgb{0xFFFFFFFF}, {image.width() / 2, image.height() / 2});
  timer.stop();
  
  timer.start("Refer");
  QImage copy = image;
  timer.stop();
  
  timer.start("Copy");
  copy.detach();
  timer.stop();
  
  timer.start("Memcpy");
  std::memcpy(copy.bits(), image.constBits(), image.sizeInBytes());
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
    const int x = 64 * (r + 1);
    drawStrokedEllipse(image, fillColor, circleToRect({x, 64}, r, CircleShape::c1x1));
    bresenhamStroked(image, fillColor, {x, 128}, r, CircleShape::c1x1);
    midpointStroked(image, fillColor, {x, 192}, r, CircleShape::c1x1);
    midpointFilled(image, fillColor, {x, 256}, r, CircleShape::c1x1);
    drawFilledEllipse(image, fillColor, circleToRect({x, 320}, r, CircleShape::c1x1));
  }
  
  const QPoint circPos = {dup.width() / 2, dup.height() / 2};
  const int circRad = std::min(dup.width(), dup.height()) / 2 - 1;
  
  timer.start("painter stroked");
  drawStrokedEllipse(dup, fillColor, circleToRect(circPos, circRad, CircleShape::c1x1));
  timer.stop();
  
  timer.start("bres stroked");
  bresenhamStroked(dup, fillColor, circPos, circRad, CircleShape::c1x1);
  timer.stop();
  
  timer.start("midpoint stroked");
  midpointStroked(dup, fillColor, circPos, circRad, CircleShape::c1x1);
  timer.stop();
  
  timer.start("painter filled");
  drawFilledEllipse(dup, fillColor, circleToRect(circPos, circRad, CircleShape::c1x1));
  timer.stop();
  
  dup.fill(0);
  
  timer.start("midpoint filled");
  midpointFilled(dup, fillColor, circPos, circRad, CircleShape::c1x1);
  timer.stop();
  
  dup.save("/Users/indikernick/Desktop/circle.png");
  image.save("/Users/indikernick/Desktop/circles.png");
  
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
  
  image.save("/Users/indikernick/Desktop/smear_0.png");
  image.fill(0);
  
  timer.start("midpoint smear");
  for (int x = 64; x < image.width() - 64; ++x) {
    midpointFilled(image, fillColor, {x, x}, 32, CircleShape::c1x1);
  }
  timer.stop();
  
  image.save("/Users/indikernick/Desktop/smear_1.png");
  image.fill(0);
  
  timer.start("painter smear");
  for (int x = 64; x < image.width() - 64; ++x) {
    drawFilledEllipse(image, fillColor, circleToRect({x, x}, 32, CircleShape::c1x1));
  }
  timer.stop();
  
  image.save("/Users/indikernick/Desktop/smear_2.png");
  image.fill(0);
  
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
  
  image.save("/Users/indikernick/Desktop/lines.png");
  image.fill(0);
  
  //testComposite();
  
  SourceCell source({32, 32}, Format::color);
  source.image.xform.angle = 0;
  LineTool tool;
  //tool.setWidth(3);
  //tool.setShape(CircleShape::c1x1);
  [[maybe_unused]] const bool ok = tool.attachCell(&source);
  assert(ok);
  QImage overlay({32, 32}, getImageFormat(Format::color));
  overlay.fill(0);
  
  ToolEvent event;
  event.type = ButtonType::primary;
  event.pos = QPoint{16, 16};
  event.colors.primary = qRgba(0, 255, 0, 255);
  event.overlay = &overlay;
  
  /*StrokedCircleTool sct;
  sct.attachCell(&source);
  event.pos.rx() += 3;
  sct.mouseDown(event);
  event.pos.rx() -= 6;
  sct.mouseUp(event);
  event.pos.rx() += 2;
  event.pos.ry() -= 2;*/
  
  event.colors.primary = qRgba(191, 63, 127, 191);
  
  timer.start("MouseDown");
  tool.mouseDown(event);
  timer.stop();
  QImage drawing = source.image.data;
  compositeOverlay(drawing, overlay);
  drawing.save("/Users/indikernick/Desktop/overlay_0.png");
  
  event.pos = QPoint{24, 20};
  timer.start("MouseMove");
  tool.mouseMove(event);
  timer.stop();
  drawing = source.image.data;
  compositeOverlay(drawing, overlay);
  drawing.save("/Users/indikernick/Desktop/overlay_1.png");
  
  event.pos = QPoint{12, 24};
  timer.start("MouseMove");
  tool.mouseMove(event);
  timer.stop();
  drawing = source.image.data;
  compositeOverlay(drawing, overlay);
  drawing.save("/Users/indikernick/Desktop/overlay_2.png");
  
  event.pos = QPoint{16, 16};
  timer.start("MouseUp");
  tool.mouseUp(event);
  timer.stop();
  drawing = source.image.data;
  compositeOverlay(drawing, overlay);
  drawing.save("/Users/indikernick/Desktop/overlay_3.png");
  
  source.image.data.save("/Users/indikernick/Desktop/brush.png");
  
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
  return 0;
  
  Application app{argc, argv};
  return app.exec();
}
