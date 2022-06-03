#pragma once

#include <jive_demo/gui/pages/Pages.h>

#include <jive_core/jive_core.h>

namespace jive_demo
{
    class WindowState
    {
    public:
        struct Listener
        {
            virtual ~Listener() = default;

            virtual void pageChanged(WindowState& state) = 0;
        };

        explicit WindowState(juce::ValueTree sourceState)
            : state{ sourceState }
            , page{ state, "page" }
        {
            if (!page.exists())
                page = Page::home;

            page.onValueChange = [this]() {
                listeners.call(&Listener::pageChanged, *this);
            };
        }

        WindowState() = delete;
        WindowState& operator=(const WindowState& other) = delete;
        WindowState& operator=(WindowState&& other) = delete;

        WindowState(const WindowState& other)
            : WindowState{ other.state }
        {
        }

        WindowState(WindowState&& other)
            : WindowState{ other.state }
        {
        }

        [[nodiscard]] Page getPage() const
        {
            return page;
        }

        void setPage(Page newPage)
        {
            page = newPage;
        }

        void addListener(Listener& listener) const
        {
            listeners.add(&listener);
        }

        void removeListener(Listener& listener) const
        {
            listeners.remove(&listener);
        }

    private:
        juce::ValueTree state;

        jive::Property<Page> page;

        mutable juce::ListenerList<Listener> listeners;
    };
} // namespace jive_demo

namespace juce
{
    template <>
    struct VariantConverter<jive_demo::Page>
    {
        static jive_demo::Page fromVar(const var& v)
        {
            return static_cast<jive_demo::Page>(static_cast<int64>(v));
        }

        static var toVar(jive_demo::Page page)
        {
            return static_cast<int64>(page);
        }
    };
} // namespace juce
