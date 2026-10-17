#include "jive_ComboBox.h"

#include <jive_layouts/layout/gui-items/jive_ContainerItem.h>

namespace jive
{
    ComboBox::Option::Option(juce::ValueTree sourceTree, int itemIndex, juce::ComboBox& box, const GuiItem& ownerItem)
        : tree{ sourceTree }
        , comboBox{ box }
        , owner{ ownerItem }
        , index{ itemIndex }
        , id{ index + 1 }
        , text{ tree, ids::text() }
        , enabled{ tree, ids::enabled() }
        , selected{ tree, ids::selected() }
        , componentID{ tree, ids::id() }
        , classes{ tree, ids::classes() }
    {
        // Options are enabled unless explicitly disabled. Without this an
        // option with no "enabled" property would resolve to false and the item
        // would be greyed-out and unselectable in the popup.
        if (!enabled.exists())
            enabled = true;

        comboBox.addItem(owner.translate(text), id);

        text.onValueChange = [this]() {
            updateText();
        };

        selected.onValueChange = [this]() {
            if (selected)
            {
                comboBox.setSelectedItemIndex(index);
                wasSelected = true;
            }
            else if (wasSelected)
            {
                comboBox.setSelectedId(0);
                wasSelected = false;
            }
        };

        if (selected)
        {
            comboBox.setSelectedItemIndex(index);
            wasSelected = true;
        }

        enabled.onValueChange = [this]() {
            comboBox.setItemEnabled(id, enabled);
        };
        comboBox.setItemEnabled(id, enabled);

        if (auto* comboBoxComponent = dynamic_cast<ComboBoxComponent*>(&comboBox))
        {
            standIn = std::make_unique<ComboBoxComponent::Item>(id);
            comboBoxComponent->getPopup().addChildComponent(*standIn);

            componentID.onValueChange = [this]() {
                updateStandIn();
            };
            classes.onValueChange = [this]() {
                updateStandIn();
            };
            updateStandIn();
        }
    }

    void ComboBox::Option::setSelected(bool shouldBeSelected)
    {
        selected = shouldBeSelected;
    }

    void ComboBox::Option::updateText()
    {
        comboBox.changeItemText(id, owner.translate(text));
    }

    bool ComboBox::Option::handlePropertyChange(juce::ValueTree& changedTree, const juce::Identifier& property)
    {
        if (changedTree != tree)
            return false;

        forwardPropertyChange(changedTree, property, text, enabled, selected, componentID, classes);
        return true;
    }

    // Style sheets match the option's stand-in by its ID and classes.
    void ComboBox::Option::updateStandIn()
    {
        standIn->setComponentID(componentID);
        standIn->getProperties().set(ids::classes(), classes.get());
    }

    ComboBox::ComboBox(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator(std::move(itemToDecorate))
        , editable{ state, ids::editable() }
        , tooltip{ state, ids::tooltip() }
        , selected{ state, ids::selected() }
        , minWidth{ state, ids::minWidth() }
        , minHeight{ state, ids::minHeight() }
        , focusable{ state, ids::focusable() }
        , onChange{ state, ids::onChange() }
        , hasAutomaticMinWidth{ !minWidth.exists() }
        , hasAutomaticMinHeight{ !minHeight.exists() }
    {
        const BoxModel::ScopedCallbackLock boxModelLock{ boxModel(*this) };

        if (!focusable.exists())
            focusable = true;

        editable.onValueChange = [this]() {
            getComboBox().setEditableText(editable);
        };
        getComboBox().setEditableText(editable);

        tooltip.onValueChange = [this]() {
            getComboBox().setTooltip(translate(tooltip));
        };
        getComboBox().setTooltip(translate(tooltip));

        selected.onValueChange = [this]() {
            getComboBox().setSelectedItemIndex(selected);

            auto currentlySelectedOption = state.getChildWithProperty(ids::selected(), true);

            if (currentlySelectedOption.isValid())
                currentlySelectedOption.setProperty(ids::selected(), false, nullptr);

            if (selected < options.size())
            {
                if (auto* selectedOption = options[selected])
                    selectedOption->setSelected(true);
            }
        };

        updateItems();
        getComboBox().setSelectedItemIndex(selected);
        getComboBox().addListener(this);

        state.addListener(this);

        if (auto* comboBox = dynamic_cast<ComboBoxComponent*>(&getComboBox()))
        {
            comboBox->onMetricsChange = [this] {
                updateMinimumSize();
            };
        }
    }

    ComboBox::~ComboBox()
    {
        if (auto* comboBox = dynamic_cast<ComboBoxComponent*>(&getComboBox()))
            comboBox->onMetricsChange = nullptr;
    }

    // The options and headers are child trees, so the changes to them come
    // through this item's listener too. As they share property names with
    // this item, changes are routed by the tree that changed.
    void ComboBox::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property)
    {
        if (tree == state)
        {
            forwardPropertyChange(tree, property, editable, tooltip, selected);
            return;
        }

        if (tree.hasType(ids::headerType()) && tree.getParent() == state && property == ids::text())
        {
            if (!updatingItems)
                updateItems();

            return;
        }

        for (auto* option : options)
        {
            if (option->handlePropertyChange(tree, property))
            {
                if (property == ids::text())
                    updateMinimumSize();

                return;
            }
        }
    }

