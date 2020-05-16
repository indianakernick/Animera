//
//  cursor.mm
//  Animera
//
//  Created by Indiana Kernick on 2/10/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "native mac.hpp"

#import <AppKit/NSCursor.h>

void hideMouseUntilMouseMoves() {
  [NSCursor setHiddenUntilMouseMoves:true];
}

// This is only really useful for debugging
void systemLog(const char *message) {
  NSLog(@"%@", [NSString stringWithUTF8String:message]);
}
