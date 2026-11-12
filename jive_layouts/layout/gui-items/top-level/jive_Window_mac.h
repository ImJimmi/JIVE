#pragma once

namespace jive::macos
{
    void setIntegratedTitleBar(void* nativeHandle, bool integrated);
    void layoutTitleBarButtons(void* nativeHandle, float titleBarHeight);
    void performWindowDrag(void* nativeHandle);
} // namespace jive::macos
