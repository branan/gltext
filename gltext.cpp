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

#include "gltext.hpp"
#include "harfbuzz/hb-ft.h"

#include <assert.h>
#include <math.h>
#include <map>

#include "gl3.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static void* glPointer(const char* funcname) {
    return (void*)wglGetProcAddress(funcname);
}
#else
#include <GL/glx.h>

static void* glPointer(const char* funcname) {
    return (void*)glXGetProcAddress((GLubyte*)funcname);
}
#endif

static PFNGLGENVERTEXARRAYSPROC gltextGenVertexArrays;
static PFNGLBINDVERTEXARRAYPROC gltextBindVertexArray;
static PFNGLDELETEVERTEXARRAYSPROC gltextDeleteVertexArrays;
static PFNGLGENBUFFERSPROC gltextGenBuffers;
static PFNGLBINDBUFFERPROC gltextBindBuffer;
static PFNGLDELETEBUFFERSPROC gltextDeleteBuffers;
static PFNGLBUFFERDATAPROC gltextBufferData;
static PFNGLENABLEVERTEXATTRIBARRAYPROC gltextEnableVertexAttribArray;
static PFNGLVERTEXATTRIBPOINTERPROC gltextVertexAttribPointer;

static void initGlPointers() {
    gltextGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)glPointer("glGenVertexArrays");
    gltextBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)glPointer("glBindVertexArray");
    gltextDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)glPointer("glDeleteVertexArrays");
    gltextGenBuffers = (PFNGLGENBUFFERSPROC)glPointer("glGenBuffers");
    gltextBindBuffer = (PFNGLBINDBUFFERPROC)glPointer("glBindBuffer");
    gltextDeleteBuffers = (PFNGLDELETEBUFFERSPROC)glPointer("glDeleteBuffers");
    gltextBufferData = (PFNGLBUFFERDATAPROC)glPointer("glBufferData");
    gltextVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)glPointer("glVertexAttribPointer");
    gltextEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)glPointer("glEnableVertexAttribArray");
}

struct FontSystem {
public:
    static FontSystem& instance() {
        static FontSystem singleton;
        return singleton;
    }
    
    FontSystem() {
        FT_Init_FreeType(&library);
        initGlPointers();
    }
    ~FontSystem() {
        FT_Done_FreeType(library);
    }
    
    FT_Library library;
};

namespace gltext {
    
struct FontPimpl {
    std::string filename;
    unsigned size;
    FT_Face face;
    hb_font_t* font;

    GLuint vao;
    GLuint vbo;
    GLuint ibo;
    GLuint tex;

    GLuint texpos_x;
    GLuint texpos_y;
    GLuint num_glyphs_cached;

    short y_size;
    short x_size;

    unsigned cache_size;

    std::map<FT_UInt, unsigned> glyphs;

    void init();
    void cleanup();
    void cacheGlyph(FT_Int codepoint);
};

Font::Font(std::string font_file, unsigned size, unsigned cache_size) {
    self = new FontPimpl;
    self->filename = font_file;
    self->size = size;
    self->cache_size = cache_size;
    self->init();
}

Font::Font()
    : self(0) {}

Font::~Font() {
    if(self) {
        self->cleanup();
        delete self;
    }
}

Font::Font(const Font& rhs) {
    *this = rhs;
}

Font& Font::operator=(const Font& rhs) {
    if(self)
        self->cleanup();
    else
        self = new FontPimpl;
    self->filename = rhs.self->filename;
    self->size =  rhs.self->size;
    self->cache_size = rhs.self->cache_size;
    self->init();
}

void Font::setDisplaySize(unsigned w, unsigned h) {
    if(!self)
        return;
}

void Font::cacheCharacters(std::string chars) {
    if(!self)
        return;
    hb_buffer_t* buffer = hb_buffer_create();
    hb_buffer_set_direction(buffer, HB_DIRECTION_LTR);
    hb_buffer_add_utf8(buffer, chars.c_str(), chars.size(), 0, chars.size());
    hb_shape(self->font, buffer, NULL, 0);
    
    unsigned len = hb_buffer_get_length(buffer);
    hb_glyph_info_t* glyphs = hb_buffer_get_glyph_infos(buffer, 0);
    hb_glyph_position_t* positions = hb_buffer_get_glyph_positions(buffer, 0);

    GLint pixel_store, row_length;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &pixel_store);
    glGetIntegerv(GL_UNPACK_ROW_LENGTH, &row_length);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    for(unsigned i = 0; i < len; i++) {
        std::map<FT_UInt, unsigned>::iterator g = self->glyphs.find(glyphs[i].codepoint);
        if(g == self->glyphs.end()) {
            self->cacheGlyph(glyphs[i].codepoint);
        }
    }
    
