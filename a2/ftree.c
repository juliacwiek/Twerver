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

    // create new tree, where the root of the tree is the current file
    struct TreeNode *fileSystem = malloc(sizeof(struct TreeNode));

    // check to see if malloc call failed
    if (fileSystem == NULL) {
    	fprintf(stderr, "Fatal: failed to allocate bytes");
    	exit(1);
    }

    // set name of the current file to this node's fname
    fileSystem->fname = malloc(sizeof(char) * strlen(fname) + 1);

    // check to see if malloc call failed
    if (fileSystem->fname == NULL) {
    	fprintf(stderr, "Fatal: failed to allocate bytes");
    	exit(1);
    }

    // make name of root of this tree be fname
    strcpy(fileSystem->fname, fname);

    // initialize struct stat and then read in info about the current file 
    // to this struct stat using lstat
    struct stat stat_buf;

    // error check lstat
    if (lstat(fname, &stat_buf) == -1) {
	    fprintf(stderr, "lstat error");       
	    exit(1);    
    }

    // set permissions attribute for this node
    int permissions = stat_buf.st_mode & 0777;
    fileSystem->permissions = permissions;
     
    // set contents and next to null intitially (values may change
    // if the file is a directory)
    fileSystem->contents = NULL;
    fileSystem->next = NULL;

    // find type of file and set type attribute:

    // regular file
    if (S_ISREG(stat_buf.st_mode)) { 
    	fileSystem->type = '-'; 
    }

    // link
    else if (S_ISLNK(stat_buf.st_mode)) { 
    	fileSystem->type = 'l';
    }

    // else, the file is a directory
    else {

    	fileSystem->type = 'd';
    	DIR *d_ptr = opendir(fname);

        if (d_ptr == NULL) {
        	fprintf(stderr, "opendir error");
        	exit(1);
        }
 
        // stores info about a directory entry
        struct dirent *curr_file;

        // traverse directory, making recursive calls as you go

        int files_traversed = 1;
        while ((curr_file = readdir(d_ptr)) != NULL) {
        	
        	// current file is not . or ..
        	if ((strcmp(curr_file->d_name,".") != 0) && (strcmp(curr_file->d_name, "..") != 0)) {
                
                // if first file in the directory (will be contents attribute of the directory)
        		if (files_traversed == 1) {

        			char path[1024];

        			// if file is in the current directory, path is file name
        			if (strcmp(fname, ".") == 0) { 
        				strcpy(path, curr_file->d_name); 
        				// set null pointer??

        			// otherwise, path is in format abc/def/filename
        			} else {
        				strcpy(path, fname);
        				strcat(path, "/");
        				strcat(path, curr_file->d_name);
        				// set null pointer???
        			}

        			fileSystem->contents = generate_ftree(path);
        			files_traversed++;
        			
                // else, not the first file in the directory
        		} else {
        		
                    struct TreeNode *prev_node;

                    if (fileSystem->contents != NULL) {
                    	struct TreeNode *curr_node = fileSystem->contents;
                    	while (curr_node->next != NULL) {
                    		curr_node = curr_node->next;
                    	}
                    	prev_node = curr_node;
                    } else { prev_node = NULL; }

        			char path[1024];

        			if (strcmp(fname, ".") == 0) { strcpy(path, curr_file->d_name); }

        			else { 
        				strcpy(path, fname);
        				strcat(path, "/");
        				strcat(path, curr_file->d_name);
        			} 

        			prev_node->next = generate_ftree(path);
        		}
        	}

        }

        // close directory and check for errors while closing
        int error = closedir(d_ptr);
        if (error != 0) {
        	fprintf(stderr, "closedir error");
        	exit(1);
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
    			print_ftree(root->contents);
    		}

            depth--;
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
				deallocate_ftree(node->next);
				free(node);
			} else {
				free(node);
			}
		}
	}

}  

