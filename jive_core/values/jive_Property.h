#pragma once

#include "jive_Object.h"

namespace jive
{
    enum class Inheritance
    {
        inheritFromParent,
        inheritFromAncestors,
        doNotInherit,
    };

    enum class Accumulation
    {
        accumulate,
        doNotAccumulate,
    };

    template <typename ValueType,
              Inheritance inheritance = Inheritance::doNotInherit,
              Accumulation accumulation = Accumulation::doNotAccumulate>
    class Property
        : protected juce::ValueTree::Listener
        , protected Object::Listener
    {
    public:
        using VariantConverter = juce::VariantConverter<ValueType>;
        using Source = std::variant<juce::ValueTree, Object::ReferenceCountedPointer>;

        Property(Source propertySource,
                 const juce::Identifier& propertyID)
            : id{ propertyID }
            , source{ propertySource }
        {
            listenerTarget = findListenerTarget(propertySource);

            if (!isValid(listenerTarget))
                listenerTarget = source;

            addThisAsListener(listenerTarget);

            if constexpr (std::is_same<ValueType, Object::ReferenceCountedPointer>())
            {
                if (auto value = getVar(source, id); value.isString())
                {
                    if (auto json = parseJSON(value.toString()); json != juce::var{})
                        set(source, json);
                }
            }
        }

        Property(const Property& other) = delete;
        Property(Property&& other) = delete;
        Property& operator=(const Property& other) = delete;
        Property& operator=(Property&& other) = delete;

        ~Property() override
        {
            removeThisAsListener(source);
            removeThisAsListener(listenerTarget);
        }

        [[nodiscard]] virtual ValueType get() const
        {
            return getFrom(getRootOfInheritance());
        }

        [[nodiscard]] auto getOr(const ValueType& valueIfNoneSpecified) const
        {
            if (auto root = getRootOfInheritance();
                isValid(root))
            {
                return getFrom(root);
            }

            if constexpr (accumulation == Accumulation::accumulate)
            {
                if (auto src = getFirstDescendantWithProperty(source);
                    src.isValid())
                {
                    return getFrom(src);
                }
            }

            return valueIfNoneSpecified;
        }

        void set(const ValueType& newValue)
        {
            set(source, VariantConverter::toVar(newValue));
        }

        void set(std::function<ValueType()> function)
        {
            set(source, juce::var{
                            [function](const auto&) {
                                return VariantConverter::toVar(function());
                            },
                        });
        }

        void setAuto()
        {
            set(source, "auto");
        }

        void clear()
        {
            std::visit(Visitor{
                           [this](juce::ValueTree& sourceTree) {
                               sourceTree.removeProperty(id, nullptr);
                           },
                           [this](Object::ReferenceCountedPointer sourceObject) {
                               sourceObject->removeProperty(id);
                           },
                       },
                       source);
        }

        [[nodiscard]] auto exists() const
        {
            return !getVar(source, id).isVoid();
        }

        [[nodiscard]] auto isAuto() const
        {
            return (!exists()) || toString().trim().equalsIgnoreCase("auto");
        }

        [[nodiscard]] auto isFunctional() const
        {
            if (!exists())
                return false;

            return std::visit(Visitor{
                                  [this](const juce::ValueTree& sourceTree) {
                                      return sourceTree[id].isMethod();
                                  },
                                  [this](Object::ReferenceCountedPointer sourceObject) {
                                      return sourceObject->getProperty(id).isMethod();
                                  },
                              },
                              source);
        }

        [[nodiscard]] auto toString() const
        {
            if (!exists())
                return juce::String{};

            return std::visit(Visitor{
                                  [this](const juce::ValueTree& sourceTree) {
                                      return sourceTree[id].toString();
                                  },
                                  [this](Object::ReferenceCountedPointer sourceObject) {
                                      return sourceObject->getProperty(id).toString();
                                  },
                              },
                              source);
        }

        [[nodiscard]] operator ValueType() const
        {
            return get();
        }

        auto& operator=(const ValueType& newValue)
        {
            set(newValue);
            return *this;
        }

        auto& operator=(std::function<ValueType()> function)
        {
            set(function);
            return *this;
        }

        auto& operator=(std::function<ValueType(std::vector<juce::var>)> function)
        {
            set(function);
            return *this;
        }

        const juce::Identifier id;
        std::function<void(void)> onValueChange = nullptr;

    protected:
        void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyChanged,
                                      const juce::Identifier& property) override
        {
            if (property != id)
                return;
            if (!treeWhosePropertyChanged.hasProperty(property))
                return;
            if (!respondToPropertyChanges(treeWhosePropertyChanged))
                return;

            if (onValueChange != nullptr)
                onValueChange();
        }

