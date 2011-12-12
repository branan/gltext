/*
 * Copyright 2011 Branan Purvine-Riley
 *
 *  This is part of gltext, a text-rendering library.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 *
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN
 * IF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef GLTEXT_FONT_HPP
#define GLTEXT_FONT_HPP

#include <stdexcept>
#include <string>

#define GLTEXT_CACHE_TEXTURE_SIZE 256

namespace gltext {

    class Exception : public std::runtime_error {
    public:
        Exception(std::string str) : std::runtime_error(str) {}
    };

    class FtException : public Exception {
    public:
        FtException() : Exception("Freetype error occured") {}
    };

    class CacheOverflowException : public Exception {
    public:
        CacheOverflowException() : Exception("Overflow in glyph cache") {}
    };

    class EmptyFontException : public Exception {
    public:
        EmptyFontException() : Exception("The request operation is not permitted on an empty Font") {}
    };

    class BadFontFormatException : public Exception {
    public:
        BadFontFormatException() : Exception("The font glyphs are not in an appropriate bitmap format") {}
    };
    
struct FontPimpl;
    
class Font {
public:
    Font(std::string font_file, unsigned size, unsigned cache_w = GLTEXT_CACHE_TEXTURE_SIZE, unsigned cache_h = GLTEXT_CACHE_TEXTURE_SIZE);
    Font();
    Font(const Font&);
    Font& operator=(const Font&);
    ~Font();

    void setDisplaySize(unsigned w, unsigned h);
    void setPenPosition(unsigned x, unsigned y);
    void setPenColor(float r, float g, float b);
    void cacheCharacters(std::string chars);
    void draw(std::string text);
private:
    FontPimpl* self;
};

}


#endif // GLTEXT_FONT_HPP