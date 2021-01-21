#include "meshes.h"

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

Meshes::Meshes()
{
    //--Generate quad VAO--//
     float quadVertices[] = { 
       // positions          // colors     //tex cords   
     1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,
     1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,
    -1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,
    };
    
    unsigned int indices[] = {
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
    };

    unsigned int VBO, EBO;
    glGenVertexArrays(1, &this->quadVAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(this->quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenVertexArrays(1, &quadTextVAO);
    glGenBuffers(1, &quadTextVBO);
    glBindVertexArray(quadTextVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadTextVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);      

     //--Generate quad VAO--//
}
