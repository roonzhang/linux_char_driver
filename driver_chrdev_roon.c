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
/* 头文件包含 ---------------------------------------------------------------- */
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
#define DEV_NUM_MAX           5

static int g_major = 0;
module_param(g_major, int, S_IRUGO);
static int g_minor = 0;

struct roon_dev {
  struct cdev cdev;
  unsigned char pool[POOL_LEN];
};

struct roon_dev * g_roon_devp;

/* 函数编写 ------------------------------------------------------------------ */
/**************************************************************************
* Function Name : int roon_open(struct inode *inode, struct file *filp)
* Description   : open
* Parameters    : struct inode *inode, struct file *file
*
* Return        : int
* Data	        : 2018 - 07 - 04
* By	        	: RoonZhang <www.roon.pro>
* Explain       : None
***************************************************************************/
static int roon_open(struct inode *inode, struct file *filp)
{
  try_module_get(THIS_MODULE);

  struct roon_dev *dev = container_of(inode->i_cdev, struct roon_dev, cdev);
  filp->private_data = dev;

  return 0;
}

/**************************************************************************
* Function Name : int roon_open(struct inode *inode, struct file *filp)
* Description   : open
* Parameters    : struct inode *inode, struct file *file
*
* Return        : int
* Data	        : 2018 - 07 - 04
* By	        	: RoonZhang <www.roon.pro>
* Explain       : None
***************************************************************************/
static int roon_release(struct inode *inode, struct file *file)
{
	module_put(THIS_MODULE);

	return 0;
}

/**************************************************************************
* Function Name : int roon_open(struct inode *inode, struct file *filp)
* Description   : open
* Parameters    : struct inode *inode, struct file *file
*
* Return        : int
* Data	        : 2018 - 07 - 04
* By	        	: RoonZhang <www.roon.pro>
* Explain       : None
***************************************************************************/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
static int roon_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
#else
static int roon_ioctl(struct inode *inode, struct file *filp, unsigned int cmd,\
   unsigned long arg)
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

/**************************************************************************
* Function Name : ssize_t roon_read(struct file *filp, char __user *buf,
*                       size_t count, loff_t *ppos)
* Description   : read
* Parameters    : struct file *filp, char __user *buf, size_t count, loff_t *ppos
*
* Return        : int
* Data	        : 2018 - 07 - 04
* By	        	: RoonZhang <www.roon.pro>
* Explain       : None
***************************************************************************/
static ssize_t roon_read(struct file *filp, char __user *buf, size_t count, \
  loff_t *ppos)
{
  int ret = 0;
  unsigned long pos = *ppos;
  unsigned int cnt = count;

  struct roon_dev *dev = filp->private_data;

  if (pos >= POOL_LEN)
    return 0;
  if (cnt > POOL_LEN - pos)
    cnt = POOL_LEN - pos;

  ret = copy_to_user(buf, dev->pool + pos, cnt);

  if (ret == 0) {
    *ppos += cnt;
    ret = cnt;
    printk(KERN_INFO, "You have been read %u bytes from %lu\n", cnt, pos);
  } else {
    return (-EFAULT);
  }

  return (ret);
}

/**************************************************************************
* Function Name : ssize_t roon_read(struct file *filp, char __user *buf, size_t
*                     count, loff_t *ppos)
* Description   : read
* Parameters    : struct file *filp, char __user *buf, size_t count, loff_t *ppos
*
* Return        : int
* Data	        : 2018 - 07 - 04
* By	        	: RoonZhang <www.roon.pro>
* Explain       : None
***************************************************************************/
static ssize_t roon_write(struct file *filp, char __user *buf, size_t count, \
  loff_t *ppos)
{
  int ret = 0;
  unsigned long pos = *ppos;
  unsigned int cnt = count;

  struct roon_dev *dev = filp->private_data;

  if (pos >= POOL_LEN)
    return 0;
  if (cnt > POOL_LEN - pos)
    cnt = POOL_LEN - pos;

  ret = copy_from_user(dev->pool + pos, buf, cnt);

  if (ret == 0) {
    *ppos += cnt;
    ret = cnt;
    printk(KERN_INFO, "You have been write %u bytes from %lu\n", cnt, pos);
  } else {
    return (-EFAULT);
  }

  return (ret);
}

