// Local Headers
#include "glitter.hpp"
#include "shader.h"
#include "meshes.h"
#include "font.h"

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>



// Standard Headers
#include <cstdio>
#include <cstdlib>
#include <map>
#include <iostream>

struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2   Size;       // Size of glyph
    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Offset to advance to next glyph
};


std::map<char, Character> Characters;

void RenderText(Shader &s, std::string text, float x, float y, float scale, glm::vec3 color, unsigned int VAO, unsigned int VBO)
{
    // activate corresponding render state	
    s.use();

    glUniform3f(glGetUniformLocation(s.ID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }           
        };

        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

int main(int argc, char * argv[]) {

    // Load GLFW and Create a Window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    auto mWindow = glfwCreateWindow(mWidth, mHeight, "OpenGL", nullptr, nullptr);

    // Check for Valid Context
    if (mWindow == nullptr) {
        fprintf(stderr, "Failed to Create OpenGL Context");
        return EXIT_FAILURE;
    }

    // Create Context and Load OpenGL Functions
    glfwMakeContextCurrent(mWindow);
    gladLoadGL();
    fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));

    // OpenGL state
    // ------------
    //glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

     // build and compile our shader program
    // ------------------------------------
    Shader ourShader("shader.vs", "shader.fs"); // you can name your shader files however you like
    Shader textShader("shader_font.vs", "shader_font.fs");
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(mWidth), 0.0f, static_cast<float>(mHeight));
    textShader.use();
    glUniformMatrix4fv(glGetUniformLocation(textShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    Meshes mesh;
    float posX = 0.0f;
    float posY = 0.0f;

    FT_Library ft;

    if(FT_Init_FreeType(&ft)) {
    fprintf(stderr, "Could not init freetype library\n");
    return 1;
    }

    FT_Face face;
    if (FT_New_Face(ft, "Monodeco.ttf", 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;  
        return -1;
    }
    FT_Set_Pixel_Sizes(face, 0, 48);
    if (FT_Load_Char(face, 'X', FT_LOAD_RENDER))
    {
        std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;  
        return -1;
    }

    // set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, 48);

    // disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // load first 128 characters of ASCII set
    for (unsigned char c = 0; c < 128; c++)
    {
        // Load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)
        };
        Characters.insert(std::pair<char, Character>(c, character));
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    FT_Done_Face(face);
    FT_Done_FreeType(ft);


    //projection *= glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0),glm::vec3(0, 1, 0));

    // Rendering Loop
    while (glfwWindowShouldClose(mWindow) == false) {
        if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(mWindow, true);

        if (glfwGetKey(mWindow, GLFW_KEY_D) == GLFW_PRESS)
            posX++;
        
        if (glfwGetKey(mWindow, GLFW_KEY_W) == GLFW_PRESS)
            posY++;

        if (glfwGetKey(mWindow, GLFW_KEY_S) == GLFW_PRESS)
            posY--;

        if (glfwGetKey(mWindow, GLFW_KEY_A) == GLFW_PRESS)
            posX--;

        // Background Fill Color
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

       

        if (glfwGetKey(mWindow, GLFW_KEY_0) == GLFW_PRESS)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // create transformations
        glm::mat4 model         = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        glm::mat4 view          = glm::mat4(1.0f);
        glm::mat4 projection    = glm::mat4(1.0f);
        projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f) * glm::lookAt(glm::vec3(posX, posY, 1.0f), glm::vec3(posX, posY, 0.0f),glm::vec3(0, 1, 0));
        
        // render the triangle
        ourShader.use();
        ourShader.setMat4("projection", projection); // note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
        ourShader.setMat4("view", view);
    
        //model = glm::translate(model, glm::vec3(posX, posY, 0.0f));
        model = glm::scale(model, glm::vec3(glm::vec2(100.0f, 100.0f), 1.0f));
        ourShader.setMat4("model", model);

        glBindVertexArray(mesh.quadVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(20.0f, 500.0f, 0.0f));
        model = glm::scale(model, glm::vec3(glm::vec2(100.0f, 100.0f), 1.0f));
        glBindVertexArray(mesh.quadVAO);
        ourShader.setMat4("model", model);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

         RenderText(textShader, "This is sample text", 25.0f, 25.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f), mesh.quadTextVAO, mesh.quadTextVBO);

        // Flip Buffers and Draw
        glfwSwapBuffers(mWindow);
        glfwPollEvents();


    }   glfwTerminate();
    return EXIT_SUCCESS;
}
