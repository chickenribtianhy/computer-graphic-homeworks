#include <mesh.h>
#include <utils.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <camera.h>
using namespace std;
Mesh::Mesh(const std::string &path)
{
  loadDataFromFile(getPath(path));
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

  glBufferData(GL_ARRAY_BUFFER,
               sizeof(Vertex) * vertices.size(), vertices.data(),
               GL_STATIC_DRAW);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               sizeof(GLuint) * indices.size(), indices.data(),
               GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  // normal
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  // my_shader = Shader("../assets/vertex_shader.vs", "../assets/fragment_shader.fs");
  // my_shader = Shader("../assets/vertex_shader.vs", "../assets/trial.fs");
  my_shader = Shader("../assets/vertex_shader.vs", "../assets/fragment_shader.fs", "../assets/geometry_shader.gs");
  // my_shader = Shader("../assets/vertex_shader.vs", "../assets/trial3.fs", "../assets/trial2.gs");
}

/**
 * TODO: implement load mesh data from file here
 * you need to open file with [path]
 *
 * File format
 *
 *
 * For each line starting with 'v' contains 3 floats, which
 * represents the position of a vertex
 *
 * For each line starting with 'n' contains 3 floats, which
 * represents the normal of a vertex
 *
 * For each line starting with 'f' contains 6 integers
 * [v0, n0, v1, n1, v2, n2], which represents the triangle face
 * v0, n0 means the vertex index and normal index of the first vertex
 * v1, n1 means the vertex index and normal index of the second vertex
 * v2, n2 means the vertex index and normal index of the third vertex
 */

template <class _InputIterator, class _Tp>
int my_find(_InputIterator __first, _InputIterator __last, const _Tp &__value_)
{
  int count = 0;
  for (; __first != __last; ++__first, ++count)
    if (*__first == __value_)
    {
      return count;
    }
  return -1;
}
void Mesh::loadDataFromFile(const std::string &path)
{
  /**
   * path stands for the where the object is storaged
   * */
  vector<vec3> _vertex;
  vector<vec3> _normal;
  vector<vec2> _pairs;

  std::ifstream infile(path);
  string _x;
  string _temp;
  char _no_use;

  // _x = infile.get();
  // getline(infile,_x);
  while (getline(infile, _x))
  {
    // cout << (int)_x << "===" << _x << endl;
    if (_x[0] == '#')
    {
      // getline(infile, _temp);
      // _x = infile.get();
      continue;
    }
    else if (_x[0] == '\r' || _x[0] == '\n')
    {
      // _x = infile.get();
      continue;
    }
    // // getline(infile, _temp);
    // // cout << _temp << endl;
    if (_x[0] == 'v')
    {
      // infile.get(); // ' '
      vec3 _v;
      stringstream(_x) >> _no_use >> _v.x >> _v.y >> _v.z;
      _vertex.push_back(_v);
      // cout << _no_use << " " << _v.x << " " << _v.y << " " << _v.z << endl;
    }
    else if (_x[0] == 'n')
    {
      // infile.get();
      vec3 _n;
      // infile >> _n.x >> _n.y >> _n.z;
      stringstream(_x) >> _no_use >> _n.x >> _n.y >> _n.z;
      _normal.push_back(_n);
      // cout << _no_use << " " << _n.x << " " << _n.y << " " << _n.z << endl;
    }
    else if (_x[0] == 'f')
    {
      // infile.get();
      stringstream ss(_x);
      ss >> _no_use;
      int _id[6];
      vec2 _one_pair;
      for (int i = 0; i < 3; ++i)
      {
        ss >> _one_pair.x >> _one_pair.y;
        int find_index = my_find(_pairs.begin(), _pairs.end(), _one_pair);
        if (find_index != -1)
        {
          /* pair registered */
          indices.push_back(find_index);
        }
        else
        {
          /* now register this pair */
          _pairs.push_back(_one_pair);
          indices.push_back(_pairs.size() - 1);
          Vertex _face;
          _face.position = _vertex[_one_pair.x];
          _face.normal = _normal[_one_pair.y];
          vertices.push_back(_face);
        }
      }
    }
    else
    {
      // cout << "false : " << (int)_x << "faq" << endl;
      assert(false);
      // cout << (int)(_x[0]) << "===" << _x << endl;
    }
    // _x = infile.get();
  }
  /*
  // checking
  cout << "++++++++++++vertices++++++++++++" << endl;
  vector<Vertex>::iterator iter;
  for (iter = vertices.begin(); iter != vertices.end(); ++iter)
  {
    cout << iter->position.x << " " << iter->position.y << " " << iter->position.z << "  "
         << iter->normal.x << " " << iter->normal.y << " " << iter->normal.z << endl;
  }
   cout << "++++++++++++pairs++++++++++++" << endl;
  vector<vec2>::iterator iter1;
  for (iter1 = _pairs.begin(); iter1 != _pairs.end(); ++iter1)
  {
    cout << iter1->x << " " << iter1->y << endl;
  }
  cout << "++++++++++++indices++++++++++++" << endl;
  vector<GLuint>::iterator iter2;
  for (iter2 = indices.begin(); iter2 != indices.end(); ++iter2)
  {
    cout << *iter2 << endl;
  } */
}

/**
 * TODO: implement your draw code here
 */
// GLFWwindow *window;
extern GLFWwindow *window;
extern int WIDTH;
extern int HEIGHT;

void Mesh::draw() const
{
  /* use shader program */
  my_shader.use();
  /* update shader uniform */
  extern float camX, camZ;
  glm::mat4 view(1.0f); // make sure to initialize matrix to identity matrix first
  // float radius = 10.0f;

  // extern glm::vec3 cameraPos;
  // extern glm::vec3 cameraFront;
  // extern glm::vec3 cameraUp;
  extern Camera my_camera;
  // view = glm::lookAt(glm::vec3(camX, 0.0f, camZ), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  // view = glm::lookAt(my_camera.Position, cameraPos + cameraFront, cameraUp);
  view = my_camera.GetViewMatrix();
  my_shader.setMat4("view", view);

  glm::mat4 model(1.0f);
  glm::mat4 projection(1.0f);

  // double timeValue = glfwGetTime();
  // float angle = static_cast<float>(sin(timeValue) / 2.0 + 0.5); // 0-1
  // model = glm::rotate(model, glm::radians(55.0f), glm::vec3(1.0f, 0.0f, 0.0f));

  projection = glm::perspective(glm::radians(45.0f), (float)(WIDTH / HEIGHT), 0.1f, 100.0f);

  my_shader.setMat4("model", model);
  my_shader.setMat4("projection", projection);

  my_shader.setVec3("objectColor", vec3(0.5f, 0.5f, 0.5f));
  my_shader.setVec3("lightColor", vec3(1.0f, 1.0f, 1.0f));
  extern glm::vec3 lightPos;
  // my_shader.setVec3("lightPos", lightPos);

  my_shader.setVec3("lightPos", my_camera.Position);

  my_shader.setVec3("viewPos", my_camera.Position);
  my_shader.setVec3("view_front", my_camera.Front);

  // attenuation
  my_shader.setFloat("k0", 1.0);
  my_shader.setFloat("k1", 0.05);
  my_shader.setFloat("k2", 0.009);

  // spot light
  my_shader.setFloat("cos_cut", glm::cos(glm::radians(5.0f)));
  my_shader.setFloat("cos_outtercut", glm::cos(glm::radians(10.0f)));

  /* drawing */
  glBindVertexArray(vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  // glDrawArrays(GL_TRIANGLES, 0, 6);
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}
