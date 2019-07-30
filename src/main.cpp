#include <iostream>
#include <vector>

#include <glm/glm.hpp>

#include "entity.h"
#include "camera.h"
#include "model.h"
#include "shader.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double x, double y);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

extern CharacterEntity *entity;

std::vector<Model> models;

int main(int argc, char **argv)
{
  // glfw: initialize and configure
  // ------------------------------
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
  #endif

  // glfw window creation
  // --------------------
  GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
  // move the window so the console is visible
  glfwSetWindowPos(window, SCR_WIDTH/2, SCR_HEIGHT/2);
  if (window == NULL) {
    printf("Failed to initialize window\n");
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);

  // tell GLFW to capture our mouse
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // glad: load all OpenGL function pointers
  // ---------------------------------------
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
      std::cout << "Failed to initialize GLAD" << std::endl;
      return -1;
  }
  // configure global opengl state
  // -----------------------------
  glEnable(GL_DEPTH_TEST);

  // build and compile shaders
  // -------------------------
  Shader ourShader("./data/shader.vs", "./data/shader.fs");

  // load models
  // -----------
  Model ourModel("./data/nanosuit/nanosuit.obj");

  models.push_back(ourModel);

  // size of collision ellipse, experiment with this to change fidelity of detection
  static vec3 boundingEllipse = {0.5f, 1.0f, 0.5f};
  entity = new CharacterEntity(models, boundingEllipse);

  // initialize player infront of model
  entity->position[1] = 10.0f;
  entity->position[2] = 5.0f;

  while (!glfwWindowShouldClose(window)) {
    // per-frame time logic
    // --------------------
    float currentFrame = glfwGetTime();
    deltaTime = (float)currentFrame - (float)lastFrame;
    lastFrame = currentFrame;

    // input
    // -----
    processInput(window);

    // render
    // ------
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // don't forget to enable shader before setting uniforms
    ourShader.use();

    entity->update();
    camera.Position = entity->position;
    entity->velocity = entity->velocity * 0.7f;

    // TO DO fix gravity
    /*float y = entity->velocity[1];
    vec3 temp;
    temp *= 0.8f;
    entity->velocity -= temp;
    entity->velocity[1] = y;
    entity->velocity[1] -= 0.5f * deltaTime;
    if (entity->grounded) {
      entity->velocity[1] = 0.0f;
    }*/

    // view/projection transformations
    mat4 projection = perspective(radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
    mat4 view = camera.GetViewMatrix();
    ourShader.setMat4("u_projection", projection);
    ourShader.setMat4("u_view", view);

    // render the loaded model
    mat4 model = mat4(1.0f);
    // bug: collision doesnt work when model is translated/scaled
    //model = translate(model, vec3(0.0f, -1.75f, 0.0f)); // translate it down so it's at the center of the scene
    //model = scale(model, vec3(0.2f, 0.2f, 0.2f));	// it's a bit too big for our scene, so scale it down
    ourShader.setMat4("u_model", model);

    for (unsigned int i = 0; i < models.size(); i++)
    {
      ourModel.Draw(ourShader);
    }

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    // -------------------------------------------------------------------------------
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  printf("Exiting\n");

  return EXIT_SUCCESS;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
  glViewport(0, 0, width, height);
}

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

    camera.ProcessMouseMovement(xoffset, yoffset, true);
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}



