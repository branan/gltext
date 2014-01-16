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

    /// The base class for all gltext exceptions
    class Exception : public std::runtime_error {
    public:
        Exception(std::string str) : std::runtime_error(str) {}
    };

    /// Thrown when an error occurs in Freetype.
    class FtException : public Exception {
    public:
        FtException() : Exception("Freetype error occured") {}
    };

    /// Thrown when there is no more room in the glyph cache, but a non-cached glyph is required
    class CacheOverflowException : public Exception {
    public:
        CacheOverflowException() : Exception("Overflow in glyph cache") {}
    };

    /// Thrown when certain operations are attempted on an un-initialized Font object
    class EmptyFontException : public Exception {
    public:
        EmptyFontException() : Exception("The request operation is not permitted on an empty Font") {}
    };

    /// Thrown when Freetype gives us a glyph that is not in 8-bit luminance format
    class BadFontFormatException : public Exception {
    public:
        BadFontFormatException() : Exception("The font glyphs are not in an appropriate bitmap format") {}
    };

/// Internal structure for the Font class
struct FontPimpl;

/**
 * @brief Font loading and rendering
 * 
 * The Font class provides capabilities to load and render any font supported by Freetype. It uses an advanced
 * text layout system, to provide the correct glyph selection and positioning for internationalized text.
 * 
 * If the display size is set correctly, this class can provide pixel-perfect rendering. Setting the display size to
 * other values will not give correct results.
 * 
 * When drawing, this class will output pixels with pre-multiplied alpha. To blend them properly, set the blend mode to (GL_ONE, GL_DST_ALPHA)
 */
class Font {
public:
    /**
     * @brief Create a new fully initialized font
     * 
     * If any exceptions are thrown, the new Font object will be placed in the empty state, as if it were built with the default constructor.
     * @param[in] font_file The path to the requrested font file
     * @param[in] size The vertical size of the font, in pixels
     * @param[in] cache_w The width of the cache texture, in pixels
     * @param[in] cache_h The height of the cache texture, in pixels
     */
    Font(std::string font_file, unsigned size, unsigned cache_w = GLTEXT_CACHE_TEXTURE_SIZE, unsigned cache_h = GLTEXT_CACHE_TEXTURE_SIZE);
    /**
     * @brief Create an empty font
     */
    Font();
    /**
     * @brief Copy constructor
     * 
     * This provides a deep copy. A new font object is initialized from scatch, copying over only basic parameters from the source.
     * Importantly, a new cache buffer and texture are created for the new object. They are not shared with the source Font.
     */
    Font(const Font&);
    /**
     * @brief deep assignment
     * 
     * This will clean up the corrent font if needed, then perform a new initialization based on basic parameters from the source.
     * Importantly, a new cache buffer and texture are created for this object. They are not shared with the source Font.
     */
    Font& operator=(const Font&);

    /**
     * @brief cleanup
     * 
     * Deletes all buffers, textures, and Freetype objects associated with this font
     */
    ~Font();

    /**
     * @brief Set the size of the display
     * 
     * In order to provide pixel-perfect rendering, you must pass the actual size of the OpenGL viewport here.
     * @param[in] w The width of the OpenGL viewport, in pixels
     * @param[in] h The height of the OpenGL viewport, in pixels
     */
    void setDisplaySize(unsigned w, unsigned h);

    /**
     * @brief Set the drawing position
     * 
     * This is in OpenGL coordinates: 0,0 is the bottom-left corner
     * @param[in] x The horizontal drawing position
     * @param[in] y The vertical drawing position
     */
    void setPenPosition(unsigned x, unsigned y);

    /**
     * @brief Set the drawing color
     */
    void setPenColor(float r, float g, float b);

    /**
     * @brief change the font size
     * This function will cause the font cache to be cleared.
     * @param[in] size The new font size
     */
    void setPointSize(unsigned size);

    /**
     * @brief load some characters into the cache
     * 
     * This function allows you to pre-load the cache with a set of common characters. When this is done,
     * there will not be any need to render those characters at runtime.
     * 
     * There is no requirement to call this function. Any character not found in the cache will be rendered
     * as-needed.
     * 
     * Note that depending on the script, the order of the characters in this string might cause different glyphs
     * to be selectd for caching. This is especially of concern in indic and arabic scripts. If you want to cache
     * the correct glyphs in those scripts, you may have to pass your actual strings to this function, instead of
     * a dummy list of characters.
     * 
     * @param[in] chars The characters to place in the cache.
     */
    void cacheCharacters(std::string chars);

    /**
     * @brief draw a line of text
     * 
     * This function draws a single line of text. No line-splitting is performed - it is up to the application to provide that functionality if needed.
     * @param[in] text The string to draw
     */
    void draw(std::string text);
private:
    FontPimpl* self;
};

}

/**
 * @mainpage gltext documentation
 * 
 * This is the documentation for the gltext library. The capabilities of this library are exposed through the gltext::Font class.
 */

#endif // GLTEXT_FONT_HPP
