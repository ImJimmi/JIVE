import lldb
#
# ABOUT
#
# This file allows LLDB to decode some common JUCE structures,
# in particular Arrays, Strings, var objects and ValueTrees.
#
# The facilities aren't quite as nice as those in VisualStudio
# so if you do find you need to disable any functionality you
# can do so in the __lldb_init_module function by commentting
# out the appropriate debugger.* line.
#
# Send bug reports, feature requests and issues to jim@credland.net
#
# USAGE
#
# Put this line in your ~/.lldbinit file:
#
#  command script import [path]
#
# Where [path] is the full path to this file. For example:
#
#  command script import /Users/me/juce-toys/juce_lldb_xcode.py
#
#
# (c) Credland Technical Limited.
# MIT License
#
# JCF_DEBUG - Debugging helpers for JUCE.  Demo application.
#
# Don't forget to install the VisualStudio or Xcode debug scripts as
# well.  These ensure that your IDEs debugger displays the contents
# of ValueTrees, Strings and Arrays in a useful way!
#
#
# Credland Technical Limited provide a range of consultancy and contract
# services including:
# - JUCE software development and support
# - information security consultancy
#
# Contact jim@credland.net
#
###################################
#
# Much Python/LLDB magic...

def __lldb_init_module(debugger, dict):
    print("-- juce decoding modules loaded.  www.credland.net")
    print(" - refer to the help for the 'type' command")

    # Array<*>
    # --inline-children shows the values of children in summaries, -O only shows values, not keys
    debugger.HandleCommand('type summary add -x "^juce::(ReferenceCounted)?Array<.*>" --inline-children -O -w juce')
    debugger.HandleCommand('type synthetic add -x "^juce::(ReferenceCounted)?Array<.*>" --python-class juce_lldb_xcode.ArrayChildrenProvider -w juce')

    # Component
    debugger.HandleCommand('type summary add juce::Component -F juce_lldb_xcode.ComponentSummary -w juce')

    # String
    debugger.HandleCommand('type summary add juce::String --summary-string "${var.text.data}" -w juce')

    # var
    debugger.HandleCommand('type summary add juce::var -F juce_lldb_xcode.var_summary -w juce')

    # Identifier
    debugger.HandleCommand('type summary add juce::Identifier --summary-string "${var.name}" -w juce')

    # NamedValueSet::NamedValue
    debugger.HandleCommand('type summary add juce::NamedValueSet::NamedValue --summary-string "${var.name}: ${var.value}" -w juce')

    # NamedValueSet (for example, ValueTree properties)
    debugger.HandleCommand('type summary add juce::NamedValueSet --inline-children -w juce')
    debugger.HandleCommand('type synthetic add juce::NamedValueSet --python-class juce_lldb_xcode.NamedValueSetProvider -w juce')

    # ValueTree and ValueTree::SharedObject
    debugger.HandleCommand('type summary add juce::ValueTree -F juce_lldb_xcode.value_tree_summary -w juce')
    debugger.HandleCommand('type synthetic add juce::ValueTree --python-class juce_lldb_xcode.ValueTreeChildrenProvider -w juce')
    debugger.HandleCommand('type summary add juce::ValueTree::SharedObject -F juce_lldb_xcode.value_tree_summary -w juce')
    debugger.HandleCommand('type synthetic add juce::ValueTree::SharedObject --python-class juce_lldb_xcode.ValueTreeChildrenProvider -w juce')

    # ScopedPointer
    debugger.HandleCommand('type summary add -x "^juce::ScopedPointer<.*>" --summary-string "ScopedPointer<>=${*var.object}" -w juce')

    # Rectangle<*>
    debugger.HandleCommand('type summary add -x "^juce::Rectangle<.*>" --summary-string "${var.pos.x} ${var.pos.y} ${var.w} ${var.h}" -w juce')

    debugger.HandleCommand('type category enable juce')

def named_value_set_summary(valueObject, dictionary):
    # Reach through the Array and the ArrayBase
    number_of_values = valueObject.GetChildMemberWithName('values').GetChildMemberWithName('values').GetChildMemberWithName('numUsed').GetSummary()
    return number_of_values + " properties"

def value_tree_summary(valueObject, dictionary):
    valueTreeType = valueObject.GetChildMemberWithName('type').GetSummary() or "none"
    # we need to look at the NonSynthetic version of the NamedValueSet to get at numUsed
    numProperties = valueObject.GetChildMemberWithName('properties').GetNonSyntheticValue().GetChildMemberWithName('values').GetChildMemberWithName('values').GetChildMemberWithName('numUsed').GetValue() or "0"
    numChildren = str(valueObject.GetChildMemberWithName('children').GetNonSyntheticValue().GetChildMemberWithName('values').GetChildMemberWithName('numUsed').GetValueAsUnsigned()) or "0"
    return "type: " + valueTreeType + ", " + numProperties + (" property" if numProperties == "1" else " properties") + ", " + numChildren + (" child" if numChildren == "1" else " children")

def rect_summary(valueObject, dictionary):
    w = valueObject.GetChildMemberWithName('w').GetValue()
    h = valueObject.GetChildMemberWithName('h').GetValue()
    pos = valueObject.GetChildMemberWithName('pos')
    x = pos.GetChildMemberWithName('x').GetValue()
    y = pos.GetChildMemberWithName('y').GetValue()
    s = "x=" + str(x) + " y=" + str(y)
    s = s + " w=" + str(w) + " h=" + str(h)
    return s

