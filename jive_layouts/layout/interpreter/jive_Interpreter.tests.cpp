#include <jive_layouts/jive_layouts.h>

static jive::UnitTest componentFactoryTest{
    "jive",
    "jive::Interpreter",
    "Component Factory",
    [](auto& test) {
        jive::Interpreter interpreter;
        test.expect(interpreter.getComponentFactory().create("MyComponent") == nullptr);

        struct MyComponent : public juce::Component
        {
        };
        interpreter.getComponentFactory().set("MyComponent", []() {
            return std::make_unique<MyComponent>();
        });
        test.expect(dynamic_cast<MyComponent*>(interpreter.getComponentFactory().create("MyComponent").get()) != nullptr);

        struct YourComponent : public juce::Component
        {
        };
        jive::ComponentFactory factory;
        factory.set("YourComponent", []() {
            return std::make_unique<YourComponent>();
        });
        interpreter.setComponentFactory(factory);
        test.expect(dynamic_cast<YourComponent*>(interpreter.getComponentFactory().create("YourComponent").get()) != nullptr);
    },
};

static jive::UnitTest fileSourcesTest{
    "jive",
    "jive::Interpreter",
    "File Sources",
    [](auto& test) {
        const auto cwd = juce::File::getSpecialLocation(juce::File::currentApplicationFile)
                             .getParentDirectory()
                             .getChildFile("interpreter-tests/");
        cwd.deleteRecursively();
        auto basicView = cwd.getChildFile("basic-view.xml");
        basicView.create();
        basicView.replaceWithText(R"(<Window width="100" height="100"/>)");

        jive::Interpreter interpreter;
        const auto item = interpreter.interpret(basicView);
        test.expect(item != nullptr);

        auto& window = *item->getComponent()->getTopLevelComponent();
        test.expectEquals(window.getWidth(), 100);
        test.expectEquals(window.getHeight(), 100);

        basicView.replaceWithText(R"(<Window width="200" height="200"/>)");
        jive::FileObserver::triggerAllTimerCallbacks();
        test.expectEquals(window.getWidth(), 200);
        test.expectEquals(window.getHeight(), 200);
    },
};

static jive::UnitTest nestedFileSourcesTest{
    "jive",
    "jive::Interpreter",
    "Nested File Sources",
    [](auto& test) {
        const auto cwd = juce::File::getSpecialLocation(juce::File::currentApplicationFile)
                             .getParentDirectory()
                             .getChildFile("interpreter-tests/");
        cwd.deleteRecursively();
        auto alpha = cwd.getChildFile("alpha.xml");
        alpha.create();
        alpha.replaceWithText(R"(<Component id="alpha"/>)");
        auto beta = cwd.getChildFile("beta.xml");
        beta.create();
        beta.replaceWithText(R"(<Component id="beta"/>)");

        auto mainView = cwd.getChildFile("nested-view.xml");
        mainView.create();
        mainView.replaceWithText(R"(
            <Window width="100" height="100">
                <Component template="alpha.xml" name="Alpha"/>
            </Window>
        )");

        // Create the window from the original source
        jive::Interpreter interpreter;
        const auto item = interpreter.interpret(mainView);
        test.expect(item != nullptr, "Item should have been created");

        auto* window = dynamic_cast<jive::Window*>(item->getComponent()->getTopLevelComponent());
        auto* child = window->getComponent()->findChildWithID("alpha");
        test.expectEquals(child->getName(),
                          juce::String{ "Alpha" },
                          "Window should have an 'alpha' child with the name 'Alpha'");

        // Replace the Alpha component with a Beta component in the main source
        mainView.replaceWithText(R"(
            <Window width="100" height="100">
                <Component template="beta.xml" name="Beta"/>
            </Window>
        )");
        jive::FileObserver::triggerAllTimerCallbacks();
        test.expect(window->getComponent()->findChildWithID("alpha") == nullptr,
                    "Window should no longer have an 'alpha' child");
        child = window->getComponent()->findChildWithID("beta");
        test.expectEquals(child->getName(),
                          juce::String{ "Beta" },
                          "Window should have a 'beta' child with the name 'Beta'");

        // Change a property in the beta source
        beta.replaceWithText(R"(<Component id="gamma"/>)");
        jive::FileObserver::triggerAllTimerCallbacks();
        test.expect(window->getComponent()->findChildWithID("alpha") == nullptr,
                    "Window should not have an 'alpha' child");
        test.expect(window->getComponent()->findChildWithID("beta") == nullptr,
                    "Window should not have a 'beta' child");
        test.expect(window->getComponent()->findChildWithID("gamma") != nullptr,
                    "Window should have a 'gamma' child");
    },
};

static jive::UnitTest topLevelFileWithNestedSourceTest{
    "jive",
    "jive::Interpreter",
    "Top-Level File With Nested Source",
    [](auto& test) {
        // Replicates the xml-views example: a top-level file (main.xml)
        // containing a Window that pulls in a nested file via "template"
        // (heading.xml). Editing *either* file should be reflected live.
        const auto cwd = juce::File::getSpecialLocation(juce::File::currentApplicationFile)
                             .getParentDirectory()
                             .getChildFile("interpreter-tests/");
        cwd.deleteRecursively();

        // Mirror the example's directory layout: main.xml lives in the source
        // directory, the nested view lives in a separate "views" directory
        // that's added via addSourceDirectory().
        auto viewsDir = cwd.getChildFile("views");
        auto heading = viewsDir.getChildFile("heading.xml");
        heading.create();
        heading.replaceWithText(R"(<Component id="heading"/>)");

        auto mainView = cwd.getChildFile("main.xml");
        mainView.create();
        mainView.replaceWithText(R"(
            <Window width="100" height="100">
                <Component template="heading.xml"/>
            </Window>
        )");

        jive::Interpreter interpreter;
        interpreter.addSourceDirectory(viewsDir);
        const auto item = interpreter.interpret(mainView);
        test.expect(item != nullptr, "Item should have been created");

        auto* window = dynamic_cast<jive::Window*>(item->getComponent()->getTopLevelComponent());
        test.expectEquals(window->getWidth(), 100);

        // Editing the top-level file should be reflected.
        mainView.replaceWithText(R"(
            <Window width="200" height="200">
                <Component template="heading.xml"/>
            </Window>
        )");
        jive::FileObserver::triggerAllTimerCallbacks();
        test.expectEquals(window->getWidth(),
                          200,
                          "Changes to the top-level source file should be reflected");

        // Editing the top-level file *again* should also be reflected - the
        // first reinterpret mustn't stop subsequent changes from being picked
        // up.
        mainView.replaceWithText(R"(
            <Window width="300" height="300">
                <Component template="heading.xml"/>
            </Window>
        )");
        jive::FileObserver::triggerAllTimerCallbacks();
        test.expectEquals(window->getWidth(),
                          300,
                          "Subsequent changes to the top-level source file should be reflected");

        // Editing the nested file should also still be reflected, even after
        // the top-level file has been reinterpreted.
        heading.replaceWithText(R"(<Component name="updated"/>)");
        jive::FileObserver::triggerAllTimerCallbacks();
        test.expectEquals(window->getComponent()->findChildWithID("heading")->getName(),
                          juce::String{ "updated" },
                          "Changes to the nested source file should be reflected");
    },
};

