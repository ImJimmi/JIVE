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

    static juce::ValueTree parseSourceXml(const juce::String& xml, const SourceFile& file)
    {
        auto sourceTree = jive::parseXML(xml);

        if (sourceTree.isValid())
            sourceTree.setProperty(ids::sourceFile(), file.getPath(), nullptr);

        return sourceTree;
    }

    juce::ValueTree Interpreter::parseFileToValueTree(const SourceFile& file) const
    {
        return parseSourceXml(file.loadAsString(), file);
    }

    static juce::String escapeForXml(const juce::String& text)
    {
        return text.replace("&", "&amp;")
            .replace("<", "&lt;")
            .replace(">", "&gt;")
            .replace("\"", "&quot;")
            .replace("'", "&apos;");
    }

    // Attributes merged in from a previously loaded template belong to that
    // template, not to the instance.
    static juce::String findPlaceholderValue(const juce::ValueTree& instance, const juce::String& name)
    {
        if (name.isEmpty())
            return {};

        if (const auto* previousTemplateProperties = instance[ids::externalSourceProperties()].getArray();
            previousTemplateProperties != nullptr && previousTemplateProperties->contains(name))
        {
            return {};
        }

        return instance[name].toString();
    }

    static juce::String fillPlaceholders(const juce::String& xml, const juce::ValueTree& instance)
    {
        juce::String result;
        auto end = 0;

        for (auto open = xml.indexOf("{{"); open >= 0; open = xml.indexOf(end, "{{"))
        {
            const auto close = xml.indexOf(open + 2, "}}");

            if (close < 0)
                break;

            result << xml.substring(end, open)
                   << escapeForXml(findPlaceholderValue(instance, xml.substring(open + 2, close).trim()));
            end = close + 2;
        }

        return result + xml.substring(end);
    }

    // Placeholders are filled in before parsing so that they can stand in for
    // types and property names, not just values.
    static juce::ValueTree instantiateTemplate(const SourceFile& file, const juce::ValueTree& instance)
    {
        return parseSourceXml(fillPlaceholders(file.loadAsString(), instance), file);
    }

    // An instance's snippets are kept aside, rather than as children, so that
    // they can fill in its template again whenever it's reloaded. Their content
    // is moved into the template, so it's marked with the file it was written
    // in to keep its paths relative to that file.
    static void storeSnippets(juce::ValueTree& instance, const SourceFile& declaringFile)
    {
        std::vector<juce::ValueTree> snippets;

        for (const auto& child : instance)
        {
            if (child.hasType(ids::snippet()) && !static_cast<bool>(child[ids::externalSourceChild()]))
                snippets.push_back(child);
        }

        if (snippets.empty())
            return;

        juce::Array<juce::var> definitions;

        for (auto& snippet : snippets)
        {
            instance.removeChild(snippet, nullptr);

            for (auto content : snippet)
            {
                if (!content.hasProperty(ids::snippetSourceFile()))
                {
                    content.setProperty(ids::sourceFile(), declaringFile.getPath(), nullptr);
                    content.setProperty(ids::snippetSourceFile(), declaringFile.getPath(), nullptr);
                }
            }

            definitions.add(snippet.toXmlString());
        }

        instance.setProperty(ids::snippets(), definitions, nullptr);
    }

    static juce::ValueTree findSnippet(const juce::ValueTree& instance, const juce::var& name)
    {
        if (const auto* definitions = instance[ids::snippets()].getArray())
        {
            for (const auto& definition : *definitions)
            {
                if (const auto snippet = juce::ValueTree::fromXml(definition.toString());
                    snippet[ids::name()] == name)
                {
                    return snippet;
                }
            }
        }

        return {};
    }

    // Snippets inside an element that uses a template are passed on to that
    // template rather than filled in, though they can still pass on this
    // template's own snippets. A snippet with nothing to fill it keeps its own
    // content, which may have snippets of its own.
    static void fillSnippets(juce::ValueTree tree, const juce::ValueTree& instance)
    {
        for (auto i = 0; i < tree.getNumChildren(); i++)
        {
            auto child = tree.getChild(i);

            if (!child.hasType(ids::snippet()) || tree.hasProperty(ids::templatePath()))
            {
                fillSnippets(child, instance);
                continue;
            }

            const auto snippet = findSnippet(instance, child[ids::name()]);
            const auto content = snippet.isValid() ? snippet : child;
            tree.removeChild(i, nullptr);

            for (auto j = 0; j < content.getNumChildren(); j++)
                tree.addChild(content.getChild(j).createCopy(), i + j, nullptr);

            if (snippet.isValid())
                i += content.getNumChildren();

            i--;
        }
    }

    static void findItemsWithSource(GuiItem* root, const SourceFile& file, std::vector<GuiItem*>& results)
    {
        if (root == nullptr)
            return;

        if (file.getPath().contains(root->state.getProperty(ids::templatePath(), "jive::no-source-found").toString())
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
            const auto isTopLevelFile = file.getPath() == item->state[ids::topLevelSourceFile()].toString();
            auto source = isTopLevelFile
                            ? parseFileToValueTree(file)
                            : instantiateTemplate(file, item->state);

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

            // An instance's snippets aren't part of its template, so they're
            // carried across to fill in the freshly-parsed template.
            for (const auto& id : { ids::snippets(), ids::snippetSourceFile() })
            {
                if (!isTopLevelFile && item->state.hasProperty(id))
                    source.setProperty(id, item->state[id], nullptr);
            }

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
        if (observedItem == nullptr || ignoreValueTreeChanges || property != ids::templatePath())
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

        const auto templatePath = tree[ids::templatePath()].toString();

        if (templatePath.isEmpty())
            return tree;

        // The template is written in the file that declares this element, but
        // once it's been loaded, this element belongs to the template's file.
        // Content from a snippet is declared in the file the snippet is in.
        const auto declaringTree = tree.getParent().isValid() ? tree.getParent() : tree;
        const auto declaringFile = tree.hasProperty(ids::snippetSourceFile())
                                     ? SourceFile{ tree[ids::snippetSourceFile()].toString() }
                                     : findSourceFile(declaringTree);
        const auto file = sourceDirectories->find(templatePath, declaringFile);

        if (!file.exists())
            return tree;

        auto newTree = instantiateTemplate(file, tree);

        if (!newTree.isValid())
            return tree;

        const juce::ScopedValueSetter svs{ ignoreValueTreeChanges, true };

        storeSnippets(tree, declaringFile);
        fillSnippets(newTree, tree);

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

        newTree.setProperty(ids::sourceFile(), file.getPath(), nullptr);

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
        // <Component template="icon.svg"/> is really an <svg>. A tree's type can
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
