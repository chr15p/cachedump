/*  
 *  cachedump.c - A simple kernel module to display the contents of the page cache
 */

#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/init.h>		/* Needed for the macros */
#include <linux/proc_fs.h>	/** This is for procfs **/
#include <linux/fs.h> 
#include <linux/seq_file.h>
#include <linux/sched.h>
#include <linux/fs_struct.h>
#include <linux/dcache.h>
#include <linux/slab.h>
#include <linux/radix-tree.h>
#include <linux/page-flags.h>



void showChildren(char *path, struct seq_file *m, struct dentry *rootdentry)
{
	struct dentry *child;
	char *mypath;
	struct radix_tree_iter iter;
	void **slot;

	list_for_each_entry(child, &(rootdentry)->d_subdirs, d_u.d_child) {
		unsigned long length;
		unsigned long active = 0;
		unsigned long inactive = 0;

		if (child->d_inode == NULL)
			continue;


		seq_printf(m, "%-12lu", child->d_inode->i_ino);
		if (child->d_inode->i_mapping != NULL) {
			seq_printf(m, "%-12lu", child->d_inode->i_mapping->nrpages);
		} else {  
			seq_printf(m, "%-12d", 0);
		}

		radix_tree_for_each_slot(slot, &(child->d_inode->i_mapping->page_tree), &iter, 0) {
			if (PageActive((struct page *)slot)) {
				active++;
			} else {
				inactive++;
			}
		}
		seq_printf(m, "%-12lu%-12lu", active, inactive);


		length = strlen(path) + strlen(child->d_name.name)+2;
		mypath = (char*) kmalloc(length, GFP_KERNEL);
		memset(mypath, 0, length);

		strcpy(mypath, path);

		////add trailing slash if needed
		if (*(path+strlen(path)-1) != '/') {
			strcat(mypath, "/");
		}
		strcat(mypath, child->d_name.name);

		seq_printf(m, "%s\n", mypath);

		showChildren(mypath, m, child);
		kfree(mypath);
	}
}


static int basicProcShow(struct seq_file *m, void *v)
{
	struct path root;
	struct dentry *rootdentry;


	get_fs_root(current->fs, &root);
	rootdentry = root.dentry;

	showChildren("/", m, rootdentry);

	return 0;
}


static int basicProcOpen(struct inode *inode, struct file *file)
{
	return single_open(file, basicProcShow, (void*)inode);
}


static const struct file_operations basic_proc_fops =
{
.owner = THIS_MODULE,
.open = basicProcOpen,
.read = seq_read,
.llseek = seq_lseek,
.release = single_release,
};


static int __init proc_init(void)
{
	struct proc_dir_entry *procfile;

	procfile = proc_create("cachedump", S_IRUGO, NULL, &basic_proc_fops);

	return 0;
}

static void __exit proc_exit(void)
{
	remove_proc_entry("cachedump", NULL);
}



module_init(proc_init);
module_exit(proc_exit);
MODULE_LICENSE("GPL");
