#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "shader.hpp"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800
#define BOARD_SIZE 16

typedef struct Coord {
  int x, y;
} Coord;

class Board {
public:
  // The status of a cell
  typedef enum Status {
    EMPTY, WALL, CHECKED, START, END, PATH
  } Status;

  // The progress of the A* algorithm
  typedef enum Progress {
    PREPARING, INITIALIZE, SOLVING, POST_SOLVE, FINISHED
  } Progress;

  typedef struct Cell {
    int f, g, h;
    Status status;
    Coord parent;
  } Cell;

  Cell c[BOARD_SIZE][BOARD_SIZE];
  Coord start, end;
  Progress progress;
  Coord openSet[BOARD_SIZE*BOARD_SIZE];
  Coord closedSet[BOARD_SIZE*BOARD_SIZE];
  int openCount, closedCount;
  int margin;
  float vertices[36*BOARD_SIZE*BOARD_SIZE];

  // Suppose that the screen is a square, therefore the board is also a square
  // This function converts the screen coordinates to OpenGL coordinates
  float screenToOpenGL(int screenCoord, int screenSize) {
    return (float)screenCoord / (float)screenSize * 2.0f - 1.0f;
  }

  // Manhattan distance between two coordinates
  int heuristic(Coord a, Coord b) {
    return abs(a.x-b.x) + abs(a.y-b.y);
  }

  // Sort an array of coordinates by fScore
  void bubbleSort(Cell board[][BOARD_SIZE], Coord arr[], int n) {
    for (int i=0; i<n-1; i++) {
      for (int j=0; j<n-i-1; j++) {
        if (board[arr[j].x][arr[j].y].f > board[arr[j+1].x][arr[j+1].y].f) {
          Coord temp = arr[j];
          arr[j] = arr[j+1];
          arr[j+1] = temp;
        }
      }
    }
  }

  // Remove the first element of an array of coordinates
  void removeFirst(Coord arr[], int n) {
    for (int i=0; i<n-1; i++) {
      arr[i] = arr[i+1];
    }
  }

  // Check if a coordinate is in an array of coordinates
  bool isIn(Coord arr[], int n, Coord c) {
    for (int i=0; i<n; i++) {
      if (arr[i].x == c.x && arr[i].y == c.y) {
        return true;
      }
    }
    return false;
  }