    bool ComboBox::isContainer() const
    {
        return false;
    }

#if JIVE_IS_PLUGIN_PROJECT
    void ComboBox::attachToParameter(juce::RangedAudioParameter* parameter, juce::UndoManager* undoManager)
    {
        if (parameter != nullptr)
            parameterAttachment = std::make_unique<juce::ComboBoxParameterAttachment>(*parameter, getComboBox(), undoManager);
        else
            parameterAttachment = nullptr;
    }
#endif

    juce::ComboBox& ComboBox::getComboBox()
    {
        return *dynamic_cast<juce::ComboBox*>(getComponent().get());
    }

    const juce::ComboBox& ComboBox::getComboBox() const
    {
        return *dynamic_cast<const juce::ComboBox*>(getComponent().get());
    }

    void ComboBox::updateItems()
    {
        const juce::ScopedValueSetter svs{ updatingItems, true };

        options.clear();
        getComboBox().clear();

        for (auto childState : state)
        {
            if (childState.hasType(ids::optionType()))
            {
                // Options can be written with inline text, e.g.
                // <Option>One</Option>, which the XML parser turns into an
                // Option with a nested <Text> child rather than a "text"
                // attribute. Fall back to that child so inline text behaves the
                // same as the "text" attribute.
                if (!childState.hasProperty(ids::text()))
                {
                    if (const auto textChild = childState.getChildWithName(ids::textType());
                        textChild.isValid())
                    {
                        childState.setProperty(ids::text(), textChild[ids::text()], nullptr);
                    }
                }

                // A juce::ComboBox can't contain an item with empty text, so
                // skip options that have none rather than triggering an
                // assertion.
                if (childState[ids::text()].toString().isEmpty())
                    continue;

                options.add(std::make_unique<Option>(childState, options.size(), getComboBox(), *this));
            }
            else if (childState.hasType(ids::headerType()))
            {
                // Headers support inline text in the same way options do.
                if (!childState.hasProperty(ids::text()))
                {
                    if (const auto textChild = childState.getChildWithName(ids::textType());
                        textChild.isValid())
                    {
                        childState.setProperty(ids::text(), textChild[ids::text()], nullptr);
                    }
                }

                // A section heading can't have empty text, so skip it if it has
                // none rather than triggering an assertion.
                if (childState[ids::text()].toString().isEmpty())
                    continue;

                getComboBox().addSectionHeading(translate(childState[ids::text()].toString()));
            }
            else if (childState.hasType(ids::separatorType()))
            {
                getComboBox().addSeparator();
            }
        }

        updateMinimumSize();
    }

    // Unless told otherwise, a combo box won't shrink below the size it needs to
    // show any of its options alongside its icon. Without a JIVE look-and-feel
    // to measure with, it falls back to a fixed minimum size.
    void ComboBox::updateMinimumSize()
    {
        if (!hasAutomaticMinWidth && !hasAutomaticMinHeight)
            return;

        const auto& comboBox = getComboBox();
        const auto* lookAndFeel = dynamic_cast<const LookAndFeel*>(&comboBox.getLookAndFeel());
        const auto size = lookAndFeel != nullptr
                            ? lookAndFeel->getIdealComboBoxSize(comboBox)
                            : juce::Rectangle{ 50.0f, 20.0f };
        const auto widthChanged = hasAutomaticMinWidth
                               && !(minWidth.exists() && juce::approximatelyEqual(size.getWidth(), minWidth.get()));
        const auto heightChanged = hasAutomaticMinHeight
                                && !(minHeight.exists() && juce::approximatelyEqual(size.getHeight(), minHeight.get()));

        if (!widthChanged && !heightChanged)
            return;

        if (widthChanged)
            minWidth = size.getWidth();
        if (heightChanged)
            minHeight = size.getHeight();

        if (static_cast<bool>(state[ids::setupInProgress()]))
            return;

        if (auto* parentItem = dynamic_cast<GuiItemDecorator*>(getParent()))
        {
            if (auto* containerParent = parentItem->getTopLevelDecorator().toType<ContainerItem>())
                containerParent->updateIdealSize();
        }
    }