        void propertyChanged(Object& objectWhosePropertyChanged,
                             const juce::Identifier& property) override
        {
            if (property != id)
                return;
            if (!objectWhosePropertyChanged.hasProperty(property))
                return;

            if (onValueChange != nullptr)
                onValueChange();
        }

        Source source;
        Source listenerTarget;

        // private:
        [[nodiscard]] auto respondToPropertyChanges(juce::ValueTree& treeWhosePropertyChanged) const
        {
            if (treeWhosePropertyChanged == std::get<juce::ValueTree>(source))
                return true;

            switch (inheritance)
            {
            case Inheritance::inheritFromParent:
                return treeWhosePropertyChanged == std::get<juce::ValueTree>(source).getParent();
            case Inheritance::inheritFromAncestors:
                return std::get<juce::ValueTree>(source).isAChildOf(treeWhosePropertyChanged);
            case Inheritance::doNotInherit:
                break;
            }

            switch (accumulation)
            {
            case Accumulation::accumulate:
                return treeWhosePropertyChanged.isAChildOf(std::get<juce::ValueTree>(source));
            case Accumulation::doNotAccumulate:
                break;
            }

            return false;
        }

        [[nodiscard]] auto respondToPropertyChanges(Object& objectWhosePropertyChanged) const
        {
            return &objectWhosePropertyChanged == static_cast<Object*>(std::get<Object::ReferenceCountedPointer>(source));
        }

        [[nodiscard]] auto getRootOfInheritance() const
        {
            if (exists() || accumulation == Accumulation::accumulate)
                return source;

            if constexpr (inheritance == Inheritance::inheritFromParent)
            {
                if (auto parent = getParent(source);
                    !getVar(parent, id).isVoid())
                {
                    return parent;
                }
            }
            if constexpr (inheritance == Inheritance::inheritFromAncestors)
            {
                for (auto ancestor = getParent(source);
                     isValid(ancestor);
                     ancestor = getParent(ancestor))
                {
                    if (!getVar(ancestor, id).isVoid())
                        return ancestor;
                }
            }

            return Source{};
        }

        [[nodiscard]] auto getFirstDescendantWithProperty(const Source& root) const
        {
            for (auto i = 0; i < getNumChildren(root); i++)
            {
                if (auto child = getChild(root, i); !getVar(child, id).isVoid())
                    return child;
            }

            for (auto i = 0; i < getNumChildren(root); i++)
            {
                if (auto descendant = getFirstDescendantWithProperty(getChild(root, i));
                    isValid(descendant))
                {
                    return descendant;
                }
            }

            return Source{};
        }

        [[nodiscard]] ValueType getFrom(const juce::ValueTree& root) const
        {
            if (!isValid(root))
                return VariantConverter::fromVar(juce::var{});

            if constexpr (accumulation == Accumulation::accumulate)
            {
                auto result = VariantConverter::fromVar(getVar(root, id));

                for (auto i = 0; i < getNumChildren(root); i++)
                    result += getFrom(getChild(root, i));

                return result;
            }
            else
            {
                auto var = root.hasProperty(id) ? root[id] : getVar(getFirstDescendantWithProperty(root), id);

                if (var.isMethod())
                {
                    juce::var::NativeFunctionArgs args{ var, nullptr, 0 };
                    var = var.getNativeFunction()(args);
                }

                return VariantConverter::fromVar(var);
            }
        }

        [[nodiscard]] ValueType getFrom(Object* root) const
        {
            if (!isValid(root))
                return VariantConverter::fromVar(juce::var{});

            if constexpr (accumulation == Accumulation::accumulate)
            {
                auto result = VariantConverter::fromVar(getVar(root, id));

                for (auto i = 0; i < getNumChildren(root); i++)
                    result += getFrom(getChild(root, i));

                return result;
            }
            else
            {
                if (root->hasMethod(id))
                {
                    juce::var::NativeFunctionArgs args{ juce::var{}, nullptr, 0 };
                    return VariantConverter::fromVar(root->invokeMethod(id, args));
                }

                return VariantConverter::fromVar(root->getProperty(id));
            }
        }

