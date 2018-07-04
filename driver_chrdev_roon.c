/* ************************************ (c) Roon ** Start File ******************************************* */
/*************************************************************************************
* File Name     : driver_chrdev_roon.c
* Description   : 字符驱动模板
* Data	        : 2018 - 07 - 04
* By            : RoonZhang <www.roon.pro>
* Email         : roon@roon.pro
* Platform      : Linux 2.6.35.3 for i.mx.283
* Explain       : None
* Modify        : None
************************************************************************************ */
/* 头文件包含 --------------------------------------------------------------------- */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/version.h>

#define ROON_IOC_MAGIC		    'L'
#define ROON_IOCTL_MAXNR		  2
#define ROON_CLR				      _IO(ROON_IOC_MAGIC, 0)
#define POOL_LEN              0x2800      /* 10K */

static int g_major = 0;
module_param(g_major, int, S_IRUGO);
static int g_minor = 0;

struct roon_dev {
  struct cdev;
  unsigned char pool[POOL_LEN];
};

struct roon_dev * g_roon_devp;

/* 函数编写 ----------------------------------------------------------------------- */
/******************************************************************************
* Function Name : int roon_open(struct inode *inode, struct file *filp)
* Description   : open
* Parameters    : struct inode *inode, struct file *file
*
* Return        : int
* Data	        : 2018 - 07 - 04
* By	        	: RoonZhang <www.roon.pro>
* Explain       : None
*******************************************************************************/
static int roon_open(struct inode *inode, struct file *filp)
{
  try_module_get(THIS_MODULE);
  filp->private_data = g_roon_devp;

  return 0;
}

/******************************************************************************
* Function Name : int roon_open(struct inode *inode, struct file *filp)
* Description   : open
* Parameters    : struct inode *inode, struct file *file
*
* Return        : int
* Data	        : 2018 - 07 - 04
* By	        	: RoonZhang <www.roon.pro>
* Explain       : None
*******************************************************************************/
static int roon_release(struct inode *inode, struct file *file)
{
	module_put(THIS_MODULE);

	return 0;
}

/******************************************************************************
* Function Name : int roon_open(struct inode *inode, struct file *filp)
* Description   : open
* Parameters    : struct inode *inode, struct file *file
*
* Return        : int
* Data	        : 2018 - 07 - 04
* By	        	: RoonZhang <www.roon.pro>
* Explain       : None
*******************************************************************************/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
static int roon_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
#else
static int roon_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
#endif
{
  struct roon_dev *dev = filp->private_data;

	if (_IOC_TYPE(cmd) != ROON_IOC_MAGIC)
	{
		return (-ENOTTY);
	}

	if (_IOC_NR(cmd) > ROON_IOCTL_MAXNR)
	{
		return (-ENOTTY);
	}

	switch (cmd)
	{
		case (ROON_CLR):
			memset(dev->pool, 0, POOL_LEN);
      printk("KERN_INFO roon's pool is cleared.\n");
			break;

		default:
			break;
	}

	return 0;
}


/* ********************************* (c) Roon ** End File ************************************ */
