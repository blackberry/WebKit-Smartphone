/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#include "AccessibilityUIElement.h"
#include "NotImplemented.h"

AccessibilityUIElement::AccessibilityUIElement(PlatformUIElement element)
    : m_element(element)
{
}

AccessibilityUIElement::AccessibilityUIElement(const AccessibilityUIElement& other)
    : m_element(other.m_element)
{
}

AccessibilityUIElement::~AccessibilityUIElement()
{
}

void AccessibilityUIElement::getLinkedUIElements(Vector<AccessibilityUIElement>& elements)
{
    notImplemented();
}

void AccessibilityUIElement::getDocumentLinks(Vector<AccessibilityUIElement>&)
{
    notImplemented();
}

void AccessibilityUIElement::getChildren(Vector<AccessibilityUIElement>& children)
{
    notImplemented();
}

void AccessibilityUIElement::getChildrenWithRange(Vector<AccessibilityUIElement>& elementVector, unsigned start, unsigned end)
{
    notImplemented();
}

int AccessibilityUIElement::childrenCount()
{
    notImplemented();
    return 0;
}

AccessibilityUIElement AccessibilityUIElement::elementAtPoint(int x, int y)
{
    notImplemented();
    return 0;
}

AccessibilityUIElement AccessibilityUIElement::getChildAtIndex(unsigned index)
{
    notImplemented();
    return 0;
}

AccessibilityUIElement AccessibilityUIElement::linkedUIElementAtIndex(unsigned index)
{
    notImplemented();
    return 0;
}

JSStringRef AccessibilityUIElement::allAttributes()
{
    notImplemented();
    return 0;
}

JSStringRef AccessibilityUIElement::attributesOfLinkedUIElements()
{
    notImplemented();
    return 0;
}

JSStringRef AccessibilityUIElement::attributesOfDocumentLinks()
{
    notImplemented();
    return 0;
}

AccessibilityUIElement AccessibilityUIElement::titleUIElement()
{
    notImplemented();
    return 0;
}

AccessibilityUIElement AccessibilityUIElement::parentElement()
{
    notImplemented();
    return 0;
}

JSStringRef AccessibilityUIElement::attributesOfChildren()
{
    notImplemented();
    return 0;
}

JSStringRef AccessibilityUIElement::parameterizedAttributeNames()
{
    notImplemented();
    return 0;
}

JSStringRef AccessibilityUIElement::role()
{
    notImplemented();
    return 0;
}

JSStringRef AccessibilityUIElement::subrole()
{
    notImplemented();
    return 0;
}

JSStringRef AccessibilityUIElement::roleDescription()
{
    notImplemented();
    return 0;
}

JSStringRef AccessibilityUIElement::title()
{
    notImplemented();
    return 0;
}

JSStringRef AccessibilityUIElement::description()
{
    notImplemented();
    return 0;
}

JSStringRef AccessibilityUIElement::stringValue()
{
    notImplemented();
    return 0;
}

JSStringRef AccessibilityUIElement::language()
{
    notImplemented();
    return 0;
}

double AccessibilityUIElement::x()
{
    notImplemented();
    return 0;
}

double AccessibilityUIElement::y()
{
    notImplemented();
    return 0;
}

double AccessibilityUIElement::width()
{
    notImplemented();
    return 0;
}

double AccessibilityUIElement::height()
{
    notImplemented();
    return 0;
}

double AccessibilityUIElement::clickPointX()
{
    notImplemented();
    return 0;
}

double AccessibilityUIElement::clickPointY()
{
    notImplemented();
    return 0;
}

JSStringRef AccessibilityUIElement::orientation() const
{
    notImplemented();
    return 0;
}

double AccessibilityUIElement::minValue()
{
    notImplemented();
    return 0;
}

double AccessibilityUIElement::maxValue()
{
    notImplemented();
    return 0;
}

JSStringRef AccessibilityUIElement::valueDescription()
{
    notImplemented();
    return 0;
}

bool AccessibilityUIElement::isEnabled()
{
    notImplemented();
    return 0;
}

int AccessibilityUIElement::insertionPointLineNumber()
{
    notImplemented();
    return 0;
}

bool AccessibilityUIElement::isActionSupported(JSStringRef action)
{
    notImplemented();
    return 0;
}

bool AccessibilityUIElement::isRequired() const
{
    notImplemented();
    return 0;
}

bool AccessibilityUIElement::isSelected() const
{
    notImplemented();
    return 0;
}

int AccessibilityUIElement::hierarchicalLevel() const
{
    notImplemented();
    return 0;
}

bool AccessibilityUIElement::ariaIsGrabbed() const
{
    notImplemented();
    return 0;
}

JSStringRef AccessibilityUIElement::ariaDropEffects() const
{
    notImplemented();
    return 0;
}

bool AccessibilityUIElement::isExpanded() const
{
    notImplemented();
    return 0;
}

JSStringRef AccessibilityUIElement::attributesOfColumnHeaders()
{
    notImplemented();
    return 0;
}

JSStringRef AccessibilityUIElement::attributesOfRowHeaders()
{
    notImplemented();
    return 0;
}

JSStringRef AccessibilityUIElement::attributesOfColumns()
{
    notImplemented();
    return 0;
}

JSStringRef AccessibilityUIElement::attributesOfRows()
{
    notImplemented();
    return 0;
}

