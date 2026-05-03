#include "fs_utils.h"
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <coreinit/filesystem.h>

#define FS_MAX_MOUNTPATH_SIZE           128

int MountFS(void *pClient, void *pCmd, char **mount_path)
{
    int result = -1;

    void *mountSrc = malloc(sizeof(FSMountSource));
    if(!mountSrc)
        return -3;

    char* mountPath = (char*) malloc(FS_MAX_MOUNTPATH_SIZE);
    if(!mountPath) {
        free(mountSrc);
        return -4;
    }

    memset(mountSrc, 0, sizeof(FSMountSource));
    memset(mountPath, 0, FS_MAX_MOUNTPATH_SIZE);

    // Mount sdcard
    if (FSGetMountSource(pClient, pCmd, FS_MOUNT_SOURCE_SD, mountSrc, -1) == 0)
    {
        result = FSMount(pClient, pCmd, mountSrc, mountPath, FS_MAX_MOUNTPATH_SIZE, -1);
        if((result == 0) && mount_path) {
            *mount_path = (char*)malloc(strlen(mountPath) + 1);
            if(*mount_path)
                strcpy(*mount_path, mountPath);
        }
    }

    free(mountPath);
    free(mountSrc);
    return result;
}

int UmountFS(void *pClient, void *pCmd, const char *mountPath)
{
    int result = -1;
    result = FSUnmount(pClient, pCmd, mountPath, -1);

    return result;
}

int LoadFileToMem(const char *filepath, u8 **inbuffer, u32 *size)
{
    //! always initialze input
	*inbuffer = NULL;
    if(size)
        *size = 0;

    int iFd = open(filepath, O_RDONLY);
	if (iFd < 0)
		return -1;

	u32 filesize = lseek(iFd, 0, SEEK_END);
    lseek(iFd, 0, SEEK_SET);

	u8 *buffer = (u8 *) malloc(filesize);
	if (buffer == NULL)
	{
        close(iFd);
		return -2;
	}

    u32 blocksize = 0x4000;
    u32 done = 0;
    int readBytes = 0;

	while(done < filesize)
    {
        if(done + blocksize > filesize) {
            blocksize = filesize - done;
        }
        readBytes = read(iFd, buffer + done, blocksize);
        if(readBytes <= 0)
            break;
        done += readBytes;
    }

    close(iFd);

	if (done != filesize)
	{
		free(buffer);
		return -3;
	}

	*inbuffer = buffer;

    //! sign is optional input
    if(size)
        *size = filesize;

	return filesize;
}

int CheckFile(const char * filepath)
{
	if(!filepath)
		return 0;

	struct stat filestat;

	char dirnoslash[strlen(filepath)+2];
	snprintf(dirnoslash, sizeof(dirnoslash), "%s", filepath);

	while(dirnoslash[strlen(dirnoslash)-1] == '/')
		dirnoslash[strlen(dirnoslash)-1] = '\0';

	char * notRoot = strrchr(dirnoslash, '/');
	if(!notRoot)
	{
		strcat(dirnoslash, "/");
	}

	if (stat(dirnoslash, &filestat) == 0)
		return 1;

	return 0;
}

int CreateSubfolder(const char * fullpath)
{
	if(!fullpath)
		return 0;

	int result = 0;

	char dirnoslash[strlen(fullpath)+1];
	strcpy(dirnoslash, fullpath);

	int pos = strlen(dirnoslash)-1;
	while(dirnoslash[pos] == '/')
	{
		dirnoslash[pos] = '\0';
		pos--;
	}

	if(CheckFile(dirnoslash))
	{
		return 1;
	}
	else
	{
		char parentpath[strlen(dirnoslash)+2];
		strcpy(parentpath, dirnoslash);
		char * ptr = strrchr(parentpath, '/');

		if(!ptr)
		{
			//!Device root directory (must be with '/')
			strcat(parentpath, "/");
			struct stat filestat;
			if (stat(parentpath, &filestat) == 0)
				return 1;

			return 0;
		}

		ptr++;
		ptr[0] = '\0';

		result = CreateSubfolder(parentpath);
	}

	if(!result)
		return 0;

	if (mkdir(dirnoslash, 0777) == -1)
	{
		return 0;
	}

	return 1;
}

int RemoveDirectory(const char *path)
{
	DIR *d = opendir(path);
	size_t path_len = strlen(path);
	int r = -1;

	if (d)
	{
		struct dirent *p;
		r = 0;
		while (!r && (p = readdir(d)))
		{
			int r2 = -1;
			char *buf;
			size_t len;

			/* Skip the names "." and ".." as we don't want to recurse on them. */
			if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
				continue;

			len = path_len + strlen(p->d_name) + 2;
			buf = (char *) malloc(len);

			if (buf)
			{
				struct stat statbuf;
				snprintf(buf, len, "%s/%s", path, p->d_name);
				if (!stat(buf, &statbuf))
				{
					if (S_ISDIR(statbuf.st_mode))
						r2 = RemoveDirectory(buf);
					else
						r2 = unlink(buf);
				}
				free(buf);
			}
			r = r2;
		}
		closedir(d);
	}

	if (!r)
		r = rmdir(path);

	return r;
}