  // Constructor
  Board() {
    // Initialize cells
    for (int i=0; i<BOARD_SIZE; i++) {
      for (int j=0; j<BOARD_SIZE; j++) {
        // Assign values to INT_MAX
        c[i][j].f = 2147483647;
        c[i][j].g = 2147483647;
        c[i][j].h = 2147483647;
        c[i][j].status = EMPTY;
      }
    }

    // Initialize start and end
    start = {-1, -1};
    end = {-1, -1};

    // Initialize booleans
    progress = PREPARING;

    // Initialize open and closed sets
    openCount = 0;
    closedCount = 0;

    // Initialize vertices and colors of the board
    margin = 2;
    for (int i=0; i<BOARD_SIZE; i++) {
      for (int j=0; j<BOARD_SIZE; j++) {
        vertices[36*(i*BOARD_SIZE+j)+ 0] = screenToOpenGL(SCREEN_WIDTH*i/BOARD_SIZE+margin, SCREEN_WIDTH); // Bottom-left
        vertices[36*(i*BOARD_SIZE+j)+ 1] = screenToOpenGL(SCREEN_HEIGHT*j/BOARD_SIZE+margin, SCREEN_HEIGHT); // TODO - Check if this is correct
        vertices[36*(i*BOARD_SIZE+j)+ 2] = 0.0f;
        vertices[36*(i*BOARD_SIZE+j)+ 3] = 1.0f;
        vertices[36*(i*BOARD_SIZE+j)+ 4] = 1.0f;
        vertices[36*(i*BOARD_SIZE+j)+ 5] = 1.0f;
        vertices[36*(i*BOARD_SIZE+j)+ 6] = screenToOpenGL(SCREEN_WIDTH*(i+1)/BOARD_SIZE-margin, SCREEN_WIDTH); // Bottom-right
        vertices[36*(i*BOARD_SIZE+j)+ 7] = screenToOpenGL(SCREEN_HEIGHT*j/BOARD_SIZE+margin, SCREEN_HEIGHT);
        vertices[36*(i*BOARD_SIZE+j)+ 8] = 0.0f;
        vertices[36*(i*BOARD_SIZE+j)+ 9] = 1.0f;
        vertices[36*(i*BOARD_SIZE+j)+10] = 1.0f;
        vertices[36*(i*BOARD_SIZE+j)+11] = 1.0f;
        vertices[36*(i*BOARD_SIZE+j)+12] = screenToOpenGL(SCREEN_WIDTH*(i+1)/BOARD_SIZE-margin, SCREEN_WIDTH); // Top-right
        vertices[36*(i*BOARD_SIZE+j)+13] = screenToOpenGL(SCREEN_HEIGHT*(j+1)/BOARD_SIZE-margin, SCREEN_HEIGHT);
        vertices[36*(i*BOARD_SIZE+j)+14] = 0.0f;
        vertices[36*(i*BOARD_SIZE+j)+15] = 1.0f;
        vertices[36*(i*BOARD_SIZE+j)+16] = 1.0f;
        vertices[36*(i*BOARD_SIZE+j)+17] = 1.0f;
        vertices[36*(i*BOARD_SIZE+j)+18] = screenToOpenGL(SCREEN_WIDTH*i/BOARD_SIZE+margin, SCREEN_WIDTH); // Bottom-left
        vertices[36*(i*BOARD_SIZE+j)+19] = screenToOpenGL(SCREEN_HEIGHT*j/BOARD_SIZE+margin, SCREEN_HEIGHT);
        vertices[36*(i*BOARD_SIZE+j)+20] = 0.0f;
        vertices[36*(i*BOARD_SIZE+j)+21] = 1.0f;
        vertices[36*(i*BOARD_SIZE+j)+22] = 1.0f;
        vertices[36*(i*BOARD_SIZE+j)+23] = 1.0f;
        vertices[36*(i*BOARD_SIZE+j)+24] = screenToOpenGL(SCREEN_WIDTH*i/BOARD_SIZE+margin, SCREEN_WIDTH); // Top-left
        vertices[36*(i*BOARD_SIZE+j)+25] = screenToOpenGL(SCREEN_HEIGHT*(j+1)/BOARD_SIZE-margin, SCREEN_HEIGHT);
        vertices[36*(i*BOARD_SIZE+j)+26] = 0.0f;
        vertices[36*(i*BOARD_SIZE+j)+27] = 1.0f;
        vertices[36*(i*BOARD_SIZE+j)+28] = 1.0f;
        vertices[36*(i*BOARD_SIZE+j)+29] = 1.0f;
        vertices[36*(i*BOARD_SIZE+j)+30] = screenToOpenGL(SCREEN_WIDTH*(i+1)/BOARD_SIZE-margin, SCREEN_WIDTH); // Top-right
        vertices[36*(i*BOARD_SIZE+j)+31] = screenToOpenGL(SCREEN_HEIGHT*(j+1)/BOARD_SIZE-margin, SCREEN_HEIGHT);
        vertices[36*(i*BOARD_SIZE+j)+32] = 0.0f;
        vertices[36*(i*BOARD_SIZE+j)+33] = 1.0f;
        vertices[36*(i*BOARD_SIZE+j)+34] = 1.0f;
        vertices[36*(i*BOARD_SIZE+j)+35] = 1.0f;
      }
    }
  };