/**************************************************************************
* Function Name : loff_t roon_llseek(struct file *filp, loff_t offset, int orig)
* Description   : lseek
* Parameters    : struct file *filp, loff_t offset, int orig
*
* Return        : int
* Data	        : 2018 - 07 - 04
* By	        	: RoonZhang <www.roon.pro>
* Explain       : None
***************************************************************************/
static loff_t roon_llseek(struct file *filp, loff_t offset, int orig)
{
  loff_t ret = 0;

  switch (orig) {
    case 0: /* SEEK_SET */
      if ((offset < 0) || ((unsigned int)offset > POOL_LEN)) {
        ret = -EINVAL;
        break;
      }

      flip->f_pos = (unsigned int)offset;
      ret = flip->f_pos;
      break;

    case 1: /* SEEK_CUR */
      if ((flip->f_pos + offset > POOL_LEN) || (flip->f_pos + offset < 0)) {
        ret = -EINVAL;
        break;
      }

      flip->f_pos += offset;
      ret = flip->f_pos;
      break;

    case 2: /* SEEK_END */
      if ((offset > 0) || ((unsigned int)offset > POOL_LEN)) {
        ret = -EINVAL;
        break;
      }

      flip->f_pos = POOL_LEN + offset;
      ret = flip->f_pos;
      break;

    default:
      ret = -EINVAL;
      break;
  }

  return (ret);
}

/* 操作接口安装结构体 --------------------------------------------------------- */
struct file_operations roon_fops =
{
	.owner 				= THIS_MODULE,
	.open  				= roon_open,
	.release 			= roon_release,

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
	.unlocked_ioctl 	= roon_ioctl,
#else
	.ioctl				= roon_ioctl,
#endif

  .llseek       = roon_llseek,
  .read         = roon_read,
  .write        = roon_write,
};

/**************************************************************************
* Function Name : loff_t roon_llseek(struct file *filp, loff_t offset, int orig)
* Description   : lseek
* Parameters    : struct file *filp, loff_t offset, int orig
*
* Return        : int
* Data	        : 2018 - 07 - 04
* By	        	: RoonZhang <www.roon.pro>
* Explain       : None
***************************************************************************/
static int roon_setup_chrdev(struct roon_dev *dev, int index)
{
  int ret = 0;
  int devno = MKDEV(g_major, index);

  /* 分配 字符设备 结构 */
	&(dev->cdev) = cdev_alloc();
  if (&(dev->cdev) != NULL)
	{
		/* 初始化字符设备结构 */
		cdev_init(&(dev->cdev), &roon_fops);
		dev->cdev.owner = THIS_MODULE;
		/* 增加到系统中 */
		if (cdev_add(&(dev->cdev), devno, 1) != 0)
		{
			printk(KERN_ERR "cdev_add error!\n");
			return (-1);
		}
	}
	else
	{
		printk(KERN_ERR "cdev_alloc error!\n");
		return (-1);
	}

  return 0;
}

/**************************************************************************
* Function Name : int __init roon_init(void)
* Description   : init
* Parameters    : void
*
* Return        : int
* Data	        : 2018 - 07 - 04
* By	        	: RoonZhang <www.roon.pro>
* Explain       : None
***************************************************************************/
static int __init roon_init(void)
{
  int ret = 0, i = 0;
  int devno = MKDEV(g_major, 0);

  if (g_major)
    ret = register_chrdev_region(devno, DEV_NUM_MAX, "roon");
  else {
    ret = alloc_chrdev_region(&devno, 0, DEV_NUM_MAX, "roon");
    g_major = MAJOR(devno);
  }

  if (ret < 0)
    return ret;

  g_roon_devp = kzalloc(sizeof(struct roon_dev)*DEV_NUM_MAX, GFP_KERNEL);
  if (!g_roon_devp) {
    ret = -ENOMEM;
    unregister_chrdev_region(devno, DEV_NUM_MAX);
    return ret;
  }

  for (i = 0; i < DEV_NUM_MAX; i++)
    roon_setup_chrdev(g_roon_devp + i, i);

  return 0;
}

/**************************************************************************
* Function Name : void __exit roon_exit()
* Description   : exit
* Parameters    : void
*
* Return        : void
* Data	        : 2018 - 07 - 04
* By	        	: RoonZhang <www.roon.pro>
* Explain       : None
***************************************************************************/
static void __exit roon_exit()
{
  int i = 0;

  /* 移除字符设备 */
  for (i = 0; i < DEV_NUM_MAX; i++)
    cdev_del(&((g_roon_devp + i)->cdev));

  kfree(g_roon_devp);

	/* 释放设备号 */
	unregister_chrdev_region(MKDEV(g_major, 0), DEV_NUM_MAX);
}

module_init(roon_init);
module_exit(roon_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("roon@roon.pro");

/* ********************************* (c) Roon ** End File ************************************ */