    // Items are translated in place, because rebuilding them would deselect and
    // reselect the current item, which reports a change.
    void ComboBox::localisedStringsChanged()
    {
        GuiItemDecorator::localisedStringsChanged();

        for (auto* option : options)
            option->updateText();

        translateHeaders();
        getComboBox().setTooltip(translate(tooltip));
        updateMinimumSize();
    }

    void ComboBox::translateHeaders()
    {
        juce::StringArray headers;

        for (const auto& childState : state)
        {
            if (childState.hasType(ids::headerType()) && childState[ids::text()].toString().isNotEmpty())
                headers.add(translate(childState[ids::text()].toString()));
        }

        auto headerIndex = 0;

        for (juce::PopupMenu::MenuItemIterator iterator{ *getComboBox().getRootMenu() }; iterator.next();)
        {
            if (iterator.getItem().isSectionHeader && headerIndex < headers.size())
                iterator.getItem().text = headers[headerIndex++];
        }
    }

    void ComboBox::valueTreeChildAdded(juce::ValueTree& parentState, juce::ValueTree& /* child */)
    {
        if (parentState != state)
            return;

        updateItems();
    }

    void ComboBox::valueTreeChildRemoved(juce::ValueTree& parentState, juce::ValueTree& /* child */, int /* index */)
    {
        if (parentState != state)
            return;

        updateItems();
    }

    void ComboBox::comboBoxChanged(juce::ComboBox* box)
    {
        jassertquiet(box == &getComboBox());

        selected = getComboBox().getSelectedItemIndex();
        onChange.triggerWithoutSelfCallback();
    }
} // namespace jive

#if JIVE_UNIT_TESTS
    #include <jive_core/geometry/jive_BorderWidth.h>
    #include <jive_core/graphics/jive_FontUtilities.h>
    #include <jive_layouts/layout/interpreter/jive_Interpreter.h>

class ComboBoxTest : public juce::UnitTest
{
public:
    ComboBoxTest()
        : juce::UnitTest{ "jive::ComboBox", "jive" }
    {
    }

    void runTest() override
    {
        testGuiItem();
        testEditable();
        testTooltip();
        testOptions();
        testHeaders();
        testSelected();
        testEvents();
        testSize();
    }

private:
    [[nodiscard]] static const juce::ComboBox& getComboBox(jive::GuiItem& item)
    {
        return dynamic_cast<jive::GuiItemDecorator&>(item)
            .toType<jive::ComboBox>()
            ->getComboBox();
    }

    [[nodiscard]] static const juce::Label& getLabel(const juce::ComboBox& comboBox)
    {
        for (const auto* child : comboBox.getChildren())
        {
            if (const auto* label = dynamic_cast<const juce::Label*>(child))
                return *label;
        }

        jassertfalse;
        static const juce::Label noLabel;
        return noLabel;
    }

    void expectTextFitsAlongsideIcon(const juce::ComboBox& comboBox,
                                     const jive::LookAndFeel& lookAndFeel,
                                     const juce::String& longestText)
    {
        const auto& label = getLabel(comboBox);
        const auto labelStyles = lookAndFeel.findMostApplicableStyles(label);
        const auto textArea = jive::getBorderWidth(label, labelStyles)
                                  .subtractedFrom(label.getBounds().toFloat());
        const auto iconBounds = lookAndFeel.getComboBoxIconBounds(comboBox,
                                                                  lookAndFeel.findMostApplicableStyles(comboBox));

        expectGreaterOrEqual(textArea.getWidth(),
                             jive::calculateStringWidth(longestText, jive::getFont(label, labelStyles)));
        expectLessOrEqual(textArea.getRight(), iconBounds.getX());
        expect(comboBox.getLocalBounds().toFloat().contains(iconBounds));
    }

