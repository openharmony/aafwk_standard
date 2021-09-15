/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef FOUNDATION_AAFWK_STANDARD_TOOLS_UTF_H
#define FOUNDATION_AAFWK_STANDARD_TOOLS_UTF_H

#include <stdint.h>

namespace OHOS {
namespace AAFwk {
namespace LIBZIP {

using UChar32 = int32_t;
using UChar = uint16_t;
using UBool = int8_t;
/**
 * This value is intended for sentinel values for APIs that
 * (take or) return single code points (UChar32).
 * It is outside of the Unicode code point range 0..0x10ffff.
 *
 * For example, a "done" or "error" value in a new API
 * could be indicated with CBU_SENTINEL.
 *
 * ICU APIs designed before ICU 2.4 usually define service-specific "done"
 * values, mostly 0xffff.
 * Those may need to be distinguished from
 * actual U+ffff text contents by calling functions like
 * CharacterIterator::hasNext() or UnicodeString::length().
 *
 * @return -1
 * @see UChar32
 */
#define UTF_SENTINEL (-1)

/**
 * Is this code point a Unicode noncharacter?
 * @param c 32-bit code point
 * @return TRUE or FALSE
 */
#define UTF_IS_UNICODE_NONCHAR(c) \
    ((c) >= 0xfdd0 && ((uint32_t)(c) <= 0xfdef || ((c)&0xfffe) == 0xfffe) && (uint32_t)(c) <= 0x10ffff)

/**
 * Is c a Unicode code point value (0..U+10ffff)
 * that can be assigned a character?
 *
 * Code points that are not characters include:
 * - single surrogate code points (U+d800..U+dfff, 2048 code points)
 * - the last two code points on each plane (U+__fffe and U+__ffff, 34 code points)
 * - U+fdd0..U+fdef (new with Unicode 3.1, 32 code points)
 * - the highest Unicode code point value is U+10ffff
 *
 * This means that all code points below U+d800 are character code points,
 * and that boundary is tested first for performance.
 *
 * @param c 32-bit code point
 * @return TRUE or FALSE
 */
#define UTF_IS_UNICODE_CHAR(c) \
    ((uint32_t)(c) < 0xd800 || ((uint32_t)(c) > 0xdfff && (uint32_t)(c) <= 0x10ffff && !UTF_IS_UNICODE_NONCHAR(c)))

/**
 * Is this code point a surrogate (U+d800..U+dfff)?
 * @param c 32-bit code point
 * @return TRUE or FALSE
 */
#define UTF_IS_SURROGATE(c) (((c)&0xfffff800) == 0xd800)

/**
 * Assuming c is a surrogate code point (U_IS_SURROGATE(c)),
 * is it a lead surrogate?
 * @param c 32-bit code point
 * @return TRUE or FALSE
 */
#define UTF_IS_SURROGATE_LEAD(c) (((c)&0x400) == 0)

// UTF-8 macros
// from utf8.h

extern const uint8_t utf8_countTrailBytes[256];

/**
 * Count the trail bytes for a UTF-8 lead byte.
 * @internal
 */
#define UTF8_COUNT_TRAIL_BYTES(leadByte) (utf8_countTrailBytes[(uint8_t)leadByte])

/**
 * Mask a UTF-8 lead byte, leave only the lower bits that form part of the code point value.
 * @internal
 */
#define UTF8_MASK_LEAD_BYTE(leadByte, countTrailBytes) ((leadByte) &= (1 << (6 - (countTrailBytes))) - 1)

/**
 * Does this code unit (byte) encode a code point by itself (US-ASCII 0..0x7f)?
 * @param c 8-bit code unit (byte)
 * @return TRUE or FALSE
 */
#define UTF8_IS_SINGLE(c) (((c)&0x80) == 0)

/**
 * Is this code unit (byte) a UTF-8 lead byte?
 * @param c 8-bit code unit (byte)
 * @return TRUE or FALSE
 */
#define UTF8_IS_LEAD(c) ((uint8_t)((c)-0xc0) < 0x3e)

/**
 * Is this code unit (byte) a UTF-8 trail byte?
 * @param c 8-bit code unit (byte)
 * @return TRUE or FALSE
 */
#define UTF8_IS_TRAIL(c) (((c)&0xc0) == 0x80)

/**
 * How many code units (bytes) are used for the UTF-8 encoding
 * of this Unicode code point?
 * @param c 32-bit code point
 * @return 1..4, or 0 if c is a surrogate or not a Unicode code point
 */
#define UTF8_LENGTH(c)                                                                                                 \
    ((uint32_t)(c) <= 0x7f ? 1                                                                                         \
                           : ((uint32_t)(c) <= 0x7ff                                                                   \
                                     ? 2                                                                               \
                                     : ((uint32_t)(c) <= 0xd7ff ? 3                                                    \
                                                                : ((uint32_t)(c) <= 0xdfff || (uint32_t)(c) > 0x10ffff \
                                                                          ? 0                                          \
                                                                          : ((uint32_t)(c) <= 0xffff ? 3 : 4)))))

/**
 * The maximum number of UTF-8 code units (bytes) per Unicode code point (U+0000..U+10ffff).
 * @return 4
 */
#define UTF8_MAX_LENGTH 4

/**
 * Function for handling "next code point" with error-checking.
 * @internal
 */
UChar32 Utf8NextCharSafeBody(const uint8_t *s, int32_t *pi, int32_t length, UChar32 c, UBool strict);

/**
 * Get a code point from a string at a code point boundary offset,
 * and advance the offset to the next code point boundary.
 * (Post-incrementing forward iteration.)
 * "Safe" macro, checks for illegal sequences and for string boundaries.
 *
 * The offset may point to the lead byte of a multi-byte sequence,
 * in which case the macro will read the whole sequence.
 * If the offset points to a trail byte or an illegal UTF-8 sequence, then
 * c is set to a negative value.
 *
 * @param s const uint8_t * string
 * @param i string offset, i<length
 * @param length string length
 * @param c output UChar32 variable, set to <0 in case of an error
 * @see UTF8_NEXT_UNSAFE
 */
#define UTF8_NEXT(s, i, length, c)                                                              \
    {                                                                                           \
        (c) = (s)[(i)++];                                                                       \
        if (((uint8_t)(c)) >= 0x80) {                                                           \
            if (UTF8_IS_LEAD(c)) {                                                              \
                (c) = Utf8NextCharSafeBody((const uint8_t *)s, &(i), (int32_t)(length), c, -1); \
            } else {                                                                            \
                (c) = UTF_SENTINEL;                                                             \
            }                                                                                   \
        }                                                                                       \
    }

/**
 * Append a code point to a string, overwriting 1 to 4 bytes.
 * The offset points to the current end of the string contents
 * and is advanced (post-increment).
 * "Unsafe" macro, assumes a valid code point and sufficient space in the
 * string.
 * Otherwise, the result is undefined.
 *
 * @param s const uint8_t * string buffer
 * @param i string offset
 * @param c code point to append
 * @see UTF8_APPEND
 */
#define UTF8_APPEND_UNSAFE(s, i, c)                                      \
    {                                                                    \
        if ((uint32_t)(c) <= 0x7f) {                                     \
            (s)[(i)++] = (uint8_t)(c);                                   \
        } else {                                                         \
            if ((uint32_t)(c) <= 0x7ff) {                                \
                (s)[(i)++] = (uint8_t)(((c) >> 6) | 0xc0);               \
            } else {                                                     \
                if ((uint32_t)(c) <= 0xffff) {                           \
                    (s)[(i)++] = (uint8_t)(((c) >> 12) | 0xe0);          \
                } else {                                                 \
                    (s)[(i)++] = (uint8_t)(((c) >> 18) | 0xf0);          \
                    (s)[(i)++] = (uint8_t)((((c) >> 12) & 0x3f) | 0x80); \
                }                                                        \
                (s)[(i)++] = (uint8_t)((((c) >> 6) & 0x3f) | 0x80);      \
            }                                                            \
            (s)[(i)++] = (uint8_t)(((c)&0x3f) | 0x80);                   \
        }                                                                \
    }

// UTF-16 macros
// from utf16.h

/**
 * Does this code unit alone encode a code point (BMP, not a surrogate)?
 * @param c 16-bit code unit
 * @return TRUE or FALSE
 */
#define UTF16_IS_SINGLE(c) !UTF_IS_SURROGATE(c)

/**
 * Is this code unit a lead surrogate (U+d800..U+dbff)?
 * @param c 16-bit code unit
 * @return TRUE or FALSE
 */
#define UTF16_IS_LEAD(c) (((c)&0xfffffc00) == 0xd800)

/**
 * Is this code unit a trail surrogate (U+dc00..U+dfff)?
 * @param c 16-bit code unit
 * @return TRUE or FALSE
 */
#define UTF16_IS_TRAIL(c) (((c)&0xfffffc00) == 0xdc00)

/**
 * Is this code unit a surrogate (U+d800..U+dfff)?
 * @param c 16-bit code unit
 * @return TRUE or FALSE
 */
#define UTF16_IS_SURROGATE(c) UTF_IS_SURROGATE(c)

/**
 * Assuming c is a surrogate code point (U16_IS_SURROGATE(c)),
 * is it a lead surrogate?
 * @param c 16-bit code unit
 * @return TRUE or FALSE
 */
#define UTF16_IS_SURROGATE_LEAD(c) (((c)&0x400) == 0)

/**
 * Helper constant for UTF16_GET_SUPPLEMENTARY.
 * @internal
 */
#define UTF16_SURROGATE_OFFSET ((0xd800 << 10UL) + 0xdc00 - 0x10000)

/**
 * Get a supplementary code point value (U+10000..U+10ffff)
 * from its lead and trail surrogates.
 * The result is undefined if the input values are not
 * lead and trail surrogates.
 *
 * @param lead lead surrogate (U+d800..U+dbff)
 * @param trail trail surrogate (U+dc00..U+dfff)
 * @return supplementary code point (U+10000..U+10ffff)
 */
#define UTF16_GET_SUPPLEMENTARY(lead, trail) (((UChar32)(lead) << 10UL) + (UChar32)(trail)-UTF16_SURROGATE_OFFSET)

/**
 * Get the lead surrogate (0xd800..0xdbff) for a
 * supplementary code point (0x10000..0x10ffff).
 * @param supplementary 32-bit code point (U+10000..U+10ffff)
 * @return lead surrogate (U+d800..U+dbff) for supplementary
 */
#define UTF16_LEAD(supplementary) (UChar)(((supplementary) >> 10) + 0xd7c0)

/**
 * Get the trail surrogate (0xdc00..0xdfff) for a
 * supplementary code point (0x10000..0x10ffff).
 * @param supplementary 32-bit code point (U+10000..U+10ffff)
 * @return trail surrogate (U+dc00..U+dfff) for supplementary
 */
#define UTF16_TRAIL(supplementary) (base_icu::UChar)(((supplementary)&0x3ff) | 0xdc00)

/**
 * How many 16-bit code units are used to encode this Unicode code point? (1 or 2)
 * The result is not defined if c is not a Unicode code point (U+0000..U+10ffff).
 * @param c 32-bit code point
 * @return 1 or 2
 */
#define UTF16_LENGTH(c) ((uint32_t)(c) <= 0xffff ? 1 : 2)

/**
 * The maximum number of 16-bit code units per Unicode code point (U+0000..U+10ffff).
 * @return 2
 */
#define UTF16_MAX_LENGTH 2

/**
 * Get a code point from a string at a code point boundary offset,
 * and advance the offset to the next code point boundary.
 * (Post-incrementing forward iteration.)
 * "Safe" macro, handles unpaired surrogates and checks for string boundaries.
 *
 * The offset may point to the lead surrogate unit
 * for a supplementary code point, in which case the macro will read
 * the following trail surrogate as well.
 * If the offset points to a trail surrogate or
 * to a single, unpaired lead surrogate, then that itself
 * will be returned as the code point.
 *
 * @param s const UChar * string
 * @param i string offset, i<length
 * @param length string length
 * @param c output UChar32 variable
 */
#define UTF16_NEXT(s, i, length, c)                                  \
    {                                                                \
        (c) = (s)[(i)++];                                            \
        if (UTF16_IS_LEAD(c)) {                                      \
            uint16_t __c2;                                           \
            if ((i) < (length) && UTF16_IS_TRAIL(__c2 = (s)[(i)])) { \
                ++(i);                                               \
                (c) = UTF16_GET_SUPPLEMENTARY((c), __c2);            \
            }                                                        \
        }                                                            \
    }

/**
 * Append a code point to a string, overwriting 1 or 2 code units.
 * The offset points to the current end of the string contents
 * and is advanced (post-increment).
 * "Unsafe" macro, assumes a valid code point and sufficient space in the string.
 * Otherwise, the result is undefined.
 *
 * @param s const UChar * string buffer
 * @param i string offset
 * @param c code point to append
 * @see CBU16_APPEND
 */
#define UTF16_APPEND_UNSAFE(s, i, c)                       \
    {                                                      \
        if ((uint32_t)(c) <= 0xffff) {                     \
            (s)[(i)++] = (uint16_t)(c);                    \
        } else {                                           \
            (s)[(i)++] = (uint16_t)(((c) >> 10) + 0xd7c0); \
            (s)[(i)++] = (uint16_t)(((c)&0x3ff) | 0xdc00); \
        }                                                  \
    }

}  // namespace LIBZIP
}  // namespace AAFwk
}  // namespace OHOS
#endif  // FOUNDATION_AAFWK_STANDARD_TOOLS_UTF_H
