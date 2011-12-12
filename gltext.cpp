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

#define GLYPH_VERT_SIZE (4*4*sizeof(GLfloat))
#define GLYPH_IDX_SIZE (6*sizeof(GLushort))

struct GlyphVert {
    float x;
    float y;
    float s;
    float t;
};

static const char* shader_vert =
"\n\
#version 130\n\
\n\
in vec2 v;\n\
in vec2 t;\n\
out vec2 c;\n\
\n\
uniform ivec2 s;\n\
uniform ivec2 p;\n\
\n\
void main() {\n\
    c = t;\n\
    gl_Position = vec4((v+vec2(p))/vec2(s) * 2.0 - 1.0, 0.0, 1.0);\n\
}\n\
";

static const char* shader_frag =
"\n\
#version 130\n\
\n\
in vec2 c;\n\
out vec4 col;\n\
\n\
uniform sampler2D tex;\n\
uniform vec3 color;\n\
\n\
void main() {\n\
    float val = texture(tex, c).r;\n\
    col = vec4(color*val, val);\n\
}\n\
";

static PFNGLGENVERTEXARRAYSPROC gltextGenVertexArrays;
static PFNGLBINDVERTEXARRAYPROC gltextBindVertexArray;
static PFNGLDELETEVERTEXARRAYSPROC gltextDeleteVertexArrays;
static PFNGLGENBUFFERSPROC gltextGenBuffers;
static PFNGLBINDBUFFERPROC gltextBindBuffer;
static PFNGLDELETEBUFFERSPROC gltextDeleteBuffers;
static PFNGLBUFFERDATAPROC gltextBufferData;
static PFNGLBUFFERSUBDATAPROC gltextBufferSubData;
static PFNGLENABLEVERTEXATTRIBARRAYPROC gltextEnableVertexAttribArray;
static PFNGLVERTEXATTRIBPOINTERPROC gltextVertexAttribPointer;
static PFNGLCREATESHADERPROC gltextCreateShader;
static PFNGLSHADERSOURCEPROC gltextShaderSource;
static PFNGLCOMPILESHADERPROC gltextCompileShader;
static PFNGLDELETESHADERPROC gltextDeleteShader;
static PFNGLCREATEPROGRAMPROC gltextCreateProgram;
static PFNGLATTACHSHADERPROC gltextAttachShader;
static PFNGLLINKPROGRAMPROC gltextLinkProgram;
static PFNGLUSEPROGRAMPROC gltextUseProgram;
static PFNGLUNIFORM2IPROC gltextUniform2i;
static PFNGLUNIFORM1IPROC gltextUniform1i;
static PFNGLUNIFORM3FPROC gltextUniform3f;
static PFNGLGETUNIFORMLOCATIONPROC gltextGetUniformLocation;
static PFNGLBINDATTRIBLOCATIONPROC gltextBindAttribLocation;

static void initGlPointers() {
    gltextGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)glPointer("glGenVertexArrays");
    gltextBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)glPointer("glBindVertexArray");
    gltextDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)glPointer("glDeleteVertexArrays");
    gltextGenBuffers = (PFNGLGENBUFFERSPROC)glPointer("glGenBuffers");
    gltextBindBuffer = (PFNGLBINDBUFFERPROC)glPointer("glBindBuffer");
    gltextDeleteBuffers = (PFNGLDELETEBUFFERSPROC)glPointer("glDeleteBuffers");
    gltextBufferData = (PFNGLBUFFERDATAPROC)glPointer("glBufferData");
    gltextBufferSubData = (PFNGLBUFFERSUBDATAPROC)glPointer("glBufferSubData");
    gltextVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)glPointer("glVertexAttribPointer");
    gltextEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)glPointer("glEnableVertexAttribArray");
    gltextCreateShader = (PFNGLCREATESHADERPROC)glPointer("glCreateShader");
    gltextShaderSource = (PFNGLSHADERSOURCEPROC)glPointer("glShaderSource");
    gltextCompileShader = (PFNGLCOMPILESHADERPROC)glPointer("glCompileShader");
    gltextDeleteShader = (PFNGLDELETESHADERPROC)glPointer("glDeleteShader");
    gltextCreateProgram = (PFNGLCREATEPROGRAMPROC)glPointer("glCreateProgram");
    gltextAttachShader = (PFNGLATTACHSHADERPROC)glPointer("glAttachShader");
    gltextLinkProgram = (PFNGLLINKPROGRAMPROC)glPointer("glLinkProgram");
    gltextUseProgram = (PFNGLUSEPROGRAMPROC)glPointer("glUseProgram");
    gltextUniform2i = (PFNGLUNIFORM2IPROC)glPointer("glUniform2i");
    gltextUniform1i = (PFNGLUNIFORM1IPROC)glPointer("glUniform1i");
    gltextUniform3f = (PFNGLUNIFORM3FPROC)glPointer("glUniform3f");
    gltextGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)glPointer("glGetUniformLocation");
    gltextBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC)glPointer("glBindAttribLocation");
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
        fs = gltextCreateShader(GL_FRAGMENT_SHADER);
        vs = gltextCreateShader(GL_VERTEX_SHADER);
        gltextShaderSource(fs, 1, &shader_frag, 0);
        gltextShaderSource(vs, 1, &shader_vert, 0);
        gltextCompileShader(fs);
        gltextCompileShader(vs);
        prog = gltextCreateProgram();
        gltextAttachShader(prog, fs);
        gltextAttachShader(prog, vs);
        gltextBindAttribLocation(prog, 0, "v");
        gltextBindAttribLocation(prog, 1, "t");
        gltextLinkProgram(prog);
        gltextUseProgram(prog);
        gltextUniform1i(gltextGetUniformLocation(prog, "tex"), 0);
        scale_loc = gltextGetUniformLocation(prog, "s");
        pos_loc = gltextGetUniformLocation(prog, "p");
        col_loc = gltextGetUniformLocation(prog, "color");
    }
    ~FontSystem() {
        FT_Done_FreeType(library);
    }

    FT_Library library;
    GLuint fs;
    GLuint vs;
    GLuint prog;
    GLuint scale_loc;
    GLuint pos_loc;
    GLuint col_loc;
};


