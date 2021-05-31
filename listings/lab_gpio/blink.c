/* Source:
 *   https://wiki.st.com/stm32mpu/wiki/How_to_control_a_GPIO_in_userspace
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <linux/gpio.h>

int main(int argc, char **argv)
{
	struct gpiohandle_request req;
	struct gpiohandle_data data;
	char chrdev_name[20];
	int fd, ret;

	strcpy(chrdev_name, "/dev/gpiochip0");  /* <--- TODO - change me! */

	fd = open(chrdev_name, 0);
	if (fd == -1) {
		ret = -errno;
		fprintf(stderr, "Failed to open %s\n", chrdev_name);

		return ret;
	}

	req.lineoffsets[0] = 14; /* <--- TODO - change me! */
	req.flags = GPIOHANDLE_REQUEST_OUTPUT;
	memcpy(req.default_values, &data, sizeof(req.default_values));
	strcpy(req.consumer_label, "");
	req.lines  = 1;

	ret = ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &req);
	if (ret == -1) {
		ret = -errno;
		fprintf(stderr, "Failed to issue GET LINEHANDLE IOCTL (%d)\n",
			ret);
	}
	if (close(fd) == -1)
		perror("Failed to close GPIO character device file");

	/*  Start led blinking */
	while(1) {

		data.values[0] = !data.values[0];
		ret = ioctl(req.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
		if (ret == -1) {
			ret = -errno;
			fprintf(stderr, "Failed to issue %s (%d)\n",
					"GPIOHANDLE_SET_LINE_VALUES_IOCTL", ret);
		}
		sleep(1);
	}

	/*  release line */
	ret = close(req.fd);
	if (ret == -1) {
		perror("Failed to close GPIO LINEHANDLE device file");
		ret = -errno;
	}
	return ret;
}
