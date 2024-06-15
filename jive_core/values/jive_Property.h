#pragma once

#include "jive_Object.h"
#include "jive_PropertyBehaviours.h"
#include "variant-converters/jive_VariantConvertion.h"

#include <jive_core/algorithms/jive_Visitor.h>
#include <jive_core/kinetics/jive_Transitions.h>

namespace jive
{
    template <typename T>
    struct isReferenceCountedObjectPointer : std::false_type
    {
    };

    template <typename T>
    struct isReferenceCountedObjectPointer<juce::ReferenceCountedObjectPtr<T>> : std::true_type
    {
    };

    template <typename ValueType,
              Inheritance inheritance = Inheritance::doNotInherit,
              Accumulation accumulation = Accumulation::doNotAccumulate,
              bool autoParseStrings = isReferenceCountedObjectPointer<ValueType>::value,
              Responsiveness responsiveness = Responsiveness::respondToChanges>
    class Property
        : protected juce::ValueTree::Listener
        , protected Object::Listener
        , private Transitions::Transition::Listener
    {
    public:
        using Source = std::variant<juce::ValueTree, Object::ReferenceCountedPointer>;

        Property(Source propertySource,
                 const juce::Identifier& propertyID)
            : id{ propertyID }
            , source{ propertySource }
            , transitionSourceID{ propertyID }
        {
            initialise();
        }

        Property(const Property& other)
            : id{ other.id }
        {
            *this = other;
        }

        Property(Property&& other)
            : id{ std::move(other.id) }
        {
            *this = std::move(other);
        }

        Property& operator=(const Property& other)
        {
            jassert(id == other.id);
            source = other.source;
            onValueChange = other.onValueChange;

            initialise();

            return *this;
        }

        Property& operator=(Property&& other)
        {
            jassert(id == other.id);
            source = std::move(other.source);
            onValueChange = std::move(other.onValueChange);

            initialise();

            return *this;
        }

        ~Property() override
        {
            observeTransition(nullptr);
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

        [[nodiscard]] auto calculateCurrent() const
        {
            if (auto* transition = getTransition())
                return transition->template calculateCurrent<ValueType>();

            return get();
        }

        void set(const ValueType& newValue)
        {
            set(source, toVar(newValue));
        }

        void set(std::function<ValueType()> function)
        {
            set(source, juce::var{
                            [function](const auto&) {
                                return toVar(function());
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
                           [this](const Object::ReferenceCountedPointer& sourceObject) {
                               if (sourceObject != nullptr)
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
                                  [this](const Object::ReferenceCountedPointer& sourceObject) {
                                      if (sourceObject != nullptr)
                                          return sourceObject->getProperty(id).isMethod();

                                      return false;
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
                                  [this](const Object::ReferenceCountedPointer& sourceObject) {
                                      return sourceObject->getProperty(id).toString();
                                  },
                              },
                              source);
        }

        void setTransitionSourceProperty(const juce::Identifier& sourceID)
        {
            observeTransition(nullptr);
            transitionSourceID = sourceID;

            if (auto* transition = getTransition())
                observeTransition(transition);
        }

        [[nodiscard]] Transitions::Transition* getTransition()
        {
            if (currentTransition == nullptr)
            {
                using TransitionsProperty = Property<Transitions::ReferenceCountedPointer,
                                                     Inheritance::doNotInherit,
                                                     Accumulation::doNotAccumulate,
                                                     true,
                                                     Responsiveness::ignoreChanges>;

                if (const TransitionsProperty transitions{ source, "transition" }; transitions.exists())
                    currentTransition = (*transitions.get())[transitionSourceID.toString()];
            }

            if (currentTransition != nullptr && currentTransition->source.isVoid())
            {
                currentTransition->source = getVar(source, id);
                currentTransition->target = currentTransition->source;
                currentTransition->commencement = now();
            }

            return currentTransition;
        }

        [[nodiscard]] const Transitions::Transition* getTransition() const
        {
            return const_cast<Property*>(this)->getTransition();
        }

        [[nodiscard]] auto isTransitioning() const
        {
            return getTransition() != nullptr;
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
        mutable std::function<void(void)> onValueChange = nullptr;
        mutable std::function<void(void)> onTransitionProgressed = nullptr;

    protected:
        void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyChanged,
                                      const juce::Identifier& property) override
        {
            if (property != id)
            {
                if (property.toString() == "transition")
                    currentTransition = getTransition();

                return;
            }
            if (!treeWhosePropertyChanged.hasProperty(property))
                return;
            if (!respondToPropertyChanges(treeWhosePropertyChanged))
                return;

            valueChanged();

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

            valueChanged();

            if (onValueChange != nullptr)
                onValueChange();
        }

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
                return fromVar<ValueType>(juce::var{});

            if constexpr (accumulation == Accumulation::accumulate)
            {
                auto result = fromVar<ValueType>(getVar(root, id));

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

                return fromVar<ValueType>(var);
            }
        }

        [[nodiscard]] ValueType getFrom(Object* root) const
        {
            if (!isValid(root))
                return fromVar<ValueType>(juce::var{});

            if constexpr (accumulation == Accumulation::accumulate)
            {
                auto result = fromVar<ValueType>(getVar(root, id));

                for (auto i = 0; i < getNumChildren(root); i++)
                    result += getFrom(getChild(root, i));

                return result;
            }
            else
            {
                if (root->hasMethod(id))
                {
                    juce::var::NativeFunctionArgs args{ juce::var{}, nullptr, 0 };
                    return fromVar<ValueType>(root->invokeMethod(id, args));
                }

                return fromVar<ValueType>(root->getProperty(id));
            }
        }

        [[nodiscard]] auto getFrom(const Source& root) const
        {
            return std::visit(Visitor{
                                  [this](const juce::ValueTree& sourceTree) {
                                      return getFrom(sourceTree);
                                  },
                                  [this](const Object::ReferenceCountedPointer& sourceObject) {
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
                                  [](const Object::ReferenceCountedPointer& sourceObject) {
                                      return sourceObject != nullptr;
                                  },
                              },
                              src);
        }

        [[nodiscard]] juce::var getVar(const Source& src, const juce::Identifier& name) const
        {
            return std::visit(Visitor{
                                  [name](const juce::ValueTree& sourceTree) {
                                      return sourceTree[name];
                                  },
                                  [name](const Object::ReferenceCountedPointer& sourceObject) {
                                      if (sourceObject != nullptr)
                                          return sourceObject->getProperty(name);

                                      return juce::var{};
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
                                  [](const Object::ReferenceCountedPointer& sourceObject) -> Source {
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
                                  [](const Object::ReferenceCountedPointer& sourceObject) {
                                      if (sourceObject == nullptr)
                                          return 0;

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
                                  [index](const Object::ReferenceCountedPointer& sourceObject) -> Source {
                                      if (sourceObject == nullptr)
                                          return nullptr;

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
                                  [](const Object::ReferenceCountedPointer& sourceObject) -> Source {
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
                           [this](const Object::ReferenceCountedPointer& sourceObject) {
                               if (sourceObject != nullptr)
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
                           [this](const Object::ReferenceCountedPointer& sourceObject) {
                               if (sourceObject != nullptr)
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

        virtual void transitionProgressed() {}

        Source source;

    private:
        void initialise()
        {
            listenerTarget = findListenerTarget(source);

            if (!isValid(listenerTarget))
                listenerTarget = source;

            if constexpr (responsiveness == Responsiveness::respondToChanges)
                addThisAsListener(listenerTarget);

            if constexpr (autoParseStrings)
            {
                if (auto value = getVar(source, id); value.isString())
                    set(get());
            }

            if constexpr (!std::is_same<ValueType, Transitions::ReferenceCountedPointer>())
                updateTransition();
        }

        void transitionProgressed(const juce::String& propertyName,
                                  const Transitions::Transition&) final
        {
            jassertquiet(propertyName == transitionSourceID.toString());

            transitionProgressed();

            if (onTransitionProgressed != nullptr)
                onTransitionProgressed();
        }

        void valueChanged()
        {
            if constexpr (!std::is_same<ValueType, Transitions::ReferenceCountedPointer>())
                updateTransition();
        }

        void updateTransition()
        {
            const auto var = getVar(source, id);

            if (auto* transition = getTransition())
            {
                const auto isUninitialised = transition->source.isVoid();
                const auto isAlreadyUpToDate = transition->target == var;

                if (isUninitialised)
                {
                    transition->source = var;
                    transition->target = var;
                    transition->commencement = now();
                }
                else if (!isAlreadyUpToDate)
                {
                    transition->source = toVar(transition->template calculateCurrent<ValueType>());
                    transition->target = var;
                    transition->commencement = now();
                }

                observeTransition(transition);
            }
            else
            {
                observeTransition(nullptr);
            }
        }

        void observeTransition(Transitions::Transition* transition)
        {
            if (observedTransition == transition)
                return;

            if (observedTransition != nullptr)
                observedTransition->removeListener(*this);

            observedTransition = transition;

            if (transition != nullptr)
                transition->addListener(*this);
        }

        Source listenerTarget;
        juce::Identifier transitionSourceID;
        Transitions::Transition* currentTransition = nullptr;
        Transitions::Transition* observedTransition = nullptr;
    };
} // namespace jive