    void testSize()
    {
        beginTest("size / fits its longest option alongside its icon");
        {
            juce::Component root;
            jive::LookAndFeel lookAndFeel{ root };

            jive::Interpreter interpreter;
            auto item = interpreter.interpret(jive::parseXML(R"(
                <Component width="400" height="200" align-items="flex-start">
                    <ComboBox>
                        <Option>One</Option>
                        <Option>A much longer option</Option>
                    </ComboBox>
                </Component>
            )"));
            root.addAndMakeVisible(*item->getComponent());

            expectTextFitsAlongsideIcon(getComboBox(*item->getChildren()[0]),
                                        lookAndFeel,
                                        "A much longer option");
        }

        beginTest("size / grows to fit longer options");
        {
            juce::Component root;
            jive::LookAndFeel lookAndFeel{ root };

            auto tree = jive::parseXML(R"(
                <Component width="400" height="200" align-items="flex-start">
                    <ComboBox>
                        <Option>One</Option>
                    </ComboBox>
                </Component>
            )");
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            root.addAndMakeVisible(*item->getComponent());

            const auto& comboBox = getComboBox(*item->getChildren()[0]);
            auto previousWidth = comboBox.getWidth();

            tree.getChild(0).appendChild(juce::ValueTree{ "Option", { { "text", "A much longer option" } } }, nullptr);
            expectGreaterThan(comboBox.getWidth(), previousWidth);
            expectTextFitsAlongsideIcon(comboBox, lookAndFeel, "A much longer option");
            previousWidth = comboBox.getWidth();

            tree.getChild(0).getChild(0).setProperty("text", "An option that's even longer than that", nullptr);
            expectGreaterThan(comboBox.getWidth(), previousWidth);
            expectTextFitsAlongsideIcon(comboBox, lookAndFeel, "An option that's even longer than that");
        }

        beginTest("size / follows changes to its font");
        {
            juce::Component root;
            jive::LookAndFeel lookAndFeel{ root };

            auto tree = jive::parseXML(R"(
                <Component width="400" height="200" align-items="flex-start">
                    <ComboBox>
                        <Option>Option</Option>
                    </ComboBox>
                </Component>
            )");
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            root.addAndMakeVisible(*item->getComponent());

            const auto& comboBox = getComboBox(*item->getChildren()[0]);
            const auto previousBounds = comboBox.getBounds();

            tree.setProperty("style", new jive::Object{ { "font-size", 30 } }, nullptr);
            juce::ignoreUnused(item->getComponent()->createComponentSnapshot(item->getComponent()->getLocalBounds()));

            expectGreaterThan(comboBox.getWidth(), previousBounds.getWidth());
            expectGreaterThan(comboBox.getHeight(), previousBounds.getHeight());
            expectTextFitsAlongsideIcon(comboBox, lookAndFeel, "Option");
        }

        beginTest("size / leaves room for a custom icon");
        {
            juce::Component root;
            jive::LookAndFeel lookAndFeel{ root };

            auto tree = jive::parseXML(R"(
                <Component width="400" height="200" align-items="flex-start">
                    <ComboBox>
                        <Option>Option</Option>
                    </ComboBox>
                </Component>
            )");
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            root.addAndMakeVisible(*item->getComponent());

            const auto& comboBox = getComboBox(*item->getChildren()[0]);
            const auto previousIconBounds = lookAndFeel.getComboBoxIconBounds(comboBox,
                                                                              lookAndFeel.findMostApplicableStyles(comboBox));

            tree.getChild(0).setProperty("style",
                                         new jive::Object{
                                             { "icon", R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 30 10"><path d="M0 0 L15 10 L30 0" /></svg>)" },
                                         },
                                         nullptr);
            juce::ignoreUnused(item->getComponent()->createComponentSnapshot(item->getComponent()->getLocalBounds()));

            const auto iconBounds = lookAndFeel.getComboBoxIconBounds(comboBox,
                                                                      lookAndFeel.findMostApplicableStyles(comboBox));
            expectEquals(iconBounds.getWidth(), iconBounds.getHeight() * 3.0f);
            expectGreaterThan(iconBounds.getWidth(), previousIconBounds.getWidth());
            expectTextFitsAlongsideIcon(comboBox, lookAndFeel, "Option");
        }
    }

    void testGuiItem()
    {
        beginTest("gui-item");

        jive::Interpreter interpreter;
        auto item = interpreter.interpret(juce::ValueTree{
            "ComboBox",
            {
                { "width", 222 },
                { "height", 333 },
            },
        });
        expect(!item->isContainer());
    }

    void testEditable()
    {
        beginTest("editable");

        {
            juce::ValueTree tree{
                "ComboBox",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& comboBox = dynamic_cast<jive::GuiItemDecorator&>(*item)
                                 .toType<jive::ComboBox>()
                                 ->getComboBox();
            expect(!comboBox.isTextEditable());

            tree.setProperty("editable", true, nullptr);
            expect(comboBox.isTextEditable());
        }
        {
            juce::ValueTree tree{
                "ComboBox",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "editable", true },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& comboBox = dynamic_cast<jive::GuiItemDecorator&>(*item)
                                 .toType<jive::ComboBox>()
                                 ->getComboBox();
            expect(comboBox.isTextEditable());
        }
    }

    void testTooltip()
    {
        beginTest("tooltip");

        {
            juce::ValueTree tree{
                "ComboBox",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& comboBox = dynamic_cast<jive::GuiItemDecorator&>(*item)
                                 .toType<jive::ComboBox>()
                                 ->getComboBox();
            expect(comboBox.getTooltip().isEmpty());

            tree.setProperty("tooltip", "246", nullptr);
            expectEquals(comboBox.getTooltip(), juce::String{ "246" });
        }
        {
            juce::ValueTree tree{
                "ComboBox",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "tooltip", "369" },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& comboBox = dynamic_cast<jive::GuiItemDecorator&>(*item)
                                 .toType<jive::ComboBox>()
                                 ->getComboBox();
            expectEquals(comboBox.getTooltip(), juce::String{ "369" });
        }
    }

    void testOptions()
    {
        beginTest("options");

        {
            juce::ValueTree tree{
                "ComboBox",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& comboBox = dynamic_cast<jive::GuiItemDecorator&>(*item)
                                 .toType<jive::ComboBox>()
                                 ->getComboBox();
            expectEquals(comboBox.getNumItems(), 0);

            tree.appendChild(juce::ValueTree{
                                 "Option",
                                 {
                                     { "text", "Choose me!" },
                                 },
                             },
                             nullptr);
            expectEquals(comboBox.getNumItems(), 1);
            expectEquals(comboBox.getItemText(0),
                         juce::String{ "Choose me!" });
            expectEquals(comboBox.getSelectedId(), 0);

            tree.getChild(0).setProperty("selected", true, nullptr);
            expectEquals(comboBox.getSelectedId(), 1);

            tree.appendChild(juce::ValueTree{
                                 "Option",
                                 {
                                     { "text", "Two" },
                                 },
                             },
                             nullptr);
            expectEquals(comboBox.getNumItems(), 2);
            expectEquals(comboBox.getItemText(1),
                         juce::String{ "Two" });
            expectEquals(comboBox.getSelectedId(), 1);

            tree.removeChild(0, nullptr);
            expectEquals(comboBox.getNumItems(), 1);
            expectEquals(comboBox.getSelectedId(), 0);

            tree.getChild(0).setProperty("enabled", false, nullptr);
            expect(!comboBox.isItemEnabled(1));

            tree.getChild(0).setProperty("text", "new text", nullptr);
            expectEquals(comboBox.getItemText(0), juce::String{ "new text" });

            tree.getChild(0).setProperty("selected", true, nullptr);
            expectEquals(comboBox.getSelectedId(), 1);

            tree.getChild(0).setProperty("selected", false, nullptr);
            expectEquals(comboBox.getSelectedId(), 0);
        }
        {
            juce::ValueTree tree{
                "ComboBox",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
                {
                    juce::ValueTree{
                        "Option",
                        {
                            { "text", "One" },
                            { "selected", true },
                        },
                    },
                    juce::ValueTree{
                        "Option",
                        {
                            { "text", "Two" },
                            { "enabled", false },
                        },
                    },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& comboBox = dynamic_cast<jive::GuiItemDecorator&>(*item)
                                 .toType<jive::ComboBox>()
                                 ->getComboBox();
            expectEquals(comboBox.getNumItems(), 2);
            expectEquals(comboBox.getItemText(0), juce::String{ "One" });
            expectEquals(comboBox.getItemText(1), juce::String{ "Two" });
            expectEquals(comboBox.getSelectedId(), 1);
            expect(!comboBox.isItemEnabled(2));
        }
        {
            // Options written with inline text (e.g. <Option>One</Option>)
            // should behave the same as those using a "text" attribute. The XML
            // parser turns inline text on a non-Text element into a nested
            // <Text> child, so the combo box needs to honour that.
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(jive::parseXML(R"(
                <ComboBox width="222" height="333">
                    <Option>One</Option>
                    <Option>Two</Option>
                </ComboBox>
            )"));
            auto& comboBox = dynamic_cast<jive::GuiItemDecorator&>(*item)
                                 .toType<jive::ComboBox>()
                                 ->getComboBox();
            expectEquals(comboBox.getNumItems(), 2);
            expectEquals(comboBox.getItemText(0), juce::String{ "One" });
            expectEquals(comboBox.getItemText(1), juce::String{ "Two" });
        }
        {
            // Headers can also be written with inline text, e.g.
            // <Header>Section</Header>.
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(jive::parseXML(R"(
                <ComboBox width="222" height="333">
                    <Option>One</Option>
                    <Header>Section</Header>
                    <Option>Two</Option>
                </ComboBox>
            )"));
            auto& comboBox = dynamic_cast<jive::GuiItemDecorator&>(*item)
                                 .toType<jive::ComboBox>()
                                 ->getComboBox();
            expectEquals(comboBox.getNumItems(), 2);

            auto foundSectionHeader = false;

            if (auto* rootMenu = comboBox.getRootMenu())
            {
                for (juce::PopupMenu::MenuItemIterator iterator{ *rootMenu }; iterator.next();)
                {
                    if (iterator.getItem().isSectionHeader
                        && iterator.getItem().text == "Section")
                    {
                        foundSectionHeader = true;
                    }
                }
            }

            expect(foundSectionHeader);
        }
        {
            // Options have stand-ins in the combo box's popup so that style
            // sheets can style them.
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(jive::parseXML(R"(
                <ComboBox width="222" height="333">
                    <Option id="first" class="large">One</Option>
                    <Option>Two</Option>
                </ComboBox>
            )"));
            auto& comboBox = dynamic_cast<const jive::ComboBoxComponent&>(getComboBox(*item));
            expectEquals(comboBox.getPopup().getNumChildComponents(), 2);
            expect(comboBox.findItem(1) != nullptr && comboBox.findItem(2) != nullptr);
            expectEquals(comboBox.findItem(1)->getComponentID(), juce::String{ "first" });
            expectEquals(comboBox.findItem(1)->getProperties()["class"].toString(), juce::String{ "large" });

            auto state = item->state;
            state.getChild(1).setProperty("class", "small", nullptr);
            expectEquals(comboBox.findItem(2)->getProperties()["class"].toString(), juce::String{ "small" });

            state.removeChild(0, nullptr);
            expectEquals(comboBox.getPopup().getNumChildComponents(), 1);
            expect(comboBox.findItem(1) != nullptr);
            expectEquals(comboBox.findItem(1)->getProperties()["class"].toString(), juce::String{ "small" });
        }
    }

    [[nodiscard]] static juce::StringArray getSectionHeadings(juce::ComboBox& comboBox)
    {
        juce::StringArray headings;

        if (auto* rootMenu = comboBox.getRootMenu())
        {
            for (juce::PopupMenu::MenuItemIterator iterator{ *rootMenu }; iterator.next();)
            {
                if (iterator.getItem().isSectionHeader)
                    headings.add(iterator.getItem().text);
            }
        }

        return headings;
    }

    void testHeaders()
    {
        beginTest("headers");

        juce::ValueTree tree{
            "ComboBox",
            {
                { "width", 222 },
                { "height", 333 },
            },
            {
                juce::ValueTree{ "Option", { { "text", "One" } } },
                juce::ValueTree{ "Header", { { "text", "Section" } } },
                juce::ValueTree{ "Option", { { "text", "Two" } } },
            },
        };
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(tree);
        auto& comboBox = dynamic_cast<jive::GuiItemDecorator&>(*item)
                             .toType<jive::ComboBox>()
                             ->getComboBox();
        expectEquals(getSectionHeadings(comboBox), juce::StringArray{ "Section" });

        tree.appendChild(juce::ValueTree{ "Option", { { "text", "Three" } } }, nullptr);
        tree.appendChild(juce::ValueTree{ "Option", { { "text", "Four" } } }, nullptr);
        tree.getChild(1).setProperty("text", "Renamed", nullptr);
        expectEquals(getSectionHeadings(comboBox), juce::StringArray{ "Renamed" });
        expectEquals(comboBox.getNumItems(), 4);

        tree.appendChild(juce::ValueTree{ "Header" }, nullptr);
        expectEquals(getSectionHeadings(comboBox), juce::StringArray{ "Renamed" });

        tree.getChild(tree.getNumChildren() - 1).setProperty("text", "Late", nullptr);
        expectEquals(getSectionHeadings(comboBox), juce::StringArray{ "Renamed", "Late" });

        tree.appendChild(juce::ValueTree{
                             "Header",
                             {},
                             {
                                 juce::ValueTree{ "Text", { { "text", "Inline" } } },
                             },
                         },
                         nullptr);
        expectEquals(getSectionHeadings(comboBox), juce::StringArray{ "Renamed", "Late", "Inline" });
        expectEquals(comboBox.getNumItems(), 4);
    }

    void testSelected()
    {
        beginTest("selected");

        {
            juce::ValueTree tree{
                "ComboBox",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& comboBox = dynamic_cast<jive::GuiItemDecorator&>(*item)
                                 .toType<jive::ComboBox>()
                                 ->getComboBox();
            expectEquals(comboBox.getSelectedId(), 0);

            tree.appendChild(juce::ValueTree{ "Option", { { "text", "1" } } }, nullptr);
            tree.appendChild(juce::ValueTree{ "Option", { { "text", "2" } } }, nullptr);
            tree.appendChild(juce::ValueTree{ "Separator" }, nullptr);
            tree.appendChild(juce::ValueTree{ "Header", { { "text", "Header" } } }, nullptr);
            tree.appendChild(juce::ValueTree{ "Option", { { "text", "3" } } }, nullptr);
            expectEquals(comboBox.getSelectedId(), 0);

            tree.setProperty("selected", 0, nullptr);
            expectEquals(comboBox.getSelectedItemIndex(), 0);
            expect(tree.getChild(0)["selected"]);
            expect(!tree.getChild(1)["selected"]);
            expect(!tree.getChild(4)["selected"]);

            tree.setProperty("selected", 2, nullptr);
            expectEquals(comboBox.getSelectedItemIndex(), 2);
            expect(!tree.getChild(0)["selected"]);
            expect(!tree.getChild(1)["selected"]);
            expect(tree.getChild(4)["selected"]);

            tree.setProperty("selected", 100, nullptr);
            expectEquals(comboBox.getSelectedId(), 0);
            expect(!tree.getChild(0)["selected"]);
            expect(!tree.getChild(1)["selected"]);
            expect(!tree.getChild(4)["selected"]);

            comboBox.setSelectedItemIndex(1, juce::NotificationType::sendNotificationSync);
            expectEquals(static_cast<int>(tree["selected"]), 1);
            expect(!tree.getChild(0)["selected"]);
            expect(tree.getChild(1)["selected"]);
            expect(!tree.getChild(4)["selected"]);
        }
        {
            juce::ValueTree tree{
                "ComboBox",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "selected", 1 },
                },
                {
                    juce::ValueTree{ "Option", { { "text", "One" } } },
                    juce::ValueTree{ "Option", { { "text", "Two" } } },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& comboBox = dynamic_cast<jive::GuiItemDecorator&>(*item)
                                 .toType<jive::ComboBox>()
                                 ->getComboBox();
            expectEquals(comboBox.getSelectedItemIndex(), 1);
        }
    }

    void testEvents()
    {
        beginTest("events");

        juce::ValueTree tree{
            "ComboBox",
            {
                { "width", 222 },
                { "height", 333 },
            },
            {
                juce::ValueTree{
                    "Option",
                    {
                        { "text", "One" },
                        { "selected", true },
                    },
                },
                juce::ValueTree{
                    "Option",
                    {
                        { "text", "Two" },
                        { "enabled", false },
                    },
                },
            },
        };
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(tree);
        auto& comboBox = dynamic_cast<jive::GuiItemDecorator&>(*item)
                             .toType<jive::ComboBox>()
                             ->getComboBox();
        jive::Event onChange{ tree, "on-change" };
        expectEquals(onChange.getAssumedTriggerCount(), 0);

        comboBox.setSelectedId(2, juce::sendNotificationSync);
        expectEquals(onChange.getAssumedTriggerCount(), 1);
    }
};

static ComboBoxTest comboBoxTest;
#endif
