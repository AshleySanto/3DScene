#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"      // Image loading Utility functions

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h" // Camera class

using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "Final Project"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao;         // Handle for the vertex array object
        GLuint vbos[2];         // Handle for the vertex buffer objects
        GLuint nVertices;    // Number of indices of the mesh
    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;
    // Triangle mesh data
    GLMesh gMesh;
    // Texture id
    GLuint gTextureGlassId;
    GLuint gTextureSilverId;
    GLuint gTextureFloorId;
    GLuint gTextureBottleId;
    glm::vec2 gUVScale(5.0f, 5.0f);
    // Shader program
    GLuint gProgramId;
    GLuint gLampProgramId;

    // camera
    Camera gCamera(glm::vec3(0.0f, 0.0f, 3.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // timing
    float gDeltaTime = 0.0f; // time between current frame and last frame
    float gLastFrame = 0.0f;

    //Object Color and Light Color
    glm::vec3 gObjectColor(1.f, 0.2f, 0.0f);
    glm::vec3 gLightColor(1.0f, 1.0f, 1.0f);
    glm::vec3 gLightColor2(0.0f, 1.0f, 0.0f);
}
// Light position and scale
glm::vec3 gLightPosition(1.5f, 4.5f, 3.0f);
glm::vec3 gLightScale(0.3f);
glm::vec3 gLightPosition2(1.5f, 4.5f, 3.0f);
glm::vec3 gLightScale2(0.3f);

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void UCreateMesh(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);
bool viewProjection = true;


/* Vertex Shader Source Code*/
const GLchar* vertexShaderSource = GLSL(440,
    layout(location = 0) in vec3 position; //Vertex data
layout(location = 1) in vec3 normal;  //Light data
layout(location = 2) in vec2 textureCoordinate;  //Texture data

out vec3 vertexNormal; // For outgoing normals to fragment shader
out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
out vec2 vertexTextureCoordinate;


//Global variables for the transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates
    vertexTextureCoordinate = textureCoordinate;

    vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)

    vertexNormal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only and exclude normal translation properties
    vertexTextureCoordinate = textureCoordinate;
}
);


/* Fragment Shader Source Code*/
const GLchar* fragmentShaderSource = GLSL(440,
    in vec3 vertexNormal; // For incoming normals
in vec3 vertexFragmentPos; // For incoming fragment position
in vec2 vertexTextureCoordinate;

out vec4 fragmentColor;

// Uniform / Global variables for object color, light color, light position, and camera/view position
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 lightColor2;
uniform vec3 lightPos2;
uniform vec3 viewPosition;
uniform sampler2D uTexture;
uniform vec2 uvScale;

void main()
{
    /*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

        //Calculate Ambient lighting*/
    float ambientStrength = 0.1f; // Set ambient or global lighting strength
    vec3 ambient = ambientStrength * lightColor; // Generate ambient light color

    //Calculate Diffuse lighting*/
    vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
    vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
    float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
    vec3 diffuse = impact * lightColor; // Generate diffuse light color

    //Calculate Specular lighting*/
    float specularIntensity = 0.8f; // Set specular light strength
    float highlightSize = 16.0f; // Set specular highlight size
    vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
    vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector
    //Calculate specular component
    float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
    vec3 specular = specularIntensity * specularComponent * lightColor;

    // Texture holds the color to be used for all three components
    vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);

    // Calculate phong result
    vec3 phong = (ambient + diffuse + specular) * textureColor.xyz;

    fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU

     //Calculate Ambient lighting*/
    float ambientStrength2 = 0.1f; // Set ambient or global lighting strength
    vec3 ambient2 = ambientStrength2 * lightColor2; // Generate ambient light color

    //Calculate Diffuse lighting*/
    vec3 norm2 = normalize(vertexNormal); // Normalize vectors to 1 unit
    vec3 lightDirection2 = normalize(lightPos2 - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
    float impact2 = max(dot(norm2, lightDirection2), 0.0);// Calculate diffuse impact by generating dot product of normal and light
    vec3 diffuse2 = impact2 * lightColor2; // Generate diffuse light color

    //Calculate Specular lighting*/
    float specularIntensity2 = 0.1f; // Set specular light strength
    float highlightSize2 = 16.0f; // Set specular highlight size
    vec3 viewDir2 = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
    vec3 reflectDir2 = reflect(-lightDirection2, norm);// Calculate reflection vector
    //Calculate specular component
    float specularComponent2 = pow(max(dot(viewDir2, reflectDir2), 0.0), highlightSize2);
    vec3 specular2 = specularIntensity2 * specularComponent2 * lightColor2;

    // Texture holds the color to be used for all three components
    vec4 textureColor2 = texture(uTexture, vertexTextureCoordinate * uvScale);

    // Calculate phong result
    vec3 phong2 = (ambient2 + diffuse2 + specular2) * textureColor2.xyz;

    fragmentColor = vec4(phong + phong2, 1.0); // Send lighting results to GPU
}
);

/* Lamp Shader Source Code*/
const GLchar* lampVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data

        //Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
}
);


