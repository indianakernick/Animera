//
//  export pattern.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 26/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef export_pattern_hpp
#define export_pattern_hpp

#include "cell.hpp"
#include <QtCore/qstring.h>
#include <QtGui/qvalidator.h>

QString evalExportPattern(const QString &, LayerIdx, FrameIdx);

class ExportPatternValidator final : public QValidator {
public:
  explicit ExportPatternValidator(QObject *);
  
  QString defaultText() const;
  
  State validate(QString &, int &) const override;
  void fixup(QString &) const override;
};

#endif
