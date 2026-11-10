#include "jive_ErrorHandling.h"

#include <jive_core/values/jive_Identifiers.h>

#include <jive_core/jive_core.h>

namespace jive
{
    [[nodiscard]] static juce::String describe(juce::ValueTree tree)
    {
        if (tree.hasProperty(ids::id()))
            return "#" + tree[ids::id()].toString();

        if (const auto parent = tree.getParent();
            parent.isValid())
        {
            juce::String description;

            description
                << describe(parent)
                << " / "
                << tree.getType().toString();

            if (parent.getNumChildren() > 1)
                description << "[" << parent.indexOf(tree) << "]";

            return description;
        }

        return tree.getType().toString();
    }

    void logValueError(juce::ValueTree tree,
                       juce::Identifier id,
                       juce::StringRef message)
    {
        juce::String buffer;

        if (tree.hasProperty(id))
            buffer << "Invalid value '" << tree[id].toString() << "' ";
        else
            buffer << "Missing value ";

        buffer << "for property '" << id.toString() << "'";

        if (auto sourceFile = findSourceFile(tree);
            sourceFile.exists())
        {
            buffer << " in " << sourceFile.getPath();
        }

        buffer << ": " << describe(tree);

        if (message.isNotEmpty())
            buffer << ". " << message;

        juce::Logger::writeToLog(buffer);
    }

    void logValueError(Object::ReferenceCountedPointer object,
                       juce::Identifier id,
                       juce::StringRef message,
                       const SourceFile& sourceFile)
    {
        juce::String buffer;

        if (object != nullptr && object->hasProperty(id))
            buffer << "Invalid value '" << object->getProperty(id).toString() << "' ";
        else
            buffer << "Missing value ";

        buffer << "for property '" << id.toString() << "'";

        if (sourceFile.exists())
            buffer << " in " << sourceFile.getPath();

        if (message.isNotEmpty())
            buffer << ". " << message;

        juce::Logger::writeToLog(buffer);
    }
} // namespace jive
