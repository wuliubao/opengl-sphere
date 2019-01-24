#include <jni.h>
#include <string>
#include <GLES3/gl3.h>
#include <android/log.h>
#include <cstdlib>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"


#define LOG_TAG "NATIVE-LIB"
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#define PI glm::pi <GLfloat> ()
#define stacks 50
#define slices 50
#define radius 0.5

int numberOfPoints = 0;
int numberOfIndexes = 0;
GLfloat vertices[(stacks + 1) * slices * 3];
GLfloat normals[(stacks + 1) * slices * 3];
GLuint indices[stacks * slices * 10];

GLuint deviceWidth = 500, deviceHeight = 500;
float moveDeltaX = 0.0f, moveDeltaY = 0.0f;

GLuint VBO, norVBO, VAO, EBO, sphereProgram;
const GLchar* vertexShaderSource =
        "#version 300 es\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec3 aNormal;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "out vec3 FragPos;\n"
        "out vec3 Normal;\n"
        "void main()\n"
        "{\n"
        "	FragPos = vec3(model * vec4(aPos, 1.0));\n"
        "	Normal = mat3(transpose(inverse(model))) * aNormal;\n"
        "	gl_Position = projection * view * vec4(FragPos, 1.0);\n"
        "}\n\0";

const GLchar* fragmentShaderSource =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec3 Normal;\n"
        "in vec3 FragPos;\n"
        "uniform vec3 objectColor;\n"
        "uniform vec3 lightColor;\n"
        "uniform vec3 lightPos;\n"
        "out vec4 color;\n"
        "void main()\n"
        "{\n"
        "	float ambientStrength = 0.1;\n"
        "	vec3 ambient = ambientStrength * lightColor;\n"
        "	vec3 norm = normalize(Normal);\n"
        "	vec3 lightDir = normalize(lightPos - FragPos);\n"
        "	float diff = max(dot(norm, lightDir), 0.0);\n"
        "	vec3 diffuse = diff * lightColor;\n"
        "	vec3 result = (ambient + diffuse) * objectColor;\n"
        "	color = vec4(result, 1.0);\n"
        "}\n\0";

void calSphereData() {
    // Calc The Vertices and Normals
    for (int i = 0; i <= slices; i++) {
        float phi = i * (PI / slices) ;

        for (int j = 0; j < stacks; j++) {
            float theta = j * (PI * 2 / stacks);
            float x = cosf(theta) * sinf(phi);
            float y = cosf(phi);
            float z = sinf(theta) * sinf(phi);

            vertices[numberOfPoints++] = x * radius;
            normals[numberOfPoints] = x / radius;
            vertices[numberOfPoints++] = y * radius;
            normals[numberOfPoints] = y / radius;
            vertices[numberOfPoints++] = z * radius;
            normals[numberOfPoints] = z / radius;
        }
    }
    // Calc The Index Positions
    for (int i = 0; i < numberOfPoints / 3 - stacks; i++) {
        if ((i + 1) % stacks == 0) {
            indices[numberOfIndexes++] = i;
            indices[numberOfIndexes++] = i - stacks + 1;
            indices[numberOfIndexes++] = i + stacks;

            indices[numberOfIndexes++] = i - stacks + 1;
            indices[numberOfIndexes++] = i + stacks;
            if (i + 1 == numberOfPoints / 3){
                indices[numberOfIndexes++] = numberOfPoints - stacks;
            } else {
                indices[numberOfIndexes++] = i + 1;
            }

        } else {
            indices[numberOfIndexes++] = i;
            indices[numberOfIndexes++] = i + 1;
            indices[numberOfIndexes++] = i + stacks;

            indices[numberOfIndexes++] = i + 1;
            indices[numberOfIndexes++] = i + stacks;
            indices[numberOfIndexes++] = i + stacks + 1;
        }
    }
}

void initSphere() {

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numberOfIndexes * sizeof(GLuint), indices, GL_STATIC_DRAW);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, numberOfPoints * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &norVBO);
    glBindBuffer(GL_ARRAY_BUFFER, norVBO);
    glBufferData(GL_ARRAY_BUFFER, numberOfPoints * sizeof(GLfloat), normals, GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);
}


bool checkGlError(const char* funcName) {
    GLint err = glGetError();
    if (err != GL_NO_ERROR) {
        ALOGE("GL error after %s(): %d\n", funcName, err);
        return true;
    }
    return false;
}


