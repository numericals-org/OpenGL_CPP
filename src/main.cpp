#include "config.h"
#include "triangle_mesh.h"
#include "material.h"
#include "linear_algebros.h"

unsigned int make_shader(const std::string& vertex_filepath, const std::string& fragment_filepath);
unsigned int make_module(const std::string& filepath, unsigned int module_type);

int main() {   

    GLFWwindow* window;

    if(!glfwInit()){
        std::cout << "GLFW couldn't start" << std::endl;
        return -1;
    }

    window = glfwCreateWindow(640, 480, "My window", NULL, NULL);
    glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        glfwTerminate();
        return -1;
    }

    glClearColor(0.1f, 0.0f, 0.0f, 1.0f);
    int w,h;
    glfwGetFramebufferSize(window, &w, &h);
    glViewport(0,0,w,h);

    TriangleMesh* triangle = new TriangleMesh();
    
    Material* material = new Material((std::string(PROJECT_SOURCE_DIR) + "/img/testImage.jpg").c_str());
    Material* mask = new Material((std::string(PROJECT_SOURCE_DIR) + "/img/mask.jpg").c_str());

    unsigned int shader = make_shader(
        "/src/shaders/vertex.txt",
        "/src/shaders/fragment.txt"
    );

    // set texture units
    glUseProgram(shader);
    glUniform1i(glGetUniformLocation(shader, "material"), 0);
    glUniform1i(glGetUniformLocation(shader, "mask"), 1);

    vec3 quad_position = {-0.2f, 0.4f, 0.0f};
    vec3 camera_pos = {-5.0f, 0.0f, 3.0f};
    vec3 camera_target = {0.0f, 0.0f, 0.0f};
    unsigned int model_location = glGetUniformLocation(shader, "model");
    unsigned int view_location = glGetUniformLocation(shader, "view");
    unsigned int proj_location = glGetUniformLocation(shader, "projection");

    mat4 view = create_look_at(camera_pos, camera_target);
    glUniformMatrix4fv(view_location, 1, GL_FALSE, view.entries);
    
    mat4 projection = create_perspective_projection(
        45.0f, 640.0f/480.0f, 0.1f, 10.0f
    );
    glUniformMatrix4fv(proj_location, 1, GL_FALSE, projection.entries);

    // enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();

        mat4 model = create_model_transform(quad_position,10 * glfwGetTime());
        glUniformMatrix4fv(model_location, 1, GL_FALSE, model.entries);

        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shader);
        material->use(0);
        mask->use(1);
        triangle->draw();
        glfwSwapBuffers(window);
    }

    glDeleteProgram(shader);
    delete triangle;
    delete material;
    delete mask;
    glfwTerminate();
    return 0;
}

unsigned int make_shader(const std::string& vertex_filepath, const std::string& fragment_filepath) {
    std::vector<unsigned int> modules;
    modules.push_back(make_module(vertex_filepath, GL_VERTEX_SHADER));
    modules.push_back(make_module(fragment_filepath, GL_FRAGMENT_SHADER));

    unsigned int shader = glCreateProgram();
    for(unsigned int shaderModule: modules){
        glAttachShader(shader, shaderModule);
    }
    glLinkProgram(shader);

    int success;
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if(!success) {
        char errorLog[1024];
        glGetProgramInfoLog(shader, 1024, NULL, errorLog);
        std::cout << "Shader Module compilation error:\n" << errorLog << std::endl;
    }

    for(unsigned int shaderModule: modules){
        glDeleteShader(shaderModule);
    }

    return shader;
}

unsigned int make_module(const std::string& filepath, unsigned int module_type) {
    std::ifstream file;
    std:: stringstream bufferedLines;
    std::string line;

    file.open(std::string(PROJECT_SOURCE_DIR) + filepath);

    while (std::getline(file, line))
    {
        bufferedLines << line << "\n";
    }

    std::string shaderSource = bufferedLines.str();
    const char* shaderSrc = shaderSource.c_str();
    bufferedLines.str("");
    file.close();

    unsigned int shaderModule = glCreateShader(module_type);
    glShaderSource(shaderModule, 1, &shaderSrc, NULL);
    glCompileShader(shaderModule);

    int success;
    glGetShaderiv(shaderModule, GL_COMPILE_STATUS, &success);
    if(!success) {
        char errorLog[1024];
        glGetShaderInfoLog(shaderModule, 1024, NULL, errorLog);
        std::cout << "Shader Module compilation error:\n" << errorLog << std::endl;
    }

    return shaderModule;
}