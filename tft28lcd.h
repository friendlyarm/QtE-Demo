#ifndef TFT28LCD_H
#define TFT28LCD_H

#include <QObject>
#include <QProcess>
#include <QThread>
#include <QMutex>
#include <QEvent>
#include <QFile>
#include <QTextStream>

class TFT28LCDKeyEvent : public QEvent
{
public:
    enum {
        KEY1 = 1,
        KEY2 = 2,
        KEY3 = 3
    };
    TFT28LCDKeyEvent ()
        : QEvent((QEvent::Type)TFT28LCDKeyEvent::TFT28LCDKEY_EVENT_TYPE)
    {}
    int key;
    static int TFT28LCDKEY_EVENT_TYPE;
};

class TFT28LCDThread : public QThread
{
    Q_OBJECT
public:
    explicit TFT28LCDThread(QObject* obj, QObject *parent = 0);
    ~TFT28LCDThread();
    void run();
private:
    int tft28lcd_main();

public slots:
    void start(Priority = InheritPriority);
private:
    QObject* evenRecvObj;
};


#endif // TFT28LCD_H