static jive::UnitTest topLevelFileReloadRemovesPropertiesTest{
    "jive",
    "jive::Interpreter",
    "Top-Level File Reload Removes Properties Removed From The File",
    [](auto& test) {
        const auto cwd = juce::File::getSpecialLocation(juce::File::currentApplicationFile)
                             .getParentDirectory()
                             .getChildFile("interpreter-removed-properties-tests/");
        cwd.deleteRecursively();
        cwd.createDirectory();

        cwd.getChildFile("styles.json").replaceWithText(R"({ "background": "#FF0000" })");

        auto mainView = cwd.getChildFile("main.xml");
        mainView.replaceWithText(R"(<Window width="100" height="100" style="styles.json"/>)");

        jive::Interpreter interpreter;
        interpreter.addSourceDirectory(cwd);
        const auto item = interpreter.interpret(mainView);

        const auto getBackground = [&item] {
            auto* lookAndFeel = dynamic_cast<jive::LookAndFeel*>(&item->getComponent()->getLookAndFeel());
            return lookAndFeel->findMostApplicableStyles(*item->getComponent())
                .find<jive::Fill>("background")
                .value_or(jive::Fill{});
        };
        test.expect(getBackground() == juce::Colours::red);

        item->state.setProperty(jive::ids::theme(), "material", nullptr);

        mainView.replaceWithText(R"(<Window width="100" height="100"/>)");
        mainView.setLastModificationTime(juce::Time::getCurrentTime() + juce::RelativeTime::seconds(10.0));
        jive::FileObserver::triggerAllTimerCallbacks();
        test.expect(!item->state.hasProperty(jive::ids::style()),
                    "A property removed from the file should be removed from the tree");
        test.expect(getBackground() != juce::Colours::red,
                    "Removing the style property should remove the style sheet's styles");
        test.expectEquals(item->state[jive::ids::theme()].toString(),
                          juce::String{ "material" },
                          "Properties set at runtime should survive a reload");

        mainView.replaceWithText(R"(<Window width="100" height="100" style="styles.json"/>)");
        mainView.setLastModificationTime(juce::Time::getCurrentTime() + juce::RelativeTime::seconds(20.0));
        jive::FileObserver::triggerAllTimerCallbacks();
        test.expect(getBackground() == juce::Colours::red,
                    "Adding the style property back should apply its styles again");

        mainView.replaceWithText(R"(<Window width="100" height="100"/>)");
        mainView.setLastModificationTime(juce::Time::getCurrentTime() + juce::RelativeTime::seconds(30.0));
        jive::FileObserver::triggerAllTimerCallbacks();
        test.expect(getBackground() != juce::Colours::red,
                    "Removing the style property after a reload should remove its styles");

        cwd.deleteRecursively();
    },
};

static jive::UnitTest templatePlaceholdersTest{
    "jive",
    "jive::Interpreter",
    "Template Placeholders",
    [](auto& test) {
        // Placeholders are filled in from the instance's attributes before the
        // template is parsed, so they can stand in for types and property
        // names as well as values.
        const auto cwd = juce::File::getSpecialLocation(juce::File::currentApplicationFile)
                             .getParentDirectory()
                             .getChildFile("interpreter-tests/");
        cwd.deleteRecursively();

        auto track = cwd.getChildFile("track.xml");
        track.create();
        track.replaceWithText(R"(
            <Component class="track {{ extra-class }}">
                <Text id="name">{{name}}</Text>
                <{{ control }} id="control" {{	control-property  }}="{{ value }}"/>
            </Component>
        )");

        auto mainView = cwd.getChildFile("mixer.xml");
        mainView.create();
        mainView.replaceWithText(R"(
            <Window width="100" height="100">
                <Component template="track.xml" name="Rock &amp; &quot;Roll&quot; &lt;3" control="Slider" control-property="value" value="0.5"/>
            </Window>
        )");

        jive::Interpreter interpreter;
        const auto item = interpreter.interpret(mainView);
        test.expect(item != nullptr, "Item should have been created");

        const auto expectInstantiated = [&test, &item](const juce::String& message) {
            test.expectEquals(jive::findElementWithID(item->state, "name")["text"].toString(),
                              juce::String{ "Rock & \"Roll\" <3" },
                              message + ": values should be substituted, whatever their characters");

            const auto control = jive::findElementWithID(item->state, "control");
            test.expectEquals(control.getType().toString(),
                              juce::String{ "Slider" },
                              message + ": placeholders should be able to name types");
            test.expectEquals(control["value"].toString(),
                              juce::String{ "0.5" },
                              message + ": placeholders should be able to name properties");

            test.expectEquals(item->state.getChild(0)["class"].toString(),
                              juce::String{ "track " },
                              message + ": placeholders without a matching attribute should be left empty");
        };

        expectInstantiated("Initial load");

        track.replaceWithText(R"(
            <Component class="track {{ extra-class }}" padding="1">
                <Text id="name">{{name}}</Text>
                <{{ control }} id="control" {{	control-property  }}="{{ value }}"/>
            </Component>
        )");
        jive::FileObserver::triggerAllTimerCallbacks();

        expectInstantiated("After reloading the template");

        cwd.deleteRecursively();
    },
};

static jive::UnitTest templatePlaceholderDefaultsTest{
    "jive",
    "jive::Interpreter",
    "Template Placeholder Defaults",
    [](auto& test) {
        // A placeholder's default is used only when the instance doesn't have
        // the attribute at all - an empty value still replaces the default.
        const auto cwd = juce::File::getSpecialLocation(juce::File::currentApplicationFile)
                             .getParentDirectory()
                             .getChildFile("interpreter-tests/");
        cwd.deleteRecursively();

        auto card = cwd.getChildFile("card.xml");
        card.create();
        card.replaceWithText(R"(
            <Component id="{{ id }}" class="{{class='card'}}">
                <Text>{{ title = "Untitled &amp; Unloved" }}</Text>
                <{{ control="Button" }}/>
            </Component>
        )");

        auto mainView = cwd.getChildFile("cards.xml");
        mainView.create();
        mainView.replaceWithText(R"(
            <Window width="100" height="100">
                <Component template="card.xml" id="defaulted"/>
                <Component template="card.xml" id="supplied" class="custom" title="Hello" control="Slider"/>
                <Component template="card.xml" id="emptied" class="" title=""/>
            </Window>
        )");

        jive::Interpreter interpreter;
        const auto item = interpreter.interpret(mainView);
        test.expect(item != nullptr, "Item should have been created");

        const auto expectCard = [&test, &item](const juce::String& id,
                                               const juce::String& expectedClass,
                                               const juce::String& expectedTitle,
                                               const juce::String& expectedControl) {
            const auto instance = jive::findElementWithID(item->state, id);
            test.expectEquals(instance["class"].toString(), expectedClass, id);
            test.expectEquals(instance.getChild(0)["text"].toString(), expectedTitle, id);
            test.expectEquals(instance.getChild(1).getType().toString(), expectedControl, id);
        };

        expectCard("defaulted", "card", "Untitled & Unloved", "Button");
        expectCard("supplied", "custom", "Hello", "Slider");
        expectCard("emptied", "", "", "Button");

        cwd.deleteRecursively();
    },
};

static juce::StringArray describeChildren(const juce::ValueTree& tree)
{
    juce::StringArray result;

    for (const auto& child : tree)
        result.add(child.getType().toString() + ":" + child["text"].toString());

    return result;
}

static jive::UnitTest templateSnippetsTest{
    "jive",
    "jive::Interpreter",
    "Template Snippets",
    [](auto& test) {
        // Snippets in an instance replace the same-named snippets in its
        // template, wherever they are. A template's snippets that aren't
        // given any content keep their own.
        const auto cwd = juce::File::getSpecialLocation(juce::File::currentApplicationFile)
                             .getParentDirectory()
                             .getChildFile("interpreter-tests/");
        cwd.deleteRecursively();

        auto panel = cwd.getChildFile("panel.xml");
        panel.create();
        const auto writePanel = [&panel](const juce::String& header) {
            panel.replaceWithText(R"(
                <Component>
                    <Text>)" + header
                                  + R"(</Text>
                    <Component id="body">
                        <Snippet name="content"/>
                    </Component>
                    <Snippet name="footer">
                        <Text>Default footer</Text>
                    </Snippet>
                </Component>
            )");
        };
        writePanel("Header");

        auto mainView = cwd.getChildFile("panels.xml");
        mainView.create();
        mainView.replaceWithText(R"(
            <Window width="100" height="100">
                <Component id="first" template="panel.xml">
                    <Snippet name="content">
                        <Text>Hello, World!</Text>
                        <Text>Line 2</Text>
                    </Snippet>
                </Component>
                <Component id="second" template="panel.xml">
                    <Snippet name="content">Inline text</Snippet>
                    <Snippet name="footer">
                        <Text>Custom footer</Text>
                    </Snippet>
                </Component>
            </Window>
        )");

        jive::Interpreter interpreter;
        const auto item = interpreter.interpret(mainView);
        test.expect(item != nullptr, "Item should have been created");

        const auto expectFilled = [&test, &item](const juce::String& header, const juce::String& message) {
            const auto first = jive::findElementWithID(item->state, "first");
            const auto second = jive::findElementWithID(item->state, "second");

            test.expectEquals(describeChildren(first).joinIntoString(", "),
                              "Text:" + header + ", Component:, Text:Default footer",
                              message + ": snippets without content should be replaced by their own children");
            test.expectEquals(describeChildren(jive::findElementWithID(first, "body")).joinIntoString(", "),
                              juce::String{ "Text:Hello, World!, Text:Line 2" },
                              message + ": snippets should be replaced by the instance's content, however deep they are");
            test.expectEquals(describeChildren(jive::findElementWithID(second, "body")).joinIntoString(", "),
                              juce::String{ "Text:Inline text" },
                              message + ": each instance should get its own content");
            test.expectEquals(describeChildren(second).joinIntoString(", "),
                              "Text:" + header + ", Component:, Text:Custom footer",
                              message + ": a snippet's content should replace the template's default content");

            test.expectEquals(item->getChildren()[0]->getChildren().size(),
                              3,
                              message + ": only the filled-in template should be interpreted");
        };

        expectFilled("Header", "Initial load");

        writePanel("Reloaded");
        jive::FileObserver::triggerAllTimerCallbacks();
        expectFilled("Reloaded", "After reloading the template");

        auto compactPanel = cwd.getChildFile("compact-panel.xml");
        compactPanel.create();
        compactPanel.replaceWithText(R"(<Component><Snippet name="footer"/><Snippet name="content"/></Component>)");
        jive::findElementWithID(item->state, "second").setProperty("template", "compact-panel.xml", nullptr);
        test.expectEquals(describeChildren(jive::findElementWithID(item->state, "second")).joinIntoString(", "),
                          juce::String{ "Text:Custom footer, Text:Inline text" },
                          "Snippets should fill in a template that replaces the previous one");

        cwd.deleteRecursively();
    },
};

