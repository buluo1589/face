#include <python3.10/Python.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <poll.h>
#include <pthread.h>

char gpio_path[100];
char file_path[100];
char buf[1];
int Is;
int door_not_open;
int IsPerson;
int fd;
int ret;
int len;
struct pollfd fds[1];

pthread_t thread1, thread2, thread3;

PyObject *pModule;
PyObject *pFunction;
PyObject *pRetValue;

void *gpio_interrupt(void *arg)
{
    while (1)
    {
        const char *value = "value";
        sprintf(file_path, "%s/%s", gpio_path, value);
        fd = open(file_path, O_RDONLY);
        if (fd < 0)
        {
            printf("open error\n");
        }
        fds[0].fd = fd;
        fds[0].events = POLLPRI;
        read(fd, buf, 1);
        ret = poll(fds, 1, -1);
        if (ret <= 0)
        {
            printf("poll error\n");
        }
        if (fds[0].revents & POLLPRI)
        {
            lseek(fd, 0, SEEK_SET);
            read(fd, buf, 1);
            if (buf[0] == '1')
            {
                IsPerson = 1;
                printf("人来了\n");
            }
            else if (buf[0] == '0')
            {
                sleep(1);
                lseek(fd, 0, SEEK_SET);
                read(fd, buf, 1);
                if (buf[0] == '0')
                {
                    sleep(1);
                    lseek(fd, 0, SEEK_SET);
                    read(fd, buf, 1);
                    if (buf[0] == '0')
                    {
                        IsPerson = 0;
                        printf("人走了\n");
                    }
                }
            }
        }
    }
}

int gpio_export(char *gpio_num)
{
    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd < 0)
    {
        perror("open()");
        return -1;
    }
    len = strlen(gpio_num);
    ret = write(fd, gpio_num, len);
    if (ret < 0)
    {
        perror("1write()");
        return -2;
    }

    close(fd);
    return 0;
}

int gpio_unexport(char *gpio_num)
{
    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd < 0)
    {
        perror("open()");
        return -3;
    }
    len = strlen(gpio_num);
    ret = write(fd, gpio_num, len);
    if (ret < 0)
    {
        perror("2write()");
        return -4;
    }

    close(fd);
    return 0;
}

int gpio_ctrl(const char *arg, char *value)
{
    sprintf(file_path, "%s/%s", gpio_path, arg);
    fd = open(file_path, O_WRONLY);
    if (fd < -1)
    {
        perror("open()");
        return -3;
    }
    len = strlen(value);
    ret = write(fd, value, len);
    if (ret < 0)
    {
        perror("3write()");
        return -4;
    }

    close(fd);
}

void *cam(void *arg)
{
    while (1)
    {
        if (IsPerson == 1)
        {
            pFunction = PyObject_GetAttrString(pModule, "match");
            if (!pFunction)
            {
                printf("get python function failed!!!\n");
            }
            pRetValue = PyObject_CallObject(pFunction, NULL);
            PyArg_Parse(pRetValue, "i", &Is);
            printf("%d\n", Is);
        }
    }
}

void *open_the_door(void *arg)
{
    if(Is==1&&door_not_open==1)
    {

    }
}

int main()
{
    char *gpio = "120";
    char key;
    sprintf(gpio_path, "/sys/class/gpio/gpio%s", gpio);
    if (access(gpio_path, F_OK))
    {
        gpio_export(gpio);
    }
    else
    {
        gpio_unexport(gpio);
    }
    gpio_ctrl("direction", "in");
    gpio_ctrl("edge", "both"); // 触发方式

    char *s = NULL;
    Is = 0;
    // 第一步：初始化python环境
    Py_Initialize();

    PyRun_SimpleString("import os,sys"); // 执行import语句，把当前路径加入路径中，为了找到math_test.py
    PyRun_SimpleString("sys.path.append('./')");
    // 第二步：调用face.py脚本
    pModule = PyImport_ImportModule("face");
    if (!pModule)
    {
        printf("import python failed1!!\n");
        return -1;
    }

    pthread_create(&thread1, NULL, gpio_interrupt, NULL);
    pthread_create(&thread2, NULL, cam, NULL);
    pthread_create(&thread3, NULL, open_the_door, NULL);

    while (1)
    {
        key = getchar();
        if (key == 'q' || key == 'Q')
            break;
    }

    gpio_unexport(gpio);

    Py_DECREF(pModule);
    Py_DECREF(pFunction);
    Py_DECREF(pRetValue);

    Py_Finalize();
}

// g++ -o face face.c -lpython3.10