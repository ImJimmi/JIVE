#include <TargetConditionals.h>

#if TARGET_OS_OSX
#include "jive_Window_mac.h"

#import <AppKit/AppKit.h>

#include <jive_style_sheets/graphics/jive_Fill.h>

#include <initializer_list>

namespace jive::macos
{
    [[nodiscard]] static NSWindow* getWindow(void* nativeHandle)
    {
        if (nativeHandle == nullptr)
            return nil;

        return [static_cast<NSView*>(nativeHandle) window];
    }

    void setIntegratedTitleBar(void* nativeHandle, bool integrated)
    {
        auto* const window = getWindow(nativeHandle);

        if (window == nil)
            return;

        const auto isIntegrated = ([window styleMask] & NSWindowStyleMaskFullSizeContentView) != 0;

        if (integrated == isIntegrated)
            return;

        const auto contentArea = [window contentRectForFrameRect:[window frame]];

        if (integrated)
            [window setStyleMask:[window styleMask] | NSWindowStyleMaskFullSizeContentView];
        else
            [window setStyleMask:[window styleMask] & ~NSWindowStyleMaskFullSizeContentView];

        [window setTitlebarAppearsTransparent:integrated];
        [window setTitleVisibility:integrated ? NSWindowTitleHidden : NSWindowTitleVisible];
        [window setFrame:[window frameRectForContentRect:contentArea] display:YES];
    }

    static NSString* const backdropViewIdentifier = @"jive-backdrop";

    [[nodiscard]] static NSVisualEffectView* findBackdropView(NSView* frameView)
    {
        for (NSView* view in [frameView subviews])
        {
            if ([[view identifier] isEqualToString:backdropViewIdentifier])
                return static_cast<NSVisualEffectView*>(view);
        }

        return nil;
    }

    void setBackdrop(void* nativeHandle, std::optional<Backdrop> backdrop)
    {
        auto* const window = getWindow(nativeHandle);

        if (window == nil)
            return;

        auto* const contentView = static_cast<NSView*>(nativeHandle);
        auto* const frameView = [contentView superview];
        auto* backdropView = findBackdropView(frameView);

        if (!backdrop.has_value())
        {
            [backdropView removeFromSuperview];
            [window setOpaque:YES];
            return;
        }

        if (backdropView == nil)
        {
            backdropView = [[[NSVisualEffectView alloc] initWithFrame:[frameView bounds]] autorelease];
            [backdropView setIdentifier:backdropViewIdentifier];
            [backdropView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
            [backdropView setBlendingMode:NSVisualEffectBlendingModeBehindWindow];
            [backdropView setState:NSVisualEffectStateFollowsWindowActiveState];
            [frameView addSubview:backdropView positioned:NSWindowBelow relativeTo:contentView];
        }

        [backdropView setMaterial:*backdrop == Backdrop::glass
                                      ? NSVisualEffectMaterialHUDWindow
                                      : NSVisualEffectMaterialWindowBackground];
        [window setOpaque:NO];
        [window setBackgroundColor:[NSColor clearColor]];
    }

    void layoutTitleBarButtons(void* nativeHandle, float titleBarHeight)
    {
        auto* const window = getWindow(nativeHandle);

        if (window == nil)
            return;

        auto* const closeButton = [window standardWindowButton:NSWindowCloseButton];

        if (closeButton == nil || [closeButton superview] == nil)
            return;

        auto* const container = [[closeButton superview] superview];

        if (container == nil)
            return;

        auto containerFrame = [container frame];
        containerFrame.size.height = titleBarHeight;
        containerFrame.origin.y = NSHeight([window frame]) - titleBarHeight;
        [container setFrame:containerFrame];

        for (const auto buttonType : { NSWindowCloseButton, NSWindowMiniaturizeButton, NSWindowZoomButton })
        {
            if (auto* const button = [window standardWindowButton:buttonType])
            {
                auto origin = [button frame].origin;
                origin.y = (titleBarHeight - NSHeight([button frame])) / 2.0;
                [button setFrameOrigin:origin];
            }
        }
    }

    void performWindowDrag(void* nativeHandle)
    {
        auto* const window = getWindow(nativeHandle);
        auto* const event = [NSApp currentEvent];

        if (window != nil && event != nil && [event type] == NSEventTypeLeftMouseDown)
            [window performWindowDragWithEvent:event];
    }
} // namespace jive::macos
#endif