  // Change color of cell a
  void changeColor(Coord a, Status s) {
    switch(s) {
      case EMPTY: // White
        vertices[36*(a.x*BOARD_SIZE+a.y)+ 3] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+ 4] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+ 5] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+ 9] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+10] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+11] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+15] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+16] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+17] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+21] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+22] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+23] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+27] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+28] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+29] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+33] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+34] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+35] = 1.0f;
        break;
      
      case WALL: // Black
        vertices[36*(a.x*BOARD_SIZE+a.y)+ 3] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+ 4] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+ 5] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+ 9] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+10] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+11] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+15] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+16] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+17] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+21] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+22] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+23] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+27] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+28] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+29] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+33] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+34] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+35] = 0.0f;
        break;
      
      case CHECKED: // Green
        vertices[36*(a.x*BOARD_SIZE+a.y)+ 3] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+ 4] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+ 5] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+ 9] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+10] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+11] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+15] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+16] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+17] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+21] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+22] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+23] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+27] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+28] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+29] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+33] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+34] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+35] = 0.0f;
        break;
      
      case START: // Red
        vertices[36*(a.x*BOARD_SIZE+a.y)+ 3] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+ 4] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+ 5] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+ 9] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+10] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+11] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+15] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+16] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+17] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+21] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+22] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+23] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+27] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+28] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+29] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+33] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+34] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+35] = 0.0f;
        break;

      case END: // Blue
        vertices[36*(a.x*BOARD_SIZE+a.y)+ 3] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+ 4] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+ 5] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+ 9] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+10] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+11] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+15] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+16] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+17] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+21] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+22] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+23] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+27] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+28] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+29] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+33] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+34] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+35] = 1.0f;
        break;

      case PATH: // Rainbow
        vertices[36*(a.x*BOARD_SIZE+a.y)+ 3] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+ 4] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+ 5] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+ 9] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+10] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+11] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+15] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+16] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+17] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+21] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+22] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+23] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+27] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+28] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+29] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+33] = 1.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+34] = 0.0f;
        vertices[36*(a.x*BOARD_SIZE+a.y)+35] = 1.0f;
        break;
    }
  };
};

// Callback function for when the window is resized
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);
}