        [[nodiscard]] auto getFrom(const Source& root) const
        {
            return std::visit(Visitor{
                                  [this](const juce::ValueTree& sourceTree) {
                                      return getFrom(sourceTree);
                                  },
                                  [this](Object::ReferenceCountedPointer sourceObject) {
                                      return getFrom(sourceObject.get());
                                  },
                              },
                              root);
        }

        [[nodiscard]] auto isValid(const Source& src) const
        {
            return std::visit(Visitor{
                                  [](const juce::ValueTree& sourceTree) {
                                      return sourceTree.isValid();
                                  },
                                  [](Object::ReferenceCountedPointer sourceObject) {
                                      return sourceObject != nullptr;
                                  },
                              },
                              src);
        }

        [[nodiscard]] auto getVar(const Source& src, const juce::Identifier& name) const
        {
            return std::visit(Visitor{
                                  [name](const juce::ValueTree& sourceTree) {
                                      return sourceTree[name];
                                  },
                                  [name](Object::ReferenceCountedPointer sourceObject) {
                                      return sourceObject->getProperty(name);
                                  },
                              },
                              src);
        }

        [[nodiscard]] auto getParent(const Source& src) const
        {
            return std::visit(Visitor{
                                  [](const juce::ValueTree& sourceTree) -> Source {
                                      return sourceTree.getParent();
                                  },
                                  [](Object::ReferenceCountedPointer sourceObject) -> Source {
                                      return sourceObject->getParent();
                                  },
                              },
                              src);
        }

        [[nodiscard]] auto getNumChildren(const Source& src) const
        {
            return std::visit(Visitor{
                                  [](const juce::ValueTree& sourceTree) {
                                      return sourceTree.getNumChildren();
                                  },
                                  [](Object::ReferenceCountedPointer sourceObject) {
                                      return static_cast<int>(std::count_if(std::begin(sourceObject->getProperties()),
                                                                            std::end(sourceObject->getProperties()),
                                                                            [](const auto& property) {
                                                                                return dynamic_cast<const Object*>(property.value.getDynamicObject()) != nullptr;
                                                                            }));
                                  },
                              },
                              src);
        }

        [[nodiscard]] auto getChild(const Source& src, int index) const
        {
            return std::visit(Visitor{
                                  [index](const juce::ValueTree& sourceTree) -> Source {
                                      return sourceTree.getChild(index);
                                  },
                                  [index](Object::ReferenceCountedPointer sourceObject) -> Source {
                                      auto current = 0;

                                      for (const auto& [_, value] : sourceObject->getProperties())
                                      {
                                          if (auto* child = dynamic_cast<Object*>(value.getDynamicObject()))
                                          {
                                              if (current++ == index)
                                                  return child;
                                          }
                                      }

                                      return nullptr;
                                  },
                              },
                              src);
        }

        [[nodiscard]] auto findListenerTarget(const Source& src) const
        {
            switch (inheritance)
            {
            case Inheritance::inheritFromParent:
                return getParent(src);
            case Inheritance::inheritFromAncestors:
                return getRoot(src);
            case Inheritance::doNotInherit:
                return src;
            }

            jassertfalse;
            return src;
        }

        [[nodiscard]] auto getRoot(const Source& src) const
        {
            return std::visit(Visitor{
                                  [](const juce::ValueTree& sourceTree) -> Source {
                                      return sourceTree.getRoot();
                                  },
                                  [](Object::ReferenceCountedPointer sourceObject) -> Source {
                                      return sourceObject->getRoot();
                                  },
                              },
                              src);
        }

        void addThisAsListener(Source& src)
        {
            std::visit(Visitor{
                           [this](juce::ValueTree& sourceTree) {
                               sourceTree.addListener(this);
                           },
                           [this](Object::ReferenceCountedPointer sourceObject) {
                               sourceObject->addListener(*this);
                           },
                       },
                       src);
        }

        void removeThisAsListener(Source& src)
        {
            std::visit(Visitor{
                           [this](juce::ValueTree& sourceTree) {
                               sourceTree.removeListener(this);
                           },
                           [this](Object::ReferenceCountedPointer sourceObject) {
                               sourceObject->removeListener(*this);
                           },
                       },
                       src);
        }

        void set(Source& src, const juce::var& value)
        {
            std::visit(Visitor{
                           [this, &value](juce::ValueTree& sourceTree) {
                               sourceTree.setProperty(id, value, nullptr);
                           },
                           [this, &value](Object::ReferenceCountedPointer sourceObject) {
                               sourceObject->setProperty(id, value);
                           },
                       },
                       src);
        }
    };
} // namespace jive
