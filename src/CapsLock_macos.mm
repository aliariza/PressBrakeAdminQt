// src/CapsLock_macos.mm
// macOS-only helper for Caps Lock state

#import <AppKit/AppKit.h>

extern "C" bool pb_capslock_on() {
  // Cocoa APIs are safest on the main thread
  if (![NSThread isMainThread]) return false;

  NSEventModifierFlags flags = 0;
  @try {
    flags = [NSEvent modifierFlags];
  } @catch (...) {
    return false;
  }
  return (flags & NSEventModifierFlagCapsLock) != 0;
}
