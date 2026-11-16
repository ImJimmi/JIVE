#include <jive_layouts/jive_layouts.h>

struct SideBarWhenWindowIsResized : public juce::UnitTest
{
    SideBarWhenWindowIsResized()
        : juce::UnitTest{ "Side-bar when the window is resized", "jive" }
    {
    }

    void runTest() final
    {
        auto state = jive::parseXML(R"(
            <Window width="800" height="500">
                <Component flex-direction="row" height="100%">
                    <Component id="side-bar" padding="15">
                        <Text margin="0 0 15 0">JIVE</Text>
                        <Button padding="10">Layouts</Button>
                        <Button padding="10">Styles</Button>
                        <Button padding="10">Animations</Button>
                    </Component>

                    <Component flex-grow="1" padding="20">
                        <Text margin="0 0 20 0">Layouts</Text>
                        <Text margin="0 0 5 0">Flex Box</Text>

                        <Component display="grid" grid-template-columns="1fr 1fr" gap="20">
                            <Component padding="10 0 0 0">
                                <Component flex-direction="row">
                                    <Text flex-grow="1" align-self="start" margin="4 0 0 0">Direction</Text>

                                    <Component>
                                        <Component flex-direction="row" gap="2" margin="0 0 7 0">
                                            <Button min-width="0" min-height="0" toggled="true">
                                                <svg viewBox="0 0 24 24" width="24" height="24">
                                                    <rect x="4" y="6" width="14" height="13" />
                                                </svg>
                                            </Button>
                                            <Button min-width="0" min-height="0">
                                                <svg viewBox="0 0 24 24" width="24" height="24">
                                                    <rect x="4" y="6" width="14" height="13" />
                                                </svg>
                                            </Button>
                                        </Component>

                                        <Text margin="0 0 0 2">Row</Text>
                                    </Component>
                                </Component>
                            </Component>

                            <Component padding="10" flex-direction="row" height="135" gap="5">
                                <Component width="35" height="35" />
                                <Component width="35" height="35" />
                                <Component width="35" height="35" />
                                <Component width="35" height="35" />
                                <Component width="35" height="35" />
                                <Component width="35" height="35" />
                            </Component>
                        </Component>
                    </Component>
                </Component>
            </Window>
        )");

        jive::Interpreter interpreter;
        auto item = interpreter.interpret(state);
        auto& window = *dynamic_cast<jive::GuiItemDecorator&>(*item).toType<jive::Window>();
        auto& sideBar = *item->getChildren()[0]->getChildren()[0];

        const auto overlaps = [&sideBar]() {
            auto previousBottom = 0;

            for (auto* child : sideBar.getChildren())
            {
                if (child->getComponent()->getY() < previousBottom)
                    return true;

                previousBottom = child->getComponent()->getBottom();
            }

            return false;
        };

        const auto windowHeight = window.getHeight();
        const auto idealWidth = sideBar.getComponent()->getWidth();
        auto everOverlapped = false;

        for (auto width = window.getWidth(); width >= 1; width--)
        {
            window.setSize(width, windowHeight);
            everOverlapped |= overlaps();
        }

        for (auto width = 1; width <= 1000; width++)
        {
            window.setSize(width, windowHeight);
            everOverlapped |= overlaps();
        }

        beginTest("the side-bar's items never overlap one another");
        expect(!everOverlapped);

        beginTest("the side-bar goes back to the width its content wants");
        expectEquals(sideBar.getComponent()->getWidth(), idealWidth);
    }
};

static SideBarWhenWindowIsResized sideBarWhenWindowIsResized;
