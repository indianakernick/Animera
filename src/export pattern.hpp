//
//  export pattern.hpp
//  Animera
//
//  Created by Indiana Kernick on 26/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_export_pattern_hpp
#define animera_export_pattern_hpp

#include "cel.hpp"
#include <QtCore/qstring.h>
#include <QtGui/qvalidator.h>
#include "export options.hpp"

QString evalExportPattern(const QString &, ExportState);
Error checkExportPattern(const QString &);

class ExportPatternValidator final : public QValidator {
public:
  explicit ExportPatternValidator(QObject *);
  
  QString defaultText() const;
  
  State validate(QString &, int &) const override;
  void fixup(QString &) const override;
};

#endif