static jive::UnitTest templateSnippetsForwardedTest{
    "jive",
    "jive::Interpreter",
    "Template Snippets Forwarded",
    [](auto& test) {
        // A template can pass its own snippets on to a template it uses, and
        // paths in a snippet stay relative to the file it was written in.
        const auto cwd = juce::File::getSpecialLocation(juce::File::currentApplicationFile)
                             .getParentDirectory()
                             .getChildFile("interpreter-tests/");
        cwd.deleteRecursively();

        auto card = cwd.getChildFile("templates/card.xml");
        card.create();
        card.replaceWithText(R"(<Component id="card"><Snippet name="body"/></Component>)");

        auto panel = cwd.getChildFile("templates/panel.xml");
        panel.create();
        panel.replaceWithText(R"(
            <Component>
                <Component template="./card.xml">
                    <Snippet name="body">
                        <Snippet name="content"/>
                    </Snippet>
                </Component>
            </Component>
        )");

        auto label = cwd.getChildFile("views/parts/label.xml");
        label.create();
        label.replaceWithText(R"(<Component id="label"><Component template="./text.xml"/></Component>)");

        auto text = cwd.getChildFile("views/parts/text.xml");
        text.create();
        text.replaceWithText(R"(<Text>Forwarded</Text>)");

        auto mainView = cwd.getChildFile("views/main.xml");
        mainView.create();
        mainView.replaceWithText(R"(
            <Window width="100" height="100">
                <Component template="../templates/panel.xml">
                    <Snippet name="content">
                        <Component template="./parts/label.xml"/>
                    </Snippet>
                </Component>
            </Window>
        )");

        jive::Interpreter interpreter;
        const auto item = interpreter.interpret(mainView);
        test.expect(item != nullptr, "Item should have been created");

        test.expectEquals(describeChildren(jive::findElementWithID(item->state, "card")).joinIntoString(", "),
                          juce::String{ "Component:" },
                          "The content should be passed on to the nested template");
        test.expectEquals(describeChildren(jive::findElementWithID(item->state, "label")).joinIntoString(", "),
                          juce::String{ "Text:Forwarded" },
                          "Paths in a template used by a snippet should be relative to that template");

        cwd.deleteRecursively();
    },
};

static jive::UnitTest fileSourceRetainsChildrenTest{
    "jive",
    "jive::Interpreter",
    "File Source Retains Children",
    [](auto& test) {
        // A component using an external "template" may still declare its own
        // children in-place (e.g. a Button's inline text). Those children
        // should be retained alongside whatever the source tree defines,
        // rather than being wiped out when the source tree is merged in.
        const auto cwd = juce::File::getSpecialLocation(juce::File::currentApplicationFile)
                             .getParentDirectory()
                             .getChildFile("interpreter-tests/");
        cwd.deleteRecursively();

        auto navButton = cwd.getChildFile("nav-button.xml");
        navButton.create();
        navButton.replaceWithText(R"(<Button padding="10"/>)");

        auto mainView = cwd.getChildFile("side-bar.xml");
        mainView.create();
        mainView.replaceWithText(R"(
            <Window width="100" height="100">
                <Button template="nav-button.xml">Layouts</Button>
            </Window>
        )");

        jive::Interpreter interpreter;
        const auto item = interpreter.interpret(mainView);
        test.expect(item != nullptr, "Item should have been created");

        const auto* text = jive::findFirstTextContent(*item);
        test.expect(text != nullptr, "Button's inline text should be retained alongside its external source");

        if (text != nullptr)
        {
            test.expectEquals(text->getTextComponent().getAttributedString().getText(),
                              juce::String{ "Layouts" });
        }
    },
};