GLuint createShader(GLenum shaderType, const char* src) {
    GLuint shader = glCreateShader(shaderType);
    if (!shader) {
        checkGlError("glCreateShader");
        return 0;
    }
    glShaderSource(shader, 1, &src, NULL);
    GLint compiled = GL_FALSE;
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint infoLogLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLen);
        if (infoLogLen > 0) {
            GLchar* infoLog = (GLchar*)malloc(infoLogLen);
            if (infoLog) {
                glGetShaderInfoLog(shader, infoLogLen, NULL, infoLog);
                ALOGE("Could not compile %s shader:%s", shaderType == GL_VERTEX_SHADER ? "vertex" : "fragment", infoLog);
                free(infoLog);
            }
        }
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}
GLuint createProgram(const char* vtxSrc, const char* fragSrc) {
    GLuint vtxShader = 0;
    GLuint fragShader = 0;
    GLuint program = 0;
    GLint linked = GL_FALSE;
    vtxShader = createShader(GL_VERTEX_SHADER, vtxSrc);
    if (!vtxShader)
        goto exit;
    fragShader = createShader(GL_FRAGMENT_SHADER, fragSrc);
    if (!fragShader)
        goto exit;
    program = glCreateProgram();
    if (!program) {
        checkGlError("glCreateProgram");
        goto exit;
    }
    glAttachShader(program, vtxShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        ALOGE("Could not link program");
        GLint infoLogLen = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLen);
        if (infoLogLen) {
            GLchar* infoLog = (GLchar*)malloc(infoLogLen);
            if (infoLog) {
                glGetProgramInfoLog(program, infoLogLen, NULL, infoLog);
                ALOGE("Could not link program:%s", infoLog);
                free(infoLog);
            }
        }
        glDeleteProgram(program);
        program = 0;
    }
    exit:
    glDeleteShader(vtxShader);
    glDeleteShader(fragShader);
    return program;
}

void drawSphere() {
    glBindVertexArray(VAO);
    glUseProgram(sphereProgram);

    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 model;

    glm::vec3 objectColor(1.0f, 0.5f, 0.31f);
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
    glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -2.0f));
    projection = glm::perspective(45.0f, (GLfloat)deviceWidth / (GLfloat)deviceHeight, 0.1f, 100.0f);
    model = glm::rotate(model, (GLfloat) glm::radians(moveDeltaX + 50.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, (GLfloat) glm::radians(moveDeltaY + 50.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    GLint modelLoc = glGetUniformLocation(sphereProgram, "model");
    GLint viewLoc = glGetUniformLocation(sphereProgram, "view");
    GLint projLoc = glGetUniformLocation(sphereProgram, "projection");
    GLint objectColorLoc = glGetUniformLocation(sphereProgram, "objectColor");
    GLint lightColorLoc = glGetUniformLocation(sphereProgram, "lightColor");
    GLint lightPosLoc = glGetUniformLocation(sphereProgram, "lightPos");

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glUniform3fv(objectColorLoc, 1, glm::value_ptr(objectColor));
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
    glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));

    glDrawElements(GL_TRIANGLES, numberOfIndexes, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);

}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_liubao_gl_GLNativeRender_nativeInit(
        JNIEnv *env,
        jclass type
        ){
    sphereProgram = createProgram(vertexShaderSource, fragmentShaderSource);
    if (!sphereProgram){
        ALOGE("create sphere program error !");
        return JNI_FALSE;
    }

    glEnable(GL_DEPTH_TEST);
    calSphereData();
    initSphere();
    return JNI_TRUE;
}

extern "C" JNIEXPORT void JNICALL
Java_com_liubao_gl_GLNativeRender_nativeDrawFrame(
        JNIEnv *env,
        jclass type
        ) {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawSphere();
}

extern "C" JNIEXPORT void JNICALL
Java_com_liubao_gl_GLNativeRender_nativeResize (
        JNIEnv* env,
        jclass type,
        jint width,
        jint height
        ){
    deviceWidth = (GLuint) width;
    deviceHeight = (GLuint) height;

    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);
}

extern "C" JNIEXPORT void JNICALL
Java_com_liubao_gl_GLNativeRender_nativeDispatchEvent(
        JNIEnv* env,
        jclass type,
        jfloat x,
        jfloat y
        ){
    moveDeltaX += x;
    moveDeltaY += y;
}
