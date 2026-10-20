#include "jive_Object.h"

#include <jive_core/logging/jive_StringStreams.h>

namespace jive
{
    class Object::InternalListener : public Object::Listener
    {
    public:
        explicit InternalListener(Object& obj)
            : object{ obj }
        {
            for (const auto& nested : findNestedObjects())
                nested->addListener(*this);

            object.addListener(*this);
        }

        void propertyChanged(Object& objectThatChanged,
                             const juce::Identifier& propertyName) final
        {
            if (&objectThatChanged == &object)
            {
                for (const auto& nested : findNestedObjects())
                    nested->addListener(*this);
            }
            else
            {
                object.listeners.callExcluding(this,
                                               &Listener::propertyChanged,
                                               objectThatChanged,
                                               propertyName);
            }
        }

    private:
        juce::Array<Object*> findNestedObjects()
        {
            juce::Array<Object*> nestedObjects;

            for (const auto& [name, value] : object.getProperties())
            {
                if (auto* nested = dynamic_cast<Object*>(value.getDynamicObject()))
                {
                    nestedObjects.add(nested);
                }
            }

            return nestedObjects;
        }

        Object& object;
    };

    Object::Object()
        : internalListener{ std::make_unique<InternalListener>(*this) }
    {
    }

    Object::Object(std::initializer_list<juce::NamedValueSet::NamedValue> initialProperties)
        : internalListener{ std::make_unique<InternalListener>(*this) }
    {
        for (const auto& [name, value] : initialProperties)
            setProperty(name, value);
    }

    Object::Object(const Object& other)
        : juce::DynamicObject{ dynamic_cast<const DynamicObject&>(other) }
        , internalListener{ std::make_unique<InternalListener>(*this) }
    {
    }

    Object::Object(Object&& other) noexcept : juce::DynamicObject{ dynamic_cast<DynamicObject&&>(other) }
                                            , internalListener{ std::make_unique<InternalListener>(*this) }
    {
    }

    static void replaceDynamicObjectsWithJiveObjects(juce::var& value, Object* parent = nullptr)
    {
        if (auto* dynamicObject = value.getDynamicObject();
            dynamicObject != nullptr && dynamic_cast<Object*>(dynamicObject) == nullptr)
        {
            value = new Object{ *dynamicObject, parent };
        }

        if (auto* array = value.getArray())
        {
            for (auto& element : *array)
                replaceDynamicObjectsWithJiveObjects(element);
        }
    }

    // Nested objects are converted before the internal listener is created so
    // that it can pass their changes on to this object's listeners.
    [[nodiscard]] static juce::DynamicObject withNestedJiveObjects(const juce::DynamicObject& source, Object* parent)
    {
        juce::DynamicObject result{ source };

        for (auto i = 0; i < result.getProperties().size(); ++i)
            replaceDynamicObjectsWithJiveObjects(*result.getProperties().getVarPointerAt(i), parent);

        return result;
    }

    Object::Object(const juce::DynamicObject& other, Object* par)
        : juce::DynamicObject{ withNestedJiveObjects(other, this) }
        , internalListener{ std::make_unique<InternalListener>(*this) }
        , parent{ par }
    {
    }

#if JUCE_VERSION >= JIVE_JUCE_VERSION(8, 0, 4)
    void Object::didModifyProperty(const juce::Identifier& propertyName,
                                   const std::optional<juce::var>& newValue)
    {
        if (auto* childObject = dynamic_cast<Object*>(newValue
                                                          .value_or(juce::var{})
                                                          .getDynamicObject()))
        {
            childObject->parent = this;
        }

        listeners.call(&Listener::propertyChanged, *this, propertyName);
    }
#else
    void Object::setProperty(const juce::Identifier& propertyName,
                             const juce::var& newValue)
    {
        if (auto* childObject = dynamic_cast<Object*>(newValue.getDynamicObject()))
            childObject->parent = this;

        const auto propertyChanged = DynamicObject::getProperties()
                                         .set(propertyName, newValue);

        if (propertyChanged)
            listeners.call(&Listener::propertyChanged, *this, propertyName);
    }

    #if JUCE_VERSION < JIVE_JUCE_VERSION(8, 0, 2)
    const juce::NamedValueSet& Object::getProperties() const
    {
        return dynamic_cast<juce::DynamicObject*>(const_cast<Object*>(this))->getProperties();
    }
    #endif
#endif

    Object* Object::getParent() noexcept
    {
        return parent;
    }

    const Object* Object::getParent() const noexcept
    {
        return parent;
    }

    Object* Object::getRoot() noexcept
    {
        if (parent == nullptr)
            return this;

        return parent->getRoot();
    }

    const Object* Object::getRoot() const noexcept
    {
        if (parent == nullptr)
            return this;

        return parent->getRoot();
    }

    juce::String Object::findNameInParent() const
    {
        if (parent == nullptr)
            return "";

        for (const auto& [name, value] : parent->getProperties())
        {
            if (value.getObject() == this)
                return name.toString();
        }

        jassertfalse;
        return "";
    }

    void Object::addListener(Listener& listener) const
    {
        listeners.add(&listener);
    }

    void Object::removeListener(Listener& listener) const
    {
        listeners.remove(&listener);
    }

