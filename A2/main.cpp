//CPSC 591/691 A#2 F2020 Boilerplate

//For more information of how the code works, look at the https://learnopengl.com/ tutorials from which this code is based off of

#define STB_IMAGE_IMPLEMENTATION

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "EdgeBuffer.h"

#include <iostream>
#include <algorithm>

using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void printFaceList(std::vector<Face> &faceList);

// window settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera settings
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Object rotation settings
glm::mat4 rotation;
float rotSpeed = 2.5f;


int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "CPSC 591/691 A2", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    
    // This shader should compute gooch shading
    Shader ourShader("../shaders/gooch.vs", "../shaders/gooch.fs");
    // This shader simply gives a colour of the outline
    Shader ourShader2("../shaders/outline.vs", "../shaders/outline.fs");


    // load model(s), default model is vase.obj, can load multiple at a time
    // -----------
    Model ourModel("../models/bunny/bunny.obj");

    unsigned int numVertices = 0;
	for (int i = 0; i < ourModel.meshes.size(); i++)
	{
		numVertices += ourModel.meshes[i].vertices.size();
	}

    printf("Meshes: %zu\n", ourModel.meshes.size());
	printf("Vertices: %zu\n", numVertices);

    // Creating an EdgeBuffer
    EdgeBuffer edgeBuffer(numVertices);
	
    // This vector will keep the information about faces
    std::vector<Face> faceList;

    //For each mesh in the .obj (The models included typically have only 1 mesh)
    for (int i = 0; i < ourModel.meshes.size(); i++)
    {
        //Print information
        printf("Mesh[%zu]\n", i);
        printf("\tvertices: %zu\n", ourModel.meshes[i].vertices.size());
        printf("\tindices: %zu\n\n", ourModel.meshes[i].indices.size());

        //The number of triangles of a mesh is the number of vertex indices / 3
        size_t numTriangles = ourModel.meshes[i].indices.size() / 3;

        unsigned int i0, i1, i2;
        Vertex v0, v1, v2;

        //For each triangle
        for (int j = 0; j < numTriangles; j++)
        {
            //Get indices of this triangle
            i0 = ourModel.meshes[i].indices[j * 3 + 0];
            i1 = ourModel.meshes[i].indices[j * 3 + 1];
            i2 = ourModel.meshes[i].indices[j * 3 + 2];

            //Get vertices of this triangle using indices
            v0 = ourModel.meshes[i].vertices[i0];
            v1 = ourModel.meshes[i].vertices[i1];
            v2 = ourModel.meshes[i].vertices[i2];

            //Get two edges of the triangle to compute triangle normal
            glm::vec3 a = v1.Position - v0.Position;
            glm::vec3 b = v2.Position - v1.Position;
            glm::vec3 triangleNormal = glm::normalize(glm::cross(a, b));

            //Compute centroid of the triangle
            glm::vec3 triangleCentroid = (v0.Position + v1.Position + v2.Position) / 3.f;

            //Sorting the vertex ids before adding them to the edge buffer and face list
            int sorted[3] = { i0, i1, i2 };
            sort(sorted, sorted + 3);
            /*cout << sorted[0] << sorted[1] << sorted[2] << endl;*/

            //Add this face(triangle) to the facelist
            Face newFace{
                {sorted[0], sorted[1], sorted[2]}, // Soreted list of vertices
                triangleNormal, // The normal of the face
                triangleCentroid // The centeroid cordinate of the face
            };
            faceList.push_back(newFace);
            /*printFaceList(faceList);*/

            //add the triangle to the edgeBuffer
            for (int i = 0; i < 3; i++) {
                for (int j = i + 1; j < 3; j++) {
                    bool exists = false; // Check if the edge already exists in edge buffer
                    for (int k = 0; k < edgeBuffer.eb[sorted[i]].size(); k++) {
                        if (edgeBuffer.eb[sorted[i]][k].V == sorted[j]) {
                            exists = true;
                            break;
                        }
                    }
                    if (exists == false) { // If it's a new edge, add it to the edge buffer
                        VFB newVFB = {
                        sorted[j], //V
                        0, //A
                        0, //F
                        0, //B
                        0, //Fa
                        0  //Ba
                        };
                        edgeBuffer.eb[sorted[i]].push_back(newVFB);
                    }
                    /*edgeBuffer.print();*/
                }
            }            
        }
    }

    //Setup all the outline data to the GPU
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    


    //This vector will be full of points that make up the silhouette edges
    std::vector<glm::vec3> vertices;

    // Infinite render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // Clear screen
        // ------
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // White background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set the current active shader to shader #1
        ourShader.use();
        // This shader will have filled-in triangles
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        //LIGHTS
        glm::vec3 lightPositions[2] = { glm::vec3(0.f, 0.f, 2.f), glm::vec3(-2.f, -1.f, 2.f) };
        glm::vec3 lightIntensities[2] = { glm::vec3(1.f, 1.f, 1.f), glm::vec3(1.f, 1.f, 1.f) };
        glUniform3fv(glGetUniformLocation(ourShader.ID, "lightPositions"), 2, glm::value_ptr(lightPositions[0]));
        glUniform3fv(glGetUniformLocation(ourShader.ID, "lightIntensities"), 2, glm::value_ptr(lightIntensities[0]));

        //CAMERA
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::vec3 viewPos = camera.Position;
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);
        ourShader.setVec3("viewPos", viewPos);

        //ACTION
        glm::mat4 model = rotation;// The model transformation of the mesh (controlled through arrows)
        
        // The default vase is a bit too big for our scene, so scale it down
        /*model = glm::scale(model, glm::vec3(.01f, .01f, .01f));*/
        model = glm::scale(model, glm::vec3(1.f, 1.f, 1.f));
        /*model = glm::scale(model, glm::vec3(10.f, 10.f, 10.f));*/

        float roughness = 0.3; // The roughness of the mesh [0,1]
        glm::vec3 objectColour = glm::vec3(0.722, 0.45, 0.2);

        ourShader.setMat4("model", model);
        ourShader.setFloat("roughness", roughness);
        ourShader.setVec3("objectColour", objectColour);
        
        ourModel.Draw(ourShader); // Rendering all the triangles in white

        // Set the current active shader to shader #2
        ourShader2.use();

        // Send vertex shader data
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);
        ourShader.setMat4("model", model);

        // Detele previous frame data. Make it ready for new silhouette points
        vertices.clear();
        
        // Every frame we need to zero out all the F, B, Fa, Ba bits.
        edgeBuffer.clearBuffer();
        
        // Filling the edge buffer
        for (int i = 0; i < faceList.size(); i++) { // For every face (triangle)
            glm::vec3 newCenter = glm::vec3(model * glm::vec4(faceList[i].center, 0.f)); // Compute the position of centroid in the real word cordinate
            glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model))); // Creating a normalMatrix
            glm::vec3 newNormal = glm::normalize(normalMatrix * faceList[i].n); // Calculating the normal in the world cordinate
            glm::vec3 viewDir = glm::normalize(newCenter - viewPos); // Calculating the view vector. This vecotr connects the viewer/camera to the centriod of the face (triangle), the direction of the vector is from the viewer to the the object.
            
            // If the dotproduct between the normal and the viewDirection is negative, this triangle is front facing
            if (glm::dot(newNormal, viewDir) <= 0.0f){
                // Updating the fornt bit (F)
                edgeBuffer.toggleFront(faceList[i].vi[0], faceList[i].vi[1]);
                edgeBuffer.toggleFront(faceList[i].vi[0], faceList[i].vi[2]);
                edgeBuffer.toggleFront(faceList[i].vi[1], faceList[i].vi[2]);
            }
            
            // Else, the trianlge in back facing (we can't see the front face of it)
            else {
                // Updating the back bit (B)
                edgeBuffer.toggleBack(faceList[i].vi[0], faceList[i].vi[1]);
                edgeBuffer.toggleBack(faceList[i].vi[0], faceList[i].vi[2]);
                edgeBuffer.toggleBack(faceList[i].vi[1], faceList[i].vi[2]);
            }
        }
        /*edgeBuffer.print();*/
        
        // Finding silhouette edges
        for (int i=0; i<edgeBuffer.eb.size(); i++){ // For every row of edge buffer
            for (int j = 0; j < edgeBuffer.eb[i].size(); j++) { // For every column of edge buffer
                if (edgeBuffer.eb[i][j].F == 1 && edgeBuffer.eb[i][j].B == 1) { // If both front bit and back bit are 1, then that edge is silhouette
                    int idx1 = i;
                    int idx2 = edgeBuffer.eb[i][j].V;
                    vertices.push_back(ourModel.meshes[0].vertices[idx1].Position);
                    vertices.push_back(ourModel.meshes[0].vertices[idx2].Position);
                } 
            }
        }

        //Send all the data to the GPU
        glBindVertexArray(VAO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_DYNAMIC_DRAW);
        glLineWidth(3.0); // Thickness 3
        glDrawArrays(GL_LINES, 0, vertices.size()); // Draw the silhouette lines
        glBindVertexArray(0);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

