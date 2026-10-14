# to run, use 
mpiexec -n 4 -pernode -machinefile machine_exec -x DISPLAY=:0.0 ./program
# need -x DISPLAY=:0.0
# this can only run on x86 machines, not arm pis