namespace gltext {
    
struct GlyphInfo {
    unsigned glyph;
    unsigned left;
    unsigned top;
};

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

    unsigned window_w, window_h;

    short y_size;
    short x_size;

    unsigned pen_x, pen_y;

    unsigned cache_w, cache_h;

    float pen_r, pen_g, pen_b;

    std::map<FT_UInt, GlyphInfo> glyphs;
    
    void init() {
        FontSystem& system = FontSystem::instance();
        FT_Error error;
        error = FT_New_Face(system.library, filename.c_str(), 0, &face);
        if(error) {
            throw FtException();
        }
        error = FT_Set_Pixel_Sizes(face, 0, size);
        if(error) {
            FT_Done_Face(face);
            throw FtException();
        }
            
        font = hb_ft_font_create(face, 0);
        
        double size_y = double(face->height) * double(face->size->metrics.y_ppem) / double(face->units_per_EM);
        double size_x = double(face->max_advance_width) * double(face->size->metrics.y_ppem) / double(face->units_per_EM);
        y_size = ceil(size_y);
        x_size = ceil(size_x);
        
        texpos_x = 0;
        texpos_y = 0;
        num_glyphs_cached = 0;
    
        pen_x = 0;
        pen_y = 0;
        pen_r = pen_g = pen_b = 1.0f;
        
        short max_glyphs = (cache_w / x_size)*(cache_h / y_size);
        
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
        gltextVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);
        gltextVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (GLvoid*)(2*sizeof(float)));
        
        glActiveTexture(GL_TEXTURE0);
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, cache_w, cache_h, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    void cleanup() {
        hb_font_destroy(font);
        glDeleteTextures(1, &tex);
        gltextDeleteBuffers(1, &vbo);
        gltextDeleteBuffers(1, &ibo);
        gltextDeleteVertexArrays(1, &vao);
    }

    std::map<FT_UInt, GlyphInfo>::iterator cacheGlyph(FT_UInt codepoint)
    {
        if(num_glyphs_cached == (cache_w / x_size)*(cache_h / y_size)) {
            throw CacheOverflowException();
        }
        FT_Error error;
        error = FT_Load_Glyph(face, codepoint, FT_LOAD_RENDER);
        if(error) {
            throw FtException();
        }
        if(face->glyph->bitmap.pixel_mode != FT_PIXEL_MODE_GRAY) {
            throw BadFontFormatException();
        }
        int pitch = face->glyph->bitmap.pitch;
        bool need_inverse_texcoords = true;
        if(pitch < 0) {
            pitch = -pitch;
            need_inverse_texcoords = false;
        } 
        glPixelStorei(GL_UNPACK_ROW_LENGTH, pitch);
        if(texpos_x + face->glyph->bitmap.width > cache_w) {
            texpos_x = 0;
            texpos_y += y_size;
        }
        glTexSubImage2D(GL_TEXTURE_2D, 0, texpos_x, texpos_y, face->glyph->bitmap.width, face->glyph->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
        GlyphVert corners[4];
        GlyphVert& bl = corners[0];
        GlyphVert& ul = corners[1];
        GlyphVert& br = corners[2];
        GlyphVert& ur = corners[3];
        bl.x = 0.0f;
        bl.y = 0.0f;
        bl.s = float(texpos_x)/float(cache_w);
        if(need_inverse_texcoords)
            bl.t = float(texpos_y+face->glyph->bitmap.rows)/float(cache_h);
        else
            bl.t = float(texpos_y)/float(cache_h);
        br.x = face->glyph->bitmap.width;
        br.y = 0.0f;
        br.s = float(texpos_x+face->glyph->bitmap.width)/float(cache_w);
        br.t = bl.t;
        ul.x = 0.0f;
        ul.y = face->glyph->bitmap.rows;
        ul.s = bl.s;
        if(need_inverse_texcoords)
            ul.t = float(texpos_y)/float(cache_h);
        else
            ul.t = float(texpos_y+face->glyph->bitmap.rows)/float(cache_h);
        ur.x = br.x;
        ur.y = ul.y;
        ur.s = br.s;
        ur.t = ul.t;
        short glyph_offset = num_glyphs_cached * 4;
        unsigned short indices[6] = {glyph_offset+0, glyph_offset+2, glyph_offset+3, glyph_offset+0, glyph_offset+3, glyph_offset+1};
        gltextBufferSubData(GL_ARRAY_BUFFER, (GLintptr)(num_glyphs_cached*GLYPH_VERT_SIZE), GLYPH_VERT_SIZE, corners);
        gltextBufferSubData(GL_ELEMENT_ARRAY_BUFFER, (GLintptr)(num_glyphs_cached*GLYPH_IDX_SIZE), GLYPH_IDX_SIZE, indices);
        texpos_x += x_size;
        GlyphInfo info;
        info.glyph = num_glyphs_cached;
        info.left = face->glyph->bitmap_left;
        info.top = face->glyph->bitmap_top - face->glyph->bitmap.rows;
        num_glyphs_cached++;
        return glyphs.insert(std::make_pair(codepoint, info)).first;
    }
};

