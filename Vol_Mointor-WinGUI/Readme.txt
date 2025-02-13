Vol monitor is a Windows GUI tool which display 12-Channel ADC waveform like a oscilloscope.

!!!The input pins are not accept higher than 3.6V, otherwise it will be damaged physically.

1. Why 12-Channel?
   I only can find out 12 colors for channels. 20 colors will hard to distinguish on screen by my eyes.
2. Why 1Ksps?
   USBHID protocol limits the data rate. ADC is around 100KHz in chip. Every time when read, it will return the 8 times over sample prepared data immediately.
3. ADC reference voltage is VDD, usually it's around 3.30V. It can be accurate calibration in GUI by measure VDD through a voltage meter.
4. It was compiled with Visual Studio 2005 which I have a license. Visual Studio Community version should be OK but I cannot verified.
5. Be sure Vol_Mointor.exe and SLABHIDDevice.dll are in the same directory.
6. VOL_monitor.png is screen capture.
7. The GUI uses some owner draw buttons. It can be removed if you want.
8. Usage:
   You can use mouse Click, Drag, different position will have different functions, try to find them out by read HELP like following. Of cause Arrows on keyboard have functions too.
   a. 在波形显示区域鼠标滚动键可以调节一格显示的时间范围；左键和右键点击可以显示两个十字坐标轴，字母按键'H''V'可以打开关闭横纵坐标轴，和=||按钮功能一样。\r\n\
   b. 在波形显示区域鼠标可以拖动调整显示区域。\r\n\
   c. 在电压区域鼠标滚动键可以调节显示电压范围，按下Ctrl可以加速；鼠标可以拖动显示区域。\r\n\
   d. 在时间轴鼠标滚动键可以调节时间轴格数；鼠标可以拖动显示区域。\r\n\
   e. 在左下角鼠标左键点击可以复位显示区域。\r\n\
   f. 有任何问题发邮件给gr_li@163.com咨询，或者淘宝咨询也可以。不实时守店，答复不及时，所以尽量把问题或想法一次性描述完。\r\n\
   
   a. The mouse scroll keys in the waveform display area can adjust the time range displayed in one grid; Left and right clicking can display two cross coordinate axes, and the letter keys' H '' V 'can turn on and off the horizontal and vertical coordinate axes, just like the=| | button function. \r\n\
   b. In the waveform display area, the mouse can be dragged to adjust the display area. \r\n\
   c. The mouse scroll keys in the voltage area can adjust the displayed voltage range, and pressing Ctrl can accelerate; The mouse can drag the display area. \r\n\
   d. The mouse scroll keys on the timeline can adjust the number of grids on the timeline; The mouse can drag the display area. \r\n\
   e. Click the left mouse button in the bottom left corner to reset the display area. \r\n\
   f. If you have any questions, please send an email to gr_li@163.com Consultation or Taobao consultation is also available. Not keeping track of the store in real time and not responding promptly, so try to describe the problem or idea in one go.\

---------------

Anything need help, you can mail to gr_li@163.com, or guanrong.li@silabs.com if it's reachable which means I'm still working in Silicon Labs. (^_^)
Uploaded Date: 2025-02-13
