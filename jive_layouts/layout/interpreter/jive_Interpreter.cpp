#include "jive_Interpreter.h"

#include <jive_layouts/layout/gui-items/jive_ContainerItem.h>
#include <jive_layouts/layout/gui-items/top-level/jive_PluginEditor.h>
#include <jive_layouts/layout/gui-items/top-level/jive_Window.h>

namespace jive
{
    const ComponentFactory& Interpreter::getComponentFactory() const
    {
        return componentFactory;
    }

    ComponentFactory& Interpreter::getComponentFactory()
    {
        return componentFactory;
    }

    void Interpreter::setComponentFactory(const ComponentFactory& newFactory)
    {
        componentFactory = newFactory;
    }

    static void decorateWithTopLevelItem(std::unique_ptr<GuiItem>& item,
                                         [[maybe_unused]] juce::AudioProcessor* pluginProcessor)
    {
        if (item->state.getType().toString() == "Window")
            item = std::make_unique<Window>(std::move(item));
#if JIVE_IS_PLUGIN_PROJECT
        else if (item->state.getType().toString() == "Editor")
            item = std::make_unique<PluginEditor>(std::move(item), pluginProcessor);
#endif
    }

    std::unique_ptr<GuiItem> Interpreter::interpret(const juce::ValueTree& tree, juce::AudioProcessor* pluginProcessor)
    {
        auto item = interpretInternal(tree, nullptr);
        decorateWithTopLevelItem(item, pluginProcessor);
        listenTo(*item);
        return item;
    }

    std::unique_ptr<GuiItem> Interpreter::interpret(const juce::XmlElement& xml, juce::AudioProcessor* pluginProcessor)
    {
        return interpret(parseXML(xml), pluginProcessor);
    }

    std::unique_ptr<GuiItem> Interpreter::interpret(const juce::String& xmlString, juce::AudioProcessor* pluginProcessor)
    {
        return interpret(jive::parseXML(xmlString), pluginProcessor);
    }

    std::unique_ptr<GuiItem> Interpreter::interpret(const void* xmlStringData, int xmlStringDataSize, juce::AudioProcessor* pluginProcessor)
    {
        return interpret(parseXML(xmlStringData, xmlStringDataSize), pluginProcessor);
    }

    juce::ValueTree Interpreter::parseFileToValueTree(const juce::File& file) const
    {
        auto sourceTree = jive::parseXML(file.loadFileAsString());

        if (sourceTree.isValid())
            sourceTree.setProperty("jive::source-directories", sourceDirectories.get(), nullptr);

        return sourceTree;
    }

    [[nodiscard]] static GuiItem* findItemWithSource(GuiItem* root, const juce::File& file)
    {
        if (root == nullptr)
            return nullptr;

        if (file.getFullPathName().contains(root->state.getProperty("source", "jive::no-source-found").toString()))
            return root;
        if (file.getFullPathName() == root->state.getProperty("jive::top-level-source-file", "jive::no-source-found").toString())
            return root;

        for (auto* child : root->getChildren())
        {
            if (auto* item = findItemWithSource(child, file))
                return item;
        }

        return nullptr;
    }

    void Interpreter::onObservedFileChanged(const juce::File& file)
    {
        auto source = parseFileToValueTree(file);

        if (!source.isValid())
        {
            jassertfalse;
            return;
        }

        if (auto* item = findItemWithSource(observedItem.get(), file))
        {
            for (auto i = 0; i < item->state.getNumProperties(); i++)
            {
                const auto name = item->state.getPropertyName(i).toString();

                if (name.startsWith("jive::"))
                    continue;
                if (source.hasProperty(name))
                    continue;

                source.setProperty(name, item->state[name], nullptr);
            }

            if (item->isTopLevel())
            {
                auto* topLevel = dynamic_cast<TopLevelGuiItem*>(item);
                jassert(topLevel != nullptr);
                topLevel->replaceDecoratedItem(interpretInternal(source, nullptr));
            }
            else
            {
                auto* parent = item->getParent();
                jassert(parent != nullptr);

                const auto index = parent->state.indexOf(item->state);
                parent->state.removeChild(item->state, nullptr);
                parent->state.addChild(source, index, nullptr);
            }
        }
    }

    std::unique_ptr<GuiItem> Interpreter::interpret(const juce::File& file,
                                                    juce::AudioProcessor* pluginProcessor)
    {
        addSourceDirectory(file.getParentDirectory());
        auto item = interpret(parseFileToValueTree(file), pluginProcessor);
        listenTo(*item);

        item->state.setProperty("jive::top-level-source-file", file.getFullPathName(), nullptr);

        auto* observer = fileObservers.add(std::make_unique<FileObserver>(file));
        observer->onFileModified = [this, f = file]() {
            onObservedFileChanged(f);
        };

        return item;
    }

    void Interpreter::listenTo(GuiItem& item)
    {
        if (observedItem != nullptr)
            observedItem->state.removeListener(this);

        observedItem = &item;
        observedItem->state.addListener(this);
    }

    void Interpreter::addSourceDirectory(const juce::File& directory)
    {
        sourceDirectories->add(directory);
    }

    void Interpreter::removeSourceDirectory(const juce::File& directory)
    {
        sourceDirectories->remove(directory);
    }

    [[nodiscard]] static GuiItem* findItem(GuiItem& root, const juce::ValueTree& state)
    {
        if (root.state == state)
            return &root;

        for (auto* const child : root.getChildren())
        {
            if (auto item = findItem(*child, state))
                return item;
        }

        return nullptr;
    }

