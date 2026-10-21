### Running 

# when compiling can compile without MPI or with. without MPI is useful for testing on other computers
make 
# or 
make MPI=1

# to run, use 
mpiexec -np 4 -machinefile paprika_exec -x DISPLAY=:0.0 ./program
# if you get a glfw 65550 error, glfw cant find displays. to fix run 
export DISPLAY=:0

# this can currently only run on x86 machines (paprika), not arm pis
### Ai
Deepseek used for some code generation refactor and debug
### Documentation 

The purpose of this code is to use GLFW, C++, OpenGL to run displays in parallel with MPI on the clusters. 
We would want to use these libraries to maximize performance, and to utilize GPUs.


The project has 3 folders,
  include # external libraries  
  shaders # shaders run on gpu 
  src     # cpp and header files 

The c++ code is organized into sections to separate responsibility
  main            # where the program is run
                  
  scene           # drawing logic goes into scene. the shaders fullscreen.frag and fullscreen.vert 
                  # are just designed to 
                  # draw a texture to the screen. what color each pixel is in that texture 
                  # is determined by the functions in the scene file 
                  # these functions are called by RenderCallback scene_render in main. 
                  # to draw your project, you should only have to create a function in scene, 
                  # and change which function is called in scene_render in main
  
  cluster         # all MPI calls are in cluster, and accessed from main through an include 
                  # this way the program can run without mpi, with each cluster function failing 
                  # safely when MPI not used 

  renderer        # renderer is all opengl logic, creating pixel buffer object, creating texture, 
                  # sending to gpu and running shaders 

 
  window_manager  # all glfw window logic for managing windows. change this file when changing 
                  # keyboard inputs or window characteristics, like hidden cursor or hidden header 
 
  shader          # shader helper for reading and compiling shaders
  

