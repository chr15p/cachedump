/*  
 *  hello-1.c - The simplest kernel module.
 */

#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/init.h>		/* Needed for the macros */
//#include <linux/sched.h>
//#include <linux/sched/signal.h>
#include <linux/proc_fs.h>    /** This is for procfs **/
#include <linux/fs.h> 
#include <linux/seq_file.h>
#include <linux/sched.h>
#include <linux/nsproxy.h>

#include <linux/mnt_namespace.h>
//#include <linux/mount.h>
//#include <linux/fs.h>
#include <linux/fs_struct.h>
#include <linux/dcache.h>
#include <linux/slab.h>



//static int pagetypeinfo_open(struct inode *inode, struct file *file)
//{
//       return seq_open(file, &pagetypeinfo_op);
//}

void showChildren(char* path,struct seq_file *m,struct dentry *rootdentry)
{
	struct dentry *child;
	char * mypath;

	list_for_each_entry(child, &(rootdentry)->d_subdirs, d_u.d_child) {
       		if(child->d_inode != NULL){
        		seq_printf(m,"%-12lu", child->d_inode->i_ino);
                        if(child->d_inode->i_mapping!=NULL){
       	                        seq_printf(m,"%-12lu",child->d_inode->i_mapping->nrpages);
               	        }else{  
                       	        seq_printf(m,"0           ");
                        }


			mypath =(char*) kmalloc(strlen(path)+strlen(child->d_iname)+2,GFP_KERNEL);
			memset(mypath,0,strlen(path)+strlen(child->d_iname)+2);
			strcpy(mypath,path);
			////add trailing slash if needed
			if(*(path+strlen(path)-1)!='/'){
				strcat(mypath,"/");
			}
			strcat(mypath,child->d_iname);
			
        		seq_printf(m,"%s\n",mypath);

			showChildren(mypath,m,child);
			kfree(mypath);
		}
	}
}


static int basicProcShow(struct seq_file *m, void *v)
{
	struct path root;
	struct dentry *rootdentry;


	get_fs_root(current->fs, &root);
	rootdentry = root.dentry;

	showChildren("/",m,rootdentry);

	return 0;
}


static int basicProcOpen(struct inode *inode, struct file *file)
{
	return single_open(file,basicProcShow,(void*)inode);
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


	printk(KERN_INFO "Hello, procmod\n");

	procfile=proc_create("chrisp", S_IRUGO, NULL,&basic_proc_fops);

	return 0;
}

static void __exit proc_exit(void)
{
	remove_proc_entry("chrisp",NULL);
	printk(KERN_INFO "Goodbye procmod\n");
}



module_init(proc_init);
module_exit(proc_exit);
MODULE_LICENSE("GPL");