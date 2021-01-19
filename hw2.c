#include <linux/cdev.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/uaccess.h>

#define MAX_CMD_STR_LEN 64

#define AM335_GPIO(bank,line)	(32 * bank + line)
#define LED_GPIO		AM335_GPIO(1, 15)
#define BTN_GPIO		AM335_GPIO(0, 27)

static int irq;
static int led_on;

static int gpio_test_major = 0;
static struct cdev cdev;
static struct class *gpio_test_class;

ssize_t gpio_cdev_read(struct file *filp, char __user *buf, size_t count,
		       loff_t *f_pos)
{
	char out[MAX_CMD_STR_LEN];
	ssize_t len;
	int err;

	pr_info("++%s", __func__);

	len = sprintf(out, "%d", led_on);

	pr_info("%s\n", out);

	err = copy_to_user(buf, out, len + 1);
	if (err)
		return -EFAULT;

	return 0;
}

ssize_t gpio_cdev_write(struct file *filp, const char *buf, size_t count,
			loff_t *f_pos)
{
	char in_buf[MAX_CMD_STR_LEN + 1] = {0};
	int err = 0;
	int val = 0;

	pr_info("++%s", __func__);

	err = copy_from_user(in_buf, buf, min((size_t)MAX_CMD_STR_LEN, count));
	if (err)
		return -EFAULT;

	err = kstrtoint(in_buf, 10, &val);
	if (err) {
		pr_err("Cannot parse input");
		return -EINVAL;
	}

	led_on = (bool)val;
	gpio_set_value(LED_GPIO, led_on);

	pr_info("--%s", __func__);
	return count;
}

static const struct file_operations gpio_cdev_fops = {
	.read = gpio_cdev_read,
	.write = gpio_cdev_write,
};

static irqreturn_t hw2_btn_isr(int num, void *priv)
{
	led_on = !led_on;
	gpio_set_value(LED_GPIO, led_on);
	pr_info("interrupt!\n");
	return IRQ_HANDLED;
}

static int __init hw2_init(void)
{
	int err;
	dev_t dev = 0;
	int devnum = 0;

	pr_info("++%s", __func__);

	err = gpio_request(BTN_GPIO, "my_button");
	if (err) {
		pr_err("Unable to request button GPIO\n");
		return -EINVAL;
	}

	gpio_direction_input(BTN_GPIO);
	irq = gpio_to_irq(BTN_GPIO);
	err = request_threaded_irq(irq, NULL, hw2_btn_isr,
			           IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
				   "my button key", NULL);
	if (err) {
		pr_err("Unable to request interrupt for button\n");
		goto free_button;
	}

	err = gpio_request(LED_GPIO, "my_led");
	if (err) {
		pr_err("Unable to request LED GPIO\n");
		goto free_irq;
	}
	gpio_direction_output(LED_GPIO, led_on);

	err = alloc_chrdev_region(&dev, 0, 1, "gpio_test");
	gpio_test_major = MAJOR(dev);
	if (err < 0) {
		pr_err("Unable to allocate major number\n");
		goto free_irq;
	}

	devnum = MKDEV(gpio_test_major, 0);
	cdev_init(&cdev, &gpio_cdev_fops);
	err = cdev_add(&cdev, devnum, 1);
	if (err) {
		pr_err("Error %d adding scull", err);
		goto free_cdev;
	}

	gpio_test_class = class_create(THIS_MODULE, "gpio_test");
	if(IS_ERR(gpio_test_class)) {
		pr_err("Error creating gpio_test_class.\n");
		goto del_cdev;
	}

	device_create(gpio_test_class, NULL, MKDEV(gpio_test_major, 0), NULL,
		      "gpio_test1");

	return 0;
del_cdev:
	cdev_del(&cdev);
free_cdev:
	unregister_chrdev_region(dev, 0);
free_irq:
	free_irq(irq, NULL);
free_button:
	gpio_free(BTN_GPIO);
	return err;
}

static void __exit hw2_exit(void)
{
	pr_info("++%s", __func__);

	free_irq(irq, NULL);
	gpio_free(BTN_GPIO);
	gpio_free(LED_GPIO);

	device_destroy(gpio_test_class, MKDEV(gpio_test_major, 0));
	class_destroy(gpio_test_class);
	cdev_del(&cdev);
	unregister_chrdev_region(MKDEV(gpio_test_major, 0), 0);
}

module_init(hw2_init);
module_exit(hw2_exit);

MODULE_AUTHOR("Viaheslav Lykhohub <viacheslav.lykhohub@globallogic.com>");
MODULE_DESCRIPTION("Working with GPIO module");
MODULE_LICENSE("GPL");