/* Fragment Shader Source Code*/
const GLchar* lampFragmentShaderSource = GLSL(440,

    out vec4 fragmentColor; // For outgoing lamp color (smaller cube) to the GPU

void main()
{
    fragmentColor = vec4(1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
}
);


// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}


int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create the mesh
    UCreateMesh(gMesh); // Calls the function to create the Vertex Buffer Object

    // Create the shader program
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;

    // Load texture
    const char* texFilename = "../../Final Project/resources/textures/glass.jpg";
    if (!UCreateTexture(texFilename, gTextureGlassId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "../../Final Project/resources/textures/silver.jpg";
    if (!UCreateTexture(texFilename, gTextureSilverId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "../../Final Project/resources/textures/floor.png";
    if (!UCreateTexture(texFilename, gTextureFloorId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "../../Final Project/resources/textures/galaxy.jpg";
    if (!UCreateTexture(texFilename, gTextureBottleId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gProgramId);
    // We set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);

    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(gWindow))
    {
        // per-frame timing
        // --------------------
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // input
        // -----
        UProcessInput(gWindow);

        // Render this frame
        URender();

        glfwPollEvents();
    }

    // Release mesh data
    UDestroyMesh(gMesh);

    // Release texture
    UDestroyTexture(gTextureGlassId);
    UDestroyTexture(gTextureSilverId);
    UDestroyTexture(gTextureFloorId);

    // Release shader program
    UDestroyShaderProgram(gProgramId);
    UDestroyShaderProgram(gLampProgramId);

    exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    // ---------------------
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLEW: initialize
    // ----------------
    // Note: if using GLEW version 1.13 or earlier
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
    static const float cameraSpeed = 2.5f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);

    //Controls up and down movement
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        gCamera.ProcessKeyboard(DOWN, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.ProcessKeyboard(UP, gDeltaTime);

    //Change projections between ortho and perspective
    //if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        //changeProjection = !changeProjection;
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    gCamera.ProcessMouseScroll(yoffset);
    if (gCamera.MovementSpeed < 2.5f)
        gCamera.MovementSpeed = 2.5f;

    // If wheel is scrolled forward to speed up the camera the speed at which the camera moves is capped at 10.0f
    if (gCamera.MovementSpeed > 10.0f)
        gCamera.MovementSpeed = 10.0f;
}

// glfw: handle mouse button events
// --------------------------------
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            cout << "Left mouse button pressed" << endl;
        else
            cout << "Left mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS)
            cout << "Middle mouse button pressed" << endl;
        else
            cout << "Middle mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            cout << "Right mouse button pressed" << endl;
        else
            cout << "Right mouse button released" << endl;
    }
    break;

    default:
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}


// Functioned called to render a frame
void URender()
{
    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    // Clear the frame and z buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. Scales the object by 2
    glm::mat4 scale = glm::scale(glm::vec3(2.0f, 2.0f, 2.0f));
    // 2. Rotates shape by 15 degrees in the x axis
    glm::mat4 rotation = glm::rotate(45.0f, glm::vec3(1.0, 1.0f, 1.0f));
    // 3. Place object at the origin
    glm::mat4 translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));
    // Model matrix: transformations are applied right-to-left order
    glm::mat4 model = translation * rotation * scale;

    // camera/view transformation
    glm::mat4 view = gCamera.GetViewMatrix();

    // Creates a perspective projection
    glm::mat4 projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);

    //Switch between ortho and perspective projections
    /*glm::mat4 projection;
        if (changeProjection) {
            glm::mat4 projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
        }
        else {
            glm::mat4 projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);
        }*/

        // Set the shader to be used
    glUseProgram(gProgramId);

    // Retrieves and passes transform matrices to the Shader program
    GLint modelLoc = glGetUniformLocation(gProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gProgramId, "view");
    GLint projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference matrix uniforms from the Cube Shader program for the cub color, light color, light position, and camera position
    GLint objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    GLint lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    GLint lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    GLint lightColorLoc2 = glGetUniformLocation(gProgramId, "lightColor2");
    GLint lightPositionLoc2 = glGetUniformLocation(gProgramId, "lightPos2");
    GLint viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");

    // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    glUniform3f(lightColorLoc2, gLightColor2.r, gLightColor2.g, gLightColor2.b);
    glUniform3f(lightPositionLoc2, gLightPosition2.x, gLightPosition2.y, gLightPosition2.z);
    const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    GLint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMesh.vao);

    // bind textures on corresponding texture units for glass
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureFloorId);

    // Draws the triangles for plane
    glDrawArrays(GL_TRIANGLES, 1, gMesh.nVertices);

    // bind textures on corresponding texture units for lid
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureSilverId);

    // Draws the triangles for cylinder
    glDrawArrays(GL_TRIANGLES, 7, gMesh.nVertices);

    // bind textures on corresponding texture units for glass
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureGlassId);

    // Draws the triangles for plane
    glDrawArrays(GL_TRIANGLES, 31, gMesh.nVertices);

    // bind textures on corresponding texture units for bottle
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureBottleId);

    // Draws the triangles for plane
    glDrawArrays(GL_TRIANGLES, 67, gMesh.nVertices);

    // LAMP: draw lamp
    //----------------
    glUseProgram(gLampProgramId);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gLampProgramId, "model");
    viewLoc = glGetUniformLocation(gLampProgramId, "view");
    projLoc = glGetUniformLocation(gLampProgramId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}


