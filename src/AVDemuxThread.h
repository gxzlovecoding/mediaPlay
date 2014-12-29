/******************************************************************************
    QtAV:  Media play library based on Qt and FFmpeg
    Copyright (C) 2012-2013 Wang Bin <wbsecg1@gmail.com>

*   This file is part of QtAV

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
******************************************************************************/

#ifndef QAV_DEMUXTHREAD_H
#define QAV_DEMUXTHREAD_H

#include <QtAV\QtAV_Global.h>
#include <QtCore/QAtomicInt>
#include <QtCore/QMutex>
#include <QtCore/QThread>
#include <QtCore/QQueue>
#include <QtCore/QRunnable>
#include <QMap>
#include <utils/BlockingQueue.h>

namespace QtAV {

class AVDemuxer;
class AVThread;
class AVDemuxThread : public QThread
{
    Q_OBJECT
public:
    explicit AVDemuxThread(QObject *parent = 0);
    explicit AVDemuxThread(AVDemuxer *dmx, QObject *parent = 0);
    void setDemuxer(AVDemuxer *dmx);
	void setAudioThread(AVThread *thread, int stream_id = -1, int index = 0);
	AVThread* audioThread(int index = 0);
    void setVideoThread(AVThread *thread, int stream_id = -1, int index = 0);
	AVThread* videoThread(int index = 0);
    void seek(qint64 pos); //ms
    //AVDemuxer* demuxer
    bool isPaused() const;
    bool isEnd() const;

	void setPreLoad(bool flag);
	bool programEnable[MAX_PROGRAM];
public slots:
    void stop(); //TODO: remove it?
    void pause(bool p);
    void nextFrame(); // show next video frame and pause

Q_SIGNALS:
    void requestClockPause(bool value);
	void onPreLoadSuccess();

private slots:
	void frameDeliveredSeekOnPause(AVThread*);
	void frameDeliveredNextFrame(AVThread*);
	void onFirstFrameDelivered(AVThread*);

protected:
    virtual void run();
    /*
     * If the pause state is true setted by pause(true), then block the thread and wait for pause state changed, i.e. pause(false)
     * and return true. Otherwise, return false immediatly.
     */
    bool tryPause(unsigned long timeout = 100);

private:
    void setAVThread(AVThread *&pOld, AVThread* pNew);
    void newSeekRequest(QRunnable *r);
    void processNextSeekTask();
    void seekInternal(qint64 pos); //must call in AVDemuxThread
    void pauseInternal(bool value);
    void processNextPauseTask();

    bool paused;
    bool user_paused;
    volatile bool end;
    AVDemuxer *demuxer;
	AVThread *audio_thread[MAX_PROGRAM], *video_thread[MAX_PROGRAM];
    int audio_stream, video_stream;
    QMutex buffer_mutex;
    QWaitCondition cond;
    BlockingQueue<QRunnable*> seek_tasks;
    // if seeking on pause, schedule a skip pause task and a pause task
    QQueue<QRunnable*> pause_tasks; // in thread tasks
	QMap<int, int> audioStreamId_ProgramIndex;
	QMap<int, int> videoStreamId_ProgramIndex;

    QAtomicInt nb_next_frame;
    friend class SeekTask;

	bool m_isPreLoad;
};

} //namespace QtAV
#endif // QAV_DEMUXTHREAD_H
