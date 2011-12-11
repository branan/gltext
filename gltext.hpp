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

#include <string>

#define GLTEXT_CACHE_TEXTURE_SIZE 256

namespace gltext {
    
struct FontPimpl;
    
class Font {
public:
    Font(std::string font_file, unsigned size, unsigned cache_size = GLTEXT_CACHE_TEXTURE_SIZE);
    Font();
    Font(const Font&);
    Font& operator=(const Font&);
    ~Font();

    void setDisplaySize(unsigned w, unsigned h);
    void cacheCharacters(std::string chars);
    void draw(std::string text, unsigned x, unsigned y);
private:
    FontPimpl* self;
};

}


#endif // GLTEXT_FONT_HPP