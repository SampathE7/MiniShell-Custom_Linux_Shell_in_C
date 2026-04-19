# MiniShell-Custom_Linux_Shell_in_C
A lightweight Unix-like shell implementation in C that replicates core functionalities of a Linux terminal, including process creation, signal handling, job control, and command execution.
This project is built to deeply understand how the OS interacts with user programs using system calls like fork(), execvp(), waitpid(), kill(), and signal mechanisms.

🚀Features
Command Execution:
-Supports execution of external commands using fork() + execvp()
-Parent process synchronizes using waitpid()
-Handles invalid commands gracefully

Built-in Commands:
-cd – Change directory (handles multiple spaces)
-pwd – Print working directory
-echo – Supports:
    -$$ → Shell PID
    -$? → Exit status of last command
    -$SHELL → Shell path
-exit – Terminates the shell

Job Control:
-Ctrl + Z → Stops foreground process (SIGTSTP)
-jobs → Displays all stopped processes
-fg → Brings last stopped job to foreground
-bg → Resumes job in background
-->Internally uses a linked list to track process states

Signal Handling:
-SIGINT (Ctrl + C) → Terminates foreground process
-SIGTSTP (Ctrl + Z) → Suspends process
-SIGCHLD → Cleans up zombie processes

Pipe Support:
-Supports multiple pipelines:   ls | grep txt | wc -l

Prompt Customization:
-Default prompt: minishell:
-Change using:  PS1=NEW_PROMPT


Key Learnings:
-Process lifecycle and state transitions
-Foreground vs background execution
-Signal-driven process control
-Inter-process communication using pipes
-Memory management in system-level programs
