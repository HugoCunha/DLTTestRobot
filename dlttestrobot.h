/**
 * @licence app begin@
 * Copyright (C) 2021 Alexander Wenzel
 *
 * This file is part of the DLT Multimeter project.
 *
 * \copyright This code is licensed under GPLv3.
 *
 * \author Alexander Wenzel <alex@eli2.de>
 *
 * \file dltcan.h
 * @licence end@
 */

#ifndef DLT_TES_ROBOT_H
#define DLT_TES_ROBOT_H

#include <QObject>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QSerialPort>
#include <QTimer>

class DLTTestRobot : public QObject
{
    Q_OBJECT
public:
    explicit DLTTestRobot(QObject *parent = nullptr);
    ~DLTTestRobot();

    void start();
    void stop();

    void clearSettings();
    void writeSettings(QXmlStreamWriter &xml);
    void readSettings(const QString &filename);

signals:

    void status(QString text);

private slots:

private:

    QTimer timer;
    unsigned int watchDogCounter,watchDogCounterLast;

};

#endif // DLT_TES_ROBOT_H
