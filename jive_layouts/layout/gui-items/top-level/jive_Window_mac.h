#pragma once

#include <optional>

namespace jive
{
    enum class Backdrop;
}

namespace jive::macos
{
    void setBackdrop(void* nativeHandle, std::optional<Backdrop> backdrop);
    void setIntegratedTitleBar(void* nativeHandle, bool integrated);
    void layoutTitleBarButtons(void* nativeHandle, float titleBarHeight);
    void performWindowDrag(void* nativeHandle);
} // namespace jive::macos
