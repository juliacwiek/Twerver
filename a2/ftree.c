#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

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

    // create new tree, root is the current file
    struct TreeNode *fileSystem = malloc(sizeof(struct TreeNode));

    // set name of the current file to this node's fname
    fileSystem->fname = malloc(sizeof(char) * strlen(fname) + 1);
    strcpy(fileSystem->fname, fname);

    // initialize struct stat and then read in info about the current file 
    // to this struct stat using lstat
    struct stat stat_buf;

    if (lstat(fname, &stat_buf) == -1) {
	        perror("lstat");        
	        //return 1;    
    }

    // set permissions attribute for this node
    int permissions = stat_buf.st_mode & 0777;
    fileSystem->permissions = permissions;
     
    // set contents and next to null intitially (values may change
    // if the file is a directory)
    fileSystem->contents = NULL;
    fileSystem->next = NULL;


    // find type of file and set as attribute
    if (S_ISREG(stat_buf.st_mode)) { fileSystem->type = '-'; }

    else if (S_ISLNK(stat_buf.st_mode)) { fileSystem->type = 'l'; }

    // else, the file is a directory
    else {

    	fileSystem->type = 'd';
    	DIR *d_ptr = opendir(fname);

        if (d_ptr == NULL) {
        	perror("opendir");
        	//return 1;
        }
 
        // stores info about a directory entry
        struct dirent *entry_ptr;

        // call readdir to get struct dirent, which represents a directory entry
        entry_ptr = readdir(d_ptr);

        // traverse directory, making recursive calls as you go
        int files_traversed = 1;
        struct dirent *curr_file = entry_ptr;
        while (curr_file != NULL) {
        	if ((strcmp(curr_file->d_name,".") == 0) && (strcmp(curr_file->d_name, "..") == 0)) {
        		struct TreeNode *curr_node = fileSystem->contents;
        		if (files_traversed == 1) {
        			fileSystem->contents = generate_ftree(curr_file->d_name);
        			curr_file = readdir(d_ptr);
        			files_traversed++;
        		} else {
        			struct TreeNode *prev_node = curr_node;
        			curr_file = readdir(d_ptr);
        			prev_node->next = generate_ftree(curr_file->d_name);
        		}
        		
        	}
              
        }

        // close directory and check for errors while closing
        int error = closedir(d_ptr);
        if (error != 0) {
        	perror("closedir");
        	//return 1;
        }


    }

    return fileSystem;
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

    if (root != NULL) {
    	// if its a directory
    	if (root->type == 'd') {
    		printf("===== %s (%c%o) =====\n", root->fname, root->type, root->permissions);
    		depth++;
    		if (root->contents != NULL) {
    			depth++;
    			print_ftree(root->contents);
    		}

    		if (root->next != NULL) {
    			print_ftree(root->next);
    		}

    	// if its a link or regular file
    	} else {
    		printf("%s (%c%o)\n", root->fname, root->type, root->permissions);
    		if (root->next != NULL) {
    			print_ftree(root->next);
    		}
    	}
    }
}


/* 
 * Deallocate all dynamically-allocated memory in the FTree rooted at node.
 * 
 */
void deallocate_ftree (struct TreeNode *node) {
   
   // Your implementation here.

	if (node != NULL) {
		if (node->type == 'd') {
			if (node->contents == NULL) {
				free(node);
			} else {
				deallocate_ftree(node->contents);
			}

		} else {
			if (node->next != NULL) {
				// deallocate_ftree((node->next)->fname);
				deallocate_ftree(node->next);
				free(node);
			} else {
				free(node);
			}
		}
	}

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