static jive::UnitTest fileSourceLiveReloadWithSharedSourceTest{
    "jive",
    "jive::Interpreter",
    "File Source Live Reload With Shared Source",
    [](auto& test) {
        // Replicates the SideBar/NavButton example: several Buttons share the
        // same "template" file, each with their own inline text. Editing the
        // source file should update *every* item that references it, and
        // each item's inline text should survive the reload.
        const auto cwd = juce::File::getSpecialLocation(juce::File::currentApplicationFile)
                             .getParentDirectory()
                             .getChildFile("interpreter-tests/");
        cwd.deleteRecursively();

        auto navButton = cwd.getChildFile("nav-button.xml");
        navButton.create();
        navButton.replaceWithText(R"(<Button padding="10"/>)");

        auto mainView = cwd.getChildFile("side-bar.xml");
        mainView.create();
        mainView.replaceWithText(R"(
            <Window width="100" height="100">
                <Button template="nav-button.xml">Layouts</Button>
                <Button template="nav-button.xml">Styles</Button>
                <Button template="nav-button.xml">Animations</Button>
            </Window>
        )");

        jive::Interpreter interpreter;
        const auto item = interpreter.interpret(mainView);
        test.expect(item != nullptr, "Item should have been created");
        test.expectEquals(static_cast<int>(item->getChildren().size()), 3);

        static const juce::StringArray labels{ "Layouts", "Styles", "Animations" };

        auto getPadding = [&](int index) {
            return static_cast<float>(item->getChildren()[index]->state["padding"]);
        };
        auto getText = [&](int index) -> juce::String {
            const auto* text = jive::findFirstTextContent(*item->getChildren()[index]);
            return text != nullptr
                     ? text->getTextComponent().getAttributedString().getText()
                     : juce::String{};
        };

        for (auto i = 0; i < 3; i++)
        {
            test.expectEquals(getPadding(i), 10.0f, "Initial padding should come from the source file");
            test.expectEquals(getText(i), labels[i], "Initial text should be retained");
        }

        navButton.replaceWithText(R"(<Button padding="20"/>)");
        jive::FileObserver::triggerAllTimerCallbacks();

        for (auto i = 0; i < 3; i++)
        {
            test.expectEquals(getPadding(i),
                              20.0f,
                              "Every button sharing the source should pick up the change, not just the first");
            test.expectEquals(getText(i),
                              labels[i],
                              "Inline text should survive a source file reload");
        }

        navButton.replaceWithText(R"(<Button padding="30"/>)");
        jive::FileObserver::triggerAllTimerCallbacks();

        for (auto i = 0; i < 3; i++)
        {
            test.expectEquals(getPadding(i), 30.0f, "Subsequent reloads should keep applying to every button");
            test.expectEquals(getText(i), labels[i], "Inline text should survive repeated reloads");
        }
    },
};

static jive::UnitTest fileSourceReloadDoesNotDuplicateOwnChildrenTest{
    "jive",
    "jive::Interpreter",
    "File Source Reload Does Not Duplicate Own Children",
    [](auto& test) {
        // Reproduces the jive-demo app: a Window contains a Component whose
        // "template" points at a file (main.xml) that itself declares children
        // of its own (a side-bar and a content area). Editing main.xml
        // shouldn't duplicate those children on every reload.
        const auto cwd = juce::File::getSpecialLocation(juce::File::currentApplicationFile)
                             .getParentDirectory()
                             .getChildFile("interpreter-tests/");
        cwd.deleteRecursively();

        auto mainXml = cwd.getChildFile("main.xml");
        mainXml.create();
        mainXml.replaceWithText(R"(
            <Component flex-direction="row">
                <Component id="side-bar" />
                <Component flex-grow="1" />
            </Component>
        )");

        juce::ValueTree windowTree{
            "Window",
            {
                { "width", 100 },
                { "height", 100 },
            },
            {
                juce::ValueTree{
                    "Component",
                    {
                        { "template", "main.xml" },
                    },
                },
            },
        };

        jive::Interpreter interpreter;
        interpreter.addSourceDirectory(cwd);
        const auto item = interpreter.interpret(windowTree);
        test.expect(item != nullptr, "Item should have been created");
        test.expectEquals(static_cast<int>(item->getChildren()[0]->getChildren().size()),
                          2,
                          "Should start with two children");

        for (auto i = 0; i < 3; i++)
        {
            mainXml.replaceWithText(juce::String(R"(
                <Component flex-direction="row" padding=")")
                                    + juce::String(i)
                                    + R"(">
                    <Component id="side-bar" />
                    <Component flex-grow="1" />
                </Component>
            )");
            jive::FileObserver::triggerAllTimerCallbacks();

            test.expectEquals(static_cast<int>(item->getChildren()[0]->getChildren().size()),
                              2,
                              "Reloading the source file shouldn't duplicate its own children");
        }
    },
};

static jive::UnitTest sourcePropertyChangedTest{
    "jive",
    "jive::Interpreter",
    "Source Property Changed",
    [](auto& test) {
        // Changing an element's "template" at runtime - like an app swapping out
        // the page shown in its content area - should replace that element's
        // content with the newly referenced file's.
        const auto cwd = juce::File::getSpecialLocation(juce::File::currentApplicationFile)
                             .getParentDirectory()
                             .getChildFile("interpreter-tests/");
        cwd.deleteRecursively();

        auto layouts = cwd.getChildFile("layouts.xml");
        layouts.create();
        layouts.replaceWithText(R"(<Component padding="10"><Component id="layouts-page"/></Component>)");

        auto styles = cwd.getChildFile("styles.xml");
        styles.create();
        styles.replaceWithText(R"(<Component><Component id="styles-page"/></Component>)");

        juce::ValueTree windowTree{
            "Window",
            {
                { "width", 100 },
                { "height", 100 },
            },
            {
                juce::ValueTree{
                    "Component",
                    {
                        { "id", "content" },
                    },
                },
            },
        };

        jive::Interpreter interpreter;
        interpreter.addSourceDirectory(cwd);
        const auto item = interpreter.interpret(windowTree);
        test.expect(item != nullptr, "Item should have been created");

        auto page = [&item]() -> juce::String {
            const auto& pages = item->getChildren()[0]->getChildren();

            if (pages.isEmpty())
                return {};

            return pages[0]->getComponent()->getComponentID();
        };

        test.expectEquals(page(), juce::String{}, "Content should start out empty");

        windowTree.getChild(0).setProperty("template", "layouts.xml", nullptr);
        test.expectEquals(page(),
                          juce::String{ "layouts-page" },
                          "Setting the source should load the file's content");

        windowTree.getChild(0).setProperty("template", "styles.xml", nullptr);
        test.expectEquals(page(),
                          juce::String{ "styles-page" },
                          "Changing the source should replace the previous file's content");

        test.expectEquals(item->getChildren()[0]->getChildren().size(),
                          1,
                          "The previous source's content shouldn't be left behind");
        test.expect(!windowTree.getChild(0).hasProperty("padding"),
                    "The previous source's properties shouldn't be left behind");
    },
};

static jive::UnitTest nestedSourceReloadDoesNotDuplicateSiblingTest{
    "jive",
    "jive::Interpreter",
    "Nested Source Reload Does Not Duplicate Siblings",
    [](auto& test) {
        // Reloading a nested source file (page.xml) shouldn't sever the
        // element's ties to the file it was declared in (main.xml) - otherwise
        // the next reload of main.xml would treat the element as inline content
        // and leave it in place alongside its freshly-parsed twin.
        const auto cwd = juce::File::getSpecialLocation(juce::File::currentApplicationFile)
                             .getParentDirectory()
                             .getChildFile("interpreter-tests/");
        cwd.deleteRecursively();

        auto page = cwd.getChildFile("page.xml");
        page.create();
        page.replaceWithText(R"(<Component><Text>Layouts</Text></Component>)");

        auto mainXml = cwd.getChildFile("main.xml");
        mainXml.create();

        auto writeMainXml = [&mainXml](int padding) {
            mainXml.replaceWithText(juce::String{ R"(
                <Component flex-direction="row">
                    <Component id="side-bar" />
                    <Component id="demo" template="page.xml" padding=")" }
                                    + juce::String{ padding }
                                    + R"(" />
                </Component>
            )");
        };
        writeMainXml(20);

        juce::ValueTree windowTree{
            "Window",
            {
                { "width", 100 },
                { "height", 100 },
            },
            {
                juce::ValueTree{
                    "Component",
                    {
                        { "template", "main.xml" },
                    },
                },
            },
        };

        jive::Interpreter interpreter;
        interpreter.addSourceDirectory(cwd);
        const auto item = interpreter.interpret(windowTree);
        test.expect(item != nullptr, "Item should have been created");
        test.expectEquals(static_cast<int>(item->getChildren()[0]->getChildren().size()),
                          2,
                          "Should start with two children");

        page.replaceWithText(R"(<Component><Text class="demo-title">Layouts</Text></Component>)");
        jive::FileObserver::triggerAllTimerCallbacks();

        writeMainXml(25);
        jive::FileObserver::triggerAllTimerCallbacks();

        test.expectEquals(static_cast<int>(item->getChildren()[0]->getChildren().size()),
                          2,
                          "An element whose own source was reloaded shouldn't be duplicated");
        test.expectEquals(static_cast<float>(item->getChildren()[0]->getChildren()[1]->state["padding"]),
                          25.0f,
                          "The surviving element should be the freshly-parsed one");
    },
};

