
#include "./ensureFolder.h"

#include "./statUtils.h"
#include "./pathUtils.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>


int ensureFolder(const char* inFolderPath)
{
  if (Stat__pathExist(inFolderPath) == 1) {
    return 0; // current folder exist -> skip
  }

  // current folder does not exist -> check the parent folder
  char* parentFolder = Path__dirname(inFolderPath);

  const int parentResult = ensureFolder(parentFolder);

  free(parentFolder);

  if (parentResult != 0) {
    return parentResult;
  }

  const int currResult = mkdir(inFolderPath, 0777); // full permissions

  if (currResult == 0) {
    // printf("Directory created successfully\n");
  } else {
    perror("Error creating directory");
  }

  return currResult;
}

// int _ensureFolder(const char* inFolderPath)
// {
//   printf("ensure folder %s\n", inFolderPath);
//   if (Stat__pathExist(inFolderPath) == 1) {
//     printf("Directory already exists.\n");
//     return 0;
//   }

//   const int result = mkdir(inFolderPath, 0777); // full permissions

//   if (result == 0) {
//     printf("Directory created successfully\n");
//   } else {
//     perror("Error creating directory");

//     // switch (errno)
//     // {
//     // case	EPERM:  printf("EPERM -> Operation not permitted\n"); break;
//     // case	ENOENT:  printf("ENOENT -> No such file or directory\n"); break;
//     // case	ESRCH:  printf("ESRCH -> No such process\n"); break;
//     // case	EINTR:  printf("EINTR -> Interrupted system call\n"); break;
//     // case	EIO:  printf("EIO -> I/O error\n"); break;
//     // case	ENXIO:  printf("ENXIO -> No such device or address\n"); break;
//     // case	E2BIG:  printf("E2BIG -> Argument list too long\n"); break;
//     // case	ENOEXEC:  printf("ENOEXEC -> Exec format error\n"); break;
//     // case	EBADF:  printf("EBADF -> Bad file number\n"); break;
//     // case	ECHILD:  printf("ECHILD -> No child processes\n"); break;
//     // case	EAGAIN:  printf("EAGAIN -> Try again\n"); break;
//     // case	ENOMEM:  printf("ENOMEM -> Out of memory\n"); break;
//     // case	EACCES:  printf("EACCES -> Permission denied\n"); break;
//     // case	EFAULT:  printf("EFAULT -> Bad address\n"); break;
//     // case	ENOTBLK:  printf("ENOTBLK -> Block device required\n"); break;
//     // case	EBUSY:  printf("EBUSY -> Device or resource busy\n"); break;
//     // case	EEXIST:  printf("EEXIST -> File exists\n"); break;
//     // case	EXDEV:  printf("EXDEV -> Cross-device link\n"); break;
//     // case	ENODEV:  printf("ENODEV -> No such device\n"); break;
//     // case	ENOTDIR:  printf("ENOTDIR -> Not a directory\n"); break;
//     // case	EISDIR:  printf("EISDIR -> Is a directory\n"); break;
//     // case	EINVAL:  printf("EINVAL -> Invalid argument\n"); break;
//     // case	ENFILE:  printf("ENFILE -> File table overflow\n"); break;
//     // case	EMFILE:  printf("EMFILE -> Too many open files\n"); break;
//     // case	ENOTTY:  printf("ENOTTY -> Not a typewriter\n"); break;
//     // case	ETXTBSY:  printf("ETXTBSY -> Text file busy\n"); break;
//     // case	EFBIG:  printf("EFBIG -> File too large\n"); break;
//     // case	ENOSPC:  printf("ENOSPC -> No space left on device\n"); break;
//     // case	ESPIPE:  printf("ESPIPE -> Illegal seek\n"); break;
//     // case	EROFS:  printf("EROFS -> Read-only file system\n"); break;
//     // case	EMLINK:  printf("EMLINK -> Too many links\n"); break;
//     // case	EPIPE:  printf("EPIPE -> Broken pipe\n"); break;
//     // case	EDOM:  printf("EDOM -> Math argument out of domain of func\n"); break;
//     // case	ERANGE:  printf("ERANGE -> Math result not representable\n"); break;
//     // default:
//     //   break;
//     // }

//     // EACCES

//     // switch (errno)
//     // {
//     // case constant expression:
//     //   /* code */
//     //   break;
//     // }
//       //  EACCES The parent directory does not allow write permission to the
//       //         process, or one of the directories in path did not allow
//       //         search permission.  (See also path_resolution(7).)

//       //  EBADF  (mkdirat()) path is relative but dirfd is neither AT_FDCWD
//       //         nor a valid file descriptor.

//       //  EDQUOT The user's quota of disk blocks or inodes on the filesystem
//       //         has been exhausted.

//       //  EEXIST path already exists (not necessarily as a directory).  This
//       //         includes the case where path is a symbolic link, dangling
//       //         or not.

//       //  EFAULT path points outside your accessible address space.

//       //  EINVAL The final component ("basename") of the new directory's
//       //         path is invalid (e.g., it contains characters not permitted
//       //         by the underlying filesystem).

//       //  ELOOP  Too many symbolic links were encountered in resolving path.

//       //  EMLINK The number of links to the parent directory would exceed
//       //         LINK_MAX.

//       //  ENAMETOOLONG
//       //         path was too long.

//       //  ENOENT A directory component in path does not exist or is a
//       //         dangling symbolic link.

//       //  ENOMEM Insufficient kernel memory was available.

//       //  ENOSPC The device containing path has no room for the new
//       //         directory.

//       //  ENOSPC The new directory cannot be created because the user's disk
//       //         quota is exhausted.

//       //  ENOTDIR
//       //         A component used as a directory in path is not, in fact, a
//       //         directory.

//       //  ENOTDIR
//       //         (mkdirat()) path is relative and dirfd is a file descriptor
//       //         referring to a file other than a directory.

//       //  EPERM  The filesystem containing path does not support the
//       //         creation of directories.

//       //  EROFS  path refers to a file on a read-only filesystem.

//       //  EOVERFLOW
//       //         UID or GID mappings (see user_namespaces(7)) have not been
//       //         configured.


//   }

//   return result;
// }

