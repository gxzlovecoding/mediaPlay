#include <QApplication>

#include <QtAV/AVPlayer.h>
#include <QtAV/GLWidgetRenderer2.h>
#include "videogroup.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	VideoGroup wall;
	wall.resize(800, 200);
	wall.show();

	//wall.play("F:\\movie\\ALAN_HACKEN.mp4");
	//wall.play("C:\\workspace\\3rd-party\\movie\\mul.ts");
	//wall.play("C:\\workspace\\3rd-party\\movie\\trailer-mpeg2.mpg");
	//wall.preload("C:\\workspace\\3rd-party\\movie\\trailer-mpeg2.mpg");
	wall.preload("C:\\workspace\\3rd-party\\movie\\mul.ts");
	/*
	QtAV::GLWidgetRenderer2 renderer1;
	QtAV::GLWidgetRenderer2 renderer2;
    renderer1.show();
	renderer2.show();
    renderer1.setWindowTitle("1");
	renderer2.setWindowTitle("2");
    QtAV::AVPlayer player;
	//player.videoOutputs().append(&renderer1);
	player.addVideoRenderer(&renderer1,8);

	//player.videoOutputs().append(&renderer2);
	//player.removeVideoRenderer(&renderer1);
	player.addVideoRenderer(&renderer2,1);
	
    //player.setRenderer(&renderer1);
	//player.setRenderer(&renderer2);

    //if (argc > 1)
    //    player.play(a.arguments().last());
	player.play("F:\\project\\multi\\save_2000.ts");
	//player.play("F:\\movie\\ALAN_HACKEN.mp4");
	//player.play("F:\\movie\\1.ts");
	*/

    return a.exec();
}