static jive::UnitTest sourceFileDeterminesElementTypeTest{
    "jive",
    "jive::Interpreter",
    "Source File Determines Element Type",
    [](auto& test) {
        // An element's type comes from the root of the file it sources, so a
        // <Component template="icon.svg"/> is really an <svg> - and stays one
        // when the file it was declared in is reloaded.
        const auto cwd = juce::File::getSpecialLocation(juce::File::currentApplicationFile)
                             .getParentDirectory()
                             .getChildFile("interpreter-tests/");
        cwd.deleteRecursively();

        auto icon = cwd.getChildFile("icon.svg");
        icon.create();
        icon.replaceWithText(R"(<svg viewBox="0 0 24 24"><path d="M0,0 L24,24"/></svg>)");

        auto mainXml = cwd.getChildFile("main.xml");
        mainXml.create();

        auto writeMainXml = [&mainXml](int padding) {
            mainXml.replaceWithText(juce::String{ R"(
                <Component padding=")" }
                                    + juce::String{ padding }
                                    + R"(">
                    <Component template="icon.svg" width="24" height="24" />
                </Component>
            )");
        };
        writeMainXml(10);

        juce::ValueTree windowTree{
            "Window",
            {
                { "width", 100 },
                { "height", 100 },
            },
            {
                juce::ValueTree{
                    "Component",
                    {
                        { "template", "main.xml" },
                    },
                },
            },
        };

        jive::Interpreter interpreter;
        interpreter.addSourceDirectory(cwd);
        const auto item = interpreter.interpret(windowTree);
        test.expect(item != nullptr, "Item should have been created");

        auto iconItem = [&item]() -> jive::GuiItem* {
            const auto& icons = item->getChildren()[0]->getChildren();
            return icons.isEmpty() ? nullptr : icons[0];
        };

        test.expect(iconItem() != nullptr, "The icon should have been created");
        test.expectEquals(iconItem()->state.getType().toString(),
                          juce::String{ "svg" },
                          "The element should take its type from its source file's root");
        auto* image = dynamic_cast<jive::GuiItemDecorator&>(*iconItem()).toType<jive::Image>();
        test.expect(image != nullptr, "The element should be treated as an image");
        test.expect(image != nullptr && image->getDrawable().isSVG(),
                    "The element should have found the source file's drawable");

        writeMainXml(15);
        jive::FileObserver::triggerAllTimerCallbacks();

        test.expect(iconItem() != nullptr, "The icon should have survived the reload");
        test.expectEquals(iconItem()->state.getType().toString(),
                          juce::String{ "svg" },
                          "Reloading the file the element was declared in shouldn't change its type");
    },
};

static jive::UnitTest stateBuildCallbackTest{
    "jive",
    "jive::Interpreter",
    "State Build Callback",
    [](auto& test) {
        // Interpreting a file, or reloading any part of it, builds brand-new
        // trees, so clients need to be told when that's happened in order to
        // (re)assign any listeners they'd attached to the old ones.
        const auto cwd = juce::File::getSpecialLocation(juce::File::currentApplicationFile)
                             .getParentDirectory()
                             .getChildFile("interpreter-tests/");
        cwd.deleteRecursively();

        auto heading = cwd.getChildFile("heading.xml");
        heading.create();
        heading.replaceWithText(R"(<Component name="original"/>)");

        auto footer = cwd.getChildFile("footer.xml");
        footer.create();
        footer.replaceWithText(R"(<Component name="footer"/>)");

        auto mainView = cwd.getChildFile("main.xml");
        mainView.create();
        mainView.replaceWithText(R"(
            <Window width="100" height="100">
                <Component id="content" template="heading.xml"/>
            </Window>
        )");

        jive::Interpreter interpreter;
        interpreter.addSourceDirectory(cwd);

        juce::StringArray namesWhenNotified;
        const jive::GuiItem* rootWhenNotified = nullptr;
        interpreter.onLoad = [&](jive::GuiItem& root) {
            namesWhenNotified.add(jive::findElementWithID(root.state, "content")["name"].toString());
            rootWhenNotified = &root;
        };

        const auto item = interpreter.interpret(mainView);
        test.expectEquals(namesWhenNotified.joinIntoString(","),
                          juce::String{ "original" },
                          "Interpreting a file should notify listeners once the tree has been built");
        test.expect(rootWhenNotified == item.get(),
                    "Listeners should be given the item that interpreting returns");

        namesWhenNotified.clear();
        heading.replaceWithText(R"(<Component name="updated"/>)");
        jive::FileObserver::triggerAllTimerCallbacks();
        test.expectEquals(namesWhenNotified.joinIntoString(","),
                          juce::String{ "updated" },
                          "Changing a source file should notify listeners once the new tree is in place");

        namesWhenNotified.clear();
        jive::findElementWithID(item->state, "content")
            .setProperty("template", "footer.xml", nullptr);
        test.expectEquals(namesWhenNotified.size(),
                          1,
                          "Pointing an element at a different source should notify listeners");

        namesWhenNotified.clear();
        mainView.replaceWithText(R"(
            <Window width="200" height="200">
                <Component id="content" name="reloaded" template="heading.xml"/>
            </Window>
        )");
        jive::FileObserver::triggerAllTimerCallbacks();
        test.expectEquals(namesWhenNotified.joinIntoString(","),
                          juce::String{ "reloaded" },
                          "Changing the top-level source file should notify listeners once the new top-level tree is in place");
    },
};

static jive::UnitTest stateBuildCallbackWithoutFileTest{
    "jive",
    "jive::Interpreter",
    "State Build Callback for a tree that wasn't read from a file",
    [](auto& test) {
        jive::Interpreter interpreter;

        auto numNotifications = 0;
        const jive::GuiItem* rootWhenNotified = nullptr;
        interpreter.onLoad = [&](jive::GuiItem& root) {
            numNotifications++;
            rootWhenNotified = &root;
        };

        const auto item = interpreter.interpret(R"(<Window width="100" height="100"/>)");
        test.expectEquals(numNotifications,
                          1,
                          "Interpreting a tree should notify listeners once");
        test.expect(rootWhenNotified == item.get(),
                    "Listeners should be given the item that interpreting returns");
    },
};

