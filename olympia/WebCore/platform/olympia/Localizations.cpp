/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#include "config.h"
#include "IntSize.h"
#include "LocalizeResource.h"
#include "LocalizedStrings.h"
#include "NotImplemented.h"
#include "PlatformString.h"
#include <locale.h>

namespace WebCore {

String fileButtonChooseFileLabel()
{
    Olympia::Platform::LocalizeResource resource;
    char* string = resource.getString(Olympia::Platform::FILE_CHOOSE_BUTTON_LABEL);
    return string ? String(string): String();
}

String fileButtonNoFileSelectedLabel()
{
    Olympia::Platform::LocalizeResource resource;
    char* string = resource.getString(Olympia::Platform::FILE_BUTTON_NO_FILE_SELECTED_LABEL);
    return string ? String(string): String();
}

String resetButtonDefaultLabel()
{
    Olympia::Platform::LocalizeResource resource;
    char* string = resource.getString(Olympia::Platform::RESET_BUTTON_LABEL);
    return string ? String(string): String();
}

String submitButtonDefaultLabel()
{
    Olympia::Platform::LocalizeResource resource;
    char* string = resource.getString(Olympia::Platform::SUBMIT_BUTTON_LABEL);
    return string ? String(string): String();
}

String inputElementAltText()
{
    notImplemented();
    return String();
}

String defaultLanguage()
{
    const char* locale = setlocale(LC_ALL, 0);
    if (!locale || !*locale || strcmp(locale, "C") == 0 || strcmp(locale, "POSIX") == 0 )
        return "en-US";

    String localeString(locale);
    int dotPosition = localeString.find('.');
    return dotPosition < 0 ? localeString : localeString.left(dotPosition);
}

String contextMenuItemTagBold()
{
    notImplemented();
    return String();
}

String contextMenuItemTagCheckGrammarWithSpelling()
{
    notImplemented();
    return String();
}

String contextMenuItemTagCheckSpelling()
{
    notImplemented();
    return String();
}

String contextMenuItemTagCheckSpellingWhileTyping()
{
    notImplemented();
    return String();
}

String contextMenuItemTagCopyImageToClipboard()
{
    notImplemented();
    return String();
}

String contextMenuItemTagCopyLinkToClipboard()
{
    notImplemented();
    return String();
}

String contextMenuItemTagCopy()
{
    notImplemented();
    return String();
}

String contextMenuItemTagCut()
{
    notImplemented();
    return String();
}

String contextMenuItemTagDefaultDirection()
{
    notImplemented();
    return String();
}

String contextMenuItemTagDownloadImageToDisk()
{
    notImplemented();
    return String();
}

String contextMenuItemTagDownloadLinkToDisk()
{
    notImplemented();
    return String();
}

String contextMenuItemTagFontMenu()
{
    notImplemented();
    return String();
}

String contextMenuItemTagGoBack()
{
    notImplemented();
    return String();
}

String contextMenuItemTagGoForward()
{
    notImplemented();
    return String();
}

String contextMenuItemTagIgnoreGrammar()
{
    notImplemented();
    return String();
}

String contextMenuItemTagIgnoreSpelling()
{
    notImplemented();
    return String();
}

String contextMenuItemTagInspectElement()
{
    notImplemented();
    return String();
}

String contextMenuItemTagItalic()
{
    notImplemented();
    return String();
}

String contextMenuItemTagLearnSpelling()
{
    notImplemented();
    return String();
}

String contextMenuItemTagLeftToRight()
{
    notImplemented();
    return String();
}

String contextMenuItemTagNoGuessesFound()
{
    notImplemented();
    return String();
}

String contextMenuItemTagOpenFrameInNewWindow()
{
    notImplemented();
    return String();
}

String contextMenuItemTagOpenImageInNewWindow()
{
    notImplemented();
    return String();
}

String contextMenuItemTagOpenLinkInNewWindow()
{
    notImplemented();
    return String();
}

String contextMenuItemTagOpenLink()
{
    notImplemented();
    return String();
}

String contextMenuItemTagOutline()
{
    notImplemented();
    return String();
}

String contextMenuItemTagPaste()
{
    notImplemented();
    return String();
}

String contextMenuItemTagReload()
{
    notImplemented();
    return String();
}

String contextMenuItemTagRightToLeft()
{
    notImplemented();
    return String();
}

String contextMenuItemTagSearchWeb()
{
    notImplemented();
    return String();
}

String contextMenuItemTagShowSpellingPanel(bool)
{
    notImplemented();
    return String();
}

String contextMenuItemTagSpellingMenu()
{
    notImplemented();
    return String();
}

String contextMenuItemTagStop()
{
    notImplemented();
    return String();
}

String contextMenuItemTagTextDirectionMenu()
{
    notImplemented();
    return String();
}

String contextMenuItemTagUnderline()
{
    notImplemented();
    return String();
}

String contextMenuItemTagWritingDirectionMenu()
{
    notImplemented();
    return String();
}

String searchableIndexIntroduction()
{
    notImplemented();
    return String();
}

String searchMenuClearRecentSearchesText()
{
    notImplemented();
    return String();
}

String searchMenuNoRecentSearchesText()
{
    notImplemented();
    return String();
}

String searchMenuRecentSearchesText()
{
    notImplemented();
    return String();
}

String imageTitle(String const&, IntSize const&)
{
    notImplemented();
    return String();
}

String AXButtonActionVerb()
{
    notImplemented();
    return String();
}

String AXCheckedCheckBoxActionVerb()
{
    notImplemented();
    return String();
}

String AXDefinitionListDefinitionText()
{
    notImplemented();
    return String();
}

String AXDefinitionListTermText()
{
    notImplemented();
    return String();
}

String AXLinkActionVerb()
{
    notImplemented();
    return String();
}

String AXRadioButtonActionVerb()
{
    notImplemented();
    return String();
}

String AXTextFieldActionVerb()
{
    notImplemented();
    return String();
}

String AXUncheckedCheckBoxActionVerb()
{
    notImplemented();
    return String();
}

String AXMenuListPopupActionVerb()
{
    notImplemented();
    return String();
}

String AXMenuListActionVerb()
{
    notImplemented();
    return String();
}

String unknownFileSizeText()
{
    notImplemented();
    return String();
}

String validationMessageStepMismatchText()
{
    notImplemented();
    return String();
}

String validationMessageRangeOverflowText()
{
    notImplemented();
    return String();
}

String validationMessageRangeUnderflowText()
{
    notImplemented();
    return String();
}

String validationMessagePatternMismatchText()
{
    notImplemented();
    return String();
}

String validationMessageTooLongText()
{
    notImplemented();
    return String();
}

String validationMessageTypeMismatchText()
{
    notImplemented();
    return String();
}

String validationMessageValueMissingText()
{
    notImplemented();
    return String();
}

String localizedMediaControlElementString(const String& controlName)
{
    notImplemented();
    return String();
}

String localizedMediaControlElementHelpText(const String& controlName)
{
    notImplemented();
    return String();
}

String localizedMediaTimeDescription(const String& controlName)
{
    notImplemented();
    return String();
}

String localizedMediaTimeDescription(float time)
{
    notImplemented();
    return String();
}
String mediaElementLoadingStateText()
{
    notImplemented();
    return String();
}

String mediaElementLiveBroadcastStateText()
{
    notImplemented();
    return String();
}

String missingPluginText()
{
    notImplemented();
    return String();
}

String crashedPluginText()
{
    notImplemented();
    return String();
}

} // namespace WebCore
