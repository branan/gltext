#include "gltext.hpp"

#include <GL/glut.h>

gltext::Font font;

void render(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    font.setPenPosition(16, 16);
    font.draw("This piece of text has a descender on the 'p'.");
    glutSwapBuffers();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE|GLUT_DEPTH);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(300, 300);
    glutCreateWindow("gltext demo");
    glutDisplayFunc(render);

    font = gltext::Font("/home/branan/projects/coredump/vfx/droid.ttf", 16, 128);
    font.setDisplaySize(300, 300);
    font.cacheCharacters("1234567890!@#$%^&*()abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ,./;'[]\\<>?:\"{}|-=_+");

    glutMainLoop();
}