static jive::UnitTest externalSourceItemsExistWhenNotifiedTest{
    "jive",
    "jive::Interpreter",
    "Items are in place by the time listeners are notified",
    [](auto& test) {
        // Listeners use the items as well as the tree, for example to attach
        // them to plugin parameters.
        const auto cwd = juce::File::getSpecialLocation(juce::File::currentApplicationFile)
                             .getParentDirectory()
                             .getChildFile("interpreter-items-tests/");
        cwd.deleteRecursively();

        auto first = cwd.getChildFile("first.xml");
        first.create();
        first.replaceWithText(R"(<Component><Component id="inner"/></Component>)");

        auto second = cwd.getChildFile("second.xml");
        second.create();
        second.replaceWithText(R"(<Component><Component id="inner"/></Component>)");

        auto mainView = cwd.getChildFile("main.xml");
        mainView.create();
        mainView.replaceWithText(R"(
            <Window width="100" height="100">
                <Component id="content" template="first.xml"/>
            </Window>
        )");

        jive::Interpreter interpreter;
        interpreter.addSourceDirectory(cwd);

        juce::StringArray itemsWhenNotified;
        interpreter.onLoad = [&](jive::GuiItem& root) {
            auto* inner = jive::findItemWithID(root, "inner");
            const auto isInPlace = inner != nullptr && inner->state == jive::findElementWithID(root.state, "inner");
            itemsWhenNotified.add(isInPlace ? "in place" : "missing");
        };

        const auto item = interpreter.interpret(mainView);
        test.expectEquals(itemsWhenNotified.joinIntoString(","),
                          juce::String{ "in place" },
                          "Interpreting the file");

        itemsWhenNotified.clear();
        jive::findElementWithID(item->state, "content").setProperty("template", "second.xml", nullptr);
        test.expectEquals(itemsWhenNotified.joinIntoString(","),
                          juce::String{ "in place" },
                          "Pointing an element at a different source");

        itemsWhenNotified.clear();
        second.replaceWithText(R"(<Component><Component id="inner" name="updated"/></Component>)");
        jive::FileObserver::triggerAllTimerCallbacks();
        test.expectEquals(itemsWhenNotified.joinIntoString(","),
                          juce::String{ "in place" },
                          "Changing the source file on disk");

        itemsWhenNotified.clear();
        mainView.replaceWithText(R"(
            <Window width="200" height="200">
                <Component id="content" template="second.xml"/>
            </Window>
        )");
        jive::FileObserver::triggerAllTimerCallbacks();
        test.expectEquals(itemsWhenNotified.joinIntoString(","),
                          juce::String{ "in place" },
                          "Changing the top-level file on disk");
    },
};

[[nodiscard]] static juce::File createLocalisationTestDirectory()
{
    const auto directory = juce::File::getSpecialLocation(juce::File::currentApplicationFile)
                               .getParentDirectory()
                               .getChildFile("localisation-tests/");
    directory.deleteRecursively();
    directory.createDirectory();

    directory.getChildFile("french.strings").replaceWithText(R"(language: French
countries: fr

"greeting" = "Bonjour"
"ok" = "D'accord"
"fruit" = "Fruits"
"apple" = "Pomme"
"pick-a-fruit" = "Choisissez un fruit"
"say-hello" = "Dire bonjour"
)");
    directory.getChildFile("german.strings").replaceWithText(R"(language: German
countries: de

"greeting" = "Hallo"
"ok" = "OK"
)");

    return directory;
}

[[nodiscard]] static juce::String getDisplayedText(jive::GuiItem& root, const juce::Identifier& id)
{
    const auto* item = jive::findItemWithID(root, id);

    if (item == nullptr)
        return "<no item with ID " + id.toString() + ">";

    return dynamic_cast<const jive::TextComponent&>(*item->getComponent())
        .getAttributedString()
        .getText();
}

