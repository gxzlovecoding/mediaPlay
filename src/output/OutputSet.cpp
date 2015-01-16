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

#include <QtAV/ImageConverterTypes.h>
#include "output/OutputSet.h"
#include "QtAV/AVPlayer.h"
#include "QtAV/VideoRenderer.h"

namespace QtAV {

OutputSet::OutputSet(AVPlayer *player):
    QObject(player)
  , mCanPauseThread(false)
  , mpPlayer(player)
  , mPauseCount(0)
  , m_preloadSuccess(false)
{
}

OutputSet::~OutputSet()
{
    mCond.wakeAll();
    //delete? may be deleted by vo's parent
    clearOutputs();
}

void OutputSet::lock()
{
    mMutex.lock();
}

void OutputSet::unlock()
{
    mMutex.unlock();
}

QList<AVOutput *> OutputSet::outputs()
{
    return mOutputs;
}

void OutputSet::sendVideoFrame(const VideoFrame &frame)
{
	if (!m_preloadSuccess)
	{
		m_firstFrame = frame.clone();

		ImageConverter *conv = ImageConverterFactory::create(ImageConverterId_FF);
		m_firstFrame.setImageConverter(conv);
		if (!m_firstFrame.convertTo(QImage::Format_ARGB32)) {
			qWarning("Failed to convert captured frame");
			return;
		}

		QString format = "png";
		m_firstImage = new QImage((const uchar*)m_firstFrame.frameData().constData(), m_firstFrame.width(), m_firstFrame.height(), m_firstFrame.bytesPerLine(), QImage::Format_ARGB32);
		/*
		//����image����
		bool ok = image.save("C://wwwroot//6.png", format.toLatin1().constData(), 0x64);
		if (!ok) {
			qWarning("Failed to save capture");
		}
		*/

		m_preloadSuccess = true;
	}

    if (mOutputs.isEmpty())
        return;
    foreach(AVOutput *output, mOutputs) {
        if (!output->isAvailable())
            continue;
        ((VideoRenderer*)output)->receive(frame);
    }
}

void OutputSet::clearOutputs()
{
    QMutexLocker lock(&mMutex);
    Q_UNUSED(lock);
    if (mOutputs.isEmpty())
        return;
    foreach(AVOutput *output, mOutputs) {
        output->removeOutputSet(this);
    }
    mOutputs.clear();
}

void OutputSet::addOutput(AVOutput *output)
{
    QMutexLocker lock(&mMutex);
    Q_UNUSED(lock);
    mOutputs.append(output);
    output->addOutputSet(this);
}

void OutputSet::removeOutput(AVOutput *output)
{
    QMutexLocker lock(&mMutex);
    Q_UNUSED(lock);
    mOutputs.removeAll(output);
    output->removeOutputSet(this);
}

void OutputSet::notifyPauseChange(AVOutput *output)
{
    if (output->isPaused()) {
        mPauseCount++;
        if (mPauseCount == mOutputs.size()) {
            mCanPauseThread = true;
        }
        //DO NOT pause here because it must be paused in AVThread
    } else {
        mPauseCount--;
        mCanPauseThread = false;
        if (mPauseCount == mOutputs.size() - 1) {
            resumeThread();
        }
    }
}

bool OutputSet::canPauseThread() const
{
    return mCanPauseThread;
}

bool OutputSet::pauseThread(unsigned long timeout)
{
    QMutexLocker lock(&mMutex);
    Q_UNUSED(lock);
    return mCond.wait(&mMutex, timeout);
}

void OutputSet::resumeThread()
{
    mCond.wakeAll();
}

} //namespace QtAV
