/*
 ***********************************************************************
 *
 *        @version  1.0
 *        @date     03/07/2014 08:29:22 PM
 *        @author   Fridolin Pokorny <fridex.devel@gmail.com>
 *
 ***********************************************************************
 */

#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#endif

#ifndef NDEBUG
#define NDEBUG
#endif

#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>

#define UNUSED(X)			((void) X)

#define MSG_PROMPT		"Press enter..."
#define MSG_PARENT		"Parent (%d): '%c'\n"
#define MSG_CHILD			"Child (%d): '%c'\n"

/**
 * @brief  child pid if nonzero
 */
static int pid = 0;

/**
 * @brief  character to be printed
 */
volatile sig_atomic_t g_printchar = 'A';
/**
 * @brief  was SIGUSR1 received?
 */
volatile sig_atomic_t sig = 0;

/**
 * @brief  was SIGINT received?
 */
volatile sig_atomic_t end = 0;

/**
 * @brief  SIGUSR2 handler
 *
 * @param signum signal number
 */
void sigusr2_handler(int signum) {
	UNUSED(signum);
	g_printchar = 'A';
}

/**
 * @brief  SIGUSR1 handler
 *
 * @param signum signal number
 */
void sigusr1_handler(int signum) {
	UNUSED(signum);
	sig = 1;
}

/**
 * @brief  SIGINT handler
 *
 * @param signum signal number
 */
void sigint_handler(int signum) {
	UNUSED(signum);
	if (pid != 0) {
		kill(pid, SIGINT);
		waitpid(pid, NULL, 0);
	}
	/*
	 * can be replaced by _exit() which is async safe
	 */
	end = 1;
}

/**
 * @brief  Print perror() and exit
 *
 * @param str error messsage to be printed
 *
 * @return never return
 */
static inline
int perror_return(const char * str) {
		perror(str);
		exit(EXIT_FAILURE);
		return EXIT_FAILURE;
}

/**
 * @brief  Print printed char
 *
 * @param str formating string
 *
 * @return   number chars written
 */
static inline
int print(const char * str) {
	int res;
	res = printf(str, getpid(), g_printchar);

	if (g_printchar != 'Z')
		g_printchar++;

	return res;
}

/**
 * @brief  Entry point
 *
 * @return   EXIT_SUCCESS on success, otherwise EXIT_FAILURE
 */
int main(void) {
	int c;
	struct sigaction sa;
	sigset_t set;

	/*
	 * SIGUSR1 should be blocked
	 */
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	sigprocmask(SIG_BLOCK, &set, NULL);

	sa.sa_flags = 0;

	/*
	 * Set up handlers
	 */
	sa.sa_handler = &sigusr1_handler;
	sigemptyset(&sa.sa_mask);
	if (sigaction(SIGUSR1, &sa, NULL) == -1) return perror_return("sigaction()");

	sa.sa_handler = &sigusr2_handler;
	if (sigaction(SIGUSR2, &sa, NULL) == -1) return perror_return("sigaction()");

	sa.sa_handler = &sigint_handler;
	if (sigaction(SIGINT, &sa, NULL) == -1) return perror_return("sigaction()");

	/*
	 * Do the hard work...
	 */
	if ((pid = fork()) < 0) {
		return perror_return("fork()");
	} else if (pid == 0) {
		/*
		 * child
		 */
		for (;;) {
			while (! sig && ! end && sigsuspend(&sa.sa_mask) == -1 && errno == EINTR)
				;	/* wait for signal SIGUSR1 or SIGINT */
			if (end) {
				/* notify parent about termination */
				kill(getppid(), SIGINT);
				return EXIT_SUCCESS;
			}
			print(MSG_CHILD);
			sig = 0;
			kill(getppid(), SIGUSR1);
		}
	} else {
		/*
		 * parent
		 */
		for (;;) {
			print(MSG_PARENT);
			kill(pid, SIGUSR1);
			while (! sig && ! end && sigsuspend(&sa.sa_mask) == -1 && errno == EINTR)
				;	/* wait for signal SIGUSR1 or SIGINT */
			if (end) return EXIT_SUCCESS;
			sig = 0;
			puts(MSG_PROMPT);
			/*
			 * wait for enter
			 * getchar() returns when SIGUSR2 was receaived, rerun it
			 */
			while (((c = getchar()) != '\n') && ! end)
				;
			if (end) return EXIT_SUCCESS;
		}
	}

	assert(! "Unreachable");
	return EXIT_SUCCESS;
}