void ProcessKeyboard(Camera_Movement dir, float deltaTime)
{
    float rotVelocity = rotSpeed * deltaTime;
    if (dir == FORWARD)
    {
        rotation = glm::rotate(rotation, -rotVelocity, glm::vec3(1.0, 0.0, 0.0));
    }
    if (dir == BACKWARD)
    {
        rotation = glm::rotate(rotation, rotVelocity, glm::vec3(1.0, 0.0, 0.0));
    }
    if (dir == LEFT)
    {
        rotation = glm::rotate(rotation, -rotVelocity, glm::vec3(0.0, 1.0, 0.0));
    }
    if (dir == RIGHT)
    {
        rotation = glm::rotate(rotation, rotVelocity, glm::vec3(0.0, 1.0, 0.0));
    }
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    //Camera controls
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);

    //Model controls
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        ProcessKeyboard(LEFT, 0.5 * deltaTime);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        ProcessKeyboard(RIGHT, 0.1 * deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

// It gets a face list as an input and prints the face id and vertices
void printFaceList(std::vector<Face> &faceList) {
    for (int i = 0; i < faceList.size(); i++) {
        cout << "Face id:  " << i << endl;
        cout << "vertices: " << faceList[i].vi[0] << faceList[i].vi[1] << faceList[i].vi[2] << endl;
    }
}
