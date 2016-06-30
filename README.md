# Signals Sending Example

This is a simple project for demonstration of signals that I made during my
studies. The main process performs ```fork(2)``` and communicates with SIGUSR1
and SIGUSR2 with its child and vice versa. Each time a signal is received,
there is printed A-Z char to stdout. If you send SIGUSR2 printing starts from
the beginning. Obtained 10/10 points.

Use on your own risk!
