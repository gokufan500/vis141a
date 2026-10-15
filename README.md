# when compiling can compile without MPI or with. without MPI is useful for testing on other computers
make 
# or 
make MPI=1

# to run, use 
mpiexec -np 4 -machinefile paprika_exec ./program
# if you get a glfw 65550 error, glfw cant find displays. to fix run 
export DISPLAY=:0

# this can currently only run on x86 machines (paprika), not arm pis

### Documentation 

Structure: 


