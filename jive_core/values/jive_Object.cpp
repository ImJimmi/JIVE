#include <jive_core/jive_core.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
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

            for (auto& [name, value] : object.getProperties())
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

    //==================================================================================================================
    Object::Object()
        : internalListener{ std::make_unique<InternalListener>(*this) }
    {
    }

    Object::Object(const Object& other)
        : juce::DynamicObject{ dynamic_cast<const DynamicObject&>(other) }
        , internalListener{ std::make_unique<InternalListener>(*this) }
    {
    }

    Object::Object(Object&& other)
        : juce::DynamicObject{ std::move(dynamic_cast<DynamicObject&&>(other)) }
        , internalListener{ std::make_unique<InternalListener>(*this) }
    {
    }

    Object::Object(const juce::DynamicObject& other)
        : juce::DynamicObject{ other }
    {
    }

    //==================================================================================================================
    void Object::setProperty(const juce::Identifier& propertyName,
                             const juce::var& newValue)
    {
        const auto propertyChanged = DynamicObject::getProperties()
                                         .set(propertyName, newValue);

        if (propertyChanged)
            listeners.call(&Listener::propertyChanged, *this, propertyName);
    }

    const juce::NamedValueSet& Object::getProperties() const
    {
        return dynamic_cast<juce::DynamicObject*>(const_cast<Object*>(this))->getProperties();
    }

    //==================================================================================================================
    void Object::addListener(Listener& listener) const
    {
        listeners.add(&listener);
    }

    void Object::removeListener(Listener& listener) const
    {
        listeners.remove(&listener);
    }

    //==================================================================================================================
    void replaceDynamicObjectsWithJiveObjects(juce::var& value)
    {
        if (auto* dynamicObject = value.getDynamicObject())
        {
            for (auto i = 0; i < dynamicObject->getProperties().size(); i++)
                replaceDynamicObjectsWithJiveObjects(*dynamicObject->getProperties().getVarPointerAt(i));

            Object::ReferenceCountedPointer object = new Object{ std::move(*dynamicObject) };
            value = object;
        }

        if (auto* array = value.getArray())
        {
            for (auto& element : *array)
                replaceDynamicObjectsWithJiveObjects(element);
        }
    }

    juce::var parseJSON(const juce::String& jsonString)
    {
        auto value = juce::JSON::parse(jsonString);
        replaceDynamicObjectsWithJiveObjects(value);

        return value;
    }
} // namespace jive

//======================================================================================================================
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

    var VariantConverter<jive::Object::ReferenceCountedPointer>::toVar(jive::Object::ReferenceCountedPointer object)
    {
        return var{ object };
    }
} // namespace juce

//======================================================================================================================
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
    }

private:
    struct Listener : public jive::Object::Listener
    {
        void propertyChanged(jive::Object&, const juce::Identifier&)
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
        listener.onPropertyChange = [&callbackCalled]() {
            callbackCalled = true;
        };

        object.setProperty("value", 123);
        expect(callbackCalled);

        object.setProperty("nested", jive::parseJSON(R"(
            {
                "number": "456",
            }
        )"));
        auto nested = juce::VariantConverter<jive::Object::ReferenceCountedPointer>::fromVar(object.getProperty("nested"));
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

        auto* object = dynamic_cast<jive::Object*>(value.getDynamicObject());
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
        auto nested = juce::VariantConverter<jive::Object::ReferenceCountedPointer>::fromVar(object->getProperty("nested"));
        expect(nested != nullptr);
        expect(nested->hasProperty("number"));
        expectEquals(nested->getProperty("number"), juce::var{ 47 });

        Listener listener;
        nested->addListener(listener);

        bool listenerCalled = false;
        listener.onPropertyChange = [&listenerCalled]() {
            listenerCalled = true;
        };

        nested->setProperty("number", 2738);
        expect(listenerCalled);
    }
};

static ObjectTest objectTest;
#endif