Font::Font(std::string font_file, unsigned size, unsigned cache_w, unsigned cache_h) {
    self = new FontPimpl;
    self->filename = font_file;
    self->size = size;
    self->cache_w = cache_w;
    self->cache_h = cache_h;
    try {
        self->init();
    } catch(Exception&) {
        delete self;
        self = 0;
        throw;
    }
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
    if(self) {
        self->cleanup();
        if(!rhs.self) {
            delete self;
            self = 0;
            return *this;
        }
    } else {
        self = new FontPimpl;
    }
    self->filename = rhs.self->filename;
    self->size =  rhs.self->size;
    self->cache_w = rhs.self->cache_w;
    self->cache_h = rhs.self->cache_h;
    self->pen_x = rhs.self->pen_x;
    self->pen_y = rhs.self->pen_y;
    try {
        self->init();
    } catch(Exception&) {
        delete self;
        self = 0;
        throw;
    }
    return *this;
}

void Font::setDisplaySize(unsigned w, unsigned h) {
    if(!self)
        throw EmptyFontException();
    self->window_w = w;
    self->window_h = h;
}

void Font::setPenPosition(unsigned x, unsigned y) {
    if(!self)
        throw EmptyFontException();
    self->pen_x = x;
    self->pen_y = y;
}

void Font::setPenColor(float r, float g, float b) {
    if(!self)
        throw EmptyFontException();
    self->pen_r = r;
    self->pen_g = g;
    self->pen_b = b;
}


void Font::cacheCharacters(std::string chars) {
    if(!self)
        throw EmptyFontException();
    hb_buffer_t* buffer = hb_buffer_create();
    hb_buffer_set_direction(buffer, HB_DIRECTION_LTR);
    hb_buffer_add_utf8(buffer, chars.c_str(), chars.size(), 0, chars.size());
    hb_shape(self->font, buffer, NULL, 0);
    
    unsigned len = hb_buffer_get_length(buffer);
    hb_glyph_info_t* glyphs = hb_buffer_get_glyph_infos(buffer, 0);
    hb_glyph_position_t* positions = hb_buffer_get_glyph_positions(buffer, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, self->tex);
    gltextBindVertexArray(self->vao);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    for(unsigned i = 0; i < len; i++) {
        std::map<FT_UInt, GlyphInfo>::iterator g = self->glyphs.find(glyphs[i].codepoint);
        if(g == self->glyphs.end()) {
            self->cacheGlyph(glyphs[i].codepoint);
        }
    }
}

void Font::draw(std::string text) {
    if(!self)
        throw EmptyFontException();
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
    gltextUseProgram(FontSystem::instance().prog);
    gltextUniform2i(FontSystem::instance().scale_loc, self->window_w, self->window_h);
    gltextUniform3f(FontSystem::instance().col_loc, self->pen_r, self->pen_g, self->pen_b);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for(unsigned i = 0; i < len; i++) {
        std::map<FT_UInt, GlyphInfo>::iterator g = self->glyphs.find(glyphs[i].codepoint);
        if(g == self->glyphs.end()) {
            g = self->cacheGlyph(glyphs[i].codepoint);
        }

        unsigned glyph = g->second.glyph;
        int left = g->second.left;
        int top = g->second.top;
        
        gltextUniform2i(FontSystem::instance().pos_loc, self->pen_x+positions[i].x_offset+left, self->pen_y+positions[i].y_offset+top);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (GLvoid*)(glyph*GLYPH_IDX_SIZE));
        self->pen_x += positions[i].x_advance >> 6;
        self->pen_y += positions[i].y_advance >> 6;
    }
}    

}