#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>

// Add your system includes here.
#include <sys/types.h>
#include <sys/stat.h>

#include "ftree.h"


/*
 * Returns the FTree rooted at the path fname.
 *
 * Use the following if the file fname doesn't exist and return NULL:
 * fprintf(stderr, "The path (%s) does not point to an existing entry!\n", fname);
 *
 */
struct TreeNode *generate_ftree(const char *fname) {

    // Your implementation here.

    // Hint: consider implementing a recursive helper function that
    // takes fname and a path.  For the initial call on the 
    // helper function, the path would be "", since fname is the root
    // of the FTree.  For files at other depths, the path would be the
    // file path from the root to that file.

    return NULL;
}


/*
 * Prints the TreeNodes encountered on a preorder traversal of an FTree.
 *
 * The only print statements that you may use in this function are:
 * printf("===== %s (%c%o) =====\n", root->fname, root->type, root->permissions)
 * printf("%s (%c%o)\n", root->fname, root->type, root->permissions)
 *
 */
void print_ftree(struct TreeNode *root) {
	
    // Here's a trick for remembering what depth (in the tree) you're at
    // and printing 2 * that many spaces at the beginning of the line.
    static int depth = 0;
    printf("%*s", depth * 2, "");

    // Your implementation here.
}


/* 
 * Deallocate all dynamically-allocated memory in the FTree rooted at node.
 * 
 */
void deallocate_ftree (struct TreeNode *node) {
   
   // Your implementation here.

}  

/* 
 * Calls lstat on a single file and prints all of the information that you need to store in a TreeNode
 * 
 */

int printInfo (const char *path) {

    struct stat stat_buf;
    
	if (lstat(path, &stat_buf) == -1) {
	        perror("lstat");        
	        return 1;    
    }

    // print file name


    // check type and print
    if (S_ISREG(stat_buf.st_mode)) {
          printf("%s is a regular file\n", path);    
    }

    else if (S_ISDIR(stat_buf.st_mode)) {
          printf("%s is a directory\n", path);    
    }

    else {
    	printf("%s is a link\n", path); 
    }


    // extract permissions and print
    int permissions = stat_buf.st_mode & 0777;
    printf("the file permissions are %o\n", permissions);

    return 0;

}

 /* 
 *  Checks if a file is a directory and, if it is, uses opendir and readdir to print the names of all the files in the directory
 * 
 */

int isDir (const char *path) {
    
    struct stat stat_buf;
    if (lstat(path, &stat_buf) == -1) {
	        perror("lstat");        
	        return 1;    
    }

    // check if the path is a directory
    if (S_ISDIR(stat_buf.st_mode)) {
        DIR *d_ptr = opendir(path);

        if (d_ptr == NULL) {
        	perror("opendir");
        	return 1;
        }
        
        // stores info about a directory entry
        struct dirent *entry_ptr;

        // call readdir to get struct dirent, which represents a directory entry
        entry_ptr = readdir(d_ptr);
        printf("Directory %s contains:\n", path);    
        while (entry_ptr != NULL) {
            printf("%s\n", entry_ptr->d_name);
            entry_ptr = readdir(d_ptr);    
        }

        int error = closedir(d_ptr);
        if (error != 0) {
        	perror("closedir");
        	return 1;
        }

    }

    return 0;

}
