/*
 * Copyright (C) Research In Motion Limited 2009. All rights reserved.
 */

#ifndef BlackBerryPlatformReplaceText_h
#define BlackBerryPlatformReplaceText_h

#include <stdint.h>

namespace BlackBerry {
namespace Platform {

/**
 * The following structures are to be used for the IPC between the Java input framework
 * and the native browser platform. They provide templates for creating the data blob
 * for marshalling the AttributedString Java object to the browser, as well as
 * for decomposition of this blob by the browser.
 *
 * The browser class owning the text of an input field in an HTML page, should
 * implement a member function like the following:
 *
 * <code>int replace(unsigned int start, unsigned int end, unsigned int attributeStart, AttributedText& attributedText);</code>
 *
 * where <code>start</code> is the index of the first character of
 * the input-field text to be replaced/deleted,
 * <code>end</code> is the index of the next after the last character
 * to be replaced/deleted/appended,
 * <code>attributeStart</code> offset in attributedText where attributes should begin to apply,
 * <code>attributeEnd</code> offset + 1 in attributedText where attributes should end to apply,
 * <code>attributedText</code> is the Java AttributedString representation of the text
 * which is to replace the input-field characters with indices <code>start...end-1</code>.
 * If the length of this text is zero, the <code>start...end-1</code> characters are
 * to be deleted. If <code>start</code> equals to the length of the input-field text,
 * the <code>attributedText</code> characters are to be appended to the input-field text.
 *
 * <code>attributedText</code> is a sequence of characters broken down into one or more runs, each of
 * them having distinctive attributes, for example, underline, highlight, or bold.
 *
 * Function <code>replace()</code> should return 0, if successful, or the error code according
 * to <code>ErrorCode</code> below.
 */

// This structure represents an AttributedText run:
struct AttribRun {
    unsigned int    offset;     // Character offset from the beginning of AttributedText
    unsigned int    length;     // Number of characters in this run
    uint64_t        attributes; // The text attributes represented by bit fields
};

// The structure providing access to the AttributedText data:
struct AttributedText {
    unsigned int    length;    // Length of the text
    unsigned short* text;      // Pointer to the first text character
    unsigned int    runsCount; // Number of runs in the text
    void*           runs;      // Pointer to the first run
};

// The following structure lets extract first three arguiments for the replace() function
// out of the shared memory data blob and construct the AttributedText structure;
// the data blob starts with the data of this structure and continues with the AttributedText
// runs followed by the AttributedText characters.

// The size of this structure in bytes must be multiple of 8 in order to align with a double-word
// boundary to allocate the runs after it.

struct ReplaceArguments {
    unsigned int    start;              // First character to be deleted
    unsigned int    end;                // Next to the last character to be deleted
    unsigned int    attributeStart;     // First character in the AttributedText to have the attributes set
    unsigned int    attributeEnd;       // Next to the last character in the AttributedText to have the attributes set
    unsigned int    replacementLength;  // Length of AttributedText
    unsigned int    runsCount;          // Number of runs of AttributedText
    unsigned int    cursorPosition;     // Location to set the cursor after replacement operation
    unsigned int    caretShape;         // Currently not used and is necessary as a padding to have the structure side being multiple of 8
};

// Error codes to be returned by the replaceText() function or the attributedText constructing code
// and, possibly, forwarded to the Java platform:

enum ReplaceTextErrorCode {
    ReplaceTextSuccess = 0,
    ReplaceTextNonSpecifiedFault,       // any error not described below.
    ReplaceTextWrongStartArgument,      // start is negative or greater than the length of the input-field text
    ReplaceTextWrongEndArgument,        // end < start
    ReplaceTextWrongAttributeStart,     // attributeStart is negative or greater that attributedText.length
    ReplaceTextWrongAttributeEnd,       // attributeEnd is less than attributedStart or greater than text length.
    ReplaceTextWrongCursorLocation,     // the provided cursor location is invalid.
    ReplaceTextInvalidRunlength,        // the provided attributes and runs are inconsisten with the provided text length.
    ReplaceTextBlobCorrupted,           // the blob length is inconsistent with its length parameters
    ReplaceTextChangeNotValidated,      // the expected change does not match the actual text.  Typically Javascript modification of text.
    ReplaceTextChangeDeniedSyncMismatch // Sync state is not consistent, change has been rejected.
};

// Since the replace() function has to interpret attributes, they may be retrieved
// by the following functions with the self-explaining names:

static int getAtt(uint64_t aAttrib, uint64_t aMask, int aShift) { return (int)((aAttrib & aMask) >> aShift) - 1; }
static const int FontWeightShift = 8;
static const uint64_t FontWeightMask = (uint64_t)511L << FontWeightShift; // use 9 bits; we have to accommodate 256 actual values plus 'not set'
static int getFontWeightAttrib(uint64_t aAttrib) { return getAtt(aAttrib, FontWeightMask, FontWeightShift); }
static const int ItalicShift = 17;
static const uint64_t ItalicMask = (uint64_t)3L << ItalicShift;
static int getItalicAttrib(uint64_t aAttrib) { return getAtt(aAttrib, ItalicMask, ItalicShift); }
static const int UnderlineShift = 24;
static const uint64_t UnderlineMask = (uint64_t)7L << UnderlineShift;
static int getUnderlineAttrib(uint64_t aAttrib) { return getAtt(aAttrib, UnderlineMask, UnderlineShift); }
static const int PASSWORD_SHIFT = 38;
static const uint64_t PASSWORD_MASK = (uint64_t)3L << PASSWORD_SHIFT;
static int getPasswordAttrib(uint64_t aAttrib) { return getAtt(aAttrib,PASSWORD_MASK,PASSWORD_SHIFT); }
/* The underline attributes have the following values (as documented in comments in AttributedString.java, and in the TID::TUnderline class in native code):
            No underline = 0,
            Standard underline = 1,
            Broken underline = 2,
            Dotted underline = 3,
            Wavy underline = 4
*/
static const int StrikethroughShift = 27;
static const uint64_t StrikethroughMask = (uint64_t)7L << StrikethroughShift;
static int getStrikethroughAttrib(uint64_t aAttrib) { return getAtt(aAttrib, StrikethroughMask, StrikethroughShift); }
static const int HighlightShift = 36;
static const uint64_t HighlightMask = (uint64_t)3L << HighlightShift;
static int getHighlightAttrib(uint64_t aAttrib) { return getAtt(aAttrib, HighlightMask, HighlightShift); }

// Unsupported Attribute Types

//static const int FONT_HEIGHT_SHIFT = 0;
//static const uint64_t FONT_HEIGHT_MASK = 127;
//static int getFontHeightAttrib(uint64_t aAttrib) { return getAtt(aAttrib,FONT_HEIGHT_MASK,FONT_HEIGHT_SHIFT); }
//static const uint64_t FONT_HEIGHT_EXCLUDES_LEADING_MASK = 1 << 7;
//static const int ANTI_ALIAS_SHIFT = 19;
//static const uint64_t ANTI_ALIAS_MASK = (uint64_t)7L << ANTI_ALIAS_SHIFT;
//static int getAntiAliasAttrib(uint64_t aAttrib) { return getAtt(aAttrib,ANTI_ALIAS_MASK,ANTI_ALIAS_SHIFT); }
//static const int BASELINE_OFFSET_SHIFT = 22;
//static const uint64_t BASELINE_OFFSET_MASK = (uint64_t)3L << BASELINE_OFFSET_SHIFT;
//static int getBaselineOffsetAttrib(uint64_t aAttrib) { return getAtt(aAttrib,BASELINE_OFFSET_MASK,BASELINE_OFFSET_SHIFT); }
//static const int JUSTIFY_SHIFT = 30;
//static const uint64_t JUSTIFY_MASK = (uint64_t)7L << JUSTIFY_SHIFT;
//static int getJustifyAttrib(uint64_t aAttrib) { return getAtt(aAttrib,JUSTIFY_MASK,JUSTIFY_SHIFT); }
//static const int PAR_DIR_SHIFT = 33;
//static const uint64_t PAR_DIR_MASK = (uint64_t)7L << PAR_DIR_SHIFT;
//static int getParDirAttrib(uint64_t aAttrib) { return getAtt(aAttrib,PAR_DIR_MASK,PAR_DIR_SHIFT); }
//static const int HAN_STYLE_SHIFT = 40;
//static const uint64_t HAN_STYLE_MASK = (uint64_t)15L << HAN_STYLE_SHIFT;
//static int getHanStyleAttrib(uint64_t aAttrib) { return getAtt(aAttrib,HAN_STYLE_MASK,HAN_STYLE_SHIFT); }
//static const int SERIF_SHIFT = 44;
//static const uint64_t SERIF_MASK = (uint64_t)3L << SERIF_SHIFT;
//static int getSerifAttrib(uint64_t aAttrib) { return getAtt(aAttrib,SERIF_MASK,SERIF_SHIFT); }
//static const int CURSIVE_SHIFT = 46;
//static const uint64_t CURSIVE_MASK = (uint64_t)3L << CURSIVE_SHIFT;
//static int getCursiveAttrib(uint64_t aAttrib) { return getAtt(aAttrib,CURSIVE_MASK,CURSIVE_SHIFT); }
//static const int FANTASY_SHIFT = 48;
//static const uint64_t FANTASY_MASK = (uint64_t)3L << FANTASY_SHIFT;
//static int getFantasyAttrib(uint64_t aAttrib) { return getAtt(aAttrib,FANTASY_MASK,FANTASY_SHIFT); }
//static const int FONT_INDEX_SHIFT = 50;
//static const uint64_t FONT_INDEX_MASK = (uint64_t)63L << FONT_INDEX_SHIFT;
//static int getFontIndex(uint64_t aAttrib) { return getAtt(aAttrib,FONT_INDEX_MASK,FONT_INDEX_SHIFT); }

// Font Weight Values
enum FontWeights {
    EUnsetWeight = -1,
    EDefaultWeight = 0,
    ELightWeight = 32,
    EMediumWeight = 64,
    EBoldWeight = 128,
    EExtraBoldWeight = 192,
    ESuperBoldWeight = 255
};

enum AttributeValue {
    AttributeNotProvided = -1,
    AttributeNotApplied = 0,
    AttributeApplied = 1
};

enum UnderlineStyles {
    UnderlineNotProvided = -1,
    NoUnderline = 0,
    StandardUnderline = 1,
    BrokenUnderline = 2,
    DottedUnderline = 3,
    WavyUnderline = 4
};

} // namespace Olympia
} // namespace Platform

#endif // BlackBerryPlatformReplaceText_h