JSStringRef AccessibilityUIElement::attributesOfVisibleCells()
{
    notImplemented();
    return 0;
}

JSStringRef AccessibilityUIElement::attributesOfHeader()
{
    notImplemented();
    return 0;
}

int AccessibilityUIElement::indexInTable()
{
    notImplemented();
    return 0;
}

JSStringRef AccessibilityUIElement::rowIndexRange()
{
    notImplemented();
    return 0;
}

JSStringRef AccessibilityUIElement::columnIndexRange()
{
    notImplemented();
    return 0;
}

int AccessibilityUIElement::lineForIndex(int)
{
    notImplemented();
    return 0;
}

JSStringRef AccessibilityUIElement::boundsForRange(unsigned location, unsigned length)
{
    notImplemented();
    return 0;
}

JSStringRef AccessibilityUIElement::stringForRange(unsigned, unsigned)
{
    notImplemented();
    return 0;
}

AccessibilityUIElement AccessibilityUIElement::cellForColumnAndRow(unsigned column, unsigned row)
{
    notImplemented();
    return 0;
}

JSStringRef AccessibilityUIElement::selectedTextRange()
{
    notImplemented();
    return 0;
}

void AccessibilityUIElement::setSelectedTextRange(unsigned location, unsigned length)
{
    notImplemented();
}

bool AccessibilityUIElement::isAttributeSettable(JSStringRef attribute)
{
    notImplemented();
    return 0;
}

bool AccessibilityUIElement::isAttributeSupported(JSStringRef attribute)
{
    notImplemented();
    return 0;
}

void AccessibilityUIElement::increment()
{
    notImplemented();
}

void AccessibilityUIElement::decrement()
{
    notImplemented();
}

void AccessibilityUIElement::showMenu()
{
    notImplemented();
}

AccessibilityUIElement AccessibilityUIElement::disclosedRowAtIndex(unsigned index)
{
    notImplemented();
    return 0;
}

AccessibilityUIElement AccessibilityUIElement::ariaOwnsElementAtIndex(unsigned index)
{
    notImplemented();
    return 0;
}

AccessibilityUIElement AccessibilityUIElement::ariaFlowToElementAtIndex(unsigned index)
{
    notImplemented();
    return 0;
}

AccessibilityUIElement AccessibilityUIElement::selectedRowAtIndex(unsigned index)
{
    notImplemented();
    return 0;
}

AccessibilityUIElement AccessibilityUIElement::disclosedByRow()
{
    notImplemented();
    return 0;
}

JSStringRef AccessibilityUIElement::accessibilityValue() const
{
    notImplemented();
    return 0;
}

JSStringRef AccessibilityUIElement::documentEncoding()
{
    notImplemented();
    return 0;
}

JSStringRef AccessibilityUIElement::documentURI()
{
    notImplemented();
    return 0;
}

unsigned AccessibilityUIElement::indexOfChild(AccessibilityUIElement*)
{
    notImplemented();
    return 0;
}

bool AccessibilityUIElement::boolAttributeValue(JSStringRef attribute)
{
    notImplemented();
    return 0;
}

JSStringRef AccessibilityUIElement::stringAttributeValue(JSStringRef attribute)
{
    notImplemented();
    return 0;
}

double AccessibilityUIElement::intValue() const
{
    notImplemented();
    return 0;
}

bool AccessibilityUIElement::isChecked() const
{
    notImplemented();
    return 0;
}

JSStringRef AccessibilityUIElement::url()
{
    notImplemented();
    return 0;
}


bool AccessibilityUIElement::addNotificationListener(JSObjectRef functionCallback)
{
    notImplemented();
    return 0;
}

bool AccessibilityUIElement::isSelectable() const
{
    notImplemented();
    return false;
}

bool AccessibilityUIElement::isMultiSelectable() const
{
    notImplemented();
    return false;
}

bool AccessibilityUIElement::isVisible() const
{
    notImplemented();
    return false;
}

bool AccessibilityUIElement::isOffScreen() const
{
    notImplemented();
    return false;
}

bool AccessibilityUIElement::isCollapsed() const
{
    notImplemented();
    return false;
}

bool AccessibilityUIElement::hasPopup() const
{
    notImplemented();
    return false;
}

void AccessibilityUIElement::takeFocus()
{
    notImplemented();
}

void AccessibilityUIElement::takeSelection()
{
    notImplemented();
}

void AccessibilityUIElement::addSelection()
{
    notImplemented();
}

void AccessibilityUIElement::removeSelection()
{
    notImplemented();
}

int AccessibilityUIElement::columnCount()
{
    notImplemented();
    return 0;
}

void AccessibilityUIElement::removeNotificationListener()
{
    notImplemented();
}

void AccessibilityUIElement::press()
{
    notImplemented();
}

int AccessibilityUIElement::rowCount()
{
    notImplemented();
    return 0;
}

JSStringRef AccessibilityUIElement::helpText() const
{
    notImplemented();
    return 0;
}

JSStringRef AccessibilityUIElement::attributedStringForRange(unsigned, unsigned)
{
    notImplemented();
    return 0;
}

bool AccessibilityUIElement::attributedStringRangeIsMisspelled(unsigned location, unsigned length)
{
    notImplemented();
    return 0;
}

bool AccessibilityUIElement::isIgnored() const
{
    // FIXME: implement
    return false;
}
