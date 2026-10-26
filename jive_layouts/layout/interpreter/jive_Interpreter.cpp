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
        interpretationsInProgress++;

        auto item = interpretInternal(tree, nullptr);
        decorateWithTopLevelItem(item, pluginProcessor);
        listenTo(*item);
        notificationPending = true;

        if (--interpretationsInProgress == 0)
            notifyStateBuild();

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

    juce::ValueTree Interpreter::parseFileToValueTree(const SourceFile& file) const
    {
        auto sourceTree = jive::parseXML(file.loadAsString());

        if (sourceTree.isValid())
            sourceTree.setProperty(ids::sourceFile(), file.getPath(), nullptr);

        return sourceTree;
    }

    static void findItemsWithSource(GuiItem* root, const SourceFile& file, std::vector<GuiItem*>& results)
    {
        if (root == nullptr)
            return;

        if (file.getPath().contains(root->state.getProperty(ids::source(), "jive::no-source-found").toString())
            || file.getPath() == root->state.getProperty(ids::topLevelSourceFile(), "jive::no-source-found").toString())
        {
            // Don't descend into an item that's about to be entirely replaced.
            results.push_back(root);
            return;
        }

        for (auto* child : root->getChildren())
            findItemsWithSource(child, file, results);
    }

    static void rememberTopLevelSourceProperties(juce::ValueTree& tree)
    {
        juce::Array<juce::var> names;

        for (auto i = 0; i < tree.getNumProperties(); i++)
            names.add(tree.getPropertyName(i).toString());

        tree.setProperty(ids::topLevelSourceProperties(), names, nullptr);
    }

    void Interpreter::onObservedFileChanged(const SourceFile& file)
    {
        std::vector<GuiItem*> items;
        findItemsWithSource(observedItem.get(), file, items);

        for (auto* item : items)
        {
            auto source = parseFileToValueTree(file);

            if (!source.isValid())
            {
                jassertfalse;
                continue;
            }

            if (item->isTopLevel())
                rememberTopLevelSourceProperties(source);

            // These children came straight from the source file, so mark
            // them as such before any inline children are appended below -
            // otherwise the reinterpretation triggered by re-parenting this
            // tree (see valueTreeChildAdded) would mistake them for inline
            // content and re-append them a second time.
            for (auto i = 0; i < source.getNumChildren(); i++)
                source.getChild(i).setProperty(ids::externalSourceChild(), true, nullptr);

            const auto* previousSourceProperties = item->state[ids::topLevelSourceProperties()].getArray();

            for (auto i = 0; i < item->state.getNumProperties(); i++)
            {
                const auto name = item->state.getPropertyName(i).toString();

                if (name.startsWith("jive::"))
                    continue;
                if (source.hasProperty(name))
                    continue;
                if (previousSourceProperties != nullptr && previousSourceProperties->contains(name))
                    continue;

                source.setProperty(name, item->state[name], nullptr);
            }

            // Reloading this item's own source file doesn't change its
            // relationship to its parent's - without the marker, the next
            // reload of the parent's file would mistake this item for inline
            // content and keep it around alongside its freshly-parsed twin.
            if (item->state.hasProperty(ids::externalSourceChild()))
                source.setProperty(ids::externalSourceChild(), true, nullptr);

            if (item->isTopLevel())
            {
                interpretationsInProgress++;

                auto* topLevel = dynamic_cast<TopLevelGuiItem*>(item);
                jassert(topLevel != nullptr);
                topLevel->replaceDecoratedItem(interpretInternal(source, nullptr));

                // Replacing the decorated item swaps in a brand-new state tree,
                // so the metadata and listener that let us find and observe the
                // top-level item need to be re-established - otherwise any
                // further changes to this file (or its nested sources) would go
                // unnoticed.
                item->state.setProperty(ids::topLevelSourceFile(), file.getPath(), nullptr);
                listenTo(*item);
                notificationPending = true;

                if (--interpretationsInProgress == 0)
                    notifyStateBuild();
            }
            else
            {
                // Children declared inline at the usage site (e.g. a Button's
                // text) aren't part of the source file, so they need to be
                // carried across onto the freshly-parsed replacement tree.
                std::vector<juce::ValueTree> inlineChildren;

                for (auto i = 0; i < item->state.getNumChildren(); i++)
                {
                    auto child = item->state.getChild(i);

                    if (!static_cast<bool>(child.getProperty(ids::externalSourceChild())))
                        inlineChildren.push_back(child);
                }

                for (auto& child : inlineChildren)
                    item->state.removeChild(child, nullptr);

                for (auto& child : inlineChildren)
                    source.appendChild(child, nullptr);

                auto* parent = item->getParent();
                jassert(parent != nullptr);

                const auto index = parent->state.indexOf(item->state);
                parent->state.removeChild(item->state, nullptr);
                parent->state.addChild(source, index, nullptr);
            }
        }
    }

    // Paths without "./" or "../" are normally relative to the root, but in a
    // file on disk they're also looked for next to that file.
    std::unique_ptr<GuiItem> Interpreter::interpret(const juce::File& file,
                                                    juce::AudioProcessor* pluginProcessor)
    {
        addSourceDirectory(file.getParentDirectory());
        return interpret(SourceFile{ file }, pluginProcessor);
    }

    std::unique_ptr<GuiItem> Interpreter::interpret(const SourceFile& file,
                                                    juce::AudioProcessor* pluginProcessor)
    {
        auto tree = parseFileToValueTree(file);
        rememberTopLevelSourceProperties(tree);
        tree.setProperty(ids::topLevelSourceFile(), file.getPath(), nullptr);

        observeFileForChanges(file);

        return interpret(tree, pluginProcessor);
    }

    // Embedded files can't change, so only files on disk are observed.
    void Interpreter::observeFileForChanges(const SourceFile& file)
    {
        const auto fileOnDisk = file.getFileOnDisk();

        if (fileOnDisk == juce::File{})
            return;

        for (auto* observer : fileObservers)
        {
            if (observer->file == fileOnDisk)
                return;
        }

        auto* observer = fileObservers.add(std::make_unique<FileObserver>(fileOnDisk));
        observer->onFileModified = [this, file]() {
            onObservedFileChanged(file);
        };
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

    void Interpreter::valueTreePropertyChanged(juce::ValueTree& tree,
                                               const juce::Identifier& property)
    {
        if (observedItem == nullptr || ignoreValueTreeChanges || property != ids::source())
            return;

        auto parentTree = tree.getParent();
        auto* parentItem = findItem(*observedItem, parentTree);

        if (parentItem == nullptr)
            return;

        for (auto* child : parentItem->getChildren())
        {
            if (child->state == tree)
            {
                parentItem->removeChild(*child);
                break;
            }
        }

        insertInterpretedChild(*parentItem, tree, parentTree.indexOf(tree));
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

        insertInterpretedChild(*parentItem, childWhichHasBeenAdded, parentTree.indexOf(childWhichHasBeenAdded));
    }

    // Listeners are told about the build only once the new item is in place,
    // so that they can find it.
    void Interpreter::insertInterpretedChild(GuiItem& parentItem, const juce::ValueTree& tree, int index)
    {
        interpretationsInProgress++;

        if (auto newChild = interpretInternal(tree, &parentItem))
            parentItem.insertChild(std::move(newChild), index);

        if (--interpretationsInProgress == 0)
            notifyStateBuild();
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

        const auto indexInParent = parentTree.indexOf(treeWhichHasBeenChanged);

        if (auto newChild = interpretInternal(treeWhichHasBeenChanged, parentItem))
            parentItem->insertChild(std::move(newChild), indexInParent);
    }

    void Interpreter::notifyStateBuild()
    {
        if (std::exchange(notificationPending, false) && observedItem != nullptr && onLoad != nullptr)
            onLoad(*observedItem);
    }

    std::unique_ptr<GuiItem> Interpreter::interpretInternal(juce::ValueTree tree,
                                                            GuiItem* const parent)
    {
        interpretationsInProgress++;

        tree = loadExternalSources(tree);

        auto item = createUndecoratedItem(tree, parent);

        if (item != nullptr)
        {
            tree.setProperty(ids::setupInProgress(), true, nullptr);
            item = decorate(std::move(item), customDecorators);
            setChildItems(*item);

            tree.setProperty(ids::setupInProgress(), false, nullptr);

            if (item->isTopLevel())
            {
                if (auto* container = dynamic_cast<GuiItemDecorator&>(*item)
                                          .getTopLevelDecorator()
                                          .toType<ContainerItem>())
                {
                    container->updateIdealSize();
                }
            }
        }

        // Listeners are told about the build only once the whole tree has been
        // interpreted, so that they're free to go looking around it.
        if (--interpretationsInProgress == 0)
            notifyStateBuild();

        return item;
    }

    std::unique_ptr<GuiItem> Interpreter::createUndecoratedItem(const juce::ValueTree& tree,
                                                                GuiItem* const parent)
    {
        if (auto component = componentFactory.create(tree.getType()))
        {
            return std::make_unique<GuiItem>(component,
                                             tree,
                                             parent);
        }

        return nullptr;
    }

    juce::ValueTree Interpreter::loadExternalSources(juce::ValueTree tree)
    {
        if (tree.isValid())
            tree.setProperty(ids::sourceDirectories(), sourceDirectories.get(), nullptr);

        const auto source = tree[ids::source()].toString();

        if (source.isEmpty())
            return tree;

        // The source is written in the file that declares this element, but
        // once it's been loaded, this element belongs to the source file.
        const auto declaringTree = tree.getParent().isValid() ? tree.getParent() : tree;
        const auto file = sourceDirectories->find(source, findSourceFile(declaringTree));

        if (!file.exists())
            return tree;

        // Files that aren't markup, like images, are loaded by the item itself.
        auto newTree = parseFileToValueTree(file);

        if (!newTree.isValid())
            return tree;

        const juce::ScopedValueSetter svs{ ignoreValueTreeChanges, true };

        // Properties merged in from a previously loaded source would otherwise
        // outlive it, overriding whatever the new source has to say.
        if (const auto* previousProperties = tree[ids::externalSourceProperties()].getArray())
        {
            for (const auto& name : *previousProperties)
                tree.removeProperty(name.toString(), nullptr);
        }

        juce::Array<juce::var> propertiesFromSource;

        for (auto i = 0; i < newTree.getNumProperties(); i++)
        {
            const auto name = newTree.getPropertyName(i);

            if (!tree.hasProperty(name))
                propertiesFromSource.add(name.toString());
        }

        for (auto i = 0; i < tree.getNumProperties(); i++)
        {
            const auto name = tree.getPropertyName(i);
            newTree.setProperty(name, tree[name], nullptr);
        }

        std::vector<juce::ValueTree> inlineChildren;

        for (auto i = 0; i < tree.getNumChildren(); i++)
        {
            auto child = tree.getChild(i);

            if (!static_cast<bool>(child.getProperty(ids::externalSourceChild())))
                inlineChildren.push_back(child);
        }

        for (auto& child : inlineChildren)
            tree.removeChild(child, nullptr);

        auto finalise = [&](juce::ValueTree destination) {
            for (auto i = 0; i < destination.getNumChildren(); i++)
                destination.getChild(i).setProperty(ids::externalSourceChild(), true, nullptr);

            for (auto& child : inlineChildren)
                destination.appendChild(child, nullptr);

            destination.setProperty(ids::externalSourceProperties(), propertiesFromSource, nullptr);

            observeFileForChanges(file);
            notificationPending = true;

            return destination;
        };

        // The root of the source file decides what kind of element this is - a
        // <Component source="icon.svg"/> is really an <svg>. A tree's type can
        // only change by swapping it out of its parent, so an element with no
        // parent to swap it in has to keep the type it was declared with.
        auto parentTree = tree.getParent();

        if (newTree.getType() != tree.getType() && parentTree.isValid())
        {
            const auto index = parentTree.indexOf(tree);
            parentTree.removeChild(index, nullptr);
            parentTree.addChild(newTree, index, nullptr);

            return finalise(newTree);
        }

        tree.copyPropertiesAndChildrenFrom(newTree, nullptr);

        return finalise(tree);
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