static jive::UnitTest localisedTextTest{
    "jive",
    "jive::Interpreter",
    "Localisation: text is translated with the strings file named by the closest lang",
    [](auto& test) {
        jive::Interpreter interpreter;
        interpreter.addSourceDirectory(createLocalisationTestDirectory());
        auto item = interpreter.interpret(jive::parseXML(R"(
            <Component width="200" height="200" lang="french.strings">
                <Text id="greeting">greeting</Text>
                <Button><Text id="button-text">ok</Text></Button>
                <Text id="untranslated">Goodbye</Text>
                <Text id="sentence">greeting<Text>ok</Text></Text>
                <Component lang="german.strings">
                    <Text id="german-greeting">greeting</Text>
                </Component>
                <Component lang="missing.strings">
                    <Text id="missing-greeting">greeting</Text>
                </Component>
            </Component>
        )"));

        test.expectEquals(getDisplayedText(*item, "greeting"), juce::String{ "Bonjour" });
        test.expectEquals(getDisplayedText(*item, "button-text"), juce::String{ "D'accord" });
        test.expectEquals(getDisplayedText(*item, "untranslated"), juce::String{ "Goodbye" });
        test.expectEquals(getDisplayedText(*item, "sentence"), juce::String{ "BonjourD'accord" });
        test.expectEquals(getDisplayedText(*item, "german-greeting"), juce::String{ "Hallo" });
        test.expectEquals(getDisplayedText(*item, "missing-greeting"), juce::String{ "Bonjour" });
        test.expectEquals(jive::findElementWithID(item->state, "greeting")["text"].toString(),
                          juce::String{ "greeting" },
                          "The tree should keep the untranslated text");
    },
};

static jive::UnitTest changingLangTest{
    "jive",
    "jive::Interpreter",
    "Localisation: changing lang translates text again",
    [](auto& test) {
        jive::Interpreter interpreter;
        interpreter.addSourceDirectory(createLocalisationTestDirectory());
        auto item = interpreter.interpret(jive::parseXML(R"(
            <Component width="200" height="200" lang="french.strings">
                <Component>
                    <Text id="greeting">greeting</Text>
                    <Text id="sentence">greeting<Text>ok</Text></Text>
                </Component>
            </Component>
        )"));
        test.expectEquals(getDisplayedText(*item, "greeting"), juce::String{ "Bonjour" });

        item->state.setProperty("lang", "german.strings", nullptr);
        test.expectEquals(getDisplayedText(*item, "greeting"), juce::String{ "Hallo" });
        test.expectEquals(getDisplayedText(*item, "sentence"), juce::String{ "HalloOK" });

        item->state.removeProperty("lang", nullptr);
        test.expectEquals(getDisplayedText(*item, "greeting"), juce::String{ "greeting" });
    },
};

static jive::UnitTest localisedWidgetsTest{
    "jive",
    "jive::Interpreter",
    "Localisation: combo box items, tooltips and titles are translated",
    [](auto& test) {
        jive::Interpreter interpreter;
        interpreter.addSourceDirectory(createLocalisationTestDirectory());
        auto item = interpreter.interpret(jive::parseXML(R"(
            <Component width="200" height="200" lang="french.strings">
                <ComboBox id="combo-box" tooltip="pick-a-fruit">
                    <Header>fruit</Header>
                    <Option>apple</Option>
                </ComboBox>
                <Button id="button" tooltip="say-hello" description="say-hello">ok</Button>
            </Component>
        )"));

        auto& comboBox = dynamic_cast<juce::ComboBox&>(*jive::findItemWithID(*item, "combo-box")->getComponent());
        const auto getHeaderText = [&comboBox] {
            for (juce::PopupMenu::MenuItemIterator iterator{ *comboBox.getRootMenu() }; iterator.next();)
            {
                if (iterator.getItem().isSectionHeader)
                    return iterator.getItem().text;
            }

            return juce::String{};
        };
        test.expectEquals(comboBox.getItemText(0), juce::String{ "Pomme" });
        test.expectEquals(getHeaderText(), juce::String{ "Fruits" });
        test.expectEquals(comboBox.getTooltip(), juce::String{ "Choisissez un fruit" });

        auto& button = dynamic_cast<juce::Button&>(*jive::findItemWithID(*item, "button")->getComponent());
        test.expectEquals(button.getTooltip(), juce::String{ "Dire bonjour" });
        test.expectEquals(button.getHelpText(), juce::String{ "Dire bonjour" });
        test.expectEquals(button.getDescription(), juce::String{ "Dire bonjour" });
        test.expectEquals(button.getTitle(), juce::String{ "D'accord" });

        item->state.removeProperty("lang", nullptr);
        test.expectEquals(comboBox.getItemText(0), juce::String{ "apple" });
        test.expectEquals(getHeaderText(), juce::String{ "fruit" });
        test.expectEquals(comboBox.getTooltip(), juce::String{ "pick-a-fruit" });
        test.expectEquals(button.getTooltip(), juce::String{ "say-hello" });
        test.expectEquals(button.getTitle(), juce::String{ "ok" });
    },
};

static jive::UnitTest stringsFileReloadTest{
    "jive",
    "jive::Interpreter",
    "Localisation: editing a strings file translates text again",
    [](auto& test) {
        const auto directory = createLocalisationTestDirectory();
        jive::Interpreter interpreter;
        interpreter.addSourceDirectory(directory);
        auto item = interpreter.interpret(jive::parseXML(R"(
            <Component width="200" height="200" lang="french.strings">
                <Text id="greeting">greeting</Text>
            </Component>
        )"));
        test.expectEquals(getDisplayedText(*item, "greeting"), juce::String{ "Bonjour" });

        const auto file = directory.getChildFile("french.strings");
        file.replaceWithText(R"("greeting" = "Salut")");
        file.setLastModificationTime(juce::Time::getCurrentTime() + juce::RelativeTime::seconds(10.0));
        jive::FileObserver::triggerAllTimerCallbacks();
        test.expectEquals(getDisplayedText(*item, "greeting"), juce::String{ "Salut" });
    },
};

static jive::UnitTest embeddedSourcesTest{
    "jive",
    "jive::Interpreter",
    "Embedded sources: views, nested sources and strings files are read from embedded zips",
    [](auto& test) {
        const auto zip = jive::createZip({
            { "views/Main.xml", R"(
                <Component width="200" height="200" lang="views/languages/english.strings">
                    <Component template="views/Child.xml"/>
                </Component>
            )" },
            { "views/Child.xml", R"(<Component><Text id="greeting">greeting</Text></Component>)" },
            { "views/languages/english.strings", R"("greeting" = "Hello")" },
        });
        const jive::SourceRoot root{ zip.getData(), zip.getSize() };

        const jive::SourceFile mainView{ "views/Main.xml" };

        if (!mainView.exists())
        {
            test.expect(false, "The main view should be embedded");
            return;
        }

        jive::Interpreter interpreter;
        auto item = interpreter.interpret(mainView);
        test.expectEquals(getDisplayedText(*item, "greeting"), juce::String{ "Hello" });
    },
};

static jive::UnitTest relativeSourceReloadTest{
    "jive",
    "jive::Interpreter",
    "Relative sources: views found in a directory root are reloaded when they change",
    [](auto& test) {
        const auto directory = juce::File::getSpecialLocation(juce::File::currentApplicationFile)
                                   .getParentDirectory()
                                   .getChildFile("relative-sources-tests/");
        directory.deleteRecursively();
        directory.getChildFile("views").createDirectory();

        const auto mainView = directory.getChildFile("views/Main.xml");
        mainView.replaceWithText(R"(<Window width="200" height="200"><Component template="views/Child.xml"/></Window>)");
        const auto childView = directory.getChildFile("views/Child.xml");
        childView.replaceWithText(R"(<Component><Text id="greeting">Hello</Text></Component>)");

        const jive::SourceRoot root{ directory };
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(jive::SourceFile{ "views/Main.xml" });
        test.expectEquals(getDisplayedText(*item, "greeting"), juce::String{ "Hello" });

        childView.replaceWithText(R"(<Component><Text id="greeting">Goodbye</Text></Component>)");
        childView.setLastModificationTime(juce::Time::getCurrentTime() + juce::RelativeTime::seconds(10.0));
        jive::FileObserver::triggerAllTimerCallbacks();
        test.expectEquals(getDisplayedText(*item, "greeting"), juce::String{ "Goodbye" });

        mainView.replaceWithText(R"(<Window width="200" height="200"><Text id="greeting">Reloaded</Text></Window>)");
        mainView.setLastModificationTime(juce::Time::getCurrentTime() + juce::RelativeTime::seconds(20.0));
        jive::FileObserver::triggerAllTimerCallbacks();
        test.expectEquals(getDisplayedText(*item, "greeting"), juce::String{ "Reloaded" });

        directory.deleteRecursively();
    },
};

static jive::UnitTest rootRelativeSourcesTest{
    "jive",
    "jive::Interpreter",
    "Relative sources: paths in a view are relative to the root, not to the view",
    [](auto& test) {
        const auto zip = jive::createZip({
            { "views/Main.xml", R"(
                <Component width="200" height="200">
                    <Component template="Child.xml"/>
                </Component>
            )" },
            { "views/Child.xml", R"(<Component><Text id="greeting">Hello</Text></Component>)" },
        });
        const jive::SourceRoot root{ zip.getData(), zip.getSize() };

        jive::Interpreter interpreter;
        auto item = interpreter.interpret(jive::SourceFile{ "views/Main.xml" });
        test.expect(jive::findItemWithID(*item, "greeting") == nullptr);
    },
};

static jive::UnitTest fileRelativeSourcesTest{
    "jive",
    "jive::Interpreter",
    "Relative sources: paths starting with ./ or ../ are relative to the file they're in",
    [](auto& test) {
        const auto zip = jive::createZip({
            { "views/Main.xml", R"(
                <Component width="200" height="200" lang="../locales/english.strings">
                    <Component template="./partials/Child.xml"/>
                </Component>
            )" },
            { "views/partials/Child.xml", R"(<Component><Component template="./Grandchild.xml"/></Component>)" },
            { "views/partials/Grandchild.xml", R"(<Component><Text id="greeting">greeting</Text></Component>)" },
            { "locales/english.strings", R"("greeting" = "Hello")" },
        });
        const jive::SourceRoot root{ zip.getData(), zip.getSize() };

        jive::Interpreter interpreter;
        auto item = interpreter.interpret(jive::SourceFile{ "views/Main.xml" });
        test.expectEquals(getDisplayedText(*item, "greeting"), juce::String{ "Hello" });
    },
};

class ViewRendererUnitTest : public juce::UnitTest
{
public:
    ViewRendererUnitTest()
        : juce::UnitTest{ "jive::Interpreter", "jive" }
    {
    }

    void runTest() final
    {
        testNestedComponents();
        testDisplayTypes();
        testInitialLayout();
        testWindowContent();
        testCustomDecorators();
        testInterpretingDifferentSources();
        testInterpretingContentAndContainers();
        testListening();
    }

private:
    void testNestedComponents()
    {
        beginTest("nested components");

        jive::Interpreter interpreter;

        {
            auto view = interpreter.interpret(juce::ValueTree{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            });

            expectEquals(view->getChildren().size(), 0);
            expectEquals(view->getComponent()->getNumChildComponents(), 0);
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
                {
                    juce::ValueTree{ "Component" },
                    juce::ValueTree{ "Component" },
                },
            };
            auto view = interpreter.interpret(tree);

            expect(view->getChildren().size() == tree.getNumChildren());
            expect(view->getComponent()->getNumChildComponents() == tree.getNumChildren());
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
                {
                    juce::ValueTree{
                        "Component",
                        {},
                        {
                            juce::ValueTree{ "Component" },
                            juce::ValueTree{ "Component" },
                            juce::ValueTree{ "Component" },
                        },
                    },
                },
            };
            auto view = interpreter.interpret(tree);

            expect(view->getChildren().size() == tree.getNumChildren());
            expect(view->getComponent()->getNumChildComponents() == tree.getNumChildren());

            expect(view->getChildren()[0]->getChildren().size() == tree.getChild(0).getNumChildren());
            expect(view->getChildren()[0]->getComponent()->getNumChildComponents() == tree.getChild(0).getNumChildren());
        }
    }

    void testDisplayTypes()
    {
        beginTest("display");

        jive::Interpreter interpreter;

        auto basicView = interpreter.interpret(juce::ValueTree{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
            },
        });
        expect(dynamic_cast<jive::GuiItemDecorator&>(*basicView)
                   .toType<jive::CommonGuiItem>()
               != nullptr);

        auto flexView = interpreter.interpret(juce::ValueTree{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
                { "display", juce::VariantConverter<jive::Display>::toVar(jive::Display::flex) },
            },
            {
                juce::ValueTree{ "Label" },
            },
        });
        expect(dynamic_cast<jive::GuiItemDecorator&>(*flexView)
                   .toType<jive::FlexContainer>()
               != nullptr);
        expect(dynamic_cast<jive::GuiItemDecorator&>(*flexView->getChildren()[0])
                   .toType<jive::FlexItem>()
               != nullptr);

        auto gridView = interpreter.interpret(juce::ValueTree{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
                { "display", "grid" },
            },
            {
                juce::ValueTree{ "Component" },
            },
        });
        expect(dynamic_cast<jive::GuiItemDecorator&>(*gridView)
                   .toType<jive::GridContainer>()
               != nullptr);
        expect(dynamic_cast<jive::GuiItemDecorator&>(*gridView->getChildren()[0])
                   .toType<jive::GridItem>()
               != nullptr);

        auto blockView = interpreter.interpret(juce::ValueTree{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
                { "display", "block" },
            },
            {
                juce::ValueTree{ "Component" },
            },
        });
        expect(dynamic_cast<jive::GuiItemDecorator&>(*blockView)
                   .toType<jive::BlockContainer>()
               != nullptr);
        expect(dynamic_cast<jive::GuiItemDecorator&>(*blockView->getChildren()[0])
                   .toType<jive::BlockItem>()
               != nullptr);
    }

    void testInitialLayout()
    {
        beginTest("initial layout");

        jive::Interpreter interpreter;
        const auto view = interpreter.interpret(juce::ValueTree{
            "Component",
            {
                { "width", 200 },
                { "height", 200 },
            },
            {
                {
                    "Component",
                    {
                        { "width", 100 },
                        { "height", 100 },
                    },
                },
                {
                    "Component",
                    {
                        { "width", 100 },
                        { "height", 100 },
                    },
                },
            },
        });
        expectNotEquals(view->getChildren()[1]->getComponent()->getPosition(),
                        juce::Point<int>{});
    }

    void testWindowContent()
    {
        beginTest("window content");

        jive::Interpreter interpreter;
        const auto view = interpreter.interpret(juce::ValueTree{
            "Window",
            {
                { "width", 200 },
                { "height", 200 },
            },
            {
                {
                    "Component",
                    {
                        { "width", 100 },
                        { "height", 100 },
                    },
                    {},
                },
            },
        });
        auto& window = *dynamic_cast<jive::GuiItemDecorator&>(*view)
                            .toType<jive::Window>();
        expect(window.getContentComponent() != nullptr);
    }

    void testCustomDecorators()
    {
        beginTest("custom decorators");

        struct MyDecorator : public jive::GuiItemDecorator
        {
            using jive::GuiItemDecorator::GuiItemDecorator;
        };

        jive::Interpreter interpreter;
        auto item = interpreter.interpret(juce::ValueTree{
            "Button",
            {
                { "width", 222 },
                { "height", 333 },
            },
        });
        expect(dynamic_cast<MyDecorator*>(item.get()) == nullptr);

        interpreter.addDecorator<MyDecorator>("Button");
        item = interpreter.interpret(juce::ValueTree{
            "Button",
            {
                { "width", 222 },
                { "height", 333 },
            },
        });
        auto* decorator = dynamic_cast<jive::GuiItemDecorator*>(item.get());
        expect(decorator->toType<MyDecorator>() != nullptr);

        struct MyOtherDecorator : public jive::GuiItemDecorator
        {
            using jive::GuiItemDecorator::GuiItemDecorator;
        };
        interpreter.addDecorator<MyOtherDecorator>("Button");
        item = interpreter.interpret(juce::ValueTree{
            "Button",
            {
                { "width", 222 },
                { "height", 333 },
            },
        });
        decorator = dynamic_cast<jive::GuiItemDecorator*>(item.get());
        expect(decorator->toType<MyDecorator>() != nullptr);
        expect(decorator->toType<MyOtherDecorator>() != nullptr);
    }

    void testInterpretingDifferentSources()
    {
        {
            beginTest("interpreting juce::ValueTree");

            jive::Interpreter interpreter;
            const auto result = interpreter.interpret(juce::ValueTree{
                "Component",
                {
                    { "width", 123 },
                    { "height", 456 },
                },
            });
            expect(result != nullptr);
        }
        {
            beginTest("interpreting juce::XmlElement");

            jive::Interpreter interpreter;
            const auto result = interpreter.interpret(*juce::parseXML(R"(
                <Component width="123" height="456"/>
            )"));
            expect(result != nullptr);
        }
        {
            beginTest("interpreting juce::String");

            jive::Interpreter interpreter;
            const auto result = interpreter.interpret(R"(
                <Component width="123" height="456"/>
            )");
            expect(result != nullptr);
        }
        {
            beginTest("interpreting string data");

            jive::Interpreter interpreter;
            const juce::String source = R"(
                <Component width="123" height="456"/>
            )";
            const auto result = interpreter.interpret(source.toRawUTF8(), source.length());
            expect(result != nullptr);
        }
    }

    void testInterpretingContentAndContainers()
    {
        beginTest("Content and containers");

        jive::Interpreter interpreter;

        static const juce::ValueTree state{
            "Component",
            {
                { "width", 100 },
                { "height", 100 },
            },
            {
                juce::ValueTree{ "Component" },
                juce::ValueTree{
                    "Text",
                    {},
                    {
                        juce::ValueTree{
                            "Text",
                            {
                                { "text", "first" },
                            },
                        },
                        juce::ValueTree{
                            "Text",
                            {
                                { "text", "second" },
                            },
                        },
                        juce::ValueTree{ "Component" },
                    },
                },
            },
        };
        const auto item = interpreter.interpret(state);
        expectEquals(item->getChildren().size(), 2);
        expectEquals(item->getChildren()[1]->getChildren().size(), 2);
        expectEquals<juce::String>(item->getChildren()[1]->getChildren()[0]->state["text"], "first");
        expectEquals<juce::String>(item->getChildren()[1]->getChildren()[1]->state["text"], "second");
    }

    void testListening()
    {
        beginTest("listening");

        jive::Interpreter interpreter;
        auto item = interpreter.interpret(juce::ValueTree{
            "Component",
            {
                { "width", 100 },
                { "height", 100 },
            },
            {
                juce::ValueTree{ "Component" },
            },
        });
        expectEquals(item->getChildren().size(), 1);

        interpreter.listenTo(*item);
        item->state.appendChild(juce::ValueTree{ "Component" }, nullptr);
        expectEquals(item->getChildren().size(), 2);
    }
};

static ViewRendererUnitTest viewRendererUnitTest;
