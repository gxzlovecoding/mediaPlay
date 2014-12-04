mkdir C:\qt\qt5.3.1\5.3\msvc2013_opengl\include\QtAV\
copy /y F:\qtspace\mediaPlay\tools\install_sdk\..\..\src\QtAV\*.h C:\qt\qt5.3.1\5.3\msvc2013_opengl\include\QtAV\
xcopy /s /q /y /i F:\qtspace\mediaPlay\tools\install_sdk\..\..\src\QtAV\private C:\qt\qt5.3.1\5.3\msvc2013_opengl\include\QtAV\private
mkdir C:\qt\qt5.3.1\5.3\msvc2013_opengl\include\QtAV\5.3.1\QtAV\
xcopy /s /q /y /i F:\qtspace\mediaPlay\tools\install_sdk\..\..\src\QtAV\private C:\qt\qt5.3.1\5.3\msvc2013_opengl\include\QtAV\5.3.1\QtAV\private
copy /y F:\qtspace\mediaPlay\lib_win_\*Qt*AV* C:\qt\qt5.3.1\5.3\msvc2013_opengl\lib\
copy /y F:\qtspace\mediaPlay\lib_win_\QtAV1.lib C:\qt\qt5.3.1\5.3\msvc2013_opengl\lib\Qt5AV.lib
copy /y F:\qtspace\mediaPlay\tools\install_sdk\mkspecs\features\av.prf C:\qt\qt5.3.1\5.3\msvc2013_opengl\bin\..\mkspecs\features\av.prf
xcopy /s /q /y /i F:\qtspace\mediaPlay\bin\QtAV C:\qt\qt5.3.1\5.3\msvc2013_opengl\qml\QtAV
copy /y F:\qtspace\mediaPlay\tools\install_sdk\..\..\qml\plugins.qmltypes C:\qt\qt5.3.1\5.3\msvc2013_opengl\qml\QtAV\
copy /y F:\qtspace\mediaPlay\tools\install_sdk\mkspecs\modules\qt_lib_av*.pri C:\qt\qt5.3.1\5.3\msvc2013_opengl\bin\..\mkspecs\modules\
move /y C:\qt\qt5.3.1\5.3\msvc2013_opengl\lib\Qt*AV*.dll C:\qt\qt5.3.1\5.3\msvc2013_opengl\bin\
