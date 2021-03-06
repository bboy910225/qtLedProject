/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "masterthread.h"

#include <QSerialPort>
#include <QTime>
#include <QDebug>
#include <QPointer>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStringList>
#include <QSqlError>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QTimer>
#include <QFile>
#include <QMessageBox>

MasterThread::MasterThread(QObject *parent) :
    QThread(parent)
{
}

//! [0]
MasterThread::~MasterThread()
{
    m_mutex.lock();
    m_quit = true;
    m_cond.wakeOne();
    m_mutex.unlock();
    wait();
}
//! [0]

//! [1] //! [2]
void MasterThread::transaction(const QString &portName, int waitTimeout, const QString &request)
{


//! [1]

//        if(arryWord[i]=="") continue;
    const QMutexLocker locker(&m_mutex);
    m_portName = portName;
    m_waitTimeout = waitTimeout;
//! [3]
    if (!isRunning()){
        qDebug() <<"run";
        m_quit = true;
        start();
    }
    else
        m_cond.wakeOne();
    while (!m_quit) {}
        qDebug() <<"Timer over";
}
//! [2] //! [3]

//! [4]
void MasterThread::run()
{
    qDebug() <<"runIn";
    bool currentPortNameChanged = false;

    m_mutex.lock();
//! [4] //! [5]
    QString currentPortName;
    if (currentPortName != m_portName) {
        currentPortName = m_portName;
        currentPortNameChanged = true;
    }

    int currentWaitTimeout = m_waitTimeout;
    QString currentRequest = m_request;
    m_mutex.unlock();
//! [5] //! [6]
    QSerialPort serial;

    if (currentPortName.isEmpty()) {
        emit error(tr("No port name specified"));
        return;
    }
    while (m_quit) {
//![6] //! [7]
        if (currentPortNameChanged) {
            serial.close();
            serial.setPortName(currentPortName);

            //設定SerialPort相關參數
            serial.setBaudRate(19200);
            serial.setDataBits(QSerialPort::Data8);
            serial.setStopBits(QSerialPort::OneStop);
            serial.setParity(QSerialPort::NoParity);
            serial.setFlowControl(QSerialPort::NoFlowControl);
            //-------------------------

            if (!serial.open(QIODevice::ReadWrite)) {
                emit error(tr("Can't open %1, error code %2")
                           .arg(m_portName).arg(serial.errorString()));
                return;
            }
        }
//! [7] //! [8]

            QList <QString> arryWord= ReadDatabase();
//            qDebug() << arryWord[1];
            currentRequest = arryWord[0];
            QByteArray requestData;
            int index=0;
            int j=0;
            QChar str;
            ushort uni;
            QVector<char> strword;
            //setting and push setting preformat to the result
            char Cdata[] = {"\x79\x77\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x03\x03\x00\x00\xA7\x41\x00\xA5\x4C\x00\xB6\xFD\x00\xA8\xEC\x00\xA9\xB3\x00\xA6\x62\x00\xAC\xDD\x00\xA4\x54\x00\xA4\x70\xFF\x98\x79"};
            char mydata[] = {"\x00\x77\x41\x00\xA5\x4C\x00\xB6\xFD\x00\xA8\xEC\x00\xA9\xB3\x00\xA6\x62\x00\xAC\xDD\x00\xA4\x54\x00\xA4\x70\xFF"};
            char defalt[] = {"\x79\x77\x00\x00\x00\x00\x01"};
            char remind[] = {"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"};
            char wordType[] = {"\x00\x02\x00\x00\x03\x03\x00"};
            int size ;
//            char result[0];
            QVector<char> result;
            QVector<QVector<char>> total;
            total.resize(arryWord.length());
            char aaa[55];
            int Rsize ;
            char check;
            for(int w=0;w < arryWord.length();w++){
                currentRequest = arryWord[w];
                // write request
                requestData = currentRequest.toLocal8Bit();
                //pregive word length
                strword.resize(3*currentRequest.length());
                qDebug() <<currentRequest.toLocal8Bit().length() + currentRequest.length();
                index=0;
                j=0;
                //deal word format
                for(int i=0;i<currentRequest.length();i++){
                    str = currentRequest.at(i);
                    uni = str.unicode();
                    if(uni >= 0x4E00 && uni <= 0x9FA5){
                        qDebug() <<"Chinese";
                        strword[j]='\x00';
                        j++;
                        strword[j]=requestData[index];
                        j++;
                        index++;
                        strword[j]=requestData[index];
                        j++;
                        index++;

                    }else{
                        strword[j]='\x00';
                        j++;
                        strword[j]='\x00';
                        j++;
                        if(requestData[index] =='\x79'){
                            strword[j]='\x78';
                            j++;
                            strword[j]='\x01';
                            j++;
                            index++;
                        }else if(requestData[index] =='\x78'){

                            strword[j]='\x78';
                            j++;
                            strword[j]='\x02';
                            j++;
                            index++;
                        }else{
                            strword[j]=requestData[index];
                            j++;
                            index++;
                        }
                    }

                }
                qDebug() <<strword;
                 size = strword.size() +sizeof(defalt) +sizeof(remind) +sizeof(wordType) ;
                 result.resize(size);
                 Rsize = 0;

                //setting send mode and format
                for(int i=0;i<sizeof (defalt)-1;i++){
                    result[Rsize] = defalt[i];
                    Rsize++;
                }

                //remind word format
                for(int i=0;i<sizeof (remind)-1;i++){
                    result[Rsize] =remind[i];
                    Rsize++;
                }

                // word type format
                for(int i=0;i<sizeof (wordType)-1;i++){
                    result[Rsize] =wordType[i];
                    Rsize++;
                }

                //data of word format
                for(int i=0;i< strword.size();i++){
                    result[Rsize] =strword[i];
                    Rsize++;
                }

                // end of word format
                 result[Rsize] ='\xFF';
                 Rsize++;

                 // calculate check code and push in result
                 check='\x00';
                for(int i=1;i<result.size()-2;i++){
                    check +=result[i];
                }

                //transfer the excpetion code
                if(check =='\x79')    result[Rsize]='\x78';
                else    result[Rsize]=check;
                result[Rsize+1]='\x79';
                qDebug() <<result.data();
                total[w] = result;

            }

//                QEventLoop loop;
//                QTimer::singleShot(10, &loop, SLOT(quit()));
//                loop.exec();
//                timer = new QTimer(this);
//                connect(timer, SIGNAL(timeout()), this, SLOT(Update(serial,total)));
//                timer->start(1000);
                  Update(serial,total);
//                QByteArray data = QByteArray::fromRawData(total[p].data(), total[p].size());
//                qDebug() <<p;
//                //write
//                serial.write(data);

//        if (serial.waitForBytesWritten(m_waitTimeout)) {
            if (true) {
//! [8] //! [10]
            // read response
//            if (serial.waitForReadyRead(currentWaitTimeout)) {
               if (true) {
                QByteArray responseData = serial.readAll();
                while (serial.waitForReadyRead(10))
                    responseData += serial.readAll();
                const QString response = QString::fromUtf8(responseData);
//! [12]
                emit this->response(response);
//! [10] //! [11] //! [12]
            } else {
                emit timeout(tr("Wait read response timeout %1").arg(QTime::currentTime().toString()));
            }
//! [9] //! [11]
        } else {
            emit timeout(tr("Wait write request timeout %1")
                         .arg(QTime::currentTime().toString()));
        }
//! [9]  //! [13]
        m_mutex.lock();
        m_cond.wait(&m_mutex);
        if (currentPortName != m_portName) {
            currentPortName = m_portName;
            currentPortNameChanged = true;
        } else {
            currentPortNameChanged = false;
        }
        currentWaitTimeout = m_waitTimeout;
        currentRequest = m_request;
        m_mutex.unlock();
    }
    qDebug() << "break";
    m_quit =false;

//! [13]
}

 void MasterThread::Delay_MSec(unsigned int msec)
{
    QEventLoop loop;//defined a loop event
    QTimer::singleShot(msec, &loop, SLOT(quit()));//create a timer
    loop.exec();//event will stop here until time over
}
 QList<QString> MasterThread::ReadDatabase(){
     QString fileName = "C:/Users/HorseKing/Downloads/leddisp/Log.txt";
     QString DS002;
     QList<QString> arryWord;
     QFile file(fileName);
          if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
          {
             qDebug()<<"can't open";

          }
         QTextStream in(&file);
         DS002 = in.readLine();//讀取一行
         qDebug()<< "readText" + DS002;
         //To get support interface
         qDebug() << "Available drivers:";
         QStringList drivers = QSqlDatabase::drivers();
         foreach(QString driver, drivers)
         qDebug() << "\t" << driver;

         QSqlDatabase _db = QSqlDatabase::addDatabase("QODBC","dbTemp");
         qDebug() << "ODBC driver valid?" << _db.isValid();

 //      Start to connect SQL Server
     //set QODBC and Name
     QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", "SQLServer");
     //connect localhost
     db.setHostName("127.0.0.1");
//     QString dsn = QString::fromLocal8Bit("DRIVER={SQL SERVER};SERVER=172.17.67.201;DATABASE=MMDB;uid=chihsuan;pwd=064724806064");
     db.setPort(1433);
   QString dsn = QString::fromLocal8Bit("DRIVER={SQL SERVER};SERVER=127.0.0.1;DATABASE=master;uid=HorseKing;pwd=Bboy910225");
     //Use already seted dsn
 //  QString dsn = QString::fromLocal8Bit("SQLServer");
     db.setDatabaseName(dsn);
     //setUserName and password
 //    db.setUserName("HorseKing");
 //    db.setPassword("Bboy910225");
     if(!db.open()) {
       qDebug() <<(db.lastError().databaseText());

     }else{
         qDebug()<<"database open success!";
         QSqlQuery query(db);
 //        where DS001 = '" + DS002.split(".")[1] +"' and DS003 = 'Y'"
         bool success = query.exec("SELECT DS002 FROM MMDB.dbo.SFTMDS" );
         if(!success){
             qDebug() <<"failed to connect database";
         }
         while(query.next())
         {
             QStringList str = query.value(0).toString().split(";");
 //            int length = query.value(0).toString().length();
 //            int index = 0;
             for(int i=0;i<str.length();i++){
                 qDebug()<<"number :"<<str[i];
                 arryWord.append( str[i]);
             }
             query.exec("update MMDB.dbo.SFTMDS set DS003 = 'N' where DS001 = '" + DS002.split(".")[1] +"' and DS003 ='Y' ");
 //            arryWord[index] = str;
 //            length -= str.length()+1;
 //            qDebug() << length;
 //            index +=1;
 //            strLength =index;
 //            qDebug()<<query.value(0).toString() <<query.value(1).toString() <<query.value(2).toString();
         }
     }
     return arryWord ;
 }
  void MasterThread::Update(QSerialPort &serial, QVector<QVector<char>> total ){
      QByteArray data = QByteArray::fromRawData(total[0].data(), total[0].size());
      serial.write(data);
      QByteArray responseData = serial.readAll();
      while (serial.waitForReadyRead(10))
          responseData += serial.readAll();
      QString response = QString::fromUtf8(responseData);
//! [12]
      emit this->response(response);
      qDebug() <<"trigger";
      sleep(10);
      data = QByteArray::fromRawData(total[1].data(), total[1].size());
      serial.write(data);
      responseData = serial.readAll();
      while (serial.waitForReadyRead(10))
          responseData += serial.readAll();
      response = QString::fromUtf8(responseData);
      if(!loop) {
          Thread_stop();
          return ;
      }
      qDebug() <<"trigger2";
      sleep(10);
      data = QByteArray::fromRawData(total[2].data(), total[2].size());
      serial.write(data);
      responseData = serial.readAll();
      while (serial.waitForReadyRead(10))
          responseData += serial.readAll();
      response = QString::fromUtf8(responseData);
      if(!loop) {
          Thread_stop();
          return ;
      }
      qDebug() <<"trigger3";
      if(!loop) {
          Thread_stop();
          return ;
      }
      else  {
          sleep(10);
          Update(serial, total );
      }
  }
  void MasterThread::Thread_stop(){
      emit this->stoped(true);
  }

