#include "tft28lcd.h"
#include <QtGui>
#include <QApplication>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <dirent.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#define MAX_EVENTS     (10)

/* sys/class/gpio */
static int gpio_d0=2, gpio_d1=3, gpio_d2=200;
static int epfd=-1;
static int fd_d0=-1, fd_d1=-1, fd_d2=-1;

static void release_gpio(int gpio)
{
    FILE* fp = fopen("/sys/class/gpio/unexport", "w");
    if (fp) {
        fprintf(fp, "%d\n", gpio);
        fclose(fp);
    }
}

static void release_tft_gpio(void)
{
    if (fd_d0 >= 0) {
        close(fd_d0);
        release_gpio(gpio_d0);
    }
    if (fd_d1 >= 0) {
        close(fd_d1);
        release_gpio(gpio_d1);
    }
    if (fd_d2 >= 0) {
        close(fd_d2);
        release_gpio(gpio_d2);
    }
}

static int init_gpio(int gpio, const char *direction, const char* edge)
{
    char path[42];
    FILE *fp;
    int fd;

    // export gpio to userspace
    fp = fopen("/sys/class/gpio/export", "w");
    if (fp) {
        fprintf(fp, "%d\n", gpio);
        fclose(fp);
    }

    // set output direction
    sprintf(path, "/sys/class/gpio/gpio%d/direction", gpio);
    fp = fopen(path, "w");
    if (fp) {
        fprintf(fp, "%s\n", direction);
        fclose(fp);
    }

    // falling edge
    sprintf(path, "/sys/class/gpio/gpio%d/edge", gpio);
    fp = fopen(path, "w");
    if (fp) {
        fprintf(fp, "%s\n", edge);
        fclose(fp);
    }

    sprintf(path, "/sys/class/gpio/gpio%d/value", gpio);
    fd = open(path, O_RDWR | O_NONBLOCK);
    if (fd < 0) {
        printf("open of gpio %d returned %d: %s\n",
                gpio, fd, strerror(errno));
    }

    return fd;
}

static void release_tft28lcd()
{
    if (epfd >= 0) {
        close(epfd);
    }
    release_tft_gpio();
}

int TFT28LCDKeyEvent::TFT28LCDKEY_EVENT_TYPE = QEvent::User + 100;
TFT28LCDThread::TFT28LCDThread(QObject* obj, QObject *parent) :
    QThread(parent), evenRecvObj(obj) {
}
TFT28LCDThread::~TFT28LCDThread() {
}

void TFT28LCDThread::start(Priority p) {
    QThread::start(p);
}

int TFT28LCDThread::tft28lcd_main()
{
    struct epoll_event ev_d0, ev_d1, ev_d2;
    struct epoll_event evlist[MAX_EVENTS];
    int i, ret;
    char ch;

    epfd = epoll_create(1);
    if (epfd < 0) {
        qDebug("error creating epoll");
        return -1;
    }

    fd_d0 = init_gpio(gpio_d0, "in", "falling");
    if (fd_d0 < 0) {
        qDebug("error opening gpio sysfs entries");
        return -1;
    }

    fd_d1 = init_gpio(gpio_d1, "in", "falling");
    if (fd_d1  < 0) {
        qDebug("error opening gpio sysfs entries");
        return -1;
    }

    fd_d2 = init_gpio(gpio_d2, "in", "falling");
    if (fd_d2 < 0) {
        qDebug("error opening gpio sysfs entries");
        return -1;
    }

    ev_d0.events = EPOLLET;
    ev_d1.events = EPOLLET;
    ev_d2.events = EPOLLET;
    ev_d0.data.fd = fd_d0;
    ev_d1.data.fd = fd_d1;
    ev_d2.data.fd = fd_d2;

    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd_d0, &ev_d0);
    if (ret != 0) {
        qDebug("epoll_ctl returned %d: %s\n", ret, strerror(errno));
        return -1;
    }

    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd_d1, &ev_d1);
    if (ret != 0) {
        qDebug("epoll_ctl returned %d: %s", ret, strerror(errno));
        return -1;
    }

    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd_d2, &ev_d2);
    if (ret != 0) {
        qDebug("epoll_ctl returned %d: %s", ret, strerror(errno));
        return -1;
    }

    while (1) {
        ret = epoll_wait(epfd, evlist, MAX_EVENTS, -1);
        if (ret  == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                qDebug("epoll_wait returned %d: %s", ret, strerror(errno));
            }
        }

        for (i=0; i<ret; ++i) {
            if (evlist[i].data.fd == ev_d0.data.fd) {
                lseek(fd_d0, 0, SEEK_SET);
                if (read(fd_d0, &ch, 1)>0) {
                    qDebug("k1 event: %c", ch);
                    TFT28LCDKeyEvent *e = new TFT28LCDKeyEvent();
                    e->key = TFT28LCDKeyEvent::KEY1;
                    QApplication::postEvent(evenRecvObj, e);
                }
            } else if (evlist[i].data.fd == ev_d1.data.fd) {
                lseek(fd_d1, 0, SEEK_SET);
                if (read(fd_d1, &ch, 1)>0) {
                    qDebug("k2 event: %c", ch);
                    TFT28LCDKeyEvent *e = new TFT28LCDKeyEvent();
                    e->key = TFT28LCDKeyEvent::KEY2;
                    QApplication::postEvent(evenRecvObj, e);
                }
            } else if (evlist[i].data.fd == ev_d2.data.fd) {
                lseek(fd_d2, 0, SEEK_SET);
                if (read(fd_d2, &ch, 1)>0) {
                    qDebug("k3 event: %c", ch);
                    TFT28LCDKeyEvent *e = new TFT28LCDKeyEvent();
                    e->key = TFT28LCDKeyEvent::KEY3;
                    QApplication::postEvent(evenRecvObj, e);
                }
            }
        }
    }
    return 0;
}

void TFT28LCDThread::run() {
    QThread::msleep(500);
    tft28lcd_main();
    release_tft28lcd();
}

