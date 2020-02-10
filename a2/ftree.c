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

	static int tree_depth = 0;

    // if file name doesn't exist
    if (fname == NULL) {
    	fprintf(stderr, "The path (%s) does not point to an existing entry!\n", fname);
    	return NULL;
    }

    // create new tree, where the root of the tree is the current file
    struct TreeNode *fileSystem = malloc(sizeof(struct TreeNode));

    // check to see if malloc call failed
    if (fileSystem == NULL) {
    	fprintf(stderr, "Fatal: failed to allocate bytes");
    	exit(1);
    }

    // initialize struct stat and then read in info about the current file 
    // to this struct stat using lstat
    struct stat stat_buf;

    // error check lstat
    if (lstat(fname, &stat_buf) == -1) {
	    fprintf(stderr, "lstat error");       
	    return NULL;   
    }

    // set permissions attribute for this node
    int permissions = stat_buf.st_mode & 0777;
    fileSystem->permissions = permissions;
     
    // set contents and next to null intitially (values may change
    // if the file is a directory)
    fileSystem->contents = NULL;
    fileSystem->next = NULL;

    // get name of file (extract from fname argument)
    if (tree_depth == 0) { 
    	fileSystem->fname = malloc(sizeof(char) * strlen(fname) + 1);
    	if (fileSystem->fname == NULL) {
            fprintf(stderr, "Fatal: failed to allocate bytes");
            exit(1);
        }
    	strcpy(fileSystem->fname, fname); 
    }
    else {
    	int len_to_slash = 0;
    	int count = strlen(fname) - 1;
    	char *p = &fname[count];
    	while ((*p != '/') && (count > -1)) {
    		len_to_slash++;
    		p--;
    		count--;
    	}

    	char file_name[1024];
    	int index = strlen(fname) - 1;
    	int q = len_to_slash - 1;

    	while (q > -1) {
    		file_name[q] = fname[index];
    		index--;
    		q--;
    	}

    	file_name[len_to_slash] = '\0';
    	fileSystem->fname = malloc(sizeof(char) * strlen(file_name) + 1);
    	if (fileSystem->fname == NULL) {
            fprintf(stderr, "Fatal: failed to allocate bytes");
            exit(1);
        }
        strcpy(fileSystem->fname, file_name);
    }

    // if regular file, set type to -
    if (S_ISREG(stat_buf.st_mode)) { 
    	fileSystem->type = '-'; 
    }

    // if link, set tope to l
    else if (S_ISLNK(stat_buf.st_mode)) { 
    	fileSystem->type = 'l';
    }

    // else, the file is a directory
    else {

    	tree_depth++;

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
   	
   	// make sure node not null
	if (node != NULL) {

	    if (node->next != NULL) {
	    	deallocate_ftree(node->next);
	    }

		if (node->contents == NULL) {
			// names must be deallocated first
			free(node->fname);
			free(node);
		} else {
			deallocate_ftree(node->contents);
			// names must be deallocated first
			free(node->fname);
			free(node);
		}
	}

}  

