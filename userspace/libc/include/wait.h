#ifndef wait_h___
#define wait_h___

#define WEXITED 4

//pid typedefs
#ifndef PID_T_DEFINED
#define PID_T_DEFINED
typedef int pid_t;
#endif // PID_T_DEFINED

/**
 * posix function signature
 * do not change the signature!
 */
extern pid_t waitpid(pid_t pid, int *status, int options);



#endif // wait_h___