// Implements the UCreateMesh function
void UCreateMesh(GLMesh& mesh)
{
    // Vertex data
    GLfloat verts[] = {
        // Vertex Positions    // Normals       //Texture Coords.
        //Plane - Floor
        -10.0f, -0.5f,-10.0f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f, //Bottom Left Vertex 1
         10.0f, -0.5f, 10.0f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f, //Bottom Right Vertex 2
         10.0f, -0.5f, 10.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, //Top Right Vertex 3 
        -10.0f, -0.5f, 10.0f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f, //Top Left Vertex 4

        //Jar Lid Cylinder
        //Bottom Circle
        //Triangle 1
        0.4f,  1.5f, 0.0f,   0.0f,-1.0f, 0.0f,     0.0f, 0.0f, //Center Bottom 9
        0.35f, 1.5f, 0.05f,  0.0f,-1.0f, 0.0f,     1.0f, 0.0f, //Bottom Left Back 10
        0.33f, 1.5f, 0.0f,   0.0f,-1.0f, 0.0f,     0.0f, 1.0f, //Bottom Left 11

        //Triangle 2
        0.4f, 1.5f, 0.0f,    0.0f,-1.0f, 0.0f,     0.0f, 0.0f, //Center Bottom 9
        0.4f, 1.5f, 0.06f,   0.0f,-1.0f, 0.0f,     1.0f, 0.0f, //Bottom Center Back 12
        0.35f,1.5f, 0.05f,   0.0f,-1.0f, 0.0f,     0.0f, 1.0f, //Bottom Left Back 10

        //Triangle 3
        0.4f, 1.5f, 0.0f,     0.0f,-1.0f, 0.0f,    0.0f, 0.0f, //Center Bottom 9
        0.55f, 1.5f, 0.05f,   0.0f,-1.0f, 0.0f,    1.0f, 0.0f, //Bottom Right Back 13
        0.4f, 1.5f, 0.06f,    0.0f,-1.0f, 0.0f,    0.0f, 1.0f, //Bottom Center Back 12

        //Triangle 4
         0.4f, 1.5f, 0.0f,     0.0f,-1.0f, 0.0f,   0.0f, 0.0f, //Center Bottom 9
         0.57f, 1.5f, 0.0f,    0.0f,-1.0f, 0.0f,   1.0f, 0.0f, //Bottom Right 14
         0.55f, 1.5f, 0.05f,   0.0f,-1.0f, 0.0f,   0.0f, 1.0f, //Bottom Right Back 13

        //Triangle 5
         0.4f, 1.5f, 0.0f,     0.0f,-1.0f, 0.0f,   0.0f, 0.0f, //Center Bottom 9
         0.55f, 1.5f, -0.05f,  0.0f,-1.0f, 0.0f,   1.0f, 0.0f, //Bottom Right Front 15
         0.57f, 1.5f, 0.0f,    0.0f,-1.0f, 0.0f,   0.0f, 1.0f, //Bottom Right 14

        //Triangle 6
         0.4f, 1.5f, 0.0f,     0.0f,-1.0f, 0.0f,   0.0f, 0.0f, //Center Bottom 9
         0.4f, 1.5f, -0.04f,   0.0f,-1.0f, 0.0f,   1.0f, 0.0f, //Bottom Center Front 16
         0.55f, 1.5f, -0.05f,  0.0f,-1.0f, 0.0f,   0.0f, 1.0f, //Bottom Right Front 15

        //Triangle 7
         0.4f, 1.5f, 0.0f,     0.0f,-1.0f, 0.0f,   0.0f, 0.0f, //Center Bottom 9
         0.35, 1.5f, -0.05f,   0.0f,-1.0f, 0.0f,   1.0f, 0.0f, //Bottom Left Front 8
         0.4f, 1.5f, -0.04f,   0.0f,-1.0f, 0.0f,   0.0f, 1.0f, //Bottom Center Front 16

        //Triangle 8
         0.4f, 1.5f, 0.0f,     0.0f,-1.0f, 0.0f,   0.0f, 0.0f, //Center Bottom 9
         0.33f, 1.5f, 0.0f,    0.0f,-1.0f, 0.0f,   1.0f, 0.0f, //Bottom Left 11
         0.35f, 1.5f, 0.05f,   0.0f,-1.0f, 0.0f,   0.0f, 1.0f, //Bottom Left Front 10

        //Top Circle
        //Triangle 1
        0.4f, 2.0f, 0.0f,    0.0f, 1.0f, 0.0f,     0.0f, 0.0f, //Center Top 17
        0.35f, 2.0f, 0.05f,  0.0f, 1.0f, 0.0f,     1.0f, 0.0f, //Top Left Back 18
        0.33f, 2.0f, 0.0f,   0.0f, 1.0f, 0.0f,     0.0f, 1.0f, //Top Left 19

        //Triangle 2
        0.4f, 2.0f, 0.0f,     0.0f, 1.0f, 0.0f,    0.0f, 0.0f, //Center Top 17
        0.4f, 2.0f, 0.06f,    0.0f, 1.0f, 0.0f,    1.0f, 0.0f, //Top Center Back 20
        0.35f, 2.0f, 0.05f,   0.0f, 1.0f, 0.0f,    0.0f, 1.0f, //Top Left Back 18

        //Triangle 3
        0.4f, 2.0f, 0.0f,     0.0f, 1.0f, 0.0f,    0.0f, 0.0f, //Center Top 17
        0.55f, 2.0f, 0.05f,   0.0f, 1.0f, 0.0f,    1.0f, 0.0f, //Top Right Back 21
        0.4f, 2.0f, 0.06f,    0.0f, 1.0f, 0.0f,    0.0f, 1.0f, //Top Center Back 20

        //Triangle 4
        0.4f, 2.0f, 0.0f,     0.0f, 1.0f, 0.0f,    0.0f, 0.0f, //Center Top 17
        0.57f, 2.0f, 0.0f,    0.0f, 1.0f, 0.0f,    1.0f, 0.0f, //Top Right 22
        0.55f, 2.0f, 0.05f,   0.0f, 1.0f, 0.0f,    0.0f, 1.0f, //Top Right Back 21

        //Triangle 5
        0.4f, 2.0f, 0.0f,      0.0f, 1.0f, 0.0f,   0.0f, 0.0f, //Center Top 17
        0.55f, 2.0f, -0.05f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, //Top Right Front 23
        0.57f, 2.0f, 0.0f,     0.0f, 1.0f, 0.0f,   0.0f, 1.0f, //Top Right 22

        //Triangle 6
        0.4f, 2.0f, 0.0f,      0.0f, 1.0f, 0.0f,   0.0f, 0.0f, //Center Top 17
        0.4f, 2.0f, -0.04f,    0.0f, 1.0f, 0.0f,   1.0f, 0.0f, //Top Center Front 24
        0.55f, 2.0f, -0.05f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f, //Top Right Front 23

        //Triangle 7
        0.4f, 2.0f, 0.0f,     0.0f, 1.0f, 0.0f,    0.0f, 0.0f, //Center Top 17
        0.35, 2.0f, -0.05f,   0.0f, 1.0f, 0.0f,    1.0f, 0.0f, //Top Left Front 25
        0.4f, 2.0f, -0.04f,   0.0f, 1.0f, 0.0f,    0.0f, 1.0f, //Top Center Front 24

        //Triangle 8
        0.4f, 2.0f, 0.0f,     0.0f, 1.0f, 0.0f,    0.0f, 0.0f, //Center Top 17
        0.33f, 2.0f, 0.0f,    0.0f, 1.0f, 0.0f,    1.0f, 0.0f, //Top Left 19
        0.35f, 2.0f, 0.05f,   0.0f, 1.0f, 0.0f,    0.0f, 1.0f, //Top Left Front 18

        //Connecting Circles
        //Plane 1 Front
        0.4f, 1.5f, -0.04f,   0.0f,  0.0f, 1.0f,    0.0f, 0.0f, //Bottom Center Front 16
        0.4f, 2.0f, -0.04f,   0.0f,  0.0f, 1.0f,    1.0f, 0.0f, //Top Center Front 24
        0.35f, 1.5f, -0.05f,  0.0f,  0.0f, 1.0f,    0.0f, 1.0f, //Bottom Left Front 26
        0.4f, 2.0f, -0.04f,   0.0f,  0.0f, 1.0f,    0.0f, 0.0f, //Top Center Front 24
        0.35f, 2.0f, -0.05f,  0.0f,  0.0f, 1.0f,    1.0f, 0.0f, //Top Left Front 27
        0.35f, 1.5f, -0.05f,  0.0f,  0.0f, 1.0f,    0.0f, 1.0f, //Bottom Left Front 26

        //Plane 2 Front
        0.55f, 1.5f, -0.05,   0.0f, 0.0f, 1.0f,     0.0f, 0.0f, //Bottom Right Front 15
        0.55f, 2.0f, -0.05f,  0.0f, 0.0f, 1.0f,     1.0f, 0.0f, //Bottom Right Front 23
        0.4f, 1.5f, -0.04f,   0.0f, 0.0f, 1.0f,     0.0f, 1.0f, //Bottom Center Front 16
        0.55f, 2.0f, -0.05f,  0.0f, 0.0f, 1.0f,     0.0f, 0.0f, //Bottom Right Front 23
        0.4f, 2.0f, -0.04f,   0.0f, 0.0f, 1.0f,     1.0f, 0.0f, //Bottom Center Front 24
        0.4f, 1.5f, -0.04f,   0.0f, 0.0f, 1.0f,     0.0f, 1.0f, //Bottom Center Front 16

        //Plane 3 Left Front
        0.35f, 1.5f, -0.05f, -1.0f, 0.0f, 0.0f,     0.0f, 0.0f,  //Bottom Left Front 26
        0.35f, 2.0f, -0.05f, -1.0f, 0.0f, 0.0f,     1.0f, 0.0f, //Bottom Left Front 27
        0.33f, 1.5f, 0.00f,  -1.0f, 0.0f, 0.0f,     0.0f, 1.0f, //Bottom Left 28
        0.35f, 2.0f, -0.05f, -1.0f, 0.0f, 0.0f,     0.0f, 0.0f, //Bottom Left Front 27
        0.33f, 2.0f, 0.00f,  -1.0f, 0.0f, 0.0f,     1.0f, 0.0f, //Bottom Left 29
        0.33f, 1.5f, 0.00f,  -1.0f, 0.0f, 0.0f,     0.0f, 1.0f, //Bottom Left 28

        //Plane 4 Right Front
        0.57f, 1.5f, 0.00f,   1.0f, 0.0f, 0.0f,    0.0f, 0.0f, //Bottom Right 29
        0.57f, 2.0f, 0.00f,   1.0f, 0.0f, 0.0f,    1.0f, 0.0f, //Top Right 30
        0.55f, 1.5f, -0.05f,  1.0f, 0.0f, 0.0f,    0.0f, 1.0f, //Bottom Right Front 15
        0.57f, 2.0f, 0.00f,   1.0f, 0.0f, 0.0f,    0.0f, 0.0f, //Top Right 30
        0.55f, 2.0f, -0.05f,  1.0f, 0.0f, 0.0f,    1.0f, 0.0f, //Top Right Front 23
        0.55f, 1.5f, -0.05f,  1.0f, 0.0f, 0.0f,    0.0f, 1.0f, //Bottom Right Front 15

        //Plane 5 Back
        0.55f, 1.5f, 0.05f,   0.0f, 0.0f, -1.0f,   0.0f, 0.0f, //Bottom Right Back 13
        0.55f, 2.0f, 0.05f,   0.0f, 0.0f, -1.0f,   1.0f, 0.0f, //Top Right Back 21
        0.4f, 1.5f, 0.06f,    0.0f, 0.0f, -1.0f,   0.0f, 1.0f, //Bottom Center Back 12
        0.55f, 2.0f, 0.05f,   0.0f, 0.0f, -1.0f,   0.0f, 0.0f, //Top Right Back 21
        0.4f, 2.0f, 0.06f,    0.0f, 0.0f, -1.0f,   1.0f, 0.0f, //Top Center Back 20
        0.4f, 1.5f, 0.06f,    0.0f, 0.0f, -1.0f,   0.0f, 1.0f, //Top Center Back 12

        //Plane 6 Back
        0.4f, 1.5f, 0.06f,   0.0f, 0.0f, -1.0f,    0.0f, 0.0f, //Bottom Center Back 12
        0.4f, 2.0f, 0.06f,   0.0f, 0.0f, -1.0f,    1.0f, 0.0f, //Top Center Back 20
        0.35f, 1.5f, 0.05f,  0.0f, 0.0f, -1.0f,    0.0f, 1.0f, //Bottom Left Back 10
        0.4f, 2.0f, 0.06f,   0.0f, 0.0f, -1.0f,    0.0f, 0.0f, //Top Center Back 20
        0.35f, 2.0f, 0.05f,  0.0f, 0.0f, -1.0f,    1.0f, 0.0f, //Top Left Back 18
        0.35f, 1.5f, 0.05f,  0.0f, 0.0f, -1.0f,    0.0f, 1.0f, //Bottom Left Back 10

        //Plane 7 Back Left
        0.35f, 1.5f, 0.05f, -1.0f, 0.0f, 0.0f,     0.0f, 0.0f, //Bottom Left Back 10
        0.35f, 2.0f, 0.05f, -1.0f, 0.0f, 0.0f,     1.0f, 0.0f, //Top Left Back 18
        0.33f, 1.5f, 0.0f,  -1.0f, 0.0f, 0.0f,     0.0f, 1.0f, //Bottom Left 11
        0.35f, 2.0f, 0.05f, -1.0f, 0.0f, 0.0f,     0.0f, 0.0f, //Top Left Back 18
        0.33f, 2.0f, 0.0f,  -1.0f, 0.0f, 0.0f,     1.0f, 0.0f, //Top Left 19
        0.33f, 1.5f, 0.0f,  -1.0f, 0.0f, 0.0f,     0.0f, 1.0f, //Bottom Left 11

        //Plane 8 Back Right
        0.57f, 1.5f, 0.0f,   1.0f, 0.0f, 0.0f,     0.0f, 0.0f, //Bottom Right 14
        0.57f, 2.0f, 0.0f,   1.0f, 0.0f, 0.0f,     1.0f, 0.0f, //Top Right 22
        0.55f, 1.5f, 0.05f,  1.0f, 0.0f, 0.0f,     0.0f, 1.0f, //Bottom Right 13
        0.57f, 2.0f, 0.0f,   1.0f, 0.0f, 0.0f,     0.0f, 0.0f, //Top Right 22
        0.55f, 2.0f, 0.05f,  1.0f, 0.0f, 0.0f,     1.0f, 0.0f, //Top Right Back 21
        0.55f, 1.5f, 0.05f,  1.0f, 0.0f, 0.0f,     0.0f, 1.0f, //Bottom Right Back 13

        //Glass Bottle Cube
       -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,    0.0f, 0.0f, //Back br Vertex 31
        0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,    0.0f, 1.0f, //Back bl Vertex 32
        0.5f, 1.5f, -0.5f,   0.0f, 0.0f, -1.0f,    1.0f, 0.0f, //Back tl Vertex 33
        0.5f, 1.5f, -0.5f,   0.0f, 0.0f, -1.0f,    1.0f, 0.0f, //Back tl Vertex 34
       -0.5f, 1.5f, -0.5f,   0.0f, 0.0f, -1.0f,    0.0f, 1.0f, //Back tr Vertex 35
       -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,    0.0f, 0.0f, //Back br Vertex 36

       -0.5f, -0.5f, 0.5f,   0.0f, 0.0f, 1.0f,     0.0f, 0.0f, //Front br Vertex 37
        0.5f, -0.5f, 0.5f,   0.0f, 0.0f, 1.0f,     0.0f, 1.0f, //Front bl Vertex 38
        0.5f, 1.5f, 0.5f,    0.0f, 0.0f, 1.0f,     1.0f, 0.0f, //Front tl Vertex 39
        0.5f, 1.5f, 0.5f,    0.0f, 0.0f, 1.0f,     1.0f, 0.0f, //Front tl Vertex 40
       -0.5f, 1.5f, 0.5f,    0.0f, 0.0f, 1.0f,     0.0f, 1.0f, //Front tr Vertex 41
       -0.5f, -0.5f, 0.5f,   0.0f, 0.0f, 1.0f,     0.0f, 0.0f, //Front br Vertex 42

       -0.5f, 1.5f, 0.5f,   -1.0f, 0.0f, 0.0f,     0.0f, 0.0f, //Left tr Vertex 43
       -0.5f, 1.5f, -0.5f,  -1.0f, 0.0f, 0.0f,     0.0f, 1.0f, //Left tl Vertex 44
       -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,     1.0f, 0.0f, //Left bl Vertex 45
       -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,     1.0f, 0.0f, //Left bl Vertex 46
       -0.5f, -0.5f, 0.5f,  -1.0f, 0.0f, 0.0f,     0.0f, 1.0f, //Left br Vertex 47
       -0.5f, 1.5f, 0.5f,   -1.0f, 0.0f, 0.0f,     0.0f, 0.0f, //Left tr Vertex 48

        0.5f, 1.5f, 0.5f,    1.0f, 0.0f, 0.0f,     0.0f, 0.0f, //Right tl Vertex 49
        0.5f, 1.5f, -0.5f,   1.0f, 0.0f, 0.0f,     0.0f, 1.0f, //Right tr Vertex 50
        0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,     1.0f, 0.0f, //Right br Vertex 51
        0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,     1.0f, 0.0f, //Right br Vertex 52
        0.5f, -0.5f, 0.5f,   1.0f, 0.0f, 0.0f,     0.0f, 1.0f, //Right bl Vertex 53
        0.5f, 1.5f, 0.5f,    1.0f, 0.0f, 0.0f,     0.0f, 0.0f, //Right tl Vertex 54

       -0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f,    0.0f, 0.0f, //Bottom bl Vertex 55
        0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f,    0.0f, 1.0f, //Bottom br Vertex 56
        0.5f, -0.5f, 0.5f,   0.0f, -1.0f, 0.0f,    1.0f, 0.0f, //Bottom tr Vertex 57
        0.5f, -0.5f, 0.5f,   0.0f, -1.0f, 0.0f,    1.0f, 0.0f, //Bottom tr Vertex 58
       -0.5f, -0.5f, 0.5f,   0.0f, -1.0f, 0.0f,    0.0f, 1.0f, //Bottom tl Vertex 59
       -0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f,    0.0f, 0.0f, //Bottom bl Vertex 60

       -0.5f, 1.5f, -0.5f,   0.0f, 1.0f, 0.0f,     0.0f, 0.0f, //Top bl Vertex 61
        0.5f, 1.5f, -0.5f,   0.0f, 1.0f, 0.0f,     0.0f, 1.0f, //Top br Vertex 62
        0.5f, 1.5f, 0.5f,    0.0f, 1.0f, 0.0f,     1.0f, 0.0f, //Top tr Vertex 63
        0.5f, 1.5f, 0.5f,    0.0f, 1.0f, 0.0f,     1.0f, 0.0f, //Top tr Vertex 64
       -0.5f, 1.5f, 0.5f,    0.0f, 1.0f, 0.0f,     0.0f, 1.0f, //Top tl Vertex 65
       -0.5f, 1.5f, -0.5f,   0.0f, 1.0f, 0.0f,     0.0f, 0.0f, //Top bl Vertex 66

        //Bottle Cylinder
        //Bottom Circle
        //Triangle 1
       -3.0,   -0.5f, 3.0f,  0.0f, -1.0f, 0.0f,    0.0f, 0.0f, //Center Bottom 67
       -3.35f, -0.5f, 3.05f, 0.0f, -1.0f, 0.0f,    1.0f, 0.0f, //Bottom Left Back 68
       -3.33f, -0.5f, 3.0f,  0.0f, -1.0f, 0.0f,    0.0f, 1.0f, //Bottom Left 69

        //Triangle 2
       -3.0,   -0.5f, 3.0f,  0.0f, -1.0f, 0.0f,    0.0f, 0.0f, //Center Bottom 67
       -3.4f,  -0.5f, 3.06f, 0.0f, -1.0f, 0.0f,    1.0f, 0.0f, //Bottom Center Back 70
       -3.35f, -0.5f, 3.05f, 0.0f, -1.0f, 0.0f,    0.0f, 1.0f, //Bottom Left Back 68

        //Triangle 3
       -3.0,   -0.5f, 3.0f,  0.0f, -1.0f, 0.0f,    0.0f, 0.0f, //Center Bottom 67
       -3.55f, -0.5f, 3.05f, 0.0f, -1.0f, 0.0f,    1.0f, 0.0f, //Bottom Right Back 71
       -3.4f,  -0.5f, 3.06f, 0.0f, -1.0f, 0.0f,    0.0f, 1.0f, //Bottom Center Back 70

        //Triangle 4
       -3.0,   -0.5f, 3.0f,  0.0f, -1.0f, 0.0f,    0.0f, 0.0f, //Center Bottom 67
       -3.57f, -0.5f, 3.0f,  0.0f, -1.0f, 0.0f,    1.0f, 0.0f, //Bottom Right 72
       -3.55f, -0.5f, 3.05f, 0.0f, -1.0f, 0.0f,    0.0f, 1.0f, //Bottom Right Back 71

        //Triangle 5
       -3.0,   -0.5f, 3.0f,  0.0f, -1.0f, 0.0f,    0.0f, 0.0f, //Center Bottom 67
       -3.55f, -0.5f, 3.05f, 0.0f, -1.0f, 0.0f,    1.0f, 0.0f, //Bottom Right Front 73
       -3.57f, -0.5f, 3.0f,  0.0f, -1.0f, 0.0f,    0.0f, 1.0f, //Bottom Right 72

        //Triangle 6
       -3.0,   -0.5f, 3.0f,  0.0f, -1.0f,0.0f,     0.0f, 0.0f, //Center Bottom 67
       -3.4f,  -0.5f, 3.04f, 0.0f, -1.0f, 0.0f,    1.0f, 0.0f, //Bottom Center Front 74
       -3.55f, -0.5f, 3.05f, 0.0f, -1.0f, 0.0f,    0.0f, 1.0f, //Bottom Right Front 73

        //Triangle 7
       -3.0,  -0.5f, 3.0f,   0.0f, -1.0f, 0.0f,    0.0f, 0.0f, //Center Bottom 67
       -3.35, -0.5f, 3.05f,  0.0f, -1.0f, 0.0f,    1.0f, 0.0f, //Bottom Left Front 8
       -3.4f, -0.5f, 3.04f,  0.0f, -1.0f, 0.0f,    0.0f, 1.0f, //Bottom Center Front 74

        //Triangle 8
       -3.0,   -0.5f, 3.0f,  0.0f, -1.0f, 0.0f,    0.0f, 0.0f, //Center Bottom 67
       -3.33f, -0.5f, 3.0f,  0.0f, -1.0f, 0.0f,    1.0f, 0.0f, //Bottom Left 69
       -3.35f, -0.5f, 3.05f, 0.0f, -1.0f, 0.0f,    0.0f, 1.0f, //Bottom Left Front 68

        //Top Circle
        //Triangle 1
        -3.0,   6.0f, 3.0f,  0.0f, -1.0f, 0.0f,     0.0f, 0.0f, //Center Bottom 75
        -3.35f, 6.0f, 3.05f, 0.0f, -1.0f, 0.0f,     1.0f, 0.0f, //Bottom Left Back 76
        -3.33f, 6.0f, 3.0f,  0.0f, -1.0f, 0.0f,     0.0f, 1.0f, //Bottom Left 77

        //Triangle 2
        -3.0,   6.0f, 3.0f,  0.0f, -1.0f, 0.0f,     0.0f, 0.0f, //Center Bottom 75
        -3.4f,  6.0f, 3.06f, 0.0f, -1.0f, 0.0f,     1.0f, 0.0f, //Bottom Center Back 78
        -3.35f, 6.0f, 3.05f, 0.0f, -1.0f, 0.0f,     0.0f, 1.0f, //Bottom Left Back 76

        //Triangle 3
        -3.0,   6.0f, 3.0f,  0.0f, -1.0f, 0.0f,     0.0f, 0.0f, //Center Bottom 75
        -3.55f, 6.0f, 3.05f, 0.0f, -1.0f, 0.0f,     1.0f, 0.0f, //Bottom Right Back 79
        -3.4f,  6.0f, 3.06f, 0.0f, -1.0f, 0.0f,     0.0f, 1.0f, //Bottom Center Back 78

        //Triangle 4
        -3.0,   6.0f, 3.0f,  0.0f, -1.0f, 0.0f,     0.0f, 0.0f, //Center Bottom 75
        -3.57f, 6.0f, 3.0f,  0.0f, -1.0f, 0.0f,     1.0f, 0.0f, //Bottom Right 80
        -3.55f, 6.0f, 3.05f, 0.0f, -1.0f, 0.0f,     0.0f, 1.0f, //Bottom Right Back 79

        //Triangle 5
        -3.0,   6.0f, 3.0f,  0.0f, -1.0f, 0.0f,     0.0f, 0.0f, //Center Bottom 75
        -3.55f, 6.0f, 3.05f, 0.0f, -1.0f, 0.0f,     1.0f, 0.0f, //Bottom Right Front 81
        -3.57f, 6.0f, 3.0f,  0.0f, -1.0f, 0.0f,     0.0f, 1.0f, //Bottom Right 80

        //Triangle 6
        -3.0,   6.0f, 3.0f,  0.0f, -1.0f, 0.0f,     0.0f, 0.0f, //Center Bottom 75
        -3.4f,  6.0f, 3.04f, 0.0f, -1.0f, 0.0f,     1.0f, 0.0f, //Bottom Center Front 82
        -3.55f, 6.0f, 3.05f, 0.0f, -1.0f, 0.0f,     0.0f, 1.0f, //Bottom Right Front 81

        //Triangle 7
        -3.0,   6.0f, 3.0f,  0.0f, -1.0f, 0.0f,     0.0f, 0.0f, //Center Bottom 75
        -3.35,  6.0f, 3.05f, 0.0f, -1.0f, 0.0f,     1.0f, 0.0f, //Bottom Left Front 83
        -3.4f,  6.0f, 3.04f, 0.0f, -1.0f, 0.0f,     0.0f, 1.0f, //Bottom Center Front 82

        //Triangle 8
        -3.0,   6.0f, 3.0f,  0.0f, -1.0f, 0.0f,     0.0f, 0.0f, //Center Bottom 75
        -3.33f, 6.0f, 3.0f,  0.0f, -1.0f, 0.0f,     1.0f, 0.0f, //Bottom Left 77
        -3.35f, 6.0f, 3.05f, 0.0f, -1.0f, 0.0f,     0.0f, 1.0f, //Bottom Left Front 76

        //Connecting Circles
        //Plane 1 Front
        -3.4f, -0.5f, 3.04f, 0.0f, 0.0f, 1.0f,  0.0f, 0.0f, //Bottom Center Front 74
        -3.4f,  6.0f, 3.04f, 0.0f, 0.0f, 1.0f,  1.0f, 0.0f, //Top Center Front 82
        -3.35f,-0.5f, 3.05f, 0.0f, 0.0f, 1.0f,  0.0f, 1.0f, //Bottom Left Front 84
        -3.4f,  6.0f, 3.04f, 0.0f, 0.0f, 1.0f,  0.0f, 0.0f, //Top Center Front 82
        -3.35f, 6.0f, 3.05f, 0.0f, 0.0f, 1.0f,  1.0f, 0.0f, //Top Left Front 85
        -3.35f,-0.5f, 3.05f, 0.0f, 0.0f, 1.0f,  0.0f, 1.0f, //Bottom Left Front 84

        //Plane 2 Front
        -3.55f, -0.5f, 3.05f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, //Bottom Right Front 73
        -3.55f,  6.0f, 3.05f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, //Bottom Right Front 81
        -3.4f,  -0.5f, 3.04f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, //Bottom Center Front 74
        -3.55f,  6.0f, 3.05f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, //Bottom Right Front 81
        -3.4f,   6.0f, 3.04f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, //Bottom Center Front 82
        -3.4f,  -0.5f, 3.04f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, //Bottom Center Front 74

        //Plane 3 Left Front
        -3.35f, -0.5f, 3.05f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  //Bottom Left Front 84
        -3.35f,  6.0f, 3.05f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, //Bottom Left Front 85
        -3.33f, -0.5f, 3.00f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, //Bottom Left 86
        -3.35f,  6.0f, 3.05f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, //Bottom Left Front 85
        -3.33f,  6.0f, 3.00f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, //Bottom Left 87
        -3.33f, -0.5f, 3.00f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, //Bottom Left 86

        //Plane 4 Right Front
        -3.33f,  6.0f, 3.0f,  1.0f, 0.0f, 0.0f, 0.0f, 0.0f, //Bottom Right 87
        -3.55f, -0.5f, 3.05f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, //Top Right 88
        -3.55f, -0.5f, 3.05f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, //Bottom Right Front 73
        -3.55f, -0.5f, 3.05f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, //Top Right 88
        -3.55f,  6.0f, 3.05f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, //Top Right Front 81
        -3.55f, -0.5f, 3.05f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, //Bottom Right Front 73

        //Plane 5 Back
        -3.55f, -0.5f, 3.05f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, //Bottom Right Back 71
        -3.55f,  6.0f, 3.05f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, //Top Right Back 79
        -3.4f,  -0.5f, 3.06f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, //Bottom Center Back 70
        -3.55f,  6.0f, 3.05f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, //Top Right Back 79
        -3.4f,   6.0f, 3.06f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, //Top Center Back 78
        -3.4f,  -0.5f, 3.06f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, //Top Center Back 70

        //Plane 6 Back
        -3.4f, -0.5f, 3.06f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, //Bottom Center Back 70
        -3.4f,  6.0f, 3.06f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, //Top Center Back 78
        -3.35f,-0.5f, 3.05f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, //Bottom Left Back 68
        -3.4f,  6.0f, 3.06f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, //Top Center Back 78
        -3.35f, 6.0f, 3.05f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, //Top Left Back 76
        -3.35f,-0.5f, 3.05f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, //Bottom Left Back 68

        //Plane 7 Back Left
        -3.35f, -0.5f, 3.05f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, //Bottom Left Back 68
        -3.35f,  6.0f, 3.05f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, //Top Left Back 76
        -3.33f, -0.5f, 3.0f,  -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, //Bottom Left 69
        -3.35f,  6.0f, 3.05f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, //Top Left Back 76
        -3.33f,  6.0f, 3.0f,  -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, //Top Left 77
        -3.33f, -0.5f, 3.0f,  -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, //Bottom Left 69

        //Plane 8 Back Right
        -3.57f, -0.5f, 3.0f,  1.0f, 0.0f, 0.0f, 0.0f, 0.0f, //Bottom Right 72
        -3.57f,  6.0f, 3.0f,  1.0f, 0.0f, 0.0f, 1.0f, 0.0f, //Top Right 80
        -3.55f, -0.5f, 3.05f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, //Bottom Right 71
        -3.57f,  6.0f, 3.0f,  1.0f, 0.0f, 0.0f, 0.0f, 0.0f, //Top Right 80
        -3.55f,  6.0f, 3.05f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, //Top Right Back 79
        -3.55f, -0.5f, 3.05f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, //Bottom Right Back 71

        //Pen Body Cube
         5.0f, -0.5f, -3.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, //Back br Vertex 91
         0.0f, -0.5f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, //Back bl Vertex 92
         0.0f, -0.7f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, //Back tl Vertex 93
         0.0f, -0.7f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, //Back tl Vertex 94
         5.0f, -0.7f, -3.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, //Back tr Vertex 95
         5.0f, -0.5f, -3.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, //Back br Vertex 96

         1.0f, -0.7f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, //Front tl Vertex 97
         1.0f, -0.5f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, //Front bl Vertex 98
         4.0f, -0.7f, -4.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, //Front tr Vertex 99
         4.0f, -0.7f, -4.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, //Front tr Vertex 100
         1.0f, -0.5f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, //Front bl Vertex 101
         4.0f, -0.5f, -4.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, //Front br Vertex 102

         1.0f, -0.7f,  0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, //Left tr Vertex 103
         0.0f, -0.7f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, //Left tl Vertex 104
         0.0f, -0.5f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, //Left bl Vertex 105
         0.0f, -0.5f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, //Left bl Vertex 106
         1.0f, -0.5f,  0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, //Left br Vertex 107
         1.0f, -0.7f,  0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, //Left tr Vertex 108

         4.0f, -0.7f, -4.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, //Right tl Vertex 109
         5.0f, -0.7f, -3.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, //Right tr Vertex 110
         5.0f, -0.5f, -3.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, //Right br Vertex 111
         5.0f, -0.5f, -3.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, //Right br Vertex 112
         4.0f, -0.5f, -4.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, //Right bl Vertex 113
         4.0f, -0.7f, -4.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, //Right tl Vertex 114

         1.0f, -0.5f,  0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, //Bottom bl Vertex 115
         4.0f, -0.5f, -4.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, //Bottom br Vertex 116
         5.0f, -0.5f, -3.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, //Bottom tr Vertex 117
         5.0f, -0.5f, -3.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, //Bottom tr Vertex 118
         0.0f, -0.5f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, //Bottom tl Vertex 119
         1.0f, -0.5f,  0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, //Bottom bl Vertex 120

         1.0f, -0.7f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, //Top bl Vertex 121
         4.0f, -0.7f, -4.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, //Top br Vertex 122
         5.0f, -0.7f, -3.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, //Top tr Vertex 123
         5.0f, -0.7f, -3.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, //Top tr Vertex 124
         0.0f, -0.7f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, //Top tl Vertex 125
         1.0f, -0.7f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, //Top bl Vertex 126

        //Pen Tip Pyramid
        4.0f, -0.7f, -4.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, //Left tl Vertex 127
        4.0f, -0.5f, -4.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, //Left bl Vertex 128
        5.0f, -0.6f, -4.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, //Center Vertex 129

        4.0f, -0.7f, -4.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, //Top tl Vertex 130
        5.0f, -0.7f, -3.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, //Top tr Vertex 131
        5.0f, -0.6f, -4.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, //Center Vertex 132

        5.0f, -0.7f, -3.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, //Right tr Vertex 133
        5.0f, -0.5f, -3.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, //Right br Vertex 134
        5.0f, -0.6f, -4.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, //Center Vertex 135

        5.0f, -0.5f, -3.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, //Bottom br Vertex 136
        4.0f, -0.5f, -4.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, //Bottom bl Vertex 137
        5.0f, -0.6f, -4.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, //Center Vertex 138

        //Book Cover Planes
         2.0f, 1.5f, 0.0f, 8.0f, 1.0f, 0.0f, 0.0f, 0.0f, //Top tl Vertex 139
        10.0f, 1.5f, 3.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, //Top tr Vertex 140
        -1.0f, 1.5f, 4.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, //Top bl Vertex 141
        -1.0f, 1.5f, 4.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, //Top bl Vertex 142
        10.0f, 1.5f, 3.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, //Top tr Vertex 143
         7.0f, 1.5f,-1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, //Top br Vertex 144

         2.0f, -0.5f, 0.0f, 8.0f, 1.0f, 0.0f, 0.0f, 0.0f, //Bottom tl Vertex 145
        10.0f, -0.5f, 3.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, //Bottom tr Vertex 146
        -1.0f, -0.5f, 4.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, //Bottom bl Vertex 147
        -1.0f, -0.5f, 4.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, //Bottom bl Vertex 148
        10.0f, -0.5f, 3.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, //Bottom tr Vertex 149
         7.0f, -0.5f,-1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, //Bottom br Vertex 150

        //Book Pages Cube
        9.5f, -0.5f, 3.25f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, //Back br Vertex 151
        2.5f, -0.5f, 7.75f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, //Back bl Vertex 152
        2.5f,  1.5f, 7.75f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, //Back tl Vertex 153
        2.5f,  1.5f, 7.75f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, //Back tl Vertex 154
        9.5f,  1.5f, 3.25f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, //Back tr Vertex 155
        9.5f, -0.5f, 3.25f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, //Back br Vertex 156

         7.0f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, //Front br Vertex 157
        -0.5f, -0.5f,  4.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, //Front bl Vertex 158
        -0.5f,  1.5f,  4.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, //Front tl Vertex 159
        -0.5f,  1.5f,  4.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, //Front tl Vertex 160
         7.0f,  1.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, //Front tr Vertex 161
         7.0f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, //Front br Vertex 162

        -0.5f,  1.5f,  4.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, //Left tr Vertex 163
         2.5f,  1.5f, 7.75f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, //Left tl Vertex 164
         2.5f, -0.5f, 7.75f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, //Left bl Vertex 165
         2.5f, -0.5f, 7.75f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, //Left bl Vertex 166
        -0.5f, -0.5f,  4.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, //Left br Vertex 167
        -0.5f,  1.5f,  4.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, //Left tr Vertex 168

         7.0f,  1.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, //Right tl Vertex 169
         9.5f,  1.5f, 3.25f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, //Right tr Vertex 170
         9.5f, -0.5f, 3.25f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, //Right br Vertex 171
         9.5f, -0.5f, 3.25f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, //Right br Vertex 172
         7.0f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, //Right bl Vertex 173
         7.0f,  1.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, //Right tl Vertex 174

        -0.5f, -0.5f,  4.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, //Bottom bl Vertex 175
         7.0f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, //Bottom br Vertex 176
         9.5f, -0.5f, 3.25f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, //Bottom tr Vertex 177
         9.5f, -0.5f, 3.25f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, //Bottom tr Vertex 178
         2.5f, -0.5f, 7.75f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, //Bottom tl Vertex 179
        -0.5f, -0.5f,  4.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, //Bottom bl Vertex 180

        -0.5f, 1.5f,  4.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, //Top bl Vertex 181
         7.0f, 1.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, //Top br Vertex 182
         9.5f, 1.5f, 3.25f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, //Top tr Vertex 183
         9.5f, 1.5f, 3.25f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, //Top tr Vertex 184
         2.5f, 1.5f, 7.75f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, //Top tl Vertex 185
        -0.5f, 1.5f,  4.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, //Top bl Vertex 186


    };

    // Index data to share position data
    GLushort indices[] = {
        //Plane
        1, 2, 3, //Triangle 1
        4, 1, 3, //Triangle 2

        //Glass Lid
        7, 8, 6, //Triangle 3
        9, 10, 11,//Triangle 4
        9, 12, 10, //Triangle 5
        9, 13, 12, //Triangle 6
        9, 14, 13, //Triangle 7
        9, 15, 14, //Triangle 8
        9, 16, 15, //Triangle 9
        9, 8, 16, //Triangle 10
        9, 11, 10, //Triangle 11
        17, 18, 19, //Triangle 12
        17, 20, 18, //Triangle 13
        17, 21, 20, //Triangle 14
        17, 22, 21, //Triangle 15
        17, 23, 22, //Triangle 16
        17, 24, 23, //Triangle 17
        17, 25, 24, //Triangle 18
        17, 19, 18, //Triangle 19
        16, 24, 26, //Triangle 20
        24, 27, 26, //Triangle 21
        15, 23, 16, //Triangle 22
        23, 24, 16, //Triangle 23
        26, 27, 28, //Triangle 24
        27, 29, 28, //Triangle 25
        29, 30, 15, //Triangle 26
        30, 23, 15, //Triangle 27
        13, 21, 12, //Triangle 28
        21, 20, 12, //Triangle 29
        12, 20, 10, //Triangle 30
        20, 18, 10, //Triangle 31
        10, 18, 11, //Triangle 32
        18, 19, 11, //Triangle 33
        14, 22, 13, //Triangle 34
        22, 21, 13, //Triangle 35

        //Glass Jar
        31, 32, 33, //Triangle 36
        34, 35, 36, //Triangle 37
        37, 38, 39, //Triangle 38
        40, 41, 42, //Triangle 39
        43, 44, 45, //Triangle 40
        46, 47, 48, //Triangle 41
        49, 50, 51, //Triangle 42
        52, 53, 54, //Triangle 43
        55, 56, 57, //Triangle 44
        58, 59, 60, //Triangle 45
        61, 62, 63, //Triangle 46
        64, 65, 66, //Triangle 47

        //Bottle
        67, 68, 69, //Triangle 48
        69, 70, 71, //Triangle 49
        69, 72, 70, //Triangle 50
        69, 73, 72, //Triangle 51
        69, 74, 73, //Triangle 52
        69, 75, 74, //Triangle 53
        69, 76, 75, //Triangle 54
        69, 68, 76, //Triangle 55
        69, 71, 70, //Triangle 56
        77, 78, 79, //Triangle 57
        77, 80, 78, //Triangle 58
        77, 81, 80, //Triangle 59
        77, 82, 81, //Triangle 60
        77, 83, 82, //Triangle 61
        77, 84, 83, //Triangle 62
        77, 85, 84, //Triangle 63
        77, 79, 78, //Triangle 64
        76, 84, 86, //Triangle 65
        84, 87, 86, //Triangle 66
        75, 83, 76, //Triangle 67
        83, 84, 76, //Triangle 68
        86, 87, 88, //Triangle 69
        87, 89, 88, //Triangle 70
        89, 90, 75, //Triangle 71
        90, 83, 75, //Triangle 72
        73, 81, 72, //Triangle 73
        81, 80, 72, //Triangle 74
        72, 80, 70, //Triangle 75
        80, 78, 70, //Triangle 76
        70, 78, 71, //Triangle 77
        78, 79, 71, //Triangle 78
        74, 82, 73, //Triangle 79
        82, 81, 73, //Triangle 80

        //Pen Body
        91, 92, 93, //Triangle 81
        94, 95, 96, //Triangle 82
        97, 98, 99, //Triangle 83
        100, 101, 102, //Triangle 84
        103, 104, 105, //Triangle 85
        106, 107, 108, //Triangle 86
        109, 110, 111, //Triangle 87
        112, 113, 114, //Triangle 88
        115, 116, 117, //Triangle 89
        118, 119, 120, //Triangle 90
        121, 122, 123, //Triangle 91
        124, 125, 126, //Triangle 92

        //Pen Tip
        127, 128, 129, //Triangle 93
        130, 131, 132, //Triangle 94
        133, 134, 135, //Triangle 95
        136, 137, 138, //Triangle 96

        //Book Cover
        139, 140, 141, //Triangle 97
        142, 143, 144, //Triangle 98
        145, 146, 147, //Triangle 99
        148, 149, 150, //Triangle 100

        //Book Pages
        151, 152, 153, //Triangle 101
        154, 155, 156, //Triangle 102
        157, 158, 159, //Triangle 103
        160, 161, 162, //Triangle 104
        163, 164, 165, //Triangle 105
        166, 167, 168, //Triangle 106
        169, 170, 171, //Triangle 107
        172, 173, 174, //Triangle 108
        175, 176, 177, //Triangle 109
        178, 179, 180, //Triangle 110
        181, 182, 183, //Triangle 111
        184, 185, 186, //Triangle 112


    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create VBO
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nVertices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}


void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(2, mesh.vbos);
}


/*Generate and load the texture*/
bool UCreateTexture(const char* filename, GLuint& textureId)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        return true;
    }

    // Error loading the image
    return false;
}


void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}


// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); // compile the vertex shader
    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    return true;
}


void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}