int main(void) {
  int x, y;
  Board board;

  // Initialize GLFW
  if (!glfwInit()) {
    std::cout << "Failed to initialize GLFW" << std::endl;
    return -1;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // OpenGL 3.3
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Use core profile
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For MacOS

  // Create a window with GLFW
  GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "A* Pathfinding", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  // Initialize GLAD
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  // Ensure we can capture the escape key being pressed below
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

  // Create shader program
  Shader shader("vertex.glsl", "fragment.glsl");

  // Create VAO and VBO
  unsigned int VAO, VBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(board.vertices), board.vertices, GL_STATIC_DRAW);

  // Set vertex attributes
  // Position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  // Color attribute
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // Use the shader program here; as we only have one
  glUseProgram(shader.ID);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  // Prepare mouse input
  int mousePre = GLFW_RELEASE;
  int mouseNow = GLFW_RELEASE;
  double mouseX, mouseY;

  // Prepare keyboard input
  int sPre = GLFW_RELEASE;
  int sNow = GLFW_RELEASE;
  int ePre = GLFW_RELEASE;
  int eNow = GLFW_RELEASE;

  // Main loop
  while (!glfwWindowShouldClose(window)) {
    // Get mouse input
    mouseNow = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    if (mouseNow == GLFW_RELEASE && mousePre == GLFW_PRESS) {
      glfwGetCursorPos(window, &mouseX, &mouseY);
      x = (int)(mouseX*BOARD_SIZE/SCREEN_WIDTH);
      y = (int)((SCREEN_HEIGHT-mouseY)*BOARD_SIZE/SCREEN_HEIGHT);

      // Change the status of the clicked cell
      if (board.c[x][y].status == Board::EMPTY) {
        board.c[x][y].status = Board::WALL;
        board.progress = Board::PREPARING;
        board.changeColor({x, y}, Board::WALL);
      } else {
        if (board.start.x == x && board.start.y == y) {
          board.start.x = -1;
          board.start.y = -1;
        } else if (board.end.x == x && board.end.y == y) {
          board.end.x = -1;
          board.end.y = -1;
        }
        board.c[x][y].status = Board::EMPTY;
        board.progress = Board::PREPARING;
        board.changeColor({x, y}, Board::EMPTY);
      }
      glBufferData(GL_ARRAY_BUFFER, sizeof(board.vertices), board.vertices, GL_STATIC_DRAW);
    }
    mousePre = mouseNow;

    // Get keyboard input
    sNow = glfwGetKey(window, GLFW_KEY_S);
    eNow = glfwGetKey(window, GLFW_KEY_E);
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, GL_TRUE);
    } else if (sNow == GLFW_RELEASE && sPre == GLFW_PRESS) {
      glfwGetCursorPos(window, &mouseX, &mouseY);
      x = (int)(mouseX*BOARD_SIZE/SCREEN_WIDTH);
      y = (int)((SCREEN_HEIGHT-mouseY)*BOARD_SIZE/SCREEN_HEIGHT);
      if (!(board.start.x == -1 && board.start.y == -1)) {
        board.c[board.start.x][board.start.y].status = Board::EMPTY;
        board.changeColor(board.start, Board::EMPTY);
      }
      board.start = {x, y};
      board.c[x][y].status = Board::START;
      board.progress = Board::PREPARING;
      board.changeColor({x, y}, Board::START);
      if (!(board.end.x == -1 && board.end.y == -1)) {
        board.progress = Board::INITIALIZE;
      }
      glBufferData(GL_ARRAY_BUFFER, sizeof(board.vertices), board.vertices, GL_STATIC_DRAW);
    } else if (eNow == GLFW_RELEASE && ePre == GLFW_PRESS) {
      glfwGetCursorPos(window, &mouseX, &mouseY);
      x = (int)(mouseX*BOARD_SIZE/SCREEN_WIDTH);
      y = (int)((SCREEN_HEIGHT-mouseY)*BOARD_SIZE/SCREEN_HEIGHT);
      if (!(board.end.x == -1 && board.end.y == -1)) {
        board.c[board.end.x][board.end.y].status = Board::EMPTY;
        board.changeColor(board.end, Board::EMPTY);
      }
      board.end = {x, y};
      board.c[x][y].status = Board::END;
      board.progress = Board::PREPARING;
      board.changeColor({x, y}, Board::END);
      if (!(board.start.x == -1 && board.start.y == -1)) {
        board.progress = Board::INITIALIZE;
      }
      glBufferData(GL_ARRAY_BUFFER, sizeof(board.vertices), board.vertices, GL_STATIC_DRAW);
    }
    sPre = sNow;
    ePre = eNow;

    // Initialize A* algorithm
    if (board.progress == Board::INITIALIZE) {
      // Reset the colors of the board
      for (int i=0; i<BOARD_SIZE; i++) {
        for (int j=0; j<BOARD_SIZE; j++) {
          if (board.c[i][j].status == Board::PATH) {
            board.c[i][j].status = Board::EMPTY;
            board.changeColor({i, j}, Board::EMPTY);
          }
        }
      }

      // Initialize the open set
      board.openSet[board.openCount++] = board.start;
      board.c[board.start.x][board.start.y].g = 0;
      board.c[board.start.x][board.start.y].h = board.heuristic(board.start, board.end);
      board.c[board.start.x][board.start.y].f = board.c[board.start.x][board.start.y].g + board.c[board.start.x][board.start.y].h;
      board.c[board.start.x][board.start.y].parent = {-1, -1};
      board.progress = Board::SOLVING;
      glBufferData(GL_ARRAY_BUFFER, sizeof(board.vertices), board.vertices, GL_STATIC_DRAW);
    }

    // Solve the maze
    if (board.progress == Board::SOLVING) {
      // If the open set is empty, there is no solution
      if (board.openCount <= 0) {
        std::cout << "No solution" << std::endl;
        return 0;
      }

      // Sort the open set by fScore
      board.bubbleSort(board.c, board.openSet, board.openCount);

      // The current cell is the one with the lowest fScore
      Coord cur = board.openSet[0];
      board.removeFirst(board.openSet, board.openCount--);
      board.closedSet[board.closedCount++] = cur;

      // Check left neighbor
      if (cur.x > 0) {
        if (board.c[cur.x-1][cur.y].status == Board::END) {
          board.c[cur.x-1][cur.y].parent = cur;
          board.progress = Board::POST_SOLVE;
        }
        if (!board.isIn(board.closedSet, board.closedCount, {cur.x-1, cur.y}) && board.c[cur.x-1][cur.y].status != Board::WALL) {
          if (board.c[cur.x-1][cur.y].f > board.c[cur.x][cur.y].g+1+board.heuristic({cur.x-1, cur.y}, board.end)) {
            board.openSet[board.openCount++] = {cur.x-1, cur.y};
            board.c[cur.x-1][cur.y].g = board.c[cur.x][cur.y].g+1;
            board.c[cur.x-1][cur.y].h = board.heuristic({cur.x-1, cur.y}, board.end);
            board.c[cur.x-1][cur.y].f = board.c[cur.x-1][cur.y].g + board.c[cur.x-1][cur.y].h;
            board.c[cur.x-1][cur.y].parent = cur;
            board.c[cur.x-1][cur.y].status = Board::CHECKED;
            board.changeColor({cur.x-1, cur.y}, Board::CHECKED);
          }
        }
      }

      // Check right neighbor
      if (cur.x < BOARD_SIZE-1) {
        if (board.c[cur.x+1][cur.y].status == Board::END) {
          board.c[cur.x+1][cur.y].parent = cur;
          board.progress = Board::POST_SOLVE;
        }
        if (!board.isIn(board.closedSet, board.closedCount, {cur.x+1, cur.y}) && board.c[cur.x+1][cur.y].status != Board::WALL) {
          if (board.c[cur.x+1][cur.y].f > board.c[cur.x][cur.y].g+1+board.heuristic({cur.x+1, cur.y}, board.end)) {
            board.openSet[board.openCount++] = {cur.x+1, cur.y};
            board.c[cur.x+1][cur.y].g = board.c[cur.x][cur.y].g+1;
            board.c[cur.x+1][cur.y].h = board.heuristic({cur.x+1, cur.y}, board.end);
            board.c[cur.x+1][cur.y].f = board.c[cur.x+1][cur.y].g + board.c[cur.x+1][cur.y].h;
            board.c[cur.x+1][cur.y].parent = cur;
            board.c[cur.x+1][cur.y].status = Board::CHECKED;
            board.changeColor({cur.x+1, cur.y}, Board::CHECKED);
          }
        }
      }

      // Check top neighbor
      if (cur.y > 0) {
        if (board.c[cur.x][cur.y-1].status == Board::END) {
          board.c[cur.x][cur.y-1].parent = cur;
          board.progress = Board::POST_SOLVE;
        }
        if (!board.isIn(board.closedSet, board.closedCount, {cur.x, cur.y-1}) && board.c[cur.x][cur.y-1].status != Board::WALL) {
          if (board.c[cur.x][cur.y-1].f > board.c[cur.x][cur.y].g+1+board.heuristic({cur.x, cur.y-1}, board.end)) {
            board.openSet[board.openCount++] = {cur.x, cur.y-1};
            board.c[cur.x][cur.y-1].g = board.c[cur.x][cur.y].g+1;
            board.c[cur.x][cur.y-1].h = board.heuristic({cur.x, cur.y-1}, board.end);
            board.c[cur.x][cur.y-1].f = board.c[cur.x][cur.y-1].g + board.c[cur.x][cur.y-1].h;
            board.c[cur.x][cur.y-1].parent = cur;
            board.c[cur.x][cur.y-1].status = Board::CHECKED;
            board.changeColor({cur.x, cur.y-1}, Board::CHECKED);
          }
        }
      }

      // Check bottom neighbor
      if (cur.y < BOARD_SIZE-1) {
        if (board.c[cur.x][cur.y+1].status == Board::END) {
          board.c[cur.x][cur.y+1].parent = cur;
          board.progress = Board::POST_SOLVE;
        }
        if (!board.isIn(board.closedSet, board.closedCount, {cur.x, cur.y+1}) && board.c[cur.x][cur.y+1].status != Board::WALL) {
          if (board.c[cur.x][cur.y+1].f > board.c[cur.x][cur.y].g+1+board.heuristic({cur.x, cur.y+1}, board.end)) {
            board.openSet[board.openCount++] = {cur.x, cur.y+1};
            board.c[cur.x][cur.y+1].g = board.c[cur.x][cur.y].g+1;
            board.c[cur.x][cur.y+1].h = board.heuristic({cur.x, cur.y+1}, board.end);
            board.c[cur.x][cur.y+1].f = board.c[cur.x][cur.y+1].g + board.c[cur.x][cur.y+1].h;
            board.c[cur.x][cur.y+1].parent = cur;
            board.c[cur.x][cur.y+1].status = Board::CHECKED;
            board.changeColor({cur.x, cur.y+1}, Board::CHECKED);
          }
        }
      }
      glBufferData(GL_ARRAY_BUFFER, sizeof(board.vertices), board.vertices, GL_STATIC_DRAW);
    }

    // Post-solve
    if (board.progress == Board::POST_SOLVE) {
      Coord cur = board.end;
      while (!(cur.x == board.start.x && cur.y == board.start.y)) {
        board.changeColor(cur, Board::PATH);
        cur = board.c[cur.x][cur.y].parent;
      }
      board.progress = Board::FINISHED;
      glBufferData(GL_ARRAY_BUFFER, sizeof(board.vertices), board.vertices, GL_STATIC_DRAW);
    }

    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw the board
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6*BOARD_SIZE*BOARD_SIZE);

    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // Clean up
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glfwTerminate();
  return 0;
}