        glPixelStorei(GL_UNPACK_ALIGNMENT, pixel_store);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, row_length);
}

void Font::draw(std::string text, unsigned x, unsigned y) {
    if(!self)
        return;
    hb_buffer_t* buffer = hb_buffer_create();
    hb_buffer_set_direction(buffer, HB_DIRECTION_LTR);
    hb_buffer_add_utf8(buffer, text.c_str(), text.size(), 0, text.size());
    hb_shape(self->font, buffer, NULL, 0);

    unsigned len = hb_buffer_get_length(buffer);
    hb_glyph_info_t* glyphs = hb_buffer_get_glyph_infos(buffer, 0);
    hb_glyph_position_t* positions = hb_buffer_get_glyph_positions(buffer, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, self->tex);
    gltextBindVertexArray(self->vao);

    GLint pixel_store, row_length;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &pixel_store);
    glGetIntegerv(GL_UNPACK_ROW_LENGTH, &row_length);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for(unsigned i = 0; i < len; i++) {
        std::map<FT_UInt, unsigned>::iterator g = self->glyphs.find(glyphs[i].codepoint);
        if(g == self->glyphs.end()) {
            self->cacheGlyph(glyphs[i].codepoint);
        }
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, pixel_store);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, row_length);
}

#define GLYPH_VERT_SIZE (4*4*sizeof(GLfloat))
#define GLYPH_IDX_SIZE (6*sizeof(GLushort))

void FontPimpl::init() {
    FontSystem& system = FontSystem::instance();
    FT_Error error;
    error = FT_New_Face(system.library, filename.c_str(), 0, &face);
    if(error) {
        assert(false); // TODO: exceptions
    }
    FT_Set_Pixel_Sizes(face, 0, size);

    font = hb_ft_font_create(face, 0);

    double size_y = double(face->height) * double(face->size->metrics.y_ppem) / double(face->units_per_EM);
    double size_x = double(face->max_advance_width) * double(face->size->metrics.y_ppem) / double(face->units_per_EM);
    y_size = ceil(size_y);
    x_size = ceil(size_x);

    texpos_x = 0;
    texpos_y = 0;
    num_glyphs_cached = 0;

    short max_glyphs = (cache_size / x_size)*(cache_size / y_size);

    gltextGenVertexArrays(1, &vao);
    gltextGenBuffers(1, &vbo);
    gltextGenBuffers(1, &ibo);
    gltextBindVertexArray(vao);
    gltextBindBuffer(GL_ARRAY_BUFFER, vbo);
    gltextBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    gltextBufferData(GL_ARRAY_BUFFER, GLYPH_VERT_SIZE*max_glyphs, NULL, GL_DYNAMIC_DRAW);
    gltextBufferData(GL_ELEMENT_ARRAY_BUFFER, GLYPH_IDX_SIZE*max_glyphs, NULL, GL_DYNAMIC_DRAW);
    gltextEnableVertexAttribArray(0);
    gltextEnableVertexAttribArray(1);
    gltextVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, GLYPH_VERT_SIZE, 0);
    gltextVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, GLYPH_VERT_SIZE, (GLvoid*)(2*sizeof(float)));

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, cache_size, cache_size, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void FontPimpl::cleanup() {
    hb_font_destroy(font);
    glDeleteTextures(1, &tex);
    gltextDeleteBuffers(1, &vbo);
    gltextDeleteBuffers(1, &ibo);
    gltextDeleteVertexArrays(1, &vao);
}

void FontPimpl::cacheGlyph(FT_Int codepoint)
{
    if(num_glyphs_cached == (cache_size / x_size)*(cache_size / y_size)) {
        printf("Glyph cache overflow! This is an ERROR until eviction code is written. Init your font with a larger cache!");
        return;
    }
    FT_Load_Glyph(face, codepoint, FT_LOAD_RENDER);
    if(face->glyph->bitmap.pixel_mode != FT_PIXEL_MODE_GRAY) {
        printf("Not grayscale. Aborting render!");
        return;
    }
    int pitch = face->glyph->bitmap.pitch;
    bool need_inverse_texcoords = true;
    if(pitch < 0) {
        pitch = -pitch;
        need_inverse_texcoords = false;
    } 
    glPixelStorei(GL_UNPACK_ROW_LENGTH, pitch);
    if(texpos_x + face->glyph->bitmap.width > cache_size) {
        texpos_x = 0;
        texpos_y += y_size;
    }
    glTexSubImage2D(GL_TEXTURE_2D, 0, texpos_x, texpos_y, face->glyph->bitmap.width, face->glyph->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
    texpos_x += x_size;
    glyphs.insert(std::make_pair(codepoint, num_glyphs_cached));
    num_glyphs_cached++;
}

}