    void Interpreter::valueTreeChildAdded(juce::ValueTree& parentTree,
                                          juce::ValueTree& childWhichHasBeenAdded)
    {
        if (observedItem == nullptr || ignoreValueTreeChanges)
            return;

        auto* parentItem = findItem(*observedItem, parentTree);

        if (parentItem == nullptr)
        {
            jassertfalse;
            return;
        }

        if (auto newChild = interpretInternal(childWhichHasBeenAdded, parentItem))
        {
            childWhichHasBeenAdded.setProperty("jive::source-directories", sourceDirectories.get(), nullptr);
            const auto indexInParent = parentTree.indexOf(childWhichHasBeenAdded);
            parentItem->insertChild(std::move(newChild), indexInParent);
        }
    }

    void Interpreter::valueTreeChildRemoved(juce::ValueTree& parentTree,
                                            juce::ValueTree& childWhichHasBeenRemoved,
                                            int)
    {
        if (observedItem == nullptr || ignoreValueTreeChanges)
            return;

        auto* parentItem = findItem(*observedItem, parentTree);

        if (parentItem == nullptr)
        {
            jassertfalse;
            return;
        }

        GuiItem* childToRemove = nullptr;

        for (auto* child : parentItem->getChildren())
        {
            if (child->state == childWhichHasBeenRemoved)
            {
                childToRemove = child;
                break;
            }
        }

        if (childToRemove == nullptr)
            return;

        parentItem->removeChild(*childToRemove);
    }

    void Interpreter::valueTreeRedirected(juce::ValueTree& treeWhichHasBeenChanged)
    {
        if (observedItem == nullptr || ignoreValueTreeChanges)
            return;

        auto parentTree = treeWhichHasBeenChanged.getParent();
        auto* parentItem = findItem(*observedItem, parentTree);

        if (parentItem == nullptr)
            return;

        GuiItem* existingChild = nullptr;

        for (auto* child : parentItem->getChildren())
        {
            if (child->state == treeWhichHasBeenChanged)
            {
                existingChild = child;
                break;
            }
        }

        if (existingChild != nullptr)
            parentItem->removeChild(*existingChild);

        if (auto newChild = interpretInternal(treeWhichHasBeenChanged, parentItem))
        {
            treeWhichHasBeenChanged.setProperty("jive::source-directories", sourceDirectories.get(), nullptr);
            const auto indexInParent = parentTree.indexOf(treeWhichHasBeenChanged);
            parentItem->insertChild(std::move(newChild), indexInParent);
        }
    }

    std::unique_ptr<GuiItem> Interpreter::interpretInternal(juce::ValueTree tree,
                                                            GuiItem* const parent)
    {
        loadExternalSources(tree);

        auto item = createUndecoratedItem(tree, parent);

        if (item != nullptr)
        {
            tree.setProperty("jive::setup-in-progress", true, nullptr);
            item = decorate(std::move(item), customDecorators);
            setChildItems(*item);

            tree.setProperty("jive::setup-in-progress", false, nullptr);

            if (item->isTopLevel())
            {
                setupItemsRecursive(*item);

                if (auto* container = dynamic_cast<GuiItemDecorator&>(*item)
                                          .getTopLevelDecorator()
                                          .toType<ContainerItem>())
                {
                    container->updateIdealSize();
                }
            }
        }

        return item;
    }

    std::unique_ptr<GuiItem> Interpreter::createUndecoratedItem(const juce::ValueTree& tree,
                                                                GuiItem* const parent)
    {
        if (auto component = createComponent(tree, parent))
        {
            return std::make_unique<GuiItem>(component,
                                             tree,
                                             parent);
        }

        return nullptr;
    }

    std::shared_ptr<juce::Component> Interpreter::createComponent(const juce::ValueTree& tree, const GuiItem* parent) const
    {
        if (auto* viewObject = dynamic_cast<jive::View*>(tree["view-object"].getObject()))
        {
            if (auto component = viewObject->createComponent(tree))
                return component;
        }

        for (auto* ancestor = parent; ancestor != nullptr; ancestor = ancestor->getParent())
        {
            if (auto component = ancestor->getView()->createComponent(tree))
                return component;
        }

        return componentFactory.create(tree.getType());
    }

    void Interpreter::setupItemsRecursive(GuiItem& item) const
    {
        for (auto* child : item.getChildren())
            setupItemsRecursive(*child);

        if (auto view = item.getView(); view != nullptr)
            view->setup(item);
    }

    void Interpreter::loadExternalSources(juce::ValueTree tree)
    {
        auto file = sourceDirectories->find(tree["source"].toString());

        if (!file.existsAsFile())
            return;

        const juce::ScopedValueSetter svs{ ignoreValueTreeChanges, true };

        auto newTree = parseFileToValueTree(file);

        for (auto i = 0; i < tree.getNumProperties(); i++)
        {
            const auto name = tree.getPropertyName(i);
            newTree.setProperty(name, tree[name], nullptr);
        }

        tree.copyPropertiesAndChildrenFrom(newTree, nullptr);

        auto* observer = fileObservers.add(std::make_unique<FileObserver>(file));
        observer->onFileModified = [this, f = file]() {
            onObservedFileChanged(f);
        };
    }

    void Interpreter::setChildItems(GuiItem& item)
    {
        std::vector<std::unique_ptr<GuiItem>> children;

        for (auto i = 0; i < item.state.getNumChildren(); i++)
        {
            if (auto child = interpretInternal(item.state.getChild(i), &item);
                child != nullptr)
            {
                if (item.isContainer() || child->isContent())
                    children.push_back(std::move(child));
            }
        }

        item.setChildren(std::move(children));
    }
} // namespace jive