def var_summary(valueObject, dictionary):
    varType = valueObject.GetChildMemberWithName('type')
    varValue = valueObject.GetChildMemberWithName('value')
    value = varType.GetType()
    if varType.GetChildMemberWithName("isString").GetValue() == "true":
        # the data inside var is char[8] and needs to be cast to a juce::String
        char8Data = varValue.GetChildMemberWithName('stringValue')
        juceStringType = valueObject.GetFrame().GetModule().FindFirstType('juce::String')
        castedStringData = char8Data.Cast(juceStringType).GetSummary()
        value = 'string=' + castedStringData
    elif varType.GetChildMemberWithName("isInt").GetValue() == "true":
        value = "int=" + varValue.GetChildMemberWithName("intValue").GetValue()
    elif varType.GetChildMemberWithName("isDouble").GetValue() == "true":
        value = "double=" + varValue.GetChildMemberWithName("doubleValue").GetValue()
    elif varType.GetChildMemberWithName("isBool").GetValue() == "true":
        value = "bool=" + varValue.GetChildMemberWithName("boolValue").GetValue()
    elif varType.GetChildMemberWithName("isVoid").GetValue() == "true":
        value = "void"
    return value

def ComponentSummary(valueObject, dictionary):
    # Component 'name' parent=None visible=True location={ 0, 0, 20, 20 } opaque=False
    # print(int(valueObject.GetChildMemberWithName('parentComponent').GetValue(), 16))
    hasParent = int(valueObject.GetChildMemberWithName('parentComponent').GetValue(), 16) == 0
    isVisible = valueObject.GetChildMemberWithName('flags').GetChildMemberWithName('visibleFlag').GetValue()
    name = string_summary(valueObject.GetChildMemberWithName('componentName'), dictionary)
    s = name + " hasParent=" + str(hasParent) + " isVisible=" + str(isVisible)
    return s


# This provider isn't DRY. It dupes some behavior from juce::Array.
# I couldn't figure out how to rely on the array provider from this one.
class NamedValueSetProvider:
    def __init__(self, valueObject, internalDict):
        self.valueObject = valueObject
        self.values = self.valueObject.GetChildMemberWithName('values')
        self.update()

    def num_children(self):
        return self.size

    def get_child_index(self, name):
        return int(name.lstrip('[').rstrip(']'))

    def get_child_at_index(self, index):
        # couldn't figure out how to delegate to the "values" member, which is a juce::Array
        # So, instead we'll create children like we do on juce::Array
        offset = index * self.data_size
        return self.first_element.CreateChildAtOffset('[' + str(index) + ']', offset, self.data_type)

    def update(self):
        # Dig into the array to get at the num of elements
        self.size = self.values.GetChildMemberWithName('values').GetChildMemberWithName('numUsed').GetValueAsUnsigned()

        self.first_element = self.values.GetChildMemberWithName('values').GetChildMemberWithName('elements').GetChildMemberWithName('data')

        # this is NamedValueSet::NamedValue
        self.data_type = self.values.GetType().GetTemplateArgumentType(0)
        self.data_size = self.data_type.GetByteSize()

    def has_children(self):
        return self.size > 0

class ValueTreeChildrenProvider:
    def __init__(self, valueObject, internalDict):
        # parents and children are stored as SharedObjects
        if (valueObject.GetTypeName() == "juce::ValueTree::SharedObject"):
            self.valueObject = valueObject
            # print ("ok, shared object is:")
            # print(self.valueObject)
        # parent is stored as a pointer to a SharedObject
        elif (valueObject.GetTypeName() == "juce::ValueTree::SharedObject *"):
            self.valueObject = valueObject.Dereference()
        else:
            # For juce::ValueTree proper, dig out the referenced object
            self.valueObject = valueObject.GetChildMemberWithName('object').GetChildMemberWithName('referencedObject')

        self.update()

    def num_children(self):
        return 4

    def get_child_index(self, name):
        if name == "type":
          return 0

        if name == "parent":
          return 1

        if name == "properties":
          return 2

        if name == "children":
          return 3

    def get_child_at_index(self, index):
        if index == 0:
            return self.type

        if index == 1:
            return self.parent

        if index == 2:
            return self.properties

        if index == 3:
            return self.children

        return None

    # update is called before every other method
    def update(self):
        self.type = self.valueObject.GetChildMemberWithName('type')
        self.parent = self.valueObject.GetChildMemberWithName('parent')
        self.properties = self.valueObject.GetChildMemberWithName('properties')
        # relies on ArrayChildrenProvider being defined for ReferenceCountedArray
        self.children = self.valueObject.GetChildMemberWithName('children')

    def has_children(self):
        return True

class ArrayChildrenProvider:
    def __init__(self, valueObject, internalDict):
        self.valueObject = valueObject
        self.count = 0
        self.update()

    def num_children(self):
        return self.count

    def get_child_index(self, name):
        return int(name.lstrip('[').rstrip(']'))

    def get_child_at_index(self, index):
        offset = index * self.data_size
        return self.first_element.CreateChildAtOffset('[' + str(index) + ']', offset, self.data_type)

    def update(self):
        self.first_element = self.valueObject.GetChildMemberWithName('values').GetChildMemberWithName('elements').GetChildMemberWithName('data')

        # ReferenceCountedArrays store pointers, *not* the objects themselves
        if (self.valueObject.GetTypeName().startswith("juce::ReferenceCounted")):
            self.data_type = self.valueObject.GetType().GetTemplateArgumentType(0).GetPointerType()
        else:
            # type of first template argument. For example juce::Array<int> would be int
            self.data_type = self.valueObject.GetType().GetTemplateArgumentType(0)

        self.data_size = self.data_type.GetByteSize()
        if self.first_element.IsValid():
            self.count = self.valueObject.GetChildMemberWithName('values').GetChildMemberWithName('numUsed').GetValueAsUnsigned()
        else:
            self.count = 0

    def has_children(self):
        return self.count > 0