    const juce::var& Object::operator[](juce::Identifier name) const noexcept
    {
        return getProperties()[name];
    }

    juce::var parseJSON(const juce::String& jsonString)
    {
        auto value = juce::JSON::parse(jsonString);
        replaceDynamicObjectsWithJiveObjects(value);

        return value;
    }
} // namespace jive

namespace juce
{
    jive::Object::ReferenceCountedPointer VariantConverter<jive::Object::ReferenceCountedPointer>::fromVar(const var& value)
    {
        if (value.isString())
            return fromVar(jive::parseJSON(value.toString()));

        if (auto* dynamicObject = value.getDynamicObject())
        {
            if (auto* object = dynamic_cast<jive::Object*>(dynamicObject))
                return object;

            jassertfalse;
        }

        return nullptr;
    }

    var VariantConverter<jive::Object::ReferenceCountedPointer>::toVar(const jive::Object::ReferenceCountedPointer& object)
    {
        return var{ object.get() };
    }
} // namespace juce

#if JIVE_UNIT_TESTS
class ObjectTest : public juce::UnitTest
{
public:
    ObjectTest()
        : juce::UnitTest{ "jive::Object", "jive" }
    {
    }

    void runTest() final
    {
        testListener();
        testJsonParsing();
        testInitialiserListConstruction();
    }

private:
    struct Listener : public jive::Object::Listener
    {
        void propertyChanged(jive::Object&, const juce::Identifier&) override
        {
            if (onPropertyChange != nullptr)
                onPropertyChange();
        }

        std::function<void()> onPropertyChange = nullptr;
    };

    void testListener()
    {
        beginTest("listeners");

        jive::Object object;

        Listener listener;
        object.addListener(listener);

        bool callbackCalled = false;
        listener.onPropertyChange = [&callbackCalled] {
            callbackCalled = true;
        };

        object.setProperty("value", 123);
        expect(callbackCalled);

        object.setProperty("nested", jive::parseJSON(R"(
            {
                "number": "456",
            }
        )"));
        const auto nested = juce::VariantConverter<jive::Object::ReferenceCountedPointer>::fromVar(object.getProperty("nested"));
        expect(nested != nullptr);
        callbackCalled = false;
        nested->setProperty("number", 789);
        expect(callbackCalled);
    }

    void testJsonParsing()
    {
        beginTest("JSON parsing");

        auto value = jive::parseJSON(R"(
            {
                "value": 337,
            }
        )");
        expect(value.isObject());

        const auto* object = dynamic_cast<jive::Object*>(value.getDynamicObject());
        expect(object != nullptr);
        expect(object->hasProperty("value"));
        expectEquals(object->getProperty("value"), juce::var{ 337 });

        juce::ValueTree state{ "State" };
        state.setProperty("value",
                          jive::parseJSON(R"(
                              {
                                  "value": 837,
                                  "nested": {
                                      "number": 47,
                                  },
                              }
                          )"),
                          nullptr);
        value = state.getProperty("value");
        expect(value.isObject());

        object = dynamic_cast<jive::Object*>(value.getDynamicObject());
        expect(object != nullptr);
        expect(object->hasProperty("value"));
        expectEquals(object->getProperty("value"), juce::var{ 837 });

        expect(object->hasProperty("nested"));
        const auto nested = juce::VariantConverter<jive::Object::ReferenceCountedPointer>::fromVar(object->getProperty("nested"));
        expect(nested != nullptr);
        expect(nested->hasProperty("number"));
        expectEquals(nested->getProperty("number"), juce::var{ 47 });

        Listener listener;
        nested->addListener(listener);

        bool listenerCalled = false;
        listener.onPropertyChange = [&listenerCalled] {
            listenerCalled = true;
        };

        nested->setProperty("number", 2738);
        expect(listenerCalled);

        beginTest("JSON parsing / changes to nested objects reach the parsed object's listeners");
        {
            const auto root = juce::VariantConverter<jive::Object::ReferenceCountedPointer>::fromVar(jive::parseJSON(R"(
                {
                    "nested": {
                        "deeper": {
                            "number": 1,
                        },
                    },
                }
            )"));

            Listener rootListener;
            root->addListener(rootListener);

            auto numCalls = 0;
            rootListener.onPropertyChange = [&numCalls] {
                ++numCalls;
            };

            auto* nestedObject = dynamic_cast<jive::Object*>(root->getProperty("nested").getDynamicObject());
            nestedObject->setProperty("number", 2);
            expectEquals(numCalls, 1);

            auto* deeperObject = dynamic_cast<jive::Object*>(nestedObject->getProperty("deeper").getDynamicObject());
            deeperObject->setProperty("number", 2);
            expectEquals(numCalls, 2);
            expect(deeperObject->getRoot() == root.get());
        }
    }

    void testInitialiserListConstruction()
    {
        beginTest("initialiser-list construction");

        const jive::Object object{
            { "foo", 10 },
            { "bar", 20 },
            {
                "wizz",
                new jive::Object{
                    { "bang", 30 },
                },
            },
        };
        expectEquals(object["foo"], juce::var{ 10 });
        expectEquals(object["bar"], juce::var{ 20 });
        expectEquals(object["wizz"]["bang"], juce::var{ 30 });
    }
};

static ObjectTest objectTest;
#endif
