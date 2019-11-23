//
//  export pattern.hpp
//  Animera
//
//  Created by Indi Kernick on 26/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef export_pattern_hpp
#define export_pattern_hpp

#include "cell.hpp"
#include <QtCore/qstring.h>
#include <QtGui/qvalidator.h>

/*
TODO: do we need more parameters for name patterns?
an object with setters to set parameters
then a function to parse the info
maybe this object should have functions for validation and fixing that are called by the validator
layer name could be useful as well as the layer index
*/

QString evalExportPattern(const QString &, LayerIdx, FrameIdx);

class ExportPatternValidator final : public QValidator {
public:
  explicit ExportPatternValidator(QObject *);
  
  QString defaultText() const;
  
  State validate(QString &, int &) const override;
  void fixup(QString &) const override;
};

#endif
