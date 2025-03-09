# UnixToolkit
Basic unix toolkit to execute a few commands from the user.

Project - Create a unix toolkit with some custom commands as well as the normal unix commands.

Method - I have all of my tool functions located in a singular C program. I have each of them set up as their own functions in the code. I did this to simplify the process of creating 
child processes to run my functions. I take in user input split it into different parameters and send those parameters to the appropriate functions. 

Executing - Using the make command compiles my program creates an executable file called mytoolkit. Running this starts the program and allows the user to input commands.
