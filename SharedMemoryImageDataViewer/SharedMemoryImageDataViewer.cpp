/*
 * Copyright (c) 2016 HARBRICK TECHNOLOGIES, INC
 *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*
 * PolySync Sample C++ API example
 *      Demonstrate access to shared memory written to by a video device driver
 *
 */
#include <QApplication>
#include <QThread>
#include <QTimer>

#include "VideoProcessor.hpp"
#include "VideoViewer.hpp"

#include <memory>
#include <iostream>

using namespace std;

int main( int argc, char *argv[] )
{
    // Object required to execute Qt GUI functionality
    QApplication app( argc, argv );

    auto videoProcessor =
            std::unique_ptr< VideoProcessor >( new VideoProcessor );

    // Check for shared memory key argument
    if( argc > 1 )
    {
        videoProcessor.get()->_sharedMemoryKey = std::stoul( argv[ 1 ] );
    }
    else
    {
        std::cout << "Must pass executable a sharedMemoryKey" << std::endl;
        return 0;
    }

    // Thread video processing loop, this will continously pull data from
    // a video device and send a QPixmap to the VideoViewer object for rendering.
    QThread * processingThread = new QThread;
    videoProcessor->moveToThread( processingThread );
    processingThread->start();

    auto videoViewer =
            std::unique_ptr< VideoViewer >( new VideoViewer );

    // Qt signal/slot connect for passing data between processor and viewer.
    QObject::connect( videoProcessor.get(), &VideoProcessor::signalPixmap,
                      videoViewer.get(), &VideoViewer::slotUpdatePixmap );

    // Start processing in one millisecond, this allows for the next line of
    // code to be called, spawning the Qt application context.
    QTimer::singleShot( 1/*ms*/, videoProcessor.get(),
                        SLOT( slotRun() ) );

    // Blocking loop until the Viewer widget is closed.
    int appReturn = app.exec();

    // Halt thread execution and release dynamic memory
    processingThread->requestInterruption();
    processingThread->deleteLater();

    return appReturn;
}
