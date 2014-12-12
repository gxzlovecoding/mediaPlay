/******************************************************************************
    QtAV:  Media play library based on Qt and FFmpeg
    Copyright (C) 2012-2014 Wang Bin <wbsecg1@gmail.com>

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

#include "AVDemuxThread.h"
#include "QtAV/AVDemuxer.h"
#include "QtAV/AVDecoder.h"
#include "QtAV/Packet.h"
#include "AVThread.h"
#include <QtCore/QTimer>
#include <QtCore/QEventLoop>
#include "utils/Logger.h"

#define RESUME_ONCE_ON_SEEK 0

namespace QtAV {

class QueueEmptyCall : public PacketQueue::StateChangeCallback
{
public:
    QueueEmptyCall(AVDemuxThread* thread):
        mDemuxThread(thread)
    {}
    virtual void call() {
        if (!mDemuxThread)
            return;
        if (mDemuxThread->isEnd())
            return;
        AVThread *thread = mDemuxThread->videoThread();
        //qDebug("try wake up video queue");
        if (thread)
            thread->packetQueue()->blockFull(false);
        //qDebug("try wake up audio queue");
        thread = mDemuxThread->audioThread();
        if (thread)
            thread->packetQueue()->blockFull(false);
    }
private:
    AVDemuxThread *mDemuxThread;
};

AVDemuxThread::AVDemuxThread(QObject *parent) :
    QThread(parent)
  , paused(false)
  , user_paused(false)
  , end(true)
  , demuxer(0)
  , nb_next_frame(0)
{
	for (int i = 0; i < MAX_PROGRAM; i++)
	{
		audio_thread[i] = 0;
		video_thread[i] = 0;
	}

    seek_tasks.setCapacity(1);
    seek_tasks.blockFull(false);
}

AVDemuxThread::AVDemuxThread(AVDemuxer *dmx, QObject *parent) :
    QThread(parent)
  , paused(false)
  , end(true)
{
	for (int i = 0; i < MAX_PROGRAM; i++)
	{
		audio_thread[i] = 0;
		video_thread[i] = 0;
	}

    setDemuxer(dmx);
    seek_tasks.setCapacity(1);
    seek_tasks.blockFull(false);
}

void AVDemuxThread::setDemuxer(AVDemuxer *dmx)
{
    demuxer = dmx;
}

void AVDemuxThread::setAVThread(AVThread*& pOld, AVThread *pNew)
{
    if (pOld == pNew)
        return;
    if (pOld) {
        if (pOld->isRunning())
            pOld->stop();
    }
    pOld = pNew;
    if (!pNew)
        return;
    pOld->packetQueue()->setEmptyCallback(new QueueEmptyCall(this));
}

void AVDemuxThread::setAudioThread(AVThread *thread, int stream_id, int index)
{
	if (stream_id >= 0)
	{
		audioStreamId_ProgramIndex[stream_id] = index;
	}
    setAVThread(audio_thread[index], thread);
}

void AVDemuxThread::setVideoThread(AVThread *thread, int stream_id, int index)
{
	if (stream_id >= 0)
	{
		videoStreamId_ProgramIndex[stream_id] = index;
	}

    setAVThread(video_thread[index], thread);
}

AVThread* AVDemuxThread::videoThread(int index)
{
    return video_thread[index];
}

AVThread* AVDemuxThread::audioThread(int index)
{
    return audio_thread[index];
}

void AVDemuxThread::seek(qint64 pos)
{
    end = false;
    // queue maybe blocked by put()

	for (int i = 0; i < MAX_PROGRAM; i++)
	{
		if (audio_thread[i]) {
			audio_thread[i]->setDemuxEnded(false);
			audio_thread[i]->packetQueue()->clear();
		}
		if (video_thread[i]) {
			video_thread[i]->setDemuxEnded(false);
			video_thread[i]->packetQueue()->clear();
		}
	}
    class SeekTask : public QRunnable {
    public:
        SeekTask(AVDemuxThread *dt, qint64 t)
            : demux_thread(dt)
            , position(t)
        {}
        void run() {
            demux_thread->seekInternal(position);
        }
    private:
        AVDemuxThread *demux_thread;
        qint64 position;
    };
    newSeekRequest(new SeekTask(this, pos));
}

void AVDemuxThread::seekInternal(qint64 pos)
{
	for (int i = 0; i < MAX_PROGRAM; i++)
	{
		if (audio_thread[i]) {
			audio_thread[i]->setDemuxEnded(false);
			audio_thread[i]->packetQueue()->clear();
		}
		if (video_thread[i]) {
			video_thread[i]->setDemuxEnded(false);
			video_thread[i]->packetQueue()->clear();
		}
	}
	qDebug("seek to %lld ms (%f%%)", pos, double(pos) / double(demuxer->duration())*100.0);
	demuxer->seek(pos);

	// TODO: why queue may not empty?
	for (int i = 0; i < MAX_PROGRAM; i++)
	{
		if (audio_thread[i]) {
			audio_thread[i]->packetQueue()->clear();
			audio_thread[i]->packetQueue()->put(Packet());
		}
		if (video_thread[i]) {
			video_thread[i]->packetQueue()->clear();
			// TODO: the first frame (key frame) will not be decoded correctly if flush() is called.
			video_thread[i]->packetQueue()->put(Packet());
		}
	}
    //if (subtitle_thread) {
    //     subtitle_thread->packetQueue()->clear();
    //    subtitle_thread->packetQueue()->put(Packet());
    //}

	bool need_direct_connection = false;
	for (int i = 0; i < MAX_PROGRAM; i++)
	{
		if (isPaused() && (video_thread[i] || audio_thread[i])) {
			AVThread *thread = video_thread[i] ? video_thread[i] : audio_thread[i];
			thread->pause(false);
			pauseInternal(false);
			// direct connection is fine here
			connect(thread, SIGNAL(frameDelivered()), this, SLOT(frameDeliveredSeekOnPause()), Qt::DirectConnection);
			need_direct_connection = true;
		}
	}
	if (need_direct_connection)
		emit requestClockPause(false); // need direct connection
}

void AVDemuxThread::newSeekRequest(QRunnable *r)
{
    if (seek_tasks.size() >= seek_tasks.capacity()) {
        QRunnable *r = seek_tasks.take();
        if (r->autoDelete())
            delete r;
    }
    seek_tasks.put(r);
}

void AVDemuxThread::processNextSeekTask()
{
    if (seek_tasks.isEmpty())
        return;
    QRunnable *task = seek_tasks.take();
    if (!task)
        return;
    task->run();
    if (task->autoDelete())
        delete task;
}

void AVDemuxThread::pauseInternal(bool value)
{
    paused = value;
}

void AVDemuxThread::processNextPauseTask()
{
    if (pause_tasks.isEmpty())
        return;
    QRunnable *task = pause_tasks.dequeue();
    if (!task)
        return;
    task->run();
    if (task->autoDelete())
        delete task;
}

bool AVDemuxThread::isPaused() const
{
    return paused;
}

bool AVDemuxThread::isEnd() const
{
    return end;
}

//No more data to put. So stop blocking the queue to take the reset elements
void AVDemuxThread::stop()
{
    //this will not affect the pause state if we pause the output
    //TODO: why remove blockFull(false) can not play another file?
    
	for (int i = 0; i < MAX_PROGRAM; i++)
	{
		if (audio_thread[i]) {
			audio_thread[i]->setDemuxEnded(true);
			audio_thread[i]->packetQueue()->clear();
			audio_thread[i]->packetQueue()->blockFull(false); //??
			while (audio_thread[i]->isRunning()) {
				qDebug("stopping audio thread.......");
				audio_thread[i]->stop();
				audio_thread[i]->wait(500);
			}
		}
		if (video_thread[i]) {
			video_thread[i]->setDemuxEnded(true);
			video_thread[i]->packetQueue()->clear();
			video_thread[i]->packetQueue()->blockFull(false); //?
			while (video_thread[i]->isRunning()) {
				qDebug("stopping video thread.......");
				video_thread[i]->stop();
				video_thread[i]->wait(500);
			}
		}
	}

    pause(false);
    cond.wakeAll();
    qDebug("all avthread finished. try to exit demux thread<<<<<<");
    end = true;
}

void AVDemuxThread::pause(bool p)
{
    if (paused == p)
        return;
    paused = p;
    user_paused = paused;
    if (!paused)
        cond.wakeAll();
}

void AVDemuxThread::nextFrame()
{
	pause(true); // must pause AVDemuxThread (set user_paused true)

	for (int i = 0; i < MAX_PROGRAM; i++)
	{
		AVThread *t = video_thread[i];
		bool connected = false;
		if (t) {
			t->pause(false);
			t->packetQueue()->blockFull(false);
			if (!connected) {
				connect(t, SIGNAL(frameDelivered()), this, SLOT(frameDeliveredNextFrame()), Qt::DirectConnection);
				connected = true;
			}
		}
		t = audio_thread[i];
		if (t) {
			t->pause(false);
			t->packetQueue()->blockFull(false);
			if (!connected) {
				connect(t, SIGNAL(frameDelivered()), this, SLOT(frameDeliveredNextFrame()), Qt::DirectConnection);
				connected = true;
			}
		}
	}
    emit requestClockPause(false);
    nb_next_frame.ref();
    pauseInternal(false);
}

void AVDemuxThread::frameDeliveredSeekOnPause()
{
	for (int i = 0; i < MAX_PROGRAM; i++)
	{
		AVThread *thread = video_thread[i] ? video_thread[i] : audio_thread[i];
		Q_ASSERT(thread);
		disconnect(thread, SIGNAL(frameDelivered()), this, SLOT(frameDeliveredSeekOnPause()));

		if (user_paused) {
			pause(true); // restore pause state
			emit requestClockPause(true); // need direct connection
			// pause video/audio thread
			thread->pause(true);
		}
	}
}

void AVDemuxThread::frameDeliveredNextFrame()
{
	for (int i = 0; i < MAX_PROGRAM; i++)
	{
		AVThread *thread = video_thread[i] ? video_thread[i] : audio_thread[i];
		Q_ASSERT(thread);
		if (nb_next_frame.deref()) {
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0) || QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
			Q_ASSERT_X((int)nb_next_frame > 0, "frameDeliveredNextFrame", "internal error. frameDeliveredNextFrame must be > 0");
#else
			Q_ASSERT_X((int)nb_next_frame.load() > 0, "frameDeliveredNextFrame", "internal error. frameDeliveredNextFrame must be > 0");
#endif
			return;
		}
		disconnect(thread, SIGNAL(frameDelivered()), this, SLOT(frameDeliveredNextFrame()));
		if (user_paused) {
			pause(true); // restore pause state
			emit requestClockPause(true); // need direct connection
			// pause both video and audio thread
			if (video_thread[i])
				video_thread[i]->pause(true);
			if (audio_thread[i])
				audio_thread[i]->pause(true);
		}
	}
}

void AVDemuxThread::run()
{
    end = false;

	int running_threads = 0;
	for (int i = 0; i < MAX_PROGRAM; i++)
	{
		if (audio_thread[i] && !audio_thread[i]->isRunning())
			audio_thread[i]->start(QThread::HighPriority);
		if (video_thread[i] && !video_thread[i]->isRunning())
			video_thread[i]->start();

		if (audio_thread[i])
			++running_threads;
		if (video_thread[i])
			++running_threads;
	}
    qDebug("demux thread start running...%d avthreads", running_threads);

    audio_stream = demuxer->audioStream();
    video_stream = demuxer->videoStream();
    int index = 0;
    Packet pkt;
    pause(false);
    qDebug("get av queue a/v thread = %p %p", audio_thread, video_thread);

	for (int i = 0; i < MAX_PROGRAM; i++)
	{
		PacketQueue *aqueue = audio_thread[i] ? audio_thread[i]->packetQueue() : 0;
		PacketQueue *vqueue = video_thread[i] ? video_thread[i]->packetQueue() : 0;

		if (aqueue) {
			aqueue->clear();
			aqueue->setBlocking(true);
		}
		if (vqueue) {
			vqueue->clear();
			vqueue->setBlocking(true);
		}
	}
    while (!end) {
        processNextSeekTask();
        if (tryPause()) {
            continue; //the queue is empty and will block
        }

		running_threads = 0;
		for (int i = 0; i < MAX_PROGRAM; i++)
		{
			running_threads += (audio_thread[i] && audio_thread[i]->isRunning()) + (video_thread[i] && video_thread[i]->isRunning());
		}
        if (!running_threads) {
            qDebug("no running avthreads. exit demuxer thread");
            break;
        }
        QMutexLocker locker(&buffer_mutex);
        Q_UNUSED(locker);
        if (end) {
            break;
        }
        if (!demuxer->readFrame()) {
            continue;
        }
        index = demuxer->stream();
        pkt = *demuxer->packet(); //TODO: how to avoid additional copy?
        //connect to stop is ok too
        if (pkt.isEnd()) {
            qDebug("read end packet %d A:%d V:%d", index, audio_stream, video_stream);
            end = true;
            //avthread can stop. do not clear queue, make sure all data are played

			for (int i = 0; i < MAX_PROGRAM; i++)
			{
				if (audio_thread[i]) {
					audio_thread[i]->setDemuxEnded(true);
				}
				if (video_thread[i]) {
					video_thread[i]->setDemuxEnded(true);
				}
			}
            break;
        }
        /*1 is empty but another is enough, then do not block to
          ensure the empty one can put packets immediatly.
          But usually it will not happen, why?
        */
        /* demux thread will be blocked only when 1 queue is full and still put
         * if vqueue is full and aqueue becomes empty, then demux thread
         * will be blocked. so we should wake up another queue when empty(or threshold?).
         * TODO: the video stream and audio stream may be group by group. provide it
         * stream data: aaaaaaavvvvvvvaaaaaaaavvvvvvvvvaaaaaa, it happens
         * stream data: aavavvavvavavavavavavavavvvaavavavava, it's ok
         */
        //TODO: use cache queue, take from cache queue if not empty?
        
		if (audioStreamId_ProgramIndex.find(index) != audioStreamId_ProgramIndex.end())
		{
			int programIndex = *audioStreamId_ProgramIndex.find(index);
            /* if vqueue if not blocked and full, and aqueue is empty, then put to
             * vqueue will block demuex thread
             */
			if (audio_thread[programIndex]->packetQueue()) {
				if (!audio_thread[programIndex] || !audio_thread[programIndex]->isRunning()) {
					audio_thread[programIndex]->packetQueue()->clear();
                    continue;
                }
                // always block full if no vqueue because empty callback may set false
                // attached picture is cover for song, 1 frame
				audio_thread[programIndex]->packetQueue()->blockFull(!video_thread[programIndex]
					|| !video_thread[programIndex]->isRunning() 
					|| !video_thread[programIndex]->packetQueue()
					|| (video_thread[programIndex]->packetQueue()->isEnough() 
					|| demuxer->hasAttacedPicture()));
				audio_thread[programIndex]->packetQueue()->put(pkt); //affect video_thread
            }
		}
		else if (videoStreamId_ProgramIndex.find(index) != videoStreamId_ProgramIndex.end()) {
			int programIndex = *videoStreamId_ProgramIndex.find(index);
			if (video_thread[programIndex]->packetQueue()) {
				if (!video_thread[programIndex] || !video_thread[programIndex]->isRunning()) {
					video_thread[programIndex]->packetQueue()->clear();
                    continue;
                }
				video_thread[programIndex]->packetQueue()->blockFull(!audio_thread[programIndex]
					|| !audio_thread[programIndex]->isRunning() 
					|| !audio_thread[programIndex]->packetQueue()
					|| audio_thread[programIndex]->packetQueue()->isEnough());
				video_thread[programIndex]->packetQueue()->put(pkt); //affect audio_thread
            }
        } else { //subtitle
            continue;
        }
    }

	for (int i = 0; i < MAX_PROGRAM; i++)
	{
		//flush. seeking will be omitted when stopped
		if (audio_thread[i] && audio_thread[i]->packetQueue())
			audio_thread[i]->packetQueue()->put(Packet());
		if (video_thread[i] && video_thread[i]->packetQueue())
			video_thread[i]->packetQueue()->put(Packet());
		while (audio_thread[i] && audio_thread[i]->isRunning()) {
			qDebug("waiting audio thread.......");
			audio_thread[i]->wait(500);
		}
		while (video_thread[i] && video_thread[i]->isRunning()) {
			qDebug("waiting video thread.......");
			video_thread[i]->wait(500);
		}
	}
    qDebug("Demux thread stops running....");
}

bool AVDemuxThread::tryPause(unsigned long timeout)
{
    if (!paused)
        return false;
    QMutexLocker lock(&buffer_mutex);
    Q_UNUSED(lock);
    cond.wait(&buffer_mutex, timeout);
    return true;
}


} //namespace QtAV
