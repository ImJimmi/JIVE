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

    Object::Object()
        : internalListener{ std::make_unique<InternalListener>(*this) }
    {
    }

    Object::Object(std::initializer_list<juce::NamedValueSet::NamedValue> initialProperties)
        : internalListener{ std::make_unique<InternalListener>(*this) }
    {
        for (auto& [name, value] : initialProperties)
            setProperty(name, value);
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

        for (auto i = 0; i < result.getProperties().size(); i++)
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
    void Object::removeProperty(const juce::Identifier& propertyName)
    {
        if (DynamicObject::getProperties().remove(propertyName))
            listeners.call(&Listener::propertyChanged, *this, propertyName);
    }

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

    const juce::var& Object::operator[](const juce::Identifier& name) const noexcept
    {
        return getProperties()[name];
    }

    [[nodiscard]] static auto findCommentStart(const juce::String& line)
    {
        auto inString = false;

        for (auto index = 0; index < line.length() - 1; ++index)
        {
            const auto character = line[index];

            if (inString && character == '\\')
                ++index;
            else if (character == '"')
                inString = !inString;
            else if (!inString && character == '/' && line[index + 1] == '/')
                return index;
        }

        return line.length();
    }

    [[nodiscard]] static auto withoutComments(const juce::String& jsonString)
    {
        auto lines = juce::StringArray::fromLines(jsonString);

        for (auto& line : lines)
            line = line.substring(0, findCommentStart(line));

        return lines.joinIntoString("\n");
    }

    juce::var parseJSON(const juce::String& jsonString)
    {
        auto value = juce::JSON::parse(withoutComments(jsonString));
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

    var VariantConverter<jive::Object::ReferenceCountedPointer>::toVar(jive::Object::ReferenceCountedPointer object)
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

        beginTest("JSON parsing / changes to nested objects reach the parsed object's listeners");
        {
            auto root = juce::VariantConverter<jive::Object::ReferenceCountedPointer>::fromVar(jive::parseJSON(R"(
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
                numCalls++;
            };

            auto* nestedObject = dynamic_cast<jive::Object*>(root->getProperty("nested").getDynamicObject());
            nestedObject->setProperty("number", 2);
            expectEquals(numCalls, 1);

            auto* deeperObject = dynamic_cast<jive::Object*>(nestedObject->getProperty("deeper").getDynamicObject());
            deeperObject->setProperty("number", 2);
            expectEquals(numCalls, 2);
            expect(deeperObject->getRoot() == root.get());
        }

        beginTest("JSON parsing / comments are ignored, but not inside strings");
        {
            auto root = juce::VariantConverter<jive::Object::ReferenceCountedPointer>::fromVar(jive::parseJSON(R"(
                // A leading comment
                {
                    "url": "https://github.com/ImJimmi/JIVE", // A trailing comment
                    // "commented": "out",
                    "quoted": "\"// not a comment\"",
                    "slash": "\\", // A comment after an escaped backslash
                }
            )"));

            expect(root != nullptr);
            expectEquals(root->getProperty("url"), juce::var{ "https://github.com/ImJimmi/JIVE" });
            expect(!root->hasProperty("commented"));
            expectEquals(root->getProperty("quoted"), juce::var{ "\"// not a comment\"" });
            expectEquals(root->getProperty("slash"), juce::var{ "\\" });